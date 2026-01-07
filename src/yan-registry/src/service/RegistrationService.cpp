/*
 *  Copyright (c)
 */

#include "RegistrationService.hpp"
#include "db/DeviceRegistrationDb.hpp"
#include "dto/StatusDto.hpp"
#include <oatpp/core/Types.hpp>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <memory>
#include <chrono>

// —————— 辅助：从 PEM CSR 提取公钥（PEM 格式） ——————
static std::string extractPublicKeyFromCSR(const std::string& csrPem) {
    if (csrPem.empty()) {
        throw std::invalid_argument("Empty CSR");
    }

    BIO* bio = BIO_new_mem_buf(csrPem.c_str(), static_cast<int>(csrPem.size()));
    if (!bio) {
        throw std::runtime_error("BIO_new_mem_buf failed");
    }

    X509_REQ* csr = PEM_read_bio_X509_REQ(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!csr) {
        char buf[256];
        ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
        throw std::runtime_error(std::string("Failed to parse CSR: ") + buf);
    }

    EVP_PKEY* pkey = X509_REQ_get_pubkey(csr);
    if (!pkey) {
        X509_REQ_free(csr);
        throw std::runtime_error("CSR has no public key");
    }

    // 将公钥写回 PEM
    BIO* outBio = BIO_new(BIO_s_mem());
    if (!PEM_write_bio_PUBKEY(outBio, pkey)) {
        EVP_PKEY_free(pkey);
        X509_REQ_free(csr);
        BIO_free(outBio);
        throw std::runtime_error("Failed to write public key to PEM");
    }

    BUF_MEM* bufPtr;
    BIO_get_mem_ptr(outBio, &bufPtr);
    std::string pubKeyPem(bufPtr->data, bufPtr->length);

    // 清理
    BIO_free(outBio);
    EVP_PKEY_free(pkey);
    X509_REQ_free(csr);

    return pubKeyPem;
}

// —————— 辅助：标准化 PEM（移除换行，便于比较） ——————
static std::string normalizePEM(const std::string& pem) {
    std::string normalized;
    normalized.reserve(pem.size());
    for (char c : pem) {
        if (c != '\n' && c != '\r') {
            normalized += c;
        }
    }
    return normalized;
}

// —————— 主服务实现 ——————
oatpp::Object<StatusDto> RegistrationService::preRegister(const oatpp::Object<PreRegisterRequestDto> request)
{
    oatpp::Object<StatusDto> status = StatusDto::createShared();
    return  status;
}

oatpp::Object<StatusDto> RegistrationService::deleteDevice(const oatpp::Int32 &id)
{
    oatpp::Object<StatusDto> status = StatusDto::createShared();
    return  status;
}

oatpp::Object<StatusDto> RegistrationService::deviceRegister(const oatpp::Object<DeviceRegisterRequestDto> request) 
{
    oatpp::Object<StatusDto> status = StatusDto::createShared();
    return  status;
    // // 1. 检查必填字段
    // if (!request.serial_number || request.serial_number->empty()) {
    //     return StatusDto::createError("serial_number is required");
    // }
    // if (!request.csr_pem || request.csr_pem->empty()) {
    //     return StatusDto::createError("csr_pem is required");
    // }
    // if (!request.timestamp) {
    //     return StatusDto::createError("timestamp is required");
    // }

    // // 2. 防重放：时间戳必须在 ±5 分钟内
    // auto now = std::chrono::duration_cast<std::chrono::seconds>(
    //     std::chrono::system_clock::now().time_since_epoch()
    // ).count();
    // if (std::abs(now - request.timestamp->getValue()) > 300) {
    //     return StatusDto::createError("Timestamp out of acceptable range (±5 minutes)");
    // }

    // // 3. 查询预注册设备
    // auto deviceOpt = m_repo->getBySerialNumber(request.serial_number);
    // if (!deviceOpt.has_value()) {
    //     return StatusDto::createError("Device not pre-registered");
    // }
    // auto device = deviceOpt.value();

    // if (device.status->compare("PENDING") != 0) {
    //     return StatusDto::createError("Device already activated or revoked");
    // }

    // // 4. 从 CSR 提取公钥
    // std::string csrPubKey;
    // try {
    //     csrPubKey = extractPublicKeyFromCSR(request.csr_pem->std_str());
    // } catch (const std::exception& e) {
    //     return StatusDto::createError(("Invalid CSR: " + std::string(e.what())).c_str());
    // }

    // // 5. 比较公钥（标准化后）
    // std::string expectedPubKey = normalizePEM(device.public_key_pem->std_str());
    // std::string actualPubKey = normalizePEM(csrPubKey);

    // if (expectedPubKey != actualPubKey) {
    //     return StatusDto::createError("CSR public key does not match pre-registered public key");
    // }

    // // 6. 激活设备
    // bool activated = m_repo->activateDevice(device.id);
    // if (!activated) {
    //     return StatusDto::createError("Failed to activate device");
    // }

    // // 7. 返回成功（SMIME 生成由上层处理）
    // return StatusDto::createSuccess("Device registered and ready for credential issuance");
}

// —————— 生成 SMIME 凭证（简化版：返回占位符） ——————
// 实际项目中应在此处调用 CA 签发逻辑
oatpp::String RegistrationService::generateSMIME(const oatpp::Object<DeviceRegisterRequestDto> request) {
    // TODO: 集成 CA 签发流程，返回 PKCS#7 SMIME
    // 示例：返回一个 fake SMIME 包含设备信息
    std::string smime = R"(MIME-Version: 1.0
Content-Type: application/pkcs7-mime; smime-type=enveloped-data; name="credential.p7m"
Content-Disposition: attachment; filename="credential.p7m"

-----BEGIN PKCS7-----
MIAGCSqGSIb3DQEHAqCAMIACAQExCzAJBgUrDgMCGgUAMIAGCSqGSIb3DQEHAaCA
L4EEgC6AZmFrZS1zbWltZS1kYXRhLWZvci1kZXZpY2Ut)";
    return smime;
}
