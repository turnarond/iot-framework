/*
 * Copyright (c) 2025 ACOINFO CloudNative Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: token_service.cpp .
 *
 * Date: 2025-12-16
 *
 * Author: Yan.chaodong <yanchaodong@acoinfo.com>
 *
 */
// 5. 安全实现 - 密钥管理器核心逻辑
// /root/work/acu/edge-framework/src/service/edge-core/token_service/security_key_manager.cpp

#include "security_key_manager.h"
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>
#include <chrono>

namespace edge {
namespace core {

SecurityKeyManager::SecurityKeyManager(std::unique_ptr<KeyStorageBackend> storage)
    : storage_(std::move(storage)) {
    
    // 启动时尝试加载已存储的密钥
    SecurityKeys keys;
    if (loadPersistedKeys(keys)) {
        if (validateKeys(keys)) {
            currentKeys_ = keys;
            LOG_INFO("Loaded existing security keys from storage");
        } else {
            LOG_WARN("Stored keys are invalid, generating new ones");
            regenerateKeys();
        }
    } else {
        LOG_INFO("No valid keys found, generating new ones");
        regenerateKeys();
    }
}

std::optional<SecurityKeys> SecurityKeyManager::getKeysForRegistration() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查当前密钥是否有效
    if (!validateKeys(currentKeys_) || areKeysExpired()) {
        LOG_INFO("Current keys are invalid or expired, refreshing");
        if (!refreshKeys()) {
            LOG_ERROR("Failed to refresh keys");
            return std::nullopt;
        }
    }
    
    // 记录访问（用于审计）
    if (accessCallback_) {
        accessCallback_("getKeysForRegistration", true);
    }
    
    // 返回副本（防止外部修改内部状态）
    return currentKeys_;
}

std::optional<SecurityKeys> SecurityKeyManager::getKeysForPublish() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 发布用的密钥可以复用当前有效的密钥
    if (!validateKeys(currentKeys_) || areKeysExpired()) {
        LOG_INFO("Keys expired, refreshing for publish");
        if (!refreshKeys()) {
            return std::nullopt;
        }
    }
    
    if (accessCallback_) {
        accessCallback_("getKeysForPublish", true);
    }
    
    return currentKeys_;
}

bool SecurityKeyManager::regenerateKeys() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        currentKeys_ = generateNewKeys();
        keysGeneratedAt_ = std::chrono::system_clock::now();
        
        if (!persistKeys(currentKeys_)) {
            LOG_ERROR("Failed to persist newly generated keys");
            return false;
        }
        
        LOG_INFO("Successfully regenerated security keys");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during key regeneration: {}", e.what());
        return false;
    }
}

bool SecurityKeyManager::refreshKeys() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // 保持发布密钥不变，只更新签名和随机数
        if (!currentKeys_.publishKey_.empty()) {
            currentKeys_.signature_ = generateSignature(currentKeys_.publishKey_);
            currentKeys_.nonce_ = generateNonce();
            keysGeneratedAt_ = std::chrono::system_clock::now();
            
            if (!persistKeys(currentKeys_)) {
                LOG_ERROR("Failed to persist refreshed keys");
                return false;
            }
            
            LOG_INFO("Successfully refreshed security keys");
            return true;
        } else {
            // 没有现有的发布密钥，需要重新生成
            return regenerateKeys();
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during key refresh: {}", e.what());
        return false;
    }
}

SecurityKeys SecurityKeyManager::generateNewKeys() {
    SecurityKeys keys;
    keys.publishKey_ = generatePublishKey();
    keys.signature_ = generateSignature(keys.publishKey_);
    keys.nonce_ = generateNonce();
    return keys;
}

std::string SecurityKeyManager::generatePublishKey() {
    // 基于设备硬件信息生成发布密钥
    std::stringstream ss;
    ss << "PK_" << std::hex << std::uppercase;
    
    // 使用设备序列号
    ss << getDeviceSerialNumber();
    
    // 添加随机盐值
    unsigned char salt[16];
    RAND_bytes(salt, sizeof(salt));
    for (int i = 0; i < 16; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
    }
    
    // 计算SHA256哈希
    std::string input = ss.str();
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.size());
    SHA256_Final(hash, &sha256);
    
    // 转换为十六进制字符串
    std::stringstream result;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        result << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return result.str();
}

std::string SecurityKeyManager::generateSignature(const std::string& publishKey) {
    std::stringstream ss;
    ss << publishKey;
    
    // 添加时间戳
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    ss << millis;
    
    // 计算HMAC-SHA256
    unsigned char* result;
    unsigned int length;
    result = HMAC(EVP_sha256(), 
                  publishKey.c_str(), publishKey.length(),
                  reinterpret_cast<const unsigned char*>(ss.str().c_str()), ss.str().length(),
                  nullptr, &length);
    
    // 转换为十六进制字符串
    std::stringstream hexStream;
    for (unsigned int i = 0; i < length; i++) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];
    }
    
    return hexStream.str();
}

std::string SecurityKeyManager::generateNonce() {
    unsigned char nonce[32];
    RAND_bytes(nonce, sizeof(nonce));
    
    std::stringstream ss;
    ss << "NONCE_";
    for (int i = 0; i < 32; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)nonce[i];
    }
    
    return ss.str();
}

bool SecurityKeyManager::persistKeys(const SecurityKeys& keys) {
    try {
        // 存储各个组件
        if (!storage_->store(STORAGE_KEY_PUBLISH, keys.publishKey_) ||
            !storage_->store(STORAGE_KEY_SIGNATURE, keys.signature_) ||
            !storage_->store(STORAGE_KEY_NONCE, keys.nonce_)) {
            return false;
        }
        
        // 存储生成时间
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            keysGeneratedAt_.time_since_epoch()).count();
        if (!storage_->store(STORAGE_KEY_GENERATED_AT, std::to_string(timestamp))) {
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to persist keys: {}", e.what());
        return false;
    }
}

bool SecurityKeyManager::loadPersistedKeys(SecurityKeys& keys) {
    try {
        std::string publishKey, signature, nonce, timestamp;
        
        if (!storage_->load(STORAGE_KEY_PUBLISH, publishKey) ||
            !storage_->load(STORAGE_KEY_SIGNATURE, signature) ||
            !storage_->load(STORAGE_KEY_NONCE, nonce)) {
            return false;
        }
        
        keys = SecurityKeys(publishKey, signature, nonce);
        
        // 加载生成时间
        if (storage_->load(STORAGE_KEY_GENERATED_AT, timestamp)) {
            auto time_t = std::stoll(timestamp);
            keysGeneratedAt_ = std::chrono::system_clock::time_point(
                std::chrono::seconds(time_t));
        }
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load persisted keys: {}", e.what());
        return false;
    }
}

bool SecurityKeyManager::validateKeys(const SecurityKeys& keys) const {
    return keys.isValid() && 
           validateKeyFormat(keys.publishKey_) &&
           validateKeyFormat(keys.signature_) &&
           validateKeyFormat(keys.nonce_);
}

bool SecurityKeyManager::validateKeyFormat(const std::string& key) const {
    if (key.empty() || key.length() < 10) {
        return false;
    }
    
    for (char c : key) {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }
    
    return true;
}

bool SecurityKeyManager::hasValidKeys() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return validateKeys(currentKeys_) && !areKeysExpired();
}

bool SecurityKeyManager::areKeysExpired() const {
    auto now = std::chrono::system_clock::now();
    return (now - keysGeneratedAt_) > KEY_VALIDITY_PERIOD;
}

void SecurityKeyManager::setAccessCallback(KeyAccessCallback callback) {
    accessCallback_ = std::move(callback);
}

// 辅助函数
std::string getDeviceSerialNumber() {
    // TODO: 根据平台实现
    return "EDGE_DEVICE_001";
}

} // namespace core
} // namespace edge