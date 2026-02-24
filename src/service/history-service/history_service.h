/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: history_service.h .
*
* Date: 2025-07-23
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include "history_storage.h"
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>

class HistoryService {
public:
    static HistoryService* GetInstance();

    // 启动服务
    bool Start();

    // 停止服务
    bool Stop();

    // 存储历史数据
    bool Store(const HistoryRecord& record);

    // 批量存储历史数据
    bool BatchStore(const std::vector<HistoryRecord>& records);

    // 查询历史数据
    std::vector<HistoryRecord> Query(const HistoryQueryParams& params);

    // 获取统计信息
    HistoryStats GetStats();

    // 手动清理过期数据
    size_t Cleanup();

    // 获取配置
    const HistoryConfig& GetConfig() const;

    // 更新配置
    bool UpdateConfig(const HistoryConfig& config);

    // 检查服务状态
    bool IsRunning() const;

private:
    HistoryService();
    ~HistoryService();

    // 初始化服务
    bool Initialize();

    // 工作线程函数
    void WorkerThreadFunc();

    // 清理线程函数
    void CleanupThreadFunc();

    // 处理存储队列
    void ProcessStorageQueue();

    // 刷新数据到存储
    void FlushData();

private:
    static HistoryService instance_;
    std::unique_ptr<HistoryStorage> storage_;
    HistoryConfig config_;

    // 工作线程
    std::thread worker_thread_;
    std::thread cleanup_thread_;
    std::atomic<bool> running_;

    // 存储队列
    std::queue<HistoryRecord> storage_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    // 批处理缓存
    std::vector<HistoryRecord> batch_cache_;
    std::mutex cache_mutex_;
    size_t last_flush_time_;
};

#define HISTORY_SERVICE HistoryService::GetInstance()
