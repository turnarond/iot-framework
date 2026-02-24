// services/registration/registration_service.cpp
#include "registration_service.h"
#include "cloud/cloud_client.h"

namespace edge {
namespace services {
namespace registration {

bool RegistrationService::registerWithCloud() {
    // 1. 准备注册数据
    auto deviceInfo = collectDeviceInfo();
    std::string registrationPayload = serializeRegistrationData(deviceInfo);
    
    // 2. 直接调用云端协同库
    auto& cloudClient = cloud::CloudClient::getInstance();
    
    // 3. 同步调用（注册过程通常可以等待）
    cloud::CloudClient::RpcResult result = cloudClient.callService(
        "device-registration", 
        "registerDevice", 
        registrationPayload,
        30000  // 30秒超时，注册过程可以稍长
    );
    
    // 4. 处理结果
    if (result.success) {
        // 解析注册响应
        if (parseRegistrationResponse(result.response)) {
            // 5. 更新本地状态
            m_registrationState = RegistrationState::REGISTERED;
            saveCredentials();  // 保存新凭证
            
            // 6. 通知其他服务
            notifySystemServices();
            
            // 7. 启动定期心跳（使用同一个云连接）
            startHeartbeatService();
            
            return true;
        }
    }
    
    // 失败处理
    m_lastError = result.error;
    m_registrationState = RegistrationState::ERROR;
    return false;
}

void RegistrationService::startHeartbeatService() {
    // 使用同一个云连接，无需额外IPC
    auto& cloudClient = cloud::CloudClient::getInstance();
    
    // 每30秒发送一次心跳
    m_heartbeatTimer = std::make_unique<Timer>(30000, [this]() {
        auto heartbeatData = generateHeartbeatData();
        bool success = cloud::CloudClient::getInstance().publish(
            "devices/" + m_deviceId + "/heartbeat",
            heartbeatData,
            cloud::QosLevel::AT_LEAST_ONCE
        );
        
        if (!success) {
            handleHeartbeatFailure();
        }
    });
    
    m_heartbeatTimer->start();
}

} // namespace registration
} // namespace services
} // namespace edge