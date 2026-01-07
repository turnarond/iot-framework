namespace aco::crypto {

// 解析 PEM CSR
Result<X509_REQ*> parse_csr_pem(std::string_view pem);

// 从 CSR 提取公钥
EVP_PKEY* extract_pubkey(X509_REQ* csr);

// 使用 IoT-CA 签发证书（需 CA 私钥路径）
Result<X509*> issue_certificate(
    EVP_PKEY* pubkey,
    std::string_view serial_number,
    std::string_view ca_cert_path,
    std::string_view ca_key_path
);

} // namespace aco::crypto