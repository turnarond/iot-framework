#!/bin/bash
# 设备端操作 (device.sh)
DEVICE_ID="EDGE-DEVICE-001"  # 唯一设备标识
DEVICE_MODEL="Edge-Compute-V2"  # 设备型号
OUTPUT_DIR="device_certs"

# 创建目录
mkdir -p $OUTPUT_DIR/{private,certs,csr}

# 1. 生成EC私钥 (P-256曲线，更安全高效)
openssl ecparam -name prime256v1 -genkey -noout -out $OUTPUT_DIR/private/device.key
chmod 600 $OUTPUT_DIR/private/device.key  # 严格权限控制

# 2. 生成CSR (包含设备身份信息)
# 重点：在subjectAltName中包含设备唯一标识
cat > $OUTPUT_DIR/csr/openssl.cnf << EOF
[ req ]
default_bits        = 256
distinguished_name  = req_distinguished_name
req_extensions      = req_ext
prompt              = no

[ req_distinguished_name ]
C  = CN
ST = Beijing
L  = Beijing
O  = IoT Edge Network
OU = Device Registration
CN = $DEVICE_ID

[ req_ext ]
keyUsage = digitalSignature, keyEncipherment
extendedKeyUsage = clientAuth
subjectAltName = @alt_names
1.3.6.1.4.1.12345.1.1 = ASN1:UTF8String:$DEVICE_MODEL  # 自定义OID存储设备型号

[ alt_names ]
DNS.1 = $DEVICE_ID
URI.1 = urn:deviceid:$DEVICE_ID
EOF

# 生成CSR
openssl req -new -key $OUTPUT_DIR/private/device.key \
  -out $OUTPUT_DIR/csr/device.csr \
  -config $OUTPUT_DIR/csr/openssl.cnf

# 3. 验证CSR (可选，用于调试)
echo "=== 设备CSR内容 ==="
openssl req -in $OUTPUT_DIR/csr/device.csr -text -noout

# 4. 生成CSR指纹 (用于服务器验证CSR完整性)
openssl dgst -sha256 -hmac "device-registration-key" $OUTPUT_DIR/csr/device.csr > $OUTPUT_DIR/csr/device.csr.hmac

echo "设备CSR生成完成: $OUTPUT_DIR/cloud_certs/csr"
echo "请将此CSR和HMAC文件安全上传至云端服务器"