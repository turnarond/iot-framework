/*
* Copyright (c) 2025 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: history_storage.h .
*
* Date: 2025-07-23
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <memory>

// 历史数据记录结构
typedef struct {
    std::string tag_name;      // 标签名
    std::string value;         // 数值
    std::string timestamp;     // 时间戳
    std::string driver_name;   // 驱动名
    std::string device_name;   // 设备名
} HistoryRecord;

// 查询参数结构
typedef struct {
    std::string tag_name;      // 标签名，空表示所有标签
    std::string start_time;    // 开始时间
    std::string end_time;      // 结束时间
    size_t limit;              // 限制返回数量
    bool desc;                 // 是否降序
} HistoryQueryParams;

// 历史数据统计信息
typedef struct {
    size_t total_records;      // 总记录数
    size_t total_size;         // 总存储大小(字节)
    size_t today_records;      // 今日记录数
    size_t today_size;         // 今日存储大小
    std::string oldest_date;   // 最早日期
    std::string newest_date;   // 最新日期
} HistoryStats;

// 历史数据配置
typedef struct {
    std::string storage_path;  // 存储路径
    size_t max_days;           // 最大保留天数
    size_t batch_size;         // 批处理大小
    size_t flush_interval;     // 刷新间隔(秒)
    size_t max_file_size;      // 单个文件最大大小(MB)
} HistoryConfig;

// 历史数据存储接口
class HistoryStorage {
public:
    virtual ~HistoryStorage() = default;

    // 初始化存储
    virtual bool Initialize(const HistoryConfig& config) = 0;

    // 存储历史数据
    virtual bool Store(const HistoryRecord& record) = 0;

    // 批量存储历史数据
    virtual bool BatchStore(const std::vector<HistoryRecord>& records) = 0;

    // 查询历史数据
    virtual std::vector<HistoryRecord> Query(const HistoryQueryParams& params) = 0;

    // 获取统计信息
    virtual HistoryStats GetStats() = 0;

    // 清理过期数据
    virtual size_t Cleanup() = 0;

    // 关闭存储
    virtual void Close() = 0;
};

// 文件存储实现
class FileStorage : public HistoryStorage {
public:
    FileStorage();
    ~FileStorage() override;

    bool Initialize(const HistoryConfig& config) override;
    bool Store(const HistoryRecord& record) override;
    bool BatchStore(const std::vector<HistoryRecord>& records) override;
    std::vector<HistoryRecord> Query(const HistoryQueryParams& params) override;
    HistoryStats GetStats() override;
    size_t Cleanup() override;
    void Close() override;

private:
    // 内部实现
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// 创建存储实例
typedef std::unique_ptr<HistoryStorage> (*CreateStorageFunc)();
std::unique_ptr<HistoryStorage> CreateFileStorage();
