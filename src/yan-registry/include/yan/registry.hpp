#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace aco::registry {

struct RegisterRequest {
    std::string serial_number;
    std::string csr_pem;        // CSR in PEM format
    int64_t timestamp;          // Unix timestamp (seconds)
};

struct CredentialPackage {
    std::vector<uint8_t> smime_data;  // PKCS#7 EnvelopedData (binary)
};

// 主注册函数（core 层）
Result<CredentialPackage> process_registration(const RegisterRequest& req);

// 时间戳验证（防重放）
bool is_timestamp_fresh(int64_t ts, int window_seconds = 300);

} // namespace aco::registry