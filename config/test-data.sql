-- 清空测试数据
DELETE FROM revoked_certificates;
DELETE FROM device_certificates;
DELETE FROM registration_audit;
DELETE FROM devices;

-- 1. 预注册设备（高安全模式：带公钥）
INSERT INTO devices (serial_number, model, owner, public_key_pem, status)
VALUES (
    'SN123456789',
    'GW-5000',
    'acme-factory',
    '-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEJd0nVXN3Z5X9X1X9X1X9X1X9X1X9
X1X9X1X9X1X9X1X9X1X9X1X9X1X9X1X9X1X9X1X9X1X9X1X9X1X9X1X9X1X9X1==
-----END PUBLIC KEY-----',
    'PENDING'
);

-- 2. 预注册设备（开发者模式：无公钥）
INSERT INTO devices (serial_number, model, owner, public_key_pem, status)
VALUES (
    'DEV987654321',
    'DEV-BOARD',
    'developer',
    NULL,
    'PENDING'
);

-- 3. 模拟一次成功的注册审计
INSERT INTO registration_audit (
    serial_number, client_ip, user_agent, timestamp_req, csr_subject, success
) VALUES (
    'SN123456789',
    '192.168.1.100',
    'aco-device/1.0 (ESP32)',
    1717020000,  -- 2024-05-30 10:00:00 UTC
    'CN=SN123456789',
    true
);

-- 4. 模拟一次失败的注册（公钥不匹配）
INSERT INTO registration_audit (
    serial_number, client_ip, timestamp_req, csr_subject, success, error_message
) VALUES (
    'SN123456789',
    '10.0.0.50',
    1717020100,
    'CN=FAKE-SN',
    false,
    'Public key mismatch with pre-registration'
);

-- 5. 激活后的设备证书（示例）
-- 注意：实际由 core 模块在注册成功时插入
/*
INSERT INTO device_certificates (device_id, certificate_pem, certificate_fingerprint, issuer_dn, not_before, not_after)
SELECT 
    id,
    '-----BEGIN CERTIFICATE-----...-----END CERTIFICATE-----',
    'a1b2c3d4e5f6...',
    'O=ACO IoT Platform, CN=Device CA',
    '2024-05-30 10:00:00',
    '2025-05-30 10:00:00'
FROM devices WHERE serial_number = 'SN123456789';
*/

INSERT INTO event_log (
    device_id,
    event_type,
    severity,
    message,
    payload
) VALUES (
    1,
    'initial_activation',
    'INFO',
    'Device reported bootstrap telemetry',
    '{
        "hardware": {"model": "GW-5000", "cpu_arch": "arm64", "total_memory_mb": 1024},
        "os": {"name": "Linux", "version": "5.15.0", "uptime_seconds": 45},
        "services": [
            {"name": "aco-agent", "version": "1.2.0", "status": "running"}
        ],
        "network": {"ip_v4": "192.168.1.100"},
        "security": {"certificate_fingerprint": "a1b2c3d4e5f6..."}
    }'
);