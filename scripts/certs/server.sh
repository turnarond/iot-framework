#!/bin/bash
# 云端服务器操作 (server.sh)
INPUT_CSR="device_certs/csr/device.csr"
INPUT_HMAC="device_certs/csr/device.csr.hmac"
OUTPUT_DIR="issued_certs"
CA_DIR="ca_system"

# 创建目录
mkdir -p $OUTPUT_DIR $CA_DIR/{private,certs,crl,newcerts}
touch $CA_DIR/index.txt
echo 1000 > $CA_DIR/serial

# 1. 验证CSR完整性 (使用HMAC)
if [ -f "$INPUT_HMAC" ]; then
  echo "验证CSR完整性..."
  GENERATED_HMAC=$(openssl dgst -sha256 -hmac "device-registration-key" $INPUT_CSR)
  STORED_HMAC=$(cat $INPUT_HMAC)
  
  if [ "$GENERATED_HMAC" != "$STORED_HMAC" ]; then
    echo "错误：CSR完整性验证失败！可能被篡改。"
    exit 1
  fi
  echo "CSR完整性验证通过"
fi

# 2. 验证CSR签名
echo "验证CSR签名..."
openssl req -in $INPUT_CSR -verify -noout
if [ $? -ne 0 ]; then
  echo "错误：CSR签名验证失败！"
  exit 1
fi

# 3. 提取设备信息
# DEVICE_ID=$(openssl req -in $INPUT_CSR -subject -noout | grep "subject=C" | cut -d'=' -f2)
DEVICE_ID="EDGE-DEVICE-001"  # 唯一设备标识
DEVICE_MODEL="Edge-Compute-V2"  # 设备型号

echo "设备ID: $DEVICE_ID"
echo "设备型号: $DEVICE_MODEL"

# 4. 验证设备是否已在注册系统中 (模拟)
REGISTERED_DEVICES="EDGE-DEVICE-001"
if [ ! $REGISTERED_DEVICES =~ $DEVICE_ID ]; then
  echo "警告：设备 $DEVICE_ID 未在注册系统中。是否继续？(y/n)"
  read -r response
  if [[ "$response" != "y" ]]; then
    echo "设备注册中止"
    exit 1
  fi
fi

# 5. 设置CA系统 (首次运行时)
if [ ! -f "$CA_DIR/private/root-ca.key" ]; then
  echo "初始化CA系统..."
  
  # 5.1 根CA私钥 (4096位RSA，安全存储)
  openssl genrsa -out $CA_DIR/private/root-ca.key 4096
  chmod 600 $CA_DIR/private/root-ca.key
  
  # 5.2 根CA证书
  openssl req -new -x509 -key $CA_DIR/private/root-ca.key \
    -out $CA_DIR/certs/root-ca.crt \
    -days 3650 -subj "/C=CN/ST=Beijing/L=Beijing/O=IoT Cloud Platform/OU=Certificate Authority/CN=IoT Root CA"
    
  # 5.3 中间CA私钥
  openssl genrsa -out $CA_DIR/private/intermediate-ca.key 4096
  chmod 600 $CA_DIR/private/intermediate-ca.key
  
  # 5.4 中间CA CSR
  openssl req -new -key $CA_DIR/private/intermediate-ca.key \
    -out $CA_DIR/certs/intermediate-ca.csr \
    -subj "/C=CN/ST=Beijing/L=Beijing/O=IoT Cloud Platform/OU=Device CA/CN=IoT Device Intermediate CA"
    
  # 5.5 用根CA签署中间CA
  openssl x509 -req -in $CA_DIR/certs/intermediate-ca.csr \
    -CA $CA_DIR/certs/root-ca.crt -CAkey $CA_DIR/private/root-ca.key \
    -CAcreateserial -out $CA_DIR/certs/intermediate-ca.crt -days 1825 \
    -extfile <(echo -e "basicConstraints=CA:TRUE,pathlen:0\nkeyUsage=keyCertSign,cRLSign")
fi

# 6. CA配置
cat > $CA_DIR/openssl.cnf << EOF
[ ca ]
default_ca = CA_default

[ CA_default ]
dir               = $CA_DIR
certs             = \$dir/certs
crl_dir           = \$dir/crl
database          = \$dir/index.txt
new_certs_dir     = \$dir/newcerts
certificate       = \$dir/certs/intermediate-ca.crt
serial            = \$dir/serial
crlnumber         = \$dir/crlnumber
crl               = \$dir/crl.pem
private_key       = \$dir/private/intermediate-ca.key
RANDFILE          = \$dir/private/.rand
x509_extensions   = usr_cert
name_opt          = ca_default
cert_opt          = ca_default
default_days      = 365
default_crl_days  = 30
default_md        = sha256
preserve          = no
policy            = policy_match

[ policy_match ]
countryName             = match
stateOrProvinceName     = match
organizationName        = match
organizationalUnitName  = optional
commonName              = supplied
emailAddress            = optional

[ usr_cert ]
basicConstraints        = CA:FALSE
nsCertType              = client
subjectKeyIdentifier    = hash
authorityKeyIdentifier  = keyid,issuer
keyUsage                = critical, digitalSignature, keyEncipherment
extendedKeyUsage        = clientAuth
subjectAltName          = @alt_names

[ alt_names ]
DNS.1                   = $DEVICE_ID
URI.1                   = urn:deviceid:$DEVICE_ID
1.3.6.1.4.1.12345.1.1   = ASN1:UTF8String:$DEVICE_MODEL
EOF

# 7. 签发设备证书
CERT_SERIAL="10$(date +%s | tail -5)"
echo "$CERT_SERIAL" > $CA_DIR/serial

echo "使用中间CA签发设备证书..."
openssl ca -config $CA_DIR/openssl.cnf \
  -in $INPUT_CSR \
  -out $OUTPUT_DIR/${DEVICE_ID}.crt \
  -batch \
  -extensions usr_cert \
  -days 365

# 8. 创建证书包 (设备证书 + 中间CA证书)
cat $OUTPUT_DIR/${DEVICE_ID}.crt $CA_DIR/certs/intermediate-ca.crt > $OUTPUT_DIR/${DEVICE_ID}_chain.crt

# 9. 生成证书指纹 (用于设备验证)
CERT_FINGERPRINT=$(openssl x509 -in $OUTPUT_DIR/${DEVICE_ID}.crt -fingerprint -sha256 -noout | cut -d= -f2)
echo "$CERT_FINGERPRINT" > $OUTPUT_DIR/${DEVICE_ID}_fingerprint.txt

echo "=== 证书签发完成 ==="
echo "设备证书: $OUTPUT_DIR/${DEVICE_ID}.crt"
echo "证书链: $OUTPUT_DIR/${DEVICE_ID}_chain.crt"
echo "指纹: $CERT_FINGERPRINT"
echo "请将证书链安全返回给设备"
