/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: cloud_handler.h .
 *
 * Date: 2025-12-19
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#pragma once

#include <mosquitto.h>
#include <string>
#include <functional>
#include <memory>
#include <stdexcept>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

enum CloudError {
    SUCCESS = MOSQ_ERR_SUCCESS,
    MOSQ_ERR_RECONNECT_FAILED = 65
};

class CloudHandler {
public:
    using MessageCallback = std::function<void(int mid, const std::string& topic, const std::string& payload)>;
    using ConnectCallback = std::function<void(int return_code)>; // MOSQ_ERR_SUCCESS 等
    using DisconnectCallback = std::function<void(int return_code)>;
    using ErrorCallback = std::function<void(int error_code, const std::string& error_msg)>;

    struct Config {
        std::string brokerHost;
        int brokerPort = 1883;
        std::string clientId;
        std::string username;
        std::string password;
        bool useTls = false;
        std::string caFile;      // 必须是 PEM 格式
        std::string certFile;    // 客户端证书（可选）
        std::string keyFile;     // 客户端私钥（可选）
        int keepAlive = 60;      // seconds
        int timeoutSec = 30;     // 仅用于 connect 超时（通过 mosquitto_connect_bind 等实现）
        bool cleanSession = true;

        // 回调
        ConnectCallback onConnect = nullptr;
        DisconnectCallback onDisconnect = nullptr;
        ErrorCallback onError = nullptr;

        // 自动重连配置
        bool enableAutoReconnect = false;
        int maxReconnectAttempts = -1;   // -1 表示无限重试
        int initialReconnectDelayMs = 1000;  // 初始延迟 1s
        int maxReconnectDelayMs = 30000;     // 最大延迟 30s
    };

    explicit CloudHandler(const Config& config);
    ~CloudHandler();

    CloudHandler(const CloudHandler&) = delete;
    CloudHandler& operator=(const CloudHandler&) = delete;

    // 连接控制
    void connect();
    void disconnect();

    // 消息操作
    void subscribe(const std::string& topic, int qos = 0);
    void publish(const std::string& topic, const std::string& payload, int qos = 0, bool retain = false);

    bool isConnected() const noexcept;

    // 回调设置
    void set_message_callback(MessageCallback cb);
    void set_connect_callback(ConnectCallback cb);
    void set_disconnect_callback(DisconnectCallback cb);
    void set_error_callback(ErrorCallback cb);

    // 循环接口
    void loop_once(int timeout_ms = 1000, int max_packets = 1);
    void loop_forever(int timeout_ms = -1);
    void loop_async(int timeout_ms = -1);
    void stop_loop();                          // 停止loop_async

private:
    static void on_connect_wrapper(struct mosquitto* mosq, void* obj, int rc);
    static void on_disconnect_wrapper(struct mosquitto* mosq, void* obj, int rc);
    static void on_message_wrapper(struct mosquitto* mosq, void* obj, const struct mosquitto_message* msg);

    // 重连
    void reconnect();
    void startReconnectTimer();
    void stopReconnectTimer();
    bool handle_loop_error(int rc);

    Config config_;
    std::unique_ptr<mosquitto, decltype(&mosquitto_destroy)> mosq_;
    std::mutex client_mutex_;  // 保护所有mosquitto调用

    // 回调
    MessageCallback message_callback_ = nullptr;
    ConnectCallback connect_callback_ = nullptr;
    DisconnectCallback disconnect_callback_ = nullptr;
    ErrorCallback error_callback_ = nullptr;

    // 状态
    std::atomic<bool> connected_{false};
    std::atomic<bool> manual_disconnect_{false}; // 标记是否用户主动断开
    std::atomic<bool> should_reconnect_{false};

    // 重连控制
    std::atomic<int> reconnect_attempts_{0};
    std::thread reconnect_thread_;
    std::mutex reconnect_mutex_;
    std::condition_variable reconnect_cv_;

    // 循环控制
    std::atomic<bool> running_{false};        // 控制loop_forever/loop_async
    std::thread loop_thread_;                 // 用于loop_async
    std::condition_variable loop_cv_;         // 用于安全停止
};