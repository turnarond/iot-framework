// cloud/impl/connection_manager.h
namespace edge {
namespace cloud {
namespace impl {

class ConnectionManager {
public:
    ConnectionManager(const ConnectionConfig& config);
    
    // 多连接支持
    struct ConnectionHandle {
        uint32_t id;
        ProtocolType protocol;
        bool isConnected;
    };
    
    std::vector<ConnectionHandle> getAvailableConnections() const;
    
    // 智能路由
    bool sendMessage(const std::string& topic, const std::string& payload, 
                    QosLevel qos, bool retain);
    
    // 负载均衡
    void enableLoadBalancing(bool enable);
    
    // 故障转移
    void enableFailover(bool enable);
    
    // 连接健康检查
    struct ConnectionHealth {
        float latencyMs;
        float packetLossPercent;
        uint32_t reconnectCount;
        time_t lastReconnectTime;
    };
    
    ConnectionHealth getHealthStatus(uint32_t connectionId) const;
    
    // 资源优化
    void setBandwidthLimit(uint32_t maxKbps);
    void setThrottlePolicy(ThrottlePolicy policy);
    
private:
    // 连接策略
    enum class ConnectionStrategy {
        PRIMARY_BACKUP,    // 主备模式
        LOAD_BALANCE,      // 负载均衡
        LATENCY_BASED,     // 延迟优先
        COST_BASED         // 成本优先
    };
    
    // 内部实现
    void connectionThread();
    void healthCheckThread();
    void offlineQueueThread();
    
    void handleReconnect();
    void handleFailover();
    
    // 连接池
    struct ConnectionEntry {
        std::unique_ptr<ProtocolAdapter> adapter;
        ConnectionHealth health;
        time_t lastActivityTime;
        bool isPrimary;
    };
    
    std::map<uint32_t, ConnectionEntry> m_connections;
    ConnectionStrategy m_strategy;
    
    // 离线队列
    struct OfflineMessage {
        std::string topic;
        std::string payload;
        QosLevel qos;
        bool retain;
        time_t timestamp;
        uint32_t retryCount;
    };
    
    std::queue<OfflineMessage> m_offlineQueue;
    std::mutex m_offlineMutex;
    size_t m_maxQueueSize;
    
    // 资源控制
    RateLimiter m_bandwidthLimiter;
    MessageThrottler m_throttler;
    
    // 线程
    std::thread m_connectionThread;
    std::thread m_healthThread;
    std::thread m_queueThread;
    volatile bool m_running;
};

} // namespace impl
} // namespace cloud
} // namespace edge