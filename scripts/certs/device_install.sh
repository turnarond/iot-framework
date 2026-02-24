#!/bin/bash
# 设备端证书安装 (device_install.sh)
CERT_CHAIN="received_certs/EDGE-DEVICE-001_chain.crt"
PRIVATE_KEY="device_certs/private/device.key"
TRUSTED_ROOT="trusted_certs/root-ca.crt"  # 设备预先内置的根CA
INSTALL_DIR="device_certs"

# 1. 分离证书链
DEVICE_CERT="${INSTALL_DIR}/certs/device.crt"
INTERMEDIATE_CA="${INSTALL_DIR}/certs/intermediate-ca.crt"

# 从证书链中提取设备证书 (第一部分)
awk 'BEGIN {flag=0} /-----BEGIN CERTIFICATE-----/ {flag++; if(flag==1) print} flag==1 {print} /-----END CERTIFICATE-----/ {if(flag==1) {flag=0; exit}}' $CERT_CHAIN > $DEVICE_CERT

# 提取中间CA证书 (第二部分)
awk 'BEGIN {flag=0} /-----BEGIN CERTIFICATE-----/ {flag++; if(flag==2) print} flag==2 {print} /-----END CERTIFICATE-----/ {if(flag==2) exit}' $CERT_CHAIN > $INTERMEDIATE_CA

echo "已分离证书链:"
echo "设备证书: $DEVICE_CERT"
echo "中间CA证书: $INTERMEDIATE_CA"

# 2. 验证证书链
echo "验证证书链..."
openssl verify -CAfile $TRUSTED_ROOT -untrusted $INTERMEDIATE_CA $DEVICE_CERT
if [ $? -ne 0 ]; then
  echo "错误：证书链验证失败！"
  exit 1
fi

# 3. 验证证书与私钥是否匹配
echo "验证证书与私钥匹配..."
CERT_MODULUS=$(openssl x509 -noout -modulus -in $DEVICE_CERT | openssl md5 -r)
KEY_MODULUS=$(openssl ec -noout -modulus -in $PRIVATE_KEY 2>/dev/null | openssl md5 -r)

if [ "$CERT_MODULUS" != "$KEY_MODULUS" ]; then
  echo "错误：证书与私钥不匹配！"
  exit 1
fi
echo "证书与私钥匹配验证通过"

# 4. 检查证书有效期
echo "检查证书有效期..."
openssl x509 -in $DEVICE_CERT -checkend 0
if [ $? -ne 0 ]; then
  echo "警告：证书已过期或即将过期！"
  # 决定是否继续
  read -p "是否继续安装?(y/n) " -r response
  if [[ "$response" != "y" ]]; then
    exit 1
  fi
fi

# 5. 验证证书中的设备ID是否与预期一致
CERT_DEVICE_ID=$(openssl x509 -in $DEVICE_CERT -text -noout | grep "Subject:.*CN=" | sed 's/.*CN=\([^,]*\).*/\1/')
EXPECTED_ID="EDGE-DEVICE-001"  # 本地存储的设备ID

if [ "$CERT_DEVICE_ID" != "$EXPECTED_ID" ]; then
  echo "错误：证书中的设备ID ($CERT_DEVICE_ID) 与预期 ($EXPECTED_ID) 不匹配！"
  exit 1
fi
echo "设备ID验证通过"

# 6. (可选) 验证证书指纹
if [ -f "received_certs/EDGE-DEVICE-001_fingerprint.txt" ]; then
  EXPECTED_FINGERPRINT=$(cat received_certs/EDGE-DEVICE-001_fingerprint.txt | tr -d ' ' | tr '[:lower:]' '[:upper:]')
  ACTUAL_FINGERPRINT=$(openssl x509 -in $DEVICE_CERT -fingerprint -sha256 -noout | cut -d= -f2 | tr -d ':')
  
  if [ "$EXPECTED_FINGERPRINT" != "$ACTUAL_FINGERPRINT" ]; then
    echo "错误：证书指纹不匹配！"
    echo "预期: $EXPECTED_FINGERPRINT"
    echo "实际: $ACTUAL_FINGERPRINT"
    exit 1
  fi
  echo "证书指纹验证通过"
fi

# 7. 安全存储证书
mkdir -p ${INSTALL_DIR}/installed
cp $DEVICE_CERT ${INSTALL_DIR}/installed/
cp $INTERMEDIATE_CA ${INSTALL_DIR}/installed/
chmod 600 ${INSTALL_DIR}/installed/*

# 8. 生成证书包用于TLS连接
cat $DEVICE_CERT $INTERMEDIATE_CA > ${INSTALL_DIR}/installed/device_full_chain.crt

echo "=== 证书安装完成 ==="
echo "证书已安全存储在: ${INSTALL_DIR}/installed/"
echo "可使用以下命令验证完整配置:"
echo "openssl verify -CAfile $TRUSTED_ROOT ${INSTALL_DIR}/installed/device_full_chain.crt"