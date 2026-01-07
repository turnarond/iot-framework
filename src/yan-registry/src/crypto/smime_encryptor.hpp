namespace aco::crypto {

struct CredentialBundle {
    X509* certificate;          // 设备证书
    EVP_PKEY* private_key;      // 设备私钥（内存中）
};

// 返回 SMIME EnvelopedData (DER binary)
Result<std::vector<uint8_t>> create_smime_package(
    const CredentialBundle& bundle,
    EVP_PKEY* recipient_pubkey  // 用于加密的公钥
);

} // namespace aco::crypto