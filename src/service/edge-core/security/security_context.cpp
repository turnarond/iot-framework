/*
 * 安全 API 实现
 */
// security_context.cpp
#include "security_context.h"
#include "cfgm/config_manager.h"
#include "lwcomm/lwcomm.h"
#include <mutex>
#include <memory>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <cstring>

// 前向声明内部状态
namespace edge::core {
// 在 InternalState 中增加存储路径（或依赖 ConfigManager）
struct InternalState {
    EVP_PKEY* privateKey = nullptr;
    X509* certificate = nullptr;
    X509* caCertificate = nullptr;
    bool keysGenerated = false;
    bool valid = false; // 是否已有有效证书
    mutable std::mutex mtx;

    // 存储路径（实际项目中应从配置读取）
    std::string KEY_PATH = std::string(LWComm::GetDataPath()) + LW_OS_DIR_SEPARATOR + "certs" + LW_OS_DIR_SEPARATOR + "device.key";
    std::string CERT_PATH = std::string(LWComm::GetDataPath()) + LW_OS_DIR_SEPARATOR + "certs" + LW_OS_DIR_SEPARATOR + "device.crt";
    std::string CSR_PATH = std::string(LWComm::GetDataPath()) + LW_OS_DIR_SEPARATOR + "certs" + LW_OS_DIR_SEPARATOR + "device.csr";
    std::string CA_CERT_PATH = std::string(LWComm::GetDataPath()) + LW_OS_DIR_SEPARATOR + "certs" + LW_OS_DIR_SEPARATOR + "server_ca.crt";

    ~InternalState() { /* ... */ }

    // 启动时加载
    bool loadFromStorage() 
    {
        // 尝试加载证书
        BIO* certBio = BIO_new_file(CERT_PATH.c_str(), "r");
        if (certBio) {
            caCertificate = PEM_read_bio_X509(certBio, nullptr, nullptr, nullptr);
            BIO_free(certBio);
        }

        // 尝试加载私钥
        BIO* keyBio = BIO_new_file(KEY_PATH.c_str(), "r");
        if (keyBio) {
            privateKey = PEM_read_bio_PrivateKey(keyBio, nullptr, nullptr, nullptr);
            BIO_free(keyBio);
        }

        // 尝试加载 ca 证书链
        BIO* caCertBio = BIO_new_file(CA_CERT_PATH.c_str(), "r");
        if (caCertBio) {
            caCertificate = PEM_read_bio_X509(caCertBio, nullptr, nullptr, nullptr);
            BIO_free(caCertBio);
        }

        // 验证：证书和ca链是否匹配
        if (caCertificate && X509_verify(caCertificate, privateKey)) {
            keysGenerated = true;
            valid = true;
            return true;
        }

        // 清理无效状态
        if (privateKey) { EVP_PKEY_free(privateKey); privateKey = nullptr; }
        if (caCertificate) { X509_free(caCertificate); caCertificate = nullptr; }
        keysGenerated = false;
        valid = false;
        return false;
    }

    // 保存到磁盘
    bool saveToStorage() 
    {
        if (!privateKey || !caCertificate) return false;

        // 保存私钥
        BIO* keyBio = BIO_new_file(KEY_PATH.c_str(), "w");
        if (!keyBio) return false;
        PEM_write_bio_PrivateKey(keyBio, privateKey, nullptr, nullptr, 0, nullptr, nullptr);
        BIO_free(keyBio);

        // 保存证书
        BIO* certBio = BIO_new_file(CERT_PATH.c_str(), "w");    
        if (!certBio) return false;
        PEM_write_bio_X509(certBio, caCertificate);
        BIO_free(certBio);

        return true;
    }
};

using namespace edge::core;

// 实现 Impl 构造
InternalState* SecurityContext::state_ = nullptr;
SecurityContext SecurityContext::instance;
SecurityContext* SecurityContext::getInstance() 
{
    return &instance;
}

int SecurityContext::init(const DeviceInfoConfig& deviceInfo) 
{
    try {
        csrInfo_.commonName = deviceInfo.deviceName;
        csrInfo_.organization = deviceInfo.projectId;
        csrInfo_.serialNumber = deviceInfo.deviceId;
        if (!deviceInfo.location.empty()) {
            csrInfo_.locality = deviceInfo.location;
        }
        state_ = new InternalState();
        state_->loadFromStorage(); // 启动时自动加载
        return 0;
    } catch (...) {
        return -1;
    }
}

bool SecurityContext::isDeviceCertValid(const std::string deviceName) const
{
    if (!state_) return false;
    std::lock_guard<std::mutex> lock(state_->mtx);
    // 检查设备名称是否匹配
    if (deviceName != csrInfo_.commonName) return false;

    return state_->valid;
}

bool SecurityContext::submitCertificate (const std::string& certPem) 
{
    if (!state_) return false;
    std::lock_guard<std::mutex> lock(state_->mtx);

    if (!state_->keysGenerated) return false;

    // 解析证书
    BIO* bio = BIO_new_mem_buf(certPem.c_str(), certPem.size());
    X509* cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!cert) return false;

    // （可选）验证证书是否由可信 CA 签发、是否匹配私钥
    // 此处简化

    if (state_->caCertificate) X509_free(state_->caCertificate);
    state_->caCertificate = cert;
    state_->valid = true;

    // 持久化保存！
    BIO* certBio = BIO_new_file(state_->CERT_PATH.c_str(), "w");
    if (!certBio) return false;
    PEM_write_bio_X509(certBio, cert);
    BIO_free(certBio);
    // if (!state_->saveToStorage()) {
    //     // 保存失败，回滚？
    //     X509_free(state_->caCertificate);
    //     state_->caCertificate = nullptr;
    //     state_->registered = false;
    //     return false;
    // }

    return true;
}

bool SecurityContext::submitCaCertificate(const std::string& caCertPem)
{
    if (!state_) return false;
    std::lock_guard<std::mutex> lock(state_->mtx);
    if (!state_->keysGenerated) return false;

    // 解析 CA 证书
    BIO* bio = BIO_new_mem_buf(caCertPem.c_str(), caCertPem.size());
    X509* caCert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!caCert) return false;

    // （可选）验证 CA 证书是否有效
    // 此处简化
    // 保存 CA 证书
    BIO* certBio = BIO_new_file(state_->CA_CERT_PATH.c_str(), "w");
    if (!certBio) return false;
    PEM_write_bio_X509(certBio, caCert);
    BIO_free(certBio);

    if (state_->caCertificate) X509_free(state_->caCertificate);
    state_->caCertificate = caCert;
    return true;
}

std::optional<std::string> SecurityContext::getDeviceCert() const
{
    if (!state_) return std::nullopt;
    std::lock_guard<std::mutex> lock(state_->mtx);
    if (!state_->caCertificate) return std::nullopt;

    char* certPem = nullptr;
    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_X509(bio, state_->caCertificate);
    BIO_get_mem_data(bio, &certPem);
    std::string certStr(certPem);
    BIO_free(bio);
    return certStr;
}

std::optional<std::string> SecurityContext::generateDeviceCsr()
{
    if (!state_) return std::nullopt;
    std::lock_guard<std::mutex> lock(state_->mtx);

    if (state_->valid) {
        // 可选：返回错误，或允许重新注册
        return std::nullopt; // 或生成新 CSR 用于续期
    }

    // 如果已有私钥，复用；否则生成
    if (!state_->keysGenerated) {
        // 生成 RSA 密钥对（2048 位）
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
        if (!ctx) return std::nullopt;

        if (EVP_PKEY_keygen_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return std::nullopt;
        }
        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return std::nullopt;
        }

        EVP_PKEY* pkey = nullptr;
        if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return std::nullopt;
        }
        EVP_PKEY_CTX_free(ctx);

        if (state_->privateKey) EVP_PKEY_free(state_->privateKey);
        state_->privateKey = pkey;
        state_->keysGenerated = true;

        // 👇 生成后立即保存私钥（防止 crash 丢失）
        // 注意：此时无证书，只保存私钥
        BIO* keyBio = BIO_new_file(state_->KEY_PATH.c_str(), "w");
        if (keyBio) {
            PEM_write_bio_PrivateKey(keyBio, state_->privateKey, nullptr, nullptr, 0, nullptr, nullptr);
            BIO_free(keyBio);
        }
    }

    // 创建 CSR
    X509_REQ* req = X509_REQ_new();
    if (!req) return std::nullopt;

    X509_REQ_set_pubkey(req, state_->privateKey);

    // 设置 Subject DN
    X509_NAME* subject = X509_NAME_new();
    X509_NAME_add_entry_by_txt(subject, "CN", MBSTRING_ASC,
                               (unsigned char*)csrInfo_.commonName.c_str(), -1, -1, 0);
    if (!csrInfo_.organization.empty()) {
        X509_NAME_add_entry_by_txt(subject, "O", MBSTRING_ASC,
                                   (unsigned char*)csrInfo_.organization.c_str(), -1, -1, 0);
    }
    if (!csrInfo_.serialNumber.empty()) {
        X509_NAME_add_entry_by_txt(subject, "serialNumber", MBSTRING_ASC,
                                   (unsigned char*)csrInfo_.serialNumber.c_str(), -1, -1, 0);
    }
    X509_NAME_add_entry_by_txt(subject, "C", MBSTRING_ASC,
                               (unsigned char*)csrInfo_.country.c_str(), -1, -1, 0);
    if (!csrInfo_.locality.empty()) {
        X509_NAME_add_entry_by_txt(subject, "L", MBSTRING_ASC,
                                   (unsigned char*)csrInfo_.locality.c_str(), -1, -1, 0);
    }
    if (!csrInfo_.state.empty()) {
        X509_NAME_add_entry_by_txt(subject, "ST", MBSTRING_ASC,
                                   (unsigned char*)csrInfo_.state.c_str(), -1, -1, 0);
    }
    X509_REQ_set_subject_name(req, subject);
    X509_NAME_free(subject);

    // 签名 CSR
    if (X509_REQ_sign(req, state_->privateKey, EVP_sha256()) == 0) {
        X509_REQ_free(req);
        return std::nullopt;
    }

    // 输出 PEM 格式
    BIO* bio = BIO_new(BIO_s_mem());
    if (!PEM_write_bio_X509_REQ(bio, req)) {
        BIO_free(bio);
        X509_REQ_free(req);
        return std::nullopt;
    }

    // 保存 CSR 到文件（可选）
    BIO* csrBio = BIO_new_file(state_->CSR_PATH.c_str(), "w");
    if (csrBio) {
        PEM_write_bio_X509_REQ(csrBio, req);
        BIO_free(csrBio);
    }

    BUF_MEM* buf;
    BIO_get_mem_ptr(bio, &buf);
    std::string pemCSR(buf->data, buf->length);

    BIO_free(bio);
    X509_REQ_free(req);

    return pemCSR;
}

bool SecurityContext::isRegistered() const 
{
    if (!state_) return false;
    std::lock_guard<std::mutex> lock(state_->mtx);
    return state_->caCertificate != nullptr;
}

bool SecurityContext::regenerateKeys() 
{
    if (!state_) return false;  
    std::lock_guard<std::mutex> lock(state_->mtx);
    if (state_->privateKey) {
        EVP_PKEY_free(state_->privateKey);
        state_->privateKey = nullptr;
    }
    if (state_->caCertificate) {
        X509_free(state_->caCertificate);
        state_->caCertificate = nullptr; 
    }
    state_->keysGenerated = false;
    return true;
}

} // namespace edge::core