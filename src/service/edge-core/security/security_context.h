/*
 * 安全 API 接口
 */
// security_context.h
#pragma once
#include "cfgm/config_types.h"
#include <string>
#include <optional>

namespace edge::core {

struct CSRInfo {
    std::string commonName;      // 必填，如 device-123
    std::string organization;    // 可选
    std::string serialNumber;    // 设备唯一标识，建议必填
    std::string country = "CN";  // 默认
    std::string locality = "";
    std::string state = "";
};


class SecurityContext {
public:

    static SecurityContext* getInstance();
    // 初始化安全管理上下文
    // 返回值：0 成功，-1 失败
    int init(const DeviceInfoConfig& deviceInfo);

    bool isDeviceCertValid(const std::string deviceName) const;

    // 获取设备crt
    // 返回值：已注册的设备crt，若未注册则为空
    std::optional<std::string> getDeviceCert() const;

    // 生成设备 CSR
    // 返回值：已注册的 CSR，若未注册则为空
    std::optional<std::string> generateDeviceCsr();
    
    // 禁止拷贝，允许移动（可选）
    SecurityContext(const SecurityContext&) = delete;
    SecurityContext& operator=(const SecurityContext&) = delete;
    
    // 提交证书
    bool submitCertificate(const std::string& certPem);

    // 提交 CA 证书
    bool submitCaCertificate(const std::string& caCertPem);
    
    // 检查是否已注册
    bool isRegistered() const;

    std::string getIdentityId() const { return identityId_; }
    // （可选）强制重新生成密钥对
    bool regenerateKeys();

private:

    static SecurityContext instance;
    // 禁止构造
    SecurityContext() = default;

    std::string identityId_;

    static struct InternalState* state_; // PIMPL 隐藏实现

    CSRInfo csrInfo_;

};
#define SECURITY_CONTEXT SecurityContext::getInstance()

} // namespace edge::core