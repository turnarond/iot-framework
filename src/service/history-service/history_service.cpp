/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: history_service.cpp .
*
* Date: 2025-07-23
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#include "history_service.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>

namespace fs = std::filesystem;

HistoryService HistoryService::instance_;

HistoryService::HistoryService() : 
    storage_(nullptr),
    running_(false),
    last_flush_time_(0) {
    // 初始化默认配置
    config_.storage_path = "/var/lib/edge-framework/history";
    config_.max_days = 30;
    config_.batch_size = 100;
    config_.flush_interval = 60;
    config_.max_file_size = 100;
}

HistoryService::~HistoryService() {
    Stop();
}

HistoryService* HistoryService::GetInstance() {
    return &instance_;
}

bool HistoryService::Start() {
    if (running_) {
        return true;
    }

    if (!Initialize()) {
        return false;
    }

    running_ = true;

    // 启动工作线程
    worker_thread_ = std::thread(&HistoryService::WorkerThreadFunc, this);
    
    // 启动清理线程
    cleanup_thread_ = std::thread(&HistoryService::CleanupThreadFunc, this);

    std::cout << "HistoryService started successfully" << std::endl;
    return true;
}

bool HistoryService::Stop() {
    if (!running_) {
        return true;
    }

    running_ = false;
    queue_cv_.notify_one();

    // 等待线程结束
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }

    if (cleanup_thread_.joinable()) {
        cleanup_thread_.join();
    }

    // 刷新剩余数据
    FlushData();

    // 关闭存储
    if (storage_) {
        storage_->Close();
    }

    std::cout << "HistoryService stopped successfully" << std::endl;
    return true;
}

bool HistoryService::Store(const HistoryRecord& record) {
    if (!running_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(queue_mutex_);
    storage_queue_.push(record);
    queue_cv_.notify_one();

    return true;
}

bool HistoryService::BatchStore(const std::vector<HistoryRecord>& records) {
    if (!running_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(queue_mutex_);
    for (const auto& record : records) {
        storage_queue_.push(record);
    }
    queue_cv_.notify_one();

    return true;
}

std::vector<HistoryRecord> HistoryService::Query(const HistoryQueryParams& params) {
    if (!running_ || !storage_) {
        return {};
    }

    return storage_->Query(params);
}

HistoryStats HistoryService::GetStats() {
    if (!running_ || !storage_) {
        return {};
    }

    return storage_->GetStats();
}

size_t HistoryService::Cleanup() {
    if (!running_ || !storage_) {
        return 0;
    }

    return storage_->Cleanup();
}

const HistoryConfig& HistoryService::GetConfig() const {
    return config_;
}

bool HistoryService::UpdateConfig(const HistoryConfig& config) {
    config_ = config;

    // 重新初始化存储
    if (storage_) {
        storage_->Close();
    }

    storage_ = CreateFileStorage();
    if (!storage_) {
        return false;
    }

    return storage_->Initialize(config_);
}

bool HistoryService::IsRunning() const {
    return running_;
}

bool HistoryService::Initialize() {
    // 确保存储目录存在
    try {
        fs::create_directories(config_.storage_path);
    } catch (const std::exception& e) {
        std::cerr << "Failed to create storage directory: " << e.what() << std::endl;
        return false;
    }

    // 创建存储实例
    storage_ = CreateFileStorage();
    if (!storage_) {
        std::cerr << "Failed to create storage instance" << std::endl;
        return false;
    }

    // 初始化存储
    if (!storage_->Initialize(config_)) {
        std::cerr << "Failed to initialize storage" << std::endl;
        return false;
    }

    // 初始化时间
    last_flush_time_ = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    return true;
}

void HistoryService::WorkerThreadFunc() {
    while (running_) {
        ProcessStorageQueue();
        
        // 检查是否需要刷新
        size_t now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        if (now - last_flush_time_ >= config_.flush_interval) {
            FlushData();
            last_flush_time_ = now;
        }

        // 短暂睡眠
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void HistoryService::CleanupThreadFunc() {
    while (running_) {
        // 每天执行一次清理
        std::this_thread::sleep_for(std::chrono::hours(24));
        
        if (running_ && storage_) {
            size_t deleted = storage_->Cleanup();
            std::cout << "Cleanup completed, deleted " << deleted << " files" << std::endl;
        }
    }
}

void HistoryService::ProcessStorageQueue() {
    std::vector<HistoryRecord> records;
    
    // 从队列中取出数据
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        while (!storage_queue_.empty() && records.size() < config_.batch_size) {
            records.push_back(storage_queue_.front());
            storage_queue_.pop();
        }
    }

    if (records.empty()) {
        return;
    }

    // 添加到批处理缓存
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        batch_cache_.insert(batch_cache_.end(), records.begin(), records.end());

        // 检查缓存大小
        if (batch_cache_.size() >= config_.batch_size) {
            FlushData();
        }
    }
}

void HistoryService::FlushData() {
    std::vector<HistoryRecord> records;
    
    // 交换缓存数据
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        if (batch_cache_.empty()) {
            return;
        }
        records.swap(batch_cache_);
    }

    // 批量存储
    if (storage_) {
        storage_->BatchStore(records);
    }
}
