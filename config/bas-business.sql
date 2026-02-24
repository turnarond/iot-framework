-- 启用外键约束
PRAGMA foreign_keys = ON;

-- ===================================================================
-- 一、设备与点位配置层
-- ===================================================================

-- 驱动模板（协议抽象）
CREATE TABLE IF NOT EXISTS t_drivers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    type INTEGER NOT NULL DEFAULT 5,
    version TEXT,
    description TEXT,
    param1_name TEXT, param1_desc TEXT,
    param2_name TEXT, param2_desc TEXT,
    param3_name TEXT, param3_desc TEXT,
    param4_name TEXT, param4_desc TEXT,
    create_time INTEGER NOT NULL  -- Unix 毫秒时间戳
);

-- 设备实例
CREATE TABLE IF NOT EXISTS t_devices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    driver_id INTEGER NOT NULL,
    conn_type INTEGER NOT NULL DEFAULT 0,  -- 连接类型，参考t_dict_conn_types
    connparam TEXT NOT NULL,      -- 连接参数（如 IP:Port）
    description TEXT,
    param1 TEXT, param2 TEXT, param3 TEXT, param4 TEXT,
    FOREIGN KEY (driver_id) REFERENCES t_drivers(id) ON DELETE RESTRICT,
    FOREIGN KEY (conn_type) REFERENCES t_dict_conn_types(id) ON DELETE RESTRICT
);

-- 点位定义
CREATE TABLE IF NOT EXISTS t_points (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    address TEXT NOT NULL,        -- 协议相关地址（如 Modbus 寄存器）
    device_id INTEGER NOT NULL,
    datatype INTEGER NOT NULL DEFAULT 2,
    point_type INTEGER NOT NULL DEFAULT 1 CHECK(point_type IN (1, 2)),
    transfer_type INTEGER NOT NULL DEFAULT 0 CHECK(transfer_type IN (0, 1, 2)),
    linear_raw_min REAL, linear_raw_max REAL,
    linear_eng_min REAL, linear_eng_max REAL,
    advanced_algo_lib TEXT,
    advanced_param1 TEXT, advanced_param2 TEXT,
    advanced_param3 TEXT, advanced_param4 TEXT,
    enable_control BOOLEAN NOT NULL DEFAULT 0,
    enable_history BOOLEAN NOT NULL DEFAULT 1,
    poll_rate INTEGER NOT NULL DEFAULT 1000,  -- 毫秒
    description TEXT,
    FOREIGN KEY (device_id) REFERENCES t_devices(id) ON DELETE CASCADE
);

-- ===================================================================
-- 二、字典表（语义化描述）
-- ===================================================================

-- 驱动类型
CREATE TABLE IF NOT EXISTS t_dict_driver_types (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    cname TEXT
);
INSERT OR IGNORE INTO t_dict_driver_types (id, name, cname) VALUES
(1, 'PLC', 'PLC 标准协议'),
(2, 'building_automation', '楼宇自动化类型'),
(3, 'GB', '国标类型'),
(4, 'power_industry', '电力行业'),
(5, 'other', '其他类型');

-- 点位类型
CREATE TABLE IF NOT EXISTS t_dict_point_types (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    cname TEXT
);
INSERT OR IGNORE INTO t_dict_point_types (id, name, cname) VALUES
(1, 'device_variable', '设备变量'),
(2, 'computed_variable', '计算变量');

-- 数据处理方式
CREATE TABLE IF NOT EXISTS t_dict_transfer_types (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    cname TEXT
);
INSERT OR IGNORE INTO t_dict_transfer_types (id, name, cname) VALUES
(0, 'none', '无处理'),
(1, 'linear_scaling', '线性缩放'),
(2, 'advanced_algorithm', '高级算法');

-- 报警方法
CREATE TABLE IF NOT EXISTS t_dict_alarm_methods (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    cname TEXT NOT NULL,
    description TEXT
);
INSERT OR IGNORE INTO t_dict_alarm_methods (id, name, cname, description) VALUES
(1, 'high_high', '高高限', '超过高高阈值报警'),
(2, 'high', '高限', '超过高阈值报警'),
(3, 'low', '低限', '低于低阈值报警'),
(4, 'low_low', '低低限', '低于低低阈值报警'),
(5, 'fixed_value', '定值', '等于某值时报警'),
(6, 'rate_of_change', '变化率', '变化率超过阈值报警'),
(7, 'duration', '持续时间', '持续超过/低于阈值报警'),
(8, 'deviation', '偏差', '与参考值偏差超过阈值报警');

-- 报警状态
CREATE TABLE IF NOT EXISTS t_dict_alarm_states (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    cname TEXT NOT NULL,
    description TEXT
);
INSERT OR IGNORE INTO t_dict_alarm_states (id, name, cname, description) VALUES
(1, 'active', '活动状态', '报警正在触发中'),
(2, 'cleared', '恢复状态', '报警已恢复'),
(3, 'acknowledged', '确认状态', '报警已被确认'),
(4, 'suppressed', '屏蔽状态', '报警被暂时屏蔽');

-- 数据类型
CREATE TABLE IF NOT EXISTS t_dict_data_types (
    id INTEGER PRIMARY KEY,
    code TEXT,
    cname TEXT NOT NULL,
    description TEXT
);
INSERT OR IGNORE INTO t_dict_data_types (id, code, cname, description) VALUES
(1, 'bool',   '布尔型',   'true/false 或 0/1'),
(2, 'int',    '整型',     '32位有符号整数'),
(3, 'float',  '浮点型',   '32位单精度浮点数'),
(4, 'string', '字符串型', '文本数据');

-- 连接类型
CREATE TABLE IF NOT EXISTS t_dict_conn_types (
    id INTEGER PRIMARY KEY,
    code TEXT NOT NULL,
    cname TEXT NOT NULL,
    description TEXT
);
INSERT OR IGNORE INTO t_dict_conn_types (id, code, cname, description) VALUES
(0, 'other', '其他', '默认不支持的类型'),
(1, 'tcpclient', 'TCP客户端', 'TCP客户端连接，需要IP和端口参数'),
(2, 'tcpserver', 'TCP服务端', 'TCP服务端连接，需要监听IP和端口参数'),
(3, 'udp', 'UDP', 'UDP连接，需要本地端口和远程地址参数'),
(4, 'serial', '串口', '串口连接，需要端口、波特率、奇偶校验等参数');

-- ===================================================================
-- 三、报警管理
-- ===================================================================

-- 报警规则（支持恢复逻辑）
CREATE TABLE IF NOT EXISTS t_alarm_rules (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    point_id INTEGER NOT NULL,
    name TEXT NOT NULL,
    method INTEGER NOT NULL CHECK(method IN (1, 2, 3, 4, 5, 6, 7, 8)),
    threshold REAL NOT NULL,
    restore_threshold REAL,
    hysteresis REAL,
    enable BOOLEAN NOT NULL DEFAULT 1,
    enable_restore BOOLEAN NOT NULL DEFAULT 1,
    -- 新增参数，用于变化率、持续时间、偏差等规则
    param1 REAL, -- 变化率规则：时间窗口（秒）
    param2 REAL, -- 持续时间规则：持续时间（秒）
    param3 REAL, -- 偏差规则：参考值
    FOREIGN KEY (point_id) REFERENCES t_points(id) ON DELETE CASCADE
);

-- 报警日志
CREATE TABLE IF NOT EXISTS t_alarm_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    rule_id INTEGER NOT NULL,
    point_id INTEGER NOT NULL,
    point_name TEXT NOT NULL,
    event_type INTEGER NOT NULL CHECK(event_type IN (1, 2)),
    value REAL NOT NULL,
    threshold REAL NOT NULL,
    restore_threshold REAL,
    message TEXT,
    ack_status INTEGER NOT NULL DEFAULT 0,
    ack_user TEXT,
    ack_time INTEGER,
    start_time INTEGER NOT NULL,
    end_time INTEGER,
    duration_ms INTEGER,
    FOREIGN KEY (rule_id) REFERENCES t_alarm_rules(id) ON DELETE CASCADE
);

-- 报警日志索引
CREATE INDEX IF NOT EXISTS idx_alarm_log_point ON t_alarm_log(point_id);
CREATE INDEX IF NOT EXISTS idx_alarm_log_start_time ON t_alarm_log(start_time);
CREATE INDEX IF NOT EXISTS idx_alarm_log_ack_status ON t_alarm_log(ack_status);

-- ===================================================================
-- 四、联动控制（事件驱动自动化）—— 优化版
-- ===================================================================

-- 事件类型（语义层）
CREATE TABLE IF NOT EXISTS t_event_type (
    id INTEGER PRIMARY KEY,
    name VARCHAR(64) NOT NULL UNIQUE,
    cname VARCHAR(64),
    description VARCHAR(256),
    producer VARCHAR(64) NOT NULL
);
INSERT OR IGNORE INTO t_event_type (id, name, cname, description, producer) VALUES
(1, 'alarm-produce', '报警产生', '由点位越限、故障等产生的报警事件', 'linkevt_alarm'),
(2, 'alarm-restore', '报警恢复', '报警恢复正常时产生的事件', 'linkevt_alarm'),
(3, 'expr-totrue', '表达式为真', '用户定义的表达式计算结果为真时触发', 'expr-engine'),
(4, 'custom-event', '自定义事件', '由用户算法插件产生的自定义事件', 'custom-plugin');

-- 表达式定义表
CREATE TABLE IF NOT EXISTS t_expressions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(64) NOT NULL UNIQUE,
    expression VARCHAR(256) NOT NULL,
    enable BOOLEAN DEFAULT 1,
    description VARCHAR(256)
);

-- 联动触发器：具体事件来源实例
CREATE TABLE IF NOT EXISTS t_linkage_trigger (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(64) NOT NULL,
    event_type_id INTEGER NOT NULL,
    alarm_rule_id INTEGER,        -- 仅当 event_type_id IN (1,2)
    expr_id INTEGER,              -- 仅当 event_type_id = 3
    custom_event_key VARCHAR(64), -- 仅当 event_type_id = 4
    enable BOOLEAN DEFAULT 1,
    description VARCHAR(256),
    
    FOREIGN KEY (event_type_id) REFERENCES t_event_type(id),
    FOREIGN KEY (expr_id) REFERENCES t_expressions(id),
    
    CHECK (
        (event_type_id IN (1, 2) AND alarm_rule_id IS NOT NULL)
        OR
        (event_type_id = 3 AND expr_id IS NOT NULL)
        OR
        (event_type_id = 4 AND custom_event_key IS NOT NULL)
    )
);

-- 动作类型模板
CREATE TABLE IF NOT EXISTS t_action_type (
    id INTEGER PRIMARY KEY,
    name VARCHAR(32) NOT NULL UNIQUE,
    libname VARCHAR(64) NOT NULL,
    cname VARCHAR(64),
    param1_name VARCHAR(256), param1_tip VARCHAR(256),
    param2_name VARCHAR(256), param2_tip VARCHAR(256),
    param3_name VARCHAR(256), param3_tip VARCHAR(256),
    param4_name VARCHAR(256), param4_tip VARCHAR(256)
);
INSERT OR IGNORE INTO t_action_type (
    id, name, libname, cname,
    param1_name, param1_tip,
    param2_name, param2_tip,
    param3_name, param3_tip,
    param4_name, param4_tip
) VALUES
(1, 'writetag_single', 'linkact_writetag_single', '单变量控制',
 '变量名称', 'tag点变量的名称',
 '变量值', '要控制的变量值',
 NULL, NULL,
 NULL, NULL),
(2, 'writetag_inturn', 'pklinkact_writetag_inturn', '多变量循环控制',
 '变量组名称', '以逗号隔开的变量组列表，如 tag1,tag2,tag3',
 '变量值', '一个要写入的变量值（所有变量写相同值）',
 NULL, NULL,
 NULL, NULL),
(3, 'writetag_multiple', 'pklinkact_writetag_multiple', '多变量同时控制',
 '变量名值对', '格式：tag1,val1;tag2,val2;tag3,val3',
 NULL, NULL,
 NULL, NULL,
 NULL, NULL);

-- 联动动作实例
CREATE TABLE IF NOT EXISTS t_linkage_action (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(64) NOT NULL,
    action_type_id INTEGER NOT NULL,
    param1 VARCHAR(256),
    param2 VARCHAR(256),
    param3 VARCHAR(256),
    param4 VARCHAR(256),
    need_confirm BOOLEAN NOT NULL DEFAULT 0,
    FOREIGN KEY (action_type_id) REFERENCES t_action_type(id)
);

-- 联动规则（支持多动作序列）
CREATE TABLE IF NOT EXISTS t_linkage_rule (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(64) NOT NULL UNIQUE,
    logic_type VARCHAR(8) NOT NULL DEFAULT 'AND',
    enable BOOLEAN NOT NULL DEFAULT 1,
    description VARCHAR(256),
    CHECK (logic_type = 'AND')
);

-- 规则 ↔ 触发器（多对多）
CREATE TABLE IF NOT EXISTS t_linkage_rule_trigger (
    rule_id INTEGER NOT NULL,
    trigger_id INTEGER NOT NULL,
    PRIMARY KEY (rule_id, trigger_id),
    FOREIGN KEY (rule_id) REFERENCES t_linkage_rule(id) ON DELETE CASCADE,
    FOREIGN KEY (trigger_id) REFERENCES t_linkage_trigger(id) ON DELETE CASCADE
);

-- 规则 ↔ 动作（多对多 + 执行顺序）
CREATE TABLE IF NOT EXISTS t_linkage_rule_action (
    rule_id INTEGER NOT NULL,
    action_id INTEGER NOT NULL,
    exec_order INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (rule_id, action_id),
    FOREIGN KEY (rule_id) REFERENCES t_linkage_rule(id) ON DELETE CASCADE,
    FOREIGN KEY (action_id) REFERENCES t_linkage_action(id) ON DELETE CASCADE
);
CREATE UNIQUE INDEX IF NOT EXISTS idx_rule_action_order ON t_linkage_rule_action(rule_id, exec_order);

-- 联动执行主日志
CREATE TABLE IF NOT EXISTS t_linkage_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    rule_id INTEGER NOT NULL,
    rule_name VARCHAR(64) NOT NULL,
    triggered_by TEXT NOT NULL,          -- JSON array of trigger info
    trigger_time INTEGER NOT NULL,       -- Unix ms
    status INTEGER NOT NULL CHECK(status IN (0, 1, 2)), -- 0=失败, 1=成功, 2=部分成功
    total_duration_ms INTEGER,
    FOREIGN KEY (rule_id) REFERENCES t_linkage_rule(id)
);

-- 动作执行明细日志
CREATE TABLE IF NOT EXISTS t_linkage_action_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    linkage_log_id INTEGER NOT NULL,
    action_id INTEGER NOT NULL,
    action_name VARCHAR(64) NOT NULL,
    exec_order INTEGER NOT NULL,
    result INTEGER NOT NULL CHECK(result IN (0, 1)),
    duration_ms INTEGER,
    error_msg TEXT,
    exec_time INTEGER NOT NULL,
    FOREIGN KEY (linkage_log_id) REFERENCES t_linkage_log(id) ON DELETE CASCADE
);

-- ===================================================================
-- 五、定时任务（复用联动动作）
-- ===================================================================

CREATE TABLE IF NOT EXISTS t_schedules (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    description TEXT,
    enable BOOLEAN NOT NULL DEFAULT 1,
    cron_expr TEXT,               -- e.g., "0 5 * * *"
    start_time INTEGER,           -- Unix ms
    interval_ms INTEGER,          -- 0 = one-time
    action_id INTEGER NOT NULL,
    FOREIGN KEY (action_id) REFERENCES t_linkage_action(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS t_schedule_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    schedule_id INTEGER NOT NULL,
    schedule_name TEXT NOT NULL,
    action_id INTEGER,
    trigger_time INTEGER NOT NULL,
    result INTEGER NOT NULL CHECK(result IN (0,1)),
    duration_ms INTEGER,
    error_msg TEXT,
    FOREIGN KEY (schedule_id) REFERENCES t_schedules(id)
);

-- ===================================================================
-- 六、用户权限管理
-- ===================================================================

-- 角色表
CREATE TABLE IF NOT EXISTS t_roles (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE,
    cname TEXT NOT NULL,
    description TEXT
);
INSERT OR IGNORE INTO t_roles (id, name, cname, description) VALUES
(1, 'admin', '管理员', '拥有所有权限'),
(2, 'operator', '运维员', '拥有设备操作和维护权限'),
(3, 'viewer', '普通用户', '仅拥有查看权限');

-- 用户表
CREATE TABLE IF NOT EXISTS t_users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    role_id INTEGER NOT NULL,
    enable BOOLEAN NOT NULL DEFAULT 1,
    created_at INTEGER NOT NULL,
    updated_at INTEGER,
    FOREIGN KEY (role_id) REFERENCES t_roles(id)
);

-- 系统配置表（用于存储NTP和网络配置）
CREATE TABLE IF NOT EXISTS t_system_config (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    config_key TEXT NOT NULL UNIQUE,
    config_value TEXT NOT NULL,
    description TEXT,
    updated_at INTEGER NOT NULL
);

-- 初始化系统配置默认值
INSERT OR IGNORE INTO t_system_config (config_key, config_value, description, updated_at) VALUES
('ntp_server', 'pool.ntp.org', 'NTP服务器地址', CAST(strftime('%s', 'now') * 1000 AS INTEGER)),
('ntp_enabled', 'false', '是否启用NTP自动同步', CAST(strftime('%s', 'now') * 1000 AS INTEGER)),
('network_mode', 'dhcp', '网络配置模式(dhcp/static)', CAST(strftime('%s', 'now') * 1000 AS INTEGER)),
('network_ip', '192.168.1.100', '静态IP地址', CAST(strftime('%s', 'now') * 1000 AS INTEGER)),
('network_netmask', '255.255.255.0', '子网掩码', CAST(strftime('%s', 'now') * 1000 AS INTEGER)),
('network_gateway', '192.168.1.1', '网关地址', CAST(strftime('%s', 'now') * 1000 AS INTEGER)),
('network_dns1', '8.8.8.8', '首选DNS服务器', CAST(strftime('%s', 'now') * 1000 AS INTEGER)),
('network_dns2', '8.8.4.4', '备用DNS服务器', CAST(strftime('%s', 'now') * 1000 AS INTEGER));

-- ===================================================================
-- 七、视频监控
-- ===================================================================

-- 视频分区管理
CREATE TABLE IF NOT EXISTS t_video_partitions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    description TEXT,
    create_time INTEGER NOT NULL DEFAULT (CAST(strftime('%s', 'now') * 1000 AS INTEGER))
);

-- 摄像头管理
CREATE TABLE IF NOT EXISTS t_video_cameras (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    model TEXT NOT NULL,
    ip TEXT NOT NULL,
    port INTEGER NOT NULL DEFAULT 554,
    username TEXT,
    password TEXT,
    stream_protocol_id INTEGER NOT NULL,
    stream_protocol_param TEXT,
    control_protocol_id INTEGER NOT NULL,
    control_protocol_param TEXT,
    partition_id INTEGER,
    status TEXT NOT NULL DEFAULT 'offline',
    description TEXT,
    create_time INTEGER NOT NULL DEFAULT (CAST(strftime('%s', 'now') * 1000 AS INTEGER)),
    FOREIGN KEY (partition_id) REFERENCES t_video_partitions(id) ON DELETE SET NULL
);

-- 摄像头状态索引
CREATE INDEX IF NOT EXISTS idx_camera_status ON t_video_cameras(status);
-- 分区ID索引
CREATE INDEX IF NOT EXISTS idx_camera_partition ON t_video_cameras(partition_id);

-- 取流协议字典表
CREATE TABLE IF NOT EXISTS t_dict_stream_protocols (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE,
    params TEXT NOT NULL,
    params_desc TEXT NOT NULL
);
INSERT OR IGNORE INTO t_dict_stream_protocols (id, name, params, params_desc) VALUES
(1, 'RTSP', 'RTSP取流地址', 'rtsp://{ip}:{port}/{path}'),
(2, 'RTMP', 'RTMP取流地址', 'rtmp://{ip}:{port}/{app}/{stream}'),
(3, 'HTTP-FLV', 'HTTP-FLV取流地址', 'http://{ip}:{port}/{path}.flv'),
(4, 'HLS', 'HLS取流地址', 'http://{ip}:{port}/{path}/playlist.m3u8');

-- 控制协议字典表
CREATE TABLE IF NOT EXISTS t_dict_control_protocols (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE,
    params TEXT NOT NULL,
    params_desc TEXT NOT NULL
);
INSERT OR IGNORE INTO t_dict_control_protocols (id, name, params, params_desc) VALUES
(1, 'ONVIF', 'ONVIF控制地址', 'http://{ip}:{port}/onvif/device_service');


-- -- 添加外键约束
-- ALTER TABLE t_video_cameras ADD FOREIGN KEY (stream_protocol_id) REFERENCES t_dict_stream_protocols(id) ON DELETE SET NULL;
-- ALTER TABLE t_video_cameras ADD FOREIGN KEY (control_protocol_id) REFERENCES t_dict_control_protocols(id) ON DELETE SET NULL;