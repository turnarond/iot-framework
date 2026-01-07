#include "smime_encryptor.hpp"
#include <openssl/cms.h>
#include <openssl/bio.h>

namespace aco::registry::crypto {

std::vector<uint8_t> encrypt_private_key(EVP_PKEY* recipient_pubkey, EVP_PKEY* private_key) {
    // 1. 将私钥序列化为 DER
    auto bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PrivateKey(bio, private_key, nullptr, nullptr, 0, nullptr, nullptr);
    char* pem_data;
    long len = BIO_get_mem_data(bio, &pem_data);
    std::string pem_str(pem_data, len);
    BIO_free(bio);

    // 2. 准备明文数据（JSON 包含 cert + encrypted priv? 或直接 priv）
    // 这里简化：直接加密私钥 PEM
    auto data_bio = BIO_new_mem_buf(pem_str.data(), pem_str.size());

    // 3. SMIME 加密
    STACK_OF(X509) *recipients = sk_X509_new_null();
    // 注意：recipient_pubkey 需包装为 X509（临时证书）
    X509* temp_cert = make_temp_cert(recipient_pubkey);
    sk_X509_push(recipients, temp_cert);

    CMS_ContentInfo* cms = CMS_encrypt(recipients, data_bio, nullptr, CMS_BINARY);
    sk_X509_pop_free(recipients, X509_free);
    X509_free(temp_cert);
    BIO_free(data_bio);

    // 4. 输出为 DER
    auto out_bio = BIO_new(BIO_s_mem());
    i2d_CMS_bio(out_bio, cms);
    CMS_ContentInfo_free(cms);

    char* der_data;
    long der_len = BIO_get_mem_data(out_bio, &der_data);
    std::vector<uint8_t> result(der_data, der_data + der_len);
    BIO_free(out_bio);
    return result;
}

} // namespace aco::registry::crypto