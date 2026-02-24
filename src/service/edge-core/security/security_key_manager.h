/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: token_service.h .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */

#pragma once
#include <string>
#include <memory>
#include <chrono>
#include <optional>
#include <functional>

namespace edge {
namespace core {

// 安全密钥结构（只用于数据传输）
struct SecurityKeys {
    std::string publishKey_;    // 发布密钥
    std::string signature_;     // 签名  
    std::string nonce_;         // 随机数
    
    SecurityKeys() = default;
    SecurityKeys(std::string pk, std::string sig, std::string nonce)
        : publishKey_(std::move(pk)), signature_(std::move(sig)), nonce_(std::move(nonce)) {}
    
    bool isValid() const {
        return !publishKey_.empty() && !signature_.empty() && !nonce_.empty();
    }
};

// 存储后端接口
class KeyStorageBackend {
public:
    virtual ~KeyStorageBackend() = default;
    virtual bool store(const std::string& key, const std::string& value) = 0;
    virtual bool load(const std::string& key, std::string& value) const = 0;
    virtual bool remove(const std::string& key) = 0;
    virtual bool exists(const std::string& key) const = 0;
};

// 安全密钥管理器 - 唯一入口点
class SecurityKeyManager {
public:
    // 回调类型，用于审计和监控
    using KeyAccessCallback = std::function<void(const std::string& operation, bool success)>;
    
    explicit SecurityKeyManager(std::unique_ptr<KeyStorageBackend> storage);
    ~SecurityKeyManager() = default;
    
    // 禁止拷贝和赋值
    SecurityKeyManager(const SecurityKeyManager&) = delete;
    SecurityKeyManager& operator=(const SecurityKeyManager&) = delete;
    
    // 核心接口 - 业务层只能通过这些接口获取密钥
    SecurityKeys getKeysForRegistration();
    SecurityKeys getKeysForPublish();
    
    // 密钥管理操作
    bool regenerateKeys();
    bool refreshKeys();
    
    // 设置回调（用于审计）
    void setAccessCallback(KeyAccessCallback callback);
    
    // 状态查询
    bool hasValidKeys() const;
    bool areKeysExpired() const;

private:
    // 内部生成方法
    SecurityKeys generateNewKeys();
    std::string generatePublishKey();
    std::string generateSignature(const std::string& publishKey);
    std::string generateNonce();
    
    // 内部存储方法
    bool persistKeys(const SecurityKeys& keys);
    bool loadPersistedKeys(SecurityKeys& keys);
    
    // 验证方法
    bool validateKeyFormat(const std::string& key) const;
    bool validateKeys(const SecurityKeys& keys) const;
    
    std::mutex mutex_;
    std::unique_ptr<KeyStorageBackend> storage_;
    SecurityKeys currentKeys_;
    std::chrono::system_clock::time_point keysGeneratedAt_;
    KeyAccessCallback accessCallback_;
    
    // 存储键名（常量）
    static constexpr const char* STORAGE_KEY_PUBLISH = "security.publish_key";
    static constexpr const char* STORAGE_KEY_SIGNATURE = "security.signature";
    static constexpr const char* STORAGE_KEY_NONCE = "security.nonce";
    static constexpr const char* STORAGE_KEY_GENERATED_AT = "security.generated_at";
    
    static constexpr auto KEY_VALIDITY_PERIOD = std::chrono::hours(24);
};

} // namespace core
} // namespace edge