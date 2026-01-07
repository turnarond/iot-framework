#include "aco/core.hpp"
#include "aco/storage/device_repository.hpp"
#include "aco/crypto/x509_helper.hpp"
#include "aco/crypto/smime_encryptor.hpp"
#include "aco/utils/time_utils.hpp"
#include <openssl/x509.h>
#include <memory>

// RAII 封装 OpenSSL 指针（避免内存泄漏）
template<typename T, auto Deleter>
struct OpenSSLDeleter {
    void operator()(T* ptr) { Deleter(ptr); }
};
using X509_REQ_ptr = std::unique_ptr<X509_REQ, OpenSSLDeleter<X509_REQ, X509_REQ_free>>;
using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, OpenSSLDeleter<EVP_PKEY, EVP_PKEY_free>>;
using X509_ptr = std::unique_ptr<X509, OpenSSLDeleter<X509, X509_free>>;

namespace aco::core {

Result<std::vector<uint8_t>> process_registration(
    const RegisterRequest& req,
    const RegistrationConfig& config
) {
    // === Step 1: 防重放检查 ===
    if (!is_timestamp_fresh(req.timestamp, config.replay_window_sec)) {
        return Result<std::vector<uint8_t>>::error("Timestamp expired");
    }

    // === Step 2: 验证设备是否预注册 ===
    auto device = storage::DeviceRepository::get_by_sn(req.serial_number);
    if (!device || device->status != "PENDING") {
        return Result<std::vector<uint8_t>>::error("Device not pre-registered or already activated");
    }

    // === Step 3: 解析 CSR ===
    auto csr_result = crypto::parse_csr_pem(req.csr_pem);
    if (!csr_result) {
        return Result<std::vector<uint8_t>>::error("Invalid CSR: " + csr_result.error_msg);
    }
    X509_REQ_ptr csr(csr_result.value);

    // === Step 4: 提取公钥 ===
    EVP_PKEY* raw_pubkey = crypto::extract_pubkey_from_csr(csr.get());
    if (!raw_pubkey) {
        return Result<std::vector<uint8_t>>::error("Failed to extract public key from CSR");
    }
    EVP_PKEY_ptr csr_pubkey(raw_pubkey);

    // === Step 5: 验证公钥是否匹配预注册（零信任核心）===
    if (!device->public_key_pem.empty()) {
        // 预注册时已提供公钥 → 必须严格匹配
        auto pre_reg_pub = crypto::parse_public_key_pem(device->public_key_pem);
        if (!pre_reg_pub || !crypto::keys_equal(pre_reg_pub.get(), csr_pubkey.get())) {
            return Result<std::vector<uint8_t>>::error("Public key mismatch with pre-registration");
        }
    } else {
        // 首次绑定模式：记录此公钥（仅限 PENDING 状态）
        storage::DeviceRepository::bind_public_key(req.serial_number, req.csr_pem);
    }

    // === Step 6: 签发设备证书 ===
    auto cert_result = crypto::issue_certificate(
        csr_pubkey.get(),
        req.serial_number,
        config.ca_cert_path,
        config.ca_key_path
    );
    if (!cert_result) {
        return Result<std::vector<uint8_t>>::error("Certificate issuance failed: " + cert_result.error_msg);
    }
    X509_ptr cert(cert_result.value);

    // === Step 7: 生成设备私钥（内存中）===
    auto privkey_result = crypto::generate_device_key();
    if (!privkey_result) {
        return Result<std::vector<uint8_t>>::error("Private key generation failed");
    }
    EVP_PKEY_ptr privkey(privkey_result.value);

    // === Step 8: 构建凭证包并 SMIME 加密 ===
    crypto::CredentialBundle bundle{
        .certificate = cert.get(),
        .private_key = privkey.get()
    };

    auto smime_result = crypto::create_smime_package(bundle, csr_pubkey.get());
    if (!smime_result) {
        return Result<std::vector<uint8_t>>::error("SMIME encryption failed: " + smime_result.error_msg);
    }

    // === Step 9: 激活设备（更新状态 + 保存证书指纹）===
    storage::DeviceRepository::activate_device(req.serial_number, cert.get());

    return Result<std::vector<uint8_t>>::ok(std::move(smime_result.value));
}

// --- 辅助函数实现 ---
Result<void> verify_challenge(const std::string& sn, int64_t ts, int window) {
    if (!is_timestamp_fresh(ts, window)) {
        return Result<void>::error("Timestamp expired");
    }
    // 可扩展：加入 nonce 检查
    return Result<void>::ok({});
}

bool is_public_key_bound(const std::string& sn, EVP_PKEY* pubkey) {
    auto device = storage::DeviceRepository::get_by_sn(sn);
    if (!device || device->public_key_pem.empty()) return false;
    
    auto stored_pub = crypto::parse_public_key_pem(device->public_key_pem);
    return stored_pub && crypto::keys_equal(stored_pub.get(), pubkey);
}

} // namespace aco::core