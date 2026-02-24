/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: cloud_client.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

namespace edge {
namespace cloud {

// 通信协议类型
enum class ProtocolType {
    MQTT,    // 消息队列遥测传输
    VSOA,    // 微服务通信协议
    HYBRID   // 混合模式，自动切换
};

// 连接配置
struct ConnectionConfig {
    ProtocolType protocol;
    std::string endpoint;
    uint16_t port;
    std::string clientId;
    std::string username;
    std::string password;
    std::string certPath;
    std::string keyPath;
    std::string caPath;
    uint32_t keepAliveSec;
    uint32_t reconnectIntervalSec;
    bool cleanSession;
    
    // 高级配置
    struct {
        uint32_t maxOfflineQueueSize;  // 离线队列最大大小(MB)
        uint32_t maxMessageSize;       // 最大消息大小(KB)
        uint8_t defaultQos;           // 默认QoS级别
        bool enableCompression;       // 启用压缩
    } advanced;
};

// 消息QoS级别
enum class QosLevel {
    AT_MOST_ONCE = 0,  // 最多一次
    AT_LEAST_ONCE = 1, // 至少一次  
    EXACTLY_ONCE = 2   // 恰好一次
};

// 云端客户端接口
class CloudClient {
public:
    // 单例访问
    static CloudClient& getInstance();
    
    // 初始化
    bool init(const ConnectionConfig& config);
    bool initWithCredentials(const std::string& credentialPath);
    
    // 连接管理
    bool connect();
    bool disconnect();
    bool reconnect();
    bool isConnected() const;
    
    // 简单的同步API - 适合大多数场景
    bool publish(const std::string& topic, const std::string& payload, 
                QosLevel qos = QosLevel::AT_LEAST_ONCE, bool retain = false);
    
    bool subscribe(const std::string& topic, QosLevel qos = QosLevel::AT_LEAST_ONCE,
                  std::function<void(const std::string&, const std::string&)> callback = nullptr);
    
    bool unsubscribe(const std::string& topic);
    
    // 高级异步API - 适合高性能场景
    struct PublishOptions {
        QosLevel qos;
        bool retain;
        uint32_t timeoutMs;
        std::function<void(bool success, const std::string& error)> onComplete;
    };
    
    void publishAsync(const std::string& topic, const std::string& payload, 
                     const PublishOptions& options);
    
    // RPC调用 (VSOA模式)
    struct RpcResult {
        bool success;
        std::string response;
        std::string error;
        uint32_t latencyMs;
    };
    
    RpcResult callService(const std::string& serviceName, const std::string& method,
                        const std::string& request, uint32_t timeoutMs = 5000);
    
    // 文件传输 - 支持大文件
    class FileTransferHandle {
    public:
        float getProgress() const;
        bool isComplete() const;
        bool hasError() const;
        std::string getError() const;
    private:
        // 实现细节
    };
    
    std::shared_ptr<FileTransferHandle> downloadFile(
        const std::string& remotePath, 
        const std::string& localPath,
        std::function<void(float progress)> onProgress = nullptr,
        std::function<void(bool success, const std::string& error)> onComplete = nullptr
    );
    
    std::shared_ptr<FileTransferHandle> uploadFile(
        const std::string& localPath,
        const std::string& remotePath,
        std::function<void(float progress)> onProgress = nullptr,
        std::function<void(bool success, const std::string& error)> onComplete = nullptr
    );
    
    // 状态同步
    bool syncDeviceState(const std::map<std::string, std::string>& state);
    bool syncAppStatus(const std::string& appId, const std::string& status);
    
    // 事件处理
    void registerConnectionStateListener(
        std::function<void(bool connected, const std::string& reason)> listener
    );
    
    void registerMessageInterceptor(
        std::function<bool(const std::string& topic, std::string& payload)> interceptor
    );
    
    // 离线支持
    uint32_t getOfflineMessageCount() const;
    void flushOfflineQueue();
    
    // 资源释放
    void cleanup();
    
    // 诊断信息
    struct Diagnostics {
        uint32_t totalMessagesSent;
        uint32_t totalMessagesReceived;
        uint32_t droppedMessages;
        float avgLatencyMs;
        uint32_t reconnectCount;
        time_t lastReconnectTime;
    };
    
    Diagnostics getDiagnostics() const;
    
private:
    CloudClient();
    ~CloudClient();
    
    // 私有实现
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace cloud
} // namespace edge