/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: cloud_handler.cpp .
 *
 * Date: 2025-12-19
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#include "cloud_handler.h"
#include <stdexcept>
#include <thread>
#include <chrono>

// 全局初始化/清理 mosquitto 库（线程安全，只执行一次）
static struct MosquittoGlobalInit {
    MosquittoGlobalInit() { mosquitto_lib_init(); }
    ~MosquittoGlobalInit() { mosquitto_lib_cleanup(); }
} s_mosq_global_init;

CloudHandler::CloudHandler(const Config& config)
    : config_(config), mosq_(nullptr, mosquitto_destroy)
{
    if (config_.brokerHost.empty()) {
        throw std::invalid_argument("brokerHost is required");
    }

    mosquitto* m = mosquitto_new(
        config_.clientId.empty() ? nullptr : config_.clientId.c_str(),
        config_.cleanSession,
        this
    );
    if (!m) {
        throw std::runtime_error("Failed to create mosquitto instance");
    }
    mosq_.reset(m);

    // 设置回调
    mosquitto_connect_callback_set(m, &CloudHandler::on_connect_wrapper);
    mosquitto_disconnect_callback_set(m, &CloudHandler::on_disconnect_wrapper);
    mosquitto_message_callback_set(m, &CloudHandler::on_message_wrapper);

    // 设置用户名/密码（如有）
    if (!config_.username.empty()) {
        int rc = mosquitto_username_pw_set(m, config_.username.c_str(),
                                           config_.password.empty() ? nullptr : config_.password.c_str());
        if (rc != MOSQ_ERR_SUCCESS) {
            throw std::runtime_error("Failed to set username/password: " + std::string(mosquitto_strerror(rc)));
        }
    }

    // TLS 配置
    if (config_.useTls) {
        // 启用 TLS
        int rc = mosquitto_tls_set(m,
            config_.caFile.empty() ? nullptr : config_.caFile.c_str(),
            nullptr, // capath (not used)
            config_.certFile.empty() ? nullptr : config_.certFile.c_str(),
            config_.keyFile.empty() ? nullptr : config_.keyFile.c_str(),
            nullptr  // pw callback (not used)
        );
        if (rc != MOSQ_ERR_SUCCESS) {
            throw std::runtime_error("mosquitto_tls_set failed: " + std::string(mosquitto_strerror(rc)));
        }

        // 可选：跳过主机名验证（仅用于测试！生产环境应验证）
        // mosquitto_tls_insecure_set(m, true); // ⚠️ 不推荐

        // 如果使用 8883 端口但未显式设置，可自动切换（非强制）
    }
    // 连接回调
    connect_callback_ = config_.onConnect;
    connect_callback_ = config_.onConnect;
    disconnect_callback_ = config_.onDisconnect;
    error_callback_ = config_.onError;
}

CloudHandler::~CloudHandler()
{
    disconnect(); // 安全断开
    stopReconnectTimer(); // 等待重连线程结束
    stopReconnectTimer();  // 停止重连线程
    // mosquitto_destroy 由 unique_ptr 自动调用
}

void CloudHandler::connect()
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    if (connected_) return;

    // 注意：mosquitto_connect 不直接支持超时参数
    // 我们可通过设置 socket 选项或使用 connect_async + loop 来实现超时
    // 此处简化：使用阻塞 connect，依赖系统默认超时（通常较长）
    // 若需精确控制 timeoutSec，建议使用 mosquitto_connect_bind 和非阻塞 socket（较复杂）
    manual_disconnect_ = false;
    int rc = mosquitto_connect_async(mosq_.get(),
                               config_.brokerHost.c_str(),
                               config_.brokerPort,
                               config_.keepAlive);
    if (rc != MOSQ_ERR_SUCCESS) {
        throw std::runtime_error("mosquitto_connect failed: " + std::string(mosquitto_strerror(rc)));
    }
    connected_ = true;
}

void CloudHandler::disconnect()
{
    {
        std::lock_guard<std::mutex> lock(client_mutex_);
        if (!connected_) return;

        manual_disconnect_ = true;
        should_reconnect_ = false;
        mosquitto_disconnect(mosq_.get());
        connected_ = false;
    }
    stopReconnectTimer();

}

// ======================
// LOOP 接口实现
// ======================
void CloudHandler::loop_once(int timeout_ms, int max_packets) 
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    if (!mosq_) return;
    int rc = mosquitto_loop(mosq_.get(), timeout_ms, max_packets);
    if (rc != MOSQ_ERR_SUCCESS && rc != MOSQ_ERR_NO_CONN) {
        throw std::runtime_error("mosquitto_loop error: " + std::string(mosquitto_strerror(rc)));
    }
    handle_loop_error(rc);
}

void CloudHandler::loop_forever(int timeout_ms) 
{
    if (timeout_ms < 0) timeout_ms = 1000; // 合理默认值

    std::unique_lock<std::mutex> lock(client_mutex_, std::defer_lock);
    running_ = true;

    while (running_) {
        lock.lock();
        if (!mosq_ || !running_) {
            lock.unlock();
            break;
        }

        // 允许在循环中短暂释放锁，避免死锁
        int rc = MOSQ_ERR_SUCCESS;
        {
            rc = mosquitto_loop(mosq_.get(), timeout_ms, 1);
        }
        lock.unlock();

        if (!handle_loop_error(rc)) {
            // 致命错误，退出循环
            running_ = false;
        }

        // 短暂休眠避免CPU占用100%
        if (rc == MOSQ_ERR_NO_CONN || rc == MOSQ_ERR_CONN_LOST) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
        }
    }
}

void CloudHandler::loop_async(int timeout_ms) 
{
    stop_loop(); // 确保之前的循环已停止
    
    running_ = true;
    loop_thread_ = std::thread([this, timeout_ms] {
        loop_forever(timeout_ms);
    });
}


void CloudHandler::stop_loop()
{
    running_ = false;
    loop_cv_.notify_all();

    if (loop_thread_.joinable()) {
        loop_thread_.join();
    }
}

bool CloudHandler::isConnected() const noexcept 
{
    return connected_.load();
}

bool CloudHandler::handle_loop_error(int rc)
{
    if (rc == MOSQ_ERR_SUCCESS) return true;

    // 可恢复的网络错误
    if (rc == MOSQ_ERR_NO_CONN || rc == MOSQ_ERR_CONN_LOST) {
        if (error_callback_) error_callback_(rc, "Network error: " + std::string(mosquitto_strerror(rc)));
        return true; // 继续循环
    }

    // 致命错误
    if (error_callback_) error_callback_(rc, "Fatal error: " + std::string(mosquitto_strerror(rc)));
    connected_ = false;
    return false; // 退出循环
}

void CloudHandler::set_message_callback(MessageCallback cb) 
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    message_callback_ = std::move(cb);
}

void CloudHandler::set_connect_callback(ConnectCallback cb) 
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    connect_callback_ = std::move(cb);
}

void CloudHandler::set_disconnect_callback(DisconnectCallback cb)
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    disconnect_callback_ = std::move(cb);
}

void CloudHandler::set_error_callback(ErrorCallback cb)
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    error_callback_ = std::move(cb);
}

void CloudHandler::subscribe(const std::string& topic, int qos)
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    if (!mosq_ || !connected_) {
        throw std::runtime_error("Not connected");
    }

    int mid;
    int rc = mosquitto_subscribe(mosq_.get(), &mid, topic.c_str(), qos);
    if (rc != MOSQ_ERR_SUCCESS) {
        throw std::runtime_error("mosquitto_subscribe failed: " + std::string(mosquitto_strerror(rc)));
    }
}

void CloudHandler::publish(const std::string& topic, const std::string& payload, int qos, bool retain)
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    if (!mosq_ || !connected_) {
        throw std::runtime_error("Not connected");
    }

    int mid;
    int rc = mosquitto_publish(mosq_.get(), &mid, topic.c_str(),
                               static_cast<int>(payload.size()),
                               payload.data(), qos, retain);
    if (rc != MOSQ_ERR_SUCCESS) {
        throw std::runtime_error("mosquitto_publish failed: " + std::string(mosquitto_strerror(rc)));
    }
}

// 实现 wrapper
void CloudHandler::on_connect_wrapper(struct mosquitto* /*mosq*/, void* obj, int rc) 
{
    auto self = static_cast<CloudHandler*>(obj);
    self->connected_ = (rc == MOSQ_ERR_SUCCESS);
    self->manual_disconnect_ = false; // 成功连接后清除手动标志
    self->should_reconnect_ = false;  // 停止重连

    if (self->connect_callback_) {
        self->connect_callback_(rc);
    }
}

void CloudHandler::on_disconnect_wrapper(struct mosquitto* /*mosq*/, void* obj, int rc) 
{
    auto self = static_cast<CloudHandler*>(obj);
    self->connected_ = false;

    bool was_manual = self->manual_disconnect_.load();

    if (self->disconnect_callback_) {
        self->disconnect_callback_(rc);
    }

    // 如果不是用户主动断开，且启用了自动重连，则启动重连
    if (!was_manual && self->config_.enableAutoReconnect) {
        self->should_reconnect_ = true;
        self->startReconnectTimer();
    }
}

void CloudHandler::on_message_wrapper(struct mosquitto* /*mosq*/, void* obj, const struct mosquitto_message* msg)
{
    auto self = static_cast<CloudHandler*>(obj);
    if (self->message_callback_ && msg->payload && msg->topic) {
        std::string topic(msg->topic);
        std::string payload(static_cast<const char*>(msg->payload), msg->payloadlen);
        self->message_callback_(msg->mid, topic, payload);
    }
}

void CloudHandler::reconnect()
{
    std::lock_guard<std::mutex> lock(client_mutex_);
    if (!mosq_ || connected_) return;

    int rc = mosquitto_reconnect(mosq_.get());
    if (rc != MOSQ_ERR_SUCCESS) {
        if (error_callback_) error_callback_(rc, "Reconnect failed: " + std::string(mosquitto_strerror(rc)));
    }
}

void CloudHandler::startReconnectTimer()
{
    if (reconnect_thread_.joinable()) return;
    if (!config_.enableAutoReconnect) return;

    reconnect_thread_ = std::thread([this]() {
        int delay = this->config_.initialReconnectDelayMs;
        this->reconnect_attempts_ = 0;

        while (this->should_reconnect_) {
            {
                std::unique_lock<std::mutex> lock(this->reconnect_mutex_);
                if (this->reconnect_cv_.wait_for(lock, std::chrono::milliseconds(delay)) == std::cv_status::no_timeout) {
                    // 被 notify 唤醒（如析构时）
                    break;
                }
            }

            if (!should_reconnect_ || !running_) break;

            this->reconnect_attempts_++;
            if (this->config_.maxReconnectAttempts > 0 && 
                this->reconnect_attempts_ > this->config_.maxReconnectAttempts) {
                // 超过最大重试次数，停止
                this->should_reconnect_ = false;
                if (this->disconnect_callback_) {
                    this->disconnect_callback_(CloudError::MOSQ_ERR_RECONNECT_FAILED); // 自定义错误码或记录日志
                }
                break;
            }

            // 尝试重连（只是发起，实际结果由 loop 处理）
            this->reconnect();

            // 指数退避（带抖动）
            delay = std::min(delay * 2, this->config_.maxReconnectDelayMs);
        }
    });
}

void CloudHandler::stopReconnectTimer() {
    should_reconnect_ = false;
    reconnect_cv_.notify_all();
    if (reconnect_thread_.joinable()) {
        reconnect_thread_.join();
    }
}
