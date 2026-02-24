#include "rtdb.hpp"
#include <functional>
#include <algorithm>
#include <thread>
#include <thread>

using namespace nodeserver;

RTDB::RTDB(size_t shards)
    : shards_(shards == 0 ? calculateShards() : shards)
{
    shards_vec_.reserve(shards_);
    for (size_t i = 0; i < shards_; ++i) {
        auto s = std::make_unique<Shard>();
        s->map.reserve(256); // 默认预留，运行时可以按需调整
        shards_vec_.push_back(std::move(s));
    }
}

size_t RTDB::addUpdateCallback(UpdateCallback cb) {
    static std::atomic<size_t> id_gen{1};
    if (!cb) return 0;
    size_t id = id_gen.fetch_add(1, std::memory_order_relaxed);
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    callbacks_.emplace(id, std::move(cb));
    return id;
}

bool RTDB::removeUpdateCallback(size_t cbId) {
    if (cbId == 0) return false;
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    auto it = callbacks_.find(cbId);
    if (it == callbacks_.end()) return false;
    callbacks_.erase(it);
    return true;
}

// 计算分片数
size_t RTDB::calculateShards() {
    unsigned int nc = std::thread::hardware_concurrency();
    if (nc == 0) nc = 4; // fallback
    return static_cast<size_t>(nc) * 2;
}

size_t RTDB::shardIndex(const std::string& name) const noexcept {
    // 使用 std::hash，然后取模
    static std::hash<std::string> hasher;
    return hasher(name) % shards_;
}

bool RTDB::registerTag(const std::string& name) {
    auto idx = shardIndex(name);
    Shard& shard = *shards_vec_[idx];
    std::unique_lock<std::shared_mutex> lock(shard.mutex);
    auto it = shard.map.find(name);
    if (it != shard.map.end()) return false; // 已存在
    auto rec = std::make_unique<TagRecord>(name);
    shard.map.emplace(name, std::move(rec));
    stats_.total_tags.fetch_add(1, std::memory_order_relaxed);
    return true;
}

bool RTDB::unregisterTag(const std::string& name) {
    auto idx = shardIndex(name);
    Shard& shard = *shards_vec_[idx];
    std::unique_lock<std::shared_mutex> lock(shard.mutex);
    auto it = shard.map.find(name);
    if (it == shard.map.end()) return false;
    shard.map.erase(it);
    stats_.total_tags.fetch_sub(1, std::memory_order_relaxed);
    return true;
}

int RTDB::setTag(const std::string& name, const std::string& value, uint64_t timestamp_ms,
                 const std::string& driver, const std::string& device) {
    if (timestamp_ms == 0) {
        timestamp_ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    auto idx = shardIndex(name);
    Shard& shard = *shards_vec_[idx];
    // exclusive lock for write
    std::unique_lock<std::shared_mutex> lock(shard.mutex);
    auto it = shard.map.find(name);
    TagRecord* rec_ptr = nullptr;
    
    if (it == shard.map.end()) {
        // create record if not exist
        auto rec = std::make_unique<TagRecord>(name);
        rec->value = value;
        rec->timestamp_ms = timestamp_ms;
        rec->driver_name = driver;
        rec->device_name = device;
        rec->version.fetch_add(1, std::memory_order_relaxed);
        auto [new_it, inserted] = shard.map.emplace(name, std::move(rec));
        if (inserted) {
            it = new_it; // 更新迭代器指向新插入的元素
            stats_.total_tags.fetch_add(1, std::memory_order_relaxed);
        }
    } else {
        TagRecord* rec = it->second.get();
        rec->value = value;
        rec->timestamp_ms = timestamp_ms;
        if (!driver.empty()) rec->driver_name = driver;
        if (!device.empty()) rec->device_name = device;
        rec->version.fetch_add(1, std::memory_order_relaxed);
    }

    stats_.writes.fetch_add(1, std::memory_order_relaxed);
    stats_.last_write_ts.store(timestamp_ms, std::memory_order_relaxed);
    // invoke callbacks for this single update (copy callbacks under lock)
    if (it != shard.map.end()) {
        TagRecord rec = *(it->second.get());
        std::vector<RTDB::UpdateCallback> cbs;
        {
            std::lock_guard<std::mutex> lock(callbacks_mutex_);
            cbs.reserve(callbacks_.size());
            for (auto &kv : callbacks_) cbs.push_back(kv.second);
        }
        for (auto &cb : cbs) {
            try { cb(rec); } catch(...) {}
        }
    }
    return 0;
}

bool RTDB::getTag(const std::string& name, TagRecord& out) {
    auto idx = shardIndex(name);
    Shard& shard = *shards_vec_[idx];
    std::shared_lock<std::shared_mutex> lock(shard.mutex);
    auto it = shard.map.find(name);
    if (it == shard.map.end()) return false;
    TagRecord* rec = it->second.get();
    out.name = rec->name;
    out.value = rec->value;
    out.timestamp_ms = rec->timestamp_ms;
    out.driver_name = rec->driver_name;
    out.device_name = rec->device_name;
    out.version.store(rec->version.load(std::memory_order_relaxed), std::memory_order_relaxed);
    stats_.reads.fetch_add(1, std::memory_order_relaxed);
    return true;
}

std::vector<TagRecord> RTDB::getTags(const std::vector<std::string>& names) {
    std::vector<TagRecord> result;
    result.reserve(names.size());

    // We can try to group by shard to reduce lock/unlock churn
    // Map shard index -> list of names
    std::unordered_map<size_t, std::vector<const std::string*>> groups;
    groups.reserve(16);
    for (const auto& n : names) {
        size_t idx = shardIndex(n);
        groups[idx].push_back(&n);
    }

    for (auto& kv : groups) {
        size_t idx = kv.first;
        Shard& shard = *shards_vec_[idx];
        std::shared_lock<std::shared_mutex> lock(shard.mutex);
        for (const std::string* pn : kv.second) {
            auto it = shard.map.find(*pn);
            if (it != shard.map.end()) {
                TagRecord rec;
                TagRecord* src = it->second.get();
                rec.name = src->name;
                rec.value = src->value;
                rec.timestamp_ms = src->timestamp_ms;
                rec.driver_name = src->driver_name;
                rec.device_name = src->device_name;
                rec.version.store(src->version.load(std::memory_order_relaxed), std::memory_order_relaxed);
                result.push_back(std::move(rec));
                stats_.reads.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

    return result;
}

size_t RTDB::setTags(const std::vector<std::tuple<std::string, std::string, uint64_t, std::string, std::string>>& entries) {
    // Group entries per shard
    std::unordered_map<size_t, std::vector<const std::tuple<std::string, std::string, uint64_t, std::string, std::string>*>> groups;
    groups.reserve(16);
    for (const auto& e : entries) {
        const std::string& name = std::get<0>(e);
        size_t idx = shardIndex(name);
        groups[idx].push_back(&e);
    }

    size_t written = 0;
    for (auto& kv : groups) {
        size_t idx = kv.first;
        Shard& shard = *shards_vec_[idx];
        std::unique_lock<std::shared_mutex> lock(shard.mutex);
        for (const auto* pe : kv.second) {
            const std::string& name = std::get<0>(*pe);
            const std::string& value = std::get<1>(*pe);
            uint64_t ts = std::get<2>(*pe);
            const std::string& driver = std::get<3>(*pe);
            const std::string& device = std::get<4>(*pe);
            if (ts == 0) {
                ts = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
            }
            auto it = shard.map.find(name);
            if (it == shard.map.end()) {
                auto rec = std::make_unique<TagRecord>(name);
                rec->value = value;
                rec->timestamp_ms = ts;
                rec->driver_name = driver;
                rec->device_name = device;
                rec->version.fetch_add(1, std::memory_order_relaxed);
                shard.map.emplace(name, std::move(rec));
                stats_.total_tags.fetch_add(1, std::memory_order_relaxed);
            } else {
                TagRecord* rec = it->second.get();
                rec->value = value;
                rec->timestamp_ms = ts;
                if (!driver.empty()) rec->driver_name = driver;
                if (!device.empty()) rec->device_name = device;
                rec->version.fetch_add(1, std::memory_order_relaxed);
            }
            ++written;
        }
    }

    stats_.writes.fetch_add(written, std::memory_order_relaxed);
    if (!entries.empty()) {
        uint64_t now = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        stats_.last_write_ts.store(now, std::memory_order_relaxed);
    }
    // After writes, call update callbacks with copies of changed records.
    if (written > 0) {
        std::vector<TagRecord> callbacks_trigger;
        callbacks_trigger.reserve(written);
        // collect changed records for callbacks (read-lock per shard)
        for (auto& kv : groups) {
            size_t idx = kv.first;
            Shard& shard = *shards_vec_[idx];
            std::shared_lock<std::shared_mutex> lock(shard.mutex);
            for (const auto* pe : kv.second) {
                const std::string& name = std::get<0>(*pe);
                auto it = shard.map.find(name);
                if (it != shard.map.end()) {
                    TagRecord rec = *(it->second.get());
                    callbacks_trigger.push_back(std::move(rec));
                }
            }
        }

        // call callbacks under callbacks_mutex_ copy to avoid holding mutex while invoking
        std::vector<RTDB::UpdateCallback> cbs;
        {
            std::lock_guard<std::mutex> lock(callbacks_mutex_);
            cbs.reserve(callbacks_.size());
            for (auto &kv : callbacks_) cbs.push_back(kv.second);
        }
        for (auto &rec : callbacks_trigger) {
            for (auto &cb : cbs) {
                try { cb(rec); } catch(...) {}
            }
        }
    }
    return written;
}

RTDBStats RTDB::getStats() const {
    RTDBStats s;
    s.total_tags.store(stats_.total_tags.load(std::memory_order_relaxed), std::memory_order_relaxed);
    s.reads.store(stats_.reads.load(std::memory_order_relaxed), std::memory_order_relaxed);
    s.writes.store(stats_.writes.load(std::memory_order_relaxed), std::memory_order_relaxed);
    s.last_write_ts.store(stats_.last_write_ts.load(std::memory_order_relaxed), std::memory_order_relaxed);
    return s;
}

size_t RTDB::size() const {
    size_t total = 0;
    for (size_t i = 0; i < shards_; ++i) {
        Shard& shard = *shards_vec_[i];
        std::shared_lock<std::shared_mutex> lock(shard.mutex);
        total += shard.map.size();
    }
    return total;
}

bool RTDB::healthCheck(std::string* outReason) const {
    // Basic health checks: shards allocated, total tags consistent
    if (shards_vec_.size() != shards_) {
        if (outReason) *outReason = "shard vector size mismatch";
        return false;
    }
    // additional checks could be added: memory usage, last write timestamp freshness, etc.
    return true;
}
