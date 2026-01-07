-- aco-registry database schema
-- Compatible with SQLite and PostgreSQL

-- 1. 预注册设备表（工厂/Web 录入）
CREATE TABLE IF NOT EXISTS devices (
    id SERIAL PRIMARY KEY,
    serial_number TEXT NOT NULL UNIQUE,      -- 设备唯一SN（如：GW-5000-123456）
    model TEXT NOT NULL,                     -- 设备型号（如：GW-5000）
    owner TEXT,                              -- 所属组织（可选）
    public_key_pem TEXT,                     -- 预注册公钥（PEM格式，可为空）
    status TEXT NOT NULL DEFAULT 'PENDING',  -- PENDING / ACTIVE / REVOKED
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    activated_at TIMESTAMP,                  -- 激活时间
    last_seen_at TIMESTAMP,                  -- 最后连接时间（业务层更新）
    
    -- 安全索引
    CONSTRAINT chk_status CHECK (status IN ('PENDING', 'ACTIVE', 'REVOKED'))
);

-- 2. 设备证书信息（注册成功后填充）
CREATE TABLE IF NOT EXISTS device_certificates (
    device_id INTEGER NOT NULL REFERENCES devices(id) ON DELETE CASCADE,
    certificate_pem TEXT NOT NULL,           -- 设备证书（PEM）
    certificate_fingerprint TEXT NOT NULL,   -- SHA256指纹（用于快速查找）
    issuer_dn TEXT NOT NULL,                 -- 签发者（如：O=ACO, CN=Device CA）
    not_before TIMESTAMP NOT NULL,
    not_after TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    PRIMARY KEY (device_id)
);

-- 3. 注册审计日志（防重放/安全分析）
CREATE TABLE IF NOT EXISTS registration_audit (
    id SERIAL PRIMARY KEY,
    serial_number TEXT NOT NULL,
    client_ip TEXT NOT NULL,                 -- 请求来源IP
    user_agent TEXT,                         -- 可选（设备类型）
    timestamp_req BIGINT NOT NULL,           -- 设备发送的时间戳（Unix秒）
    server_received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    csr_subject TEXT,                        -- CSR中的Subject（如：CN=SN123）
    success BOOLEAN NOT NULL,                -- 是否成功
    error_message TEXT                       -- 失败原因（如：Public key mismatch）
);

-- 4. 凭证吊销列表（未来扩展）
CREATE TABLE IF NOT EXISTS revoked_certificates (
    fingerprint TEXT PRIMARY KEY,            -- 证书指纹
    revoked_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    reason TEXT                              -- 如：keyCompromise, cessationOfOperation
);

-- 5. 设备运行时事件日志（由 data broker 写入）
CREATE TABLE IF NOT EXISTS event_log (
    id SERIAL PRIMARY KEY,
    device_id INTEGER NOT NULL REFERENCES devices(id) ON DELETE CASCADE,
    event_type TEXT NOT NULL,
    severity TEXT NOT NULL DEFAULT 'INFO',
    message TEXT,
    payload TEXT,  -- 兼容 SQLite；PostgreSQL 可用 JSONB
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
-- 索引优化查询

-- ===== 索引 =====
CREATE INDEX IF NOT EXISTS idx_devices_sn ON devices(serial_number);
CREATE INDEX IF NOT EXISTS idx_devices_status ON devices(status);
CREATE INDEX IF NOT EXISTS idx_cert_fingerprint ON device_certificates(certificate_fingerprint);
CREATE INDEX IF NOT EXISTS idx_audit_sn ON registration_audit(serial_number);
CREATE INDEX IF NOT EXISTS idx_audit_time ON registration_audit(server_received_at);


CREATE INDEX IF NOT EXISTS idx_event_device ON event_log(device_id);
CREATE INDEX IF NOT EXISTS idx_event_type ON event_log(event_type);
CREATE INDEX IF NOT EXISTS idx_event_time ON event_log(created_at);