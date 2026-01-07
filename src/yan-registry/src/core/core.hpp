#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "aco/result.hpp"

namespace aco::core {

// ===== 输入结构 =====
struct RegisterRequest {
    std::string serial_number;  // 设备唯一序列号
    std::string csr_pem;        // CSR in PEM format
    int64_t timestamp;          // Unix timestamp (seconds)
};

// ===== 配置结构 =====
struct RegistrationConfig {
    std::string ca_cert_path;   // IoT Device CA 证书路径
    std::string ca_key_path;    // IoT Device CA 私钥路径
    int replay_window_sec = 300; // 防重放窗口（默认5分钟）
};

// ===== 主入口函数 =====
/**
 * @brief 完整设备注册流程（预注册验证 → 挑战检查 → 凭证签发）
 * @return 成功：SMIME 加密的凭证包（binary）
 *         失败：错误原因（"Device not pre-registered", "Timestamp expired" 等）
 */
Result<std::vector<uint8_t>> process_registration(
    const RegisterAssistant& req,
    const RegistrationConfig& config
);

// ===== 辅助函数（用于单元测试）=====
Result<void> verify_challenge(
    const std::string& serial_number,
    int64_t timestamp,
    int window_sec
);

bool is_public_key_bound(
    const std::string& serial_number,
    EVP_PKEY* csr_pubkey
);

} // namespace aco::core