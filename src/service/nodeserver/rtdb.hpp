#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <memory>
#include <functional>
#include <chrono>

namespace nodeserver {

// 单点数据结构（尽量保持扁平）
struct TagRecord {
    std::string name;
    std::string value;
    uint64_t timestamp_ms = 0; // Unix ms
    std::string driver_name;
    std::string device_name;
    std::atomic<uint64_t> version{0};

    TagRecord() = default;
    TagRecord(const std::string& n): name(n) {}

    // 复制构造函数
    TagRecord(const TagRecord& other) {
        name = other.name;
        value = other.value;
        timestamp_ms = other.timestamp_ms;
        driver_name = other.driver_name;
        device_name = other.device_name;
        version.store(other.version.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    // 复制赋值运算符
    TagRecord& operator=(const TagRecord& other) {
        if (this != &other) {
            name = other.name;
            value = other.value;
            timestamp_ms = other.timestamp_ms;
            driver_name = other.driver_name;
            device_name = other.device_name;
            version.store(other.version.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
        return *this;
    }
};

struct RTDBStats {
    std::atomic<uint64_t> total_tags{0};
    std::atomic<uint64_t> reads{0};
    std::atomic<uint64_t> writes{0};
    std::atomic<uint64_t> last_write_ts{0};

    // 默认构造函数
    RTDBStats() = default;

    // 复制构造函数
    RTDBStats(const RTDBStats& other) {
        total_tags.store(other.total_tags.load(std::memory_order_relaxed), std::memory_order_relaxed);
        reads.store(other.reads.load(std::memory_order_relaxed), std::memory_order_relaxed);
        writes.store(other.writes.load(std::memory_order_relaxed), std::memory_order_relaxed);
        last_write_ts.store(other.last_write_ts.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    // 复制赋值运算符
    RTDBStats& operator=(const RTDBStats& other) {
        if (this != &other) {
            total_tags.store(other.total_tags.load(std::memory_order_relaxed), std::memory_order_relaxed);
            reads.store(other.reads.load(std::memory_order_relaxed), std::memory_order_relaxed);
            writes.store(other.writes.load(std::memory_order_relaxed), std::memory_order_relaxed);
            last_write_ts.store(other.last_write_ts.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
        return *this;
    }
};

/**
 * 高性能 RTDB（分片 sharded map + shared_mutex）
 * - 读多写少场景：使用 std::shared_mutex
 * - 分片减少锁冲突
 * - 支持单点/批量读写、注册/注销、统计与健康检查
 */
class RTDB {
public:
    // 如果传入 0，则根据 CPU 核数自动设置为 num_cores * 2
    explicit RTDB(size_t shards = 0);

    // 注册点位（在写入前可先注册以预分配）
    bool registerTag(const std::string& name);

    // 注销点位（删除数据）
    bool unregisterTag(const std::string& name);

    // 单点写入（线程安全）
    // 返回 0 成功，非0 失败
    int setTag(const std::string& name, const std::string& value, uint64_t timestamp_ms = 0,
               const std::string& driver = std::string(), const std::string& device = std::string());

    // 单点读取，返回 true 表示找到
    bool getTag(const std::string& name, TagRecord& out);

    // 批量读取（按名字列表），返回实际返回的记录数
    std::vector<TagRecord> getTags(const std::vector<std::string>& names);

    // 批量写入，接受 vector of tuples(name,value,timestamp,driver,device)
    // 返回写入成功的数量
    size_t setTags(const std::vector<std::tuple<std::string, std::string, uint64_t, std::string, std::string>>& entries);

    // 获取统计信息（非强一致，只作监控）
    RTDBStats getStats() const;

    // 当前 tag 数量
    size_t size() const;

    // 健康检查（返回 true 表示 OK）
    bool healthCheck(std::string* outReason = nullptr) const;

    // Update callbacks: called when a tag is updated. Returns callback id.
    using UpdateCallback = std::function<void(const TagRecord&)>;
    size_t addUpdateCallback(UpdateCallback cb);
    bool removeUpdateCallback(size_t cbId);

private:
    struct Shard {
        mutable std::shared_mutex mutex;
        std::unordered_map<std::string, std::unique_ptr<TagRecord>> map;
    };

    size_t shardIndex(const std::string& name) const noexcept;
    static size_t calculateShards();

    const size_t shards_;
    std::vector<std::unique_ptr<Shard>> shards_vec_;

    RTDBStats stats_;
    // Update callbacks storage
    std::unordered_map<size_t, UpdateCallback> callbacks_;
    mutable std::mutex callbacks_mutex_;
};

} // namespace nodeserver
