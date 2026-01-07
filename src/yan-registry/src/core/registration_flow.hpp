#include <string>
#include "aco/result.hpp"
#include <vector>
#include <inttypes.h>
namespace aco::core {

struct RegisterRequest {
    std::string serial_number;
    std::string csr_pem;
    int64_t timestamp;
};

// 主流程：输入请求 → 输出 SMIME 包
Result<std::vector<uint8_t>> process_registration(
    const RegisterRequest& req,
    const std::string& ca_cert_path,
    const std::string& ca_key_path
);

} // namespace aco::core