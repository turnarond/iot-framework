-- 启用外键约束
PRAGMA foreign_keys = ON;
PRAGMA journal_mode = WAL;  -- 使用WAL模式提高并发性能
PRAGMA synchronous = NORMAL; -- 平衡性能与数据安全

-- ===================================================================
-- 一、字典表
-- ===================================================================

-- 设备类型字典表
CREATE TABLE IF NOT EXISTS dict_device_type (
    type_id INTEGER PRIMARY KEY AUTOINCREMENT,
    type_code TEXT NOT NULL UNIQUE,
    type_name TEXT NOT NULL,
    description TEXT,
    created_time INTEGER NOT NULL DEFAULT (strftime('%s', 'now') * 1000)
);

INSERT INTO dict_device_type (type_id, type_code, type_name, description) VALUES
(1, 'UNKNOWN', '未知设备', '未知设备类型'),
(2, 'EDGE_DEVICE', '边缘设备', '直接连接到网络的设备'),
(3, 'GATEWAY', '网关设备', '边缘计算网关'),
(4, 'SENSOR', '传感器设备', '物联网传感器'),
(5, 'CONTROLLER', '控制器', '工业控制设备'),
(6, 'END_DEVICE', '终端设备', '直接连接到传感器或控制器的设备');

-- 设备注册阶段字典表
CREATE TABLE IF NOT EXISTS dict_registration_phase (
    phase_id INTEGER PRIMARY KEY,
    phase_name TEXT NOT NULL UNIQUE,
    description TEXT NOT NULL
);

-- 设备注册阶段字典表初始化数据
INSERT OR IGNORE INTO dict_registration_phase (phase_id, phase_name, description) VALUES
    (0, 'UNCONFIGURED', '设备未配置'),
    (1, 'CONFIGURED', '设备已配置'),
    (2, 'CONNECTING', '设备正在连接注册Broker'),
    (3, 'CONNECTED', '设备已连接注册Broker'),
    (4, 'DISCONNECTED', '设备已断开注册Broker'),
    (5, 'REGISTERING', '正在注册设备'),
    (6, 'REGISTERED', '设备已注册'),
    (7, 'EXPIRED', '设备已注销'),
    (8, 'FAILED', '设备注册失败');

-- 事件严重程度字典表
CREATE TABLE IF NOT EXISTS dict_event_severity (
    severity_id INTEGER PRIMARY KEY,
    severity_name TEXT NOT NULL UNIQUE,
    description TEXT NOT NULL
);

INSERT INTO dict_event_severity (severity_id, severity_name, description) VALUES
(0, 'info', '信息性事件，无需操作'),
(1, 'warning', '警告事件，需要注意'),
(2, 'error', '错误事件，需要干预'),
(3, 'critical', '严重事件，需要立即处理');

-- 配置来源类型字典表
CREATE TABLE IF NOT EXISTS dict_config_source (
    source_id INTEGER PRIMARY KEY,
    source_name TEXT NOT NULL UNIQUE,
    description TEXT NOT NULL
);

-- 云服务类型字典表
CREATE TABLE IF NOT EXISTS dict_service_type (
    service_id INTEGER PRIMARY KEY AUTOINCREMENT,
    service_code TEXT NOT NULL UNIQUE CHECK(service_code IN ('status', 'log', 'config', 'command')),
    service_name TEXT NOT NULL,
    description TEXT
);

INSERT INTO dict_service_type (service_id, service_code, service_name, description) VALUES
(1, 'status', '状态服务', '设备状态上报服务'),
(2, 'log', '日志服务', '设备日志上传服务'),
(3, 'config', '配置服务', '设备配置管理服务'),
(4, 'command', '命令服务', '远程命令执行服务');

-- 连接状态字典表
CREATE TABLE IF NOT EXISTS dict_connection_status (
    status_id INTEGER PRIMARY KEY,
    status_name TEXT NOT NULL UNIQUE,
    description TEXT NOT NULL
);

-- IP地址类型字典表
CREATE TABLE IF NOT EXISTS dict_ip_address_type (
    type_id INTEGER PRIMARY KEY,
    type_name TEXT NOT NULL UNIQUE CHECK(type_name IN ('static', 'dhcp', 'auto', 'link-local')),
    description TEXT NOT NULL
);

INSERT INTO dict_ip_address_type (type_id, type_name, description) VALUES
(1, 'static', '静态IP地址'),
(2, 'dhcp', 'DHCP获取'),
(3, 'auto', '自动配置'),
(4, 'link-local', '链路本地地址');

-- 网络接口类型字典表
CREATE TABLE IF NOT EXISTS dict_interface_type (
    type_id INTEGER PRIMARY KEY AUTOINCREMENT,
    type_name TEXT NOT NULL UNIQUE CHECK(type_name IN ('ethernet', 'wifi', 'cellular', 'virtual', 'loopback')),
    description TEXT NOT NULL
);

INSERT INTO dict_interface_type (type_id, type_name, description) VALUES
(1, 'ethernet', '有线以太网接口'),
(2, 'wifi', '无线局域网接口'),
(3, 'cellular', '4G/5G移动网络接口'),
(4, 'virtual', '软件定义的虚拟网络接口'),
(5, 'loopback', '本机通信接口');

-- 网络接口状态字典表
CREATE TABLE IF NOT EXISTS dict_interface_status (
    status_id INTEGER PRIMARY KEY AUTOINCREMENT,
    status_name TEXT NOT NULL UNIQUE CHECK(status_name IN ('up', 'down', 'unknown', 'connected', 'disconnected')),
    status_category TEXT NOT NULL CHECK(status_category IN ('operational', 'physical')),
    description TEXT NOT NULL
);

INSERT INTO dict_interface_status (status_id, status_name, status_category, description) VALUES
(1, 'up', 'operational', '接口已启用并正常工作'),
(2, 'down', 'operational', '接口已禁用或故障'),
(3, 'unknown', 'operational', '接口状态未知'),
(4, 'connected', 'physical', '物理连接已建立'),
(5, 'disconnected', 'physical', '无物理连接');

-- ===================================================================
-- 二、设备身份管理层
-- ===================================================================

-- 注册Broker配置表
CREATE TABLE IF NOT EXISTS registration_broker (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    broker_name TEXT NOT NULL,            -- 注册Broker名称
    protocol TEXT NOT NULL DEFAULT 'mqtt', -- 协议类型
    endpoint TEXT NOT NULL,               -- 注册MQTT Broker地址
    port INTEGER NOT NULL DEFAULT 1883,   -- 端口号
    username TEXT NOT NULL,               -- 连接用户名
    password TEXT NOT NULL,               -- 连接密码 (应用层加密)
    ssl_enable BOOLEAN NOT NULL DEFAULT 0, -- 是否启用SSL
    connection_timeout INTEGER DEFAULT 10000, -- 连接超时时间(ms)
    keepalive INTEGER DEFAULT 60,         -- 保活时间(秒)
    reconnect_interval INTEGER DEFAULT 60000, -- 重连间隔(ms)
    is_active BOOLEAN NOT NULL DEFAULT 0, -- 是否当前使用的配置
    created_time INTEGER NOT NULL DEFAULT (strftime('%s', 'now') * 1000) -- 创建时间 (Unix ms)
);

-- 服务凭证表
CREATE TABLE IF NOT EXISTS service_credential (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    service_broker_endpoint TEXT NOT NULL, -- 服务MQTT Broker地址
    service_broker_port INTEGER NOT NULL DEFAULT 1883, -- 服务端口
    service_username TEXT NOT NULL,       -- 服务用户名
    service_token TEXT NOT NULL,          -- 服务访问令牌
    service_type_id INTEGER NOT NULL,     -- 服务类型ID
    issued_at INTEGER NOT NULL,           -- 令牌签发时间 (Unix ms)
    expires_at INTEGER NOT NULL,          -- 令牌过期时间 (Unix ms)
    refresh_token TEXT,                   -- 刷新令牌 (可选)
    is_active BOOLEAN NOT NULL DEFAULT 0, -- 是否当前使用的凭证
    created_time INTEGER NOT NULL DEFAULT (strftime('%s', 'now') * 1000), -- 创建时间 (Unix ms)
    FOREIGN KEY (service_type_id) REFERENCES dict_service_type(service_id)
);

-- 云服务连接配置表
CREATE TABLE IF NOT EXISTS cloud_connection (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    service_type_id INTEGER NOT NULL,
    credential_id INTEGER,                -- 关联的服务凭证ID
    endpoint TEXT NOT NULL,               -- 服务端点
    port INTEGER NOT NULL,
    protocol TEXT NOT NULL DEFAULT 'mqtt', -- 协议类型
    qos_level INTEGER DEFAULT 1,          -- MQTT QoS等级
    reconnect_interval INTEGER DEFAULT 5000, -- 重连间隔(ms)
    last_connect_time INTEGER,            -- 最后连接成功时间
    last_disconnect_time INTEGER,         -- 最后断开时间
    connection_status INTEGER DEFAULT 0,  -- 0=断开,1=连接中,2=已连接
    failure_count INTEGER DEFAULT 0,      -- 连续失败次数
    FOREIGN KEY (service_type_id) REFERENCES dict_service_type(service_id),
    FOREIGN KEY (credential_id) REFERENCES service_credential(id),
    FOREIGN KEY (connection_status) REFERENCES dict_connection_status(status_id)
);

-- 设备基本信息表 (仅存储当前设备自身信息)
CREATE TABLE IF NOT EXISTS device_identity (
    device_id TEXT PRIMARY KEY NOT NULL,  -- 设备唯一ID (SN)
    device_name TEXT NOT NULL,            -- 设备名称 (用户可读)
    device_type INTEGER NOT NULL,         -- 设备类型ID
    device_model TEXT,                    -- 设备型号
    manufacturer TEXT NOT NULL,           -- 设备厂商
    project_id TEXT NOT NULL,             -- 所属项目ID
    location TEXT,                        -- 设备物理位置
    registration_broker_id INTEGER NOT NULL, -- 注册Broker配置ID
    registration_phase INTEGER NOT NULL DEFAULT 0, -- 0=未配置,1=已配置,2=注册中,3=已注册
    firmware_version TEXT NOT NULL,       -- 固件版本
    last_heartbeat INTEGER,               -- 最后心跳时间 (Unix ms)
    last_modified_time INTEGER NOT NULL DEFAULT (strftime('%s', 'now') * 1000), -- 最后修改时间 (Unix ms)
    registration_time INTEGER,             -- 首次注册时间
    is_active BOOLEAN NOT NULL DEFAULT 1,  -- 是否激活
    deregistration_time INTEGER,           -- 注销时间
    FOREIGN KEY (device_type) REFERENCES dict_device_type(type_id),
    FOREIGN KEY (registration_broker_id) REFERENCES registration_broker(id),
    FOREIGN KEY (registration_phase) REFERENCES dict_registration_phase(phase_id)
);

-- ===================================================================
-- 三、系统监控与日志
-- ===================================================================

-- 系统事件日志
CREATE TABLE IF NOT EXISTS system_event_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    event_type TEXT NOT NULL,             -- 事件类型
    severity INTEGER NOT NULL,            -- 严重程度
    message TEXT NOT NULL,                -- 事件描述
    details TEXT,                         -- 详细信息(JSON格式)
    timestamp INTEGER NOT NULL DEFAULT (strftime('%s', 'now') * 1000), -- 事件时间(Unix ms)
    FOREIGN KEY (severity) REFERENCES dict_event_severity(severity_id)
);

-- 配置历史记录表
CREATE TABLE IF NOT EXISTS config_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    source_type INTEGER NOT NULL,         -- 来源类型
    source_id TEXT,                       -- 来源标识 (如云端任务ID)
    metadata TEXT NOT NULL,
    created_time INTEGER NOT NULL DEFAULT (strftime('%s', 'now') * 1000),
    FOREIGN KEY (source_type) REFERENCES dict_config_source(source_id)
);

-- 网络接口信息表
CREATE TABLE IF NOT EXISTS network_interfaces (
    interface_id INTEGER PRIMARY KEY AUTOINCREMENT,
    interface_name TEXT NOT NULL UNIQUE,  -- 如 eth0, wlan0
    interface_type INTEGER NOT NULL,      -- 接口类型ID
    enabled BOOLEAN NOT NULL DEFAULT 1,
    mac_address TEXT CHECK(mac_address GLOB '[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]'),
    description TEXT,
    mtu INTEGER DEFAULT 1500,
    created_time INTEGER NOT NULL DEFAULT (strftime('%s', 'now') * 1000),
    updated_time INTEGER NOT NULL DEFAULT (strftime('%s', 'now') * 1000),
    last_status_update INTEGER,
    operational_status INTEGER DEFAULT 3, -- 默认为unknown
    physical_status INTEGER DEFAULT 3,    -- 默认为unknown
    FOREIGN KEY (interface_type) REFERENCES dict_interface_type(type_id),
    FOREIGN KEY (operational_status) REFERENCES dict_interface_status(status_id),
    FOREIGN KEY (physical_status) REFERENCES dict_interface_status(status_id)
);

-- IPv4/IPv6 配置表
CREATE TABLE IF NOT EXISTS ip_configurations (
    config_id INTEGER PRIMARY KEY AUTOINCREMENT,
    interface_id INTEGER NOT NULL,
    ip_version INTEGER NOT NULL CHECK(ip_version IN (4, 6)),  -- IPv4 or IPv6
    address_type INTEGER NOT NULL,        -- 地址类型ID
    ip_address TEXT NOT NULL,
    subnet_mask TEXT,
    prefix_length INTEGER,  -- 对于IPv6更有意义
    gateway TEXT,
    dns_servers TEXT,  -- JSON数组格式
    search_domains TEXT,  -- JSON数组格式
    metric INTEGER DEFAULT 100,  -- 路由优先级
    active BOOLEAN NOT NULL DEFAULT 1,
    created_time INTEGER NOT NULL DEFAULT (strftime('%s', 'now') * 1000),
    updated_time INTEGER NOT NULL DEFAULT (strftime('%s', 'now') * 1000),
    FOREIGN KEY (interface_id) REFERENCES network_interfaces(interface_id) ON DELETE CASCADE,
    FOREIGN KEY (address_type) REFERENCES dict_ip_address_type(type_id),
    UNIQUE(interface_id, ip_version)
);

-- ===================================================================
-- 四、索引优化
-- ===================================================================

-- 为高频查询字段创建必要索引
CREATE INDEX IF NOT EXISTS idx_device_info_phase ON device_identity(registration_phase);
CREATE INDEX IF NOT EXISTS idx_device_info_active ON device_identity(is_active);
CREATE INDEX IF NOT EXISTS idx_service_credential_expires ON service_credential(expires_at);
CREATE INDEX IF NOT EXISTS idx_service_credential_active ON service_credential(is_active);
CREATE INDEX IF NOT EXISTS idx_registration_broker_active ON registration_broker(is_active);
CREATE INDEX IF NOT EXISTS idx_cloud_connection_status ON cloud_connection(connection_status);
CREATE INDEX IF NOT EXISTS idx_cloud_connection_service_type ON cloud_connection(service_type_id);
CREATE INDEX IF NOT EXISTS idx_system_event_log_time ON system_event_log(timestamp);
CREATE INDEX IF NOT EXISTS idx_network_interface_name ON network_interfaces(interface_name);
CREATE INDEX IF NOT EXISTS idx_ip_config_interface ON ip_configurations(interface_id);