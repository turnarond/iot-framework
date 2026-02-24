
-- 启用外键约束（必须在连接级别设置，但此处显式声明意图）
PRAGMA foreign_keys = ON;

-- 点位配置
-- 1. 驱动表（支持4个参数描述）
CREATE TABLE IF NOT EXISTS t_drivers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    description TEXT,
    param1_name TEXT,
    param1_desc TEXT,
    param2_name TEXT,
    param2_desc TEXT,
    param3_name TEXT,
    param3_desc TEXT,
    param4_name TEXT,
    param4_desc TEXT,
    create_time INTEGER NOT NULL  -- Unix ms
);

-- 2. 装置（设备）表（支持4个参数值）
CREATE TABLE IF NOT EXISTS t_devices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    driver_id INTEGER NOT NULL,
    connparam TEXT NOT NULL,
    description TEXT,
    param1 TEXT,
    param2 TEXT,
    param3 TEXT,
    param4 TEXT,
    FOREIGN KEY (driver_id) REFERENCES t_drivers(id) ON DELETE RESTRICT
);

-- 3. 点位表
CREATE TABLE IF NOT EXISTS t_points (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    address TEXT NOT NULL,
    device_id INTEGER NOT NULL,
    datatype INTEGER NOT NULL DEFAULT 2,
    point_type INTEGER NOT NULL DEFAULT 1 CHECK(point_type IN (1, 2)),
    transfer_type INTEGER NOT NULL DEFAULT 0 CHECK(transfer_type IN (0, 1, 2)),
    linear_raw_min REAL,
    linear_raw_max REAL,
    linear_eng_min REAL,
    linear_eng_max REAL,
    advanced_algo_lib TEXT,
    advanced_param1 TEXT,
    advanced_param2 TEXT,
    advanced_param3 TEXT,
    advanced_param4 TEXT,
    enable_control BOOLEAN NOT NULL DEFAULT 0,
    enable_history BOOLEAN NOT NULL DEFAULT 1,
    poll_rate INTEGER NOT NULL DEFAULT 1000,
    description TEXT,
    FOREIGN KEY (device_id) REFERENCES t_devices(id) ON DELETE CASCADE
);

-- 4. 报警规则表
CREATE TABLE IF NOT EXISTS t_alarm_rules (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    point_id INTEGER NOT NULL,
    name TEXT NOT NULL,
    method INTEGER NOT NULL CHECK(method IN (1, 2, 3, 4, 5)),
    threshold REAL NOT NULL,
    enable BOOLEAN NOT NULL DEFAULT 1,
    FOREIGN KEY (point_id) REFERENCES t_points(id) ON DELETE CASCADE
);

-- 5. 字典：点位类型
CREATE TABLE IF NOT EXISTS t_dict_point_types (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT
);
INSERT OR IGNORE INTO t_dict_point_types (id, name, description) VALUES
(1, 'device_variable', '设备变量'),
(2, 'computed_variable', '计算变量');

-- 6. 字典：数据处理方式
CREATE TABLE IF NOT EXISTS t_dict_transfer_types (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT
);
INSERT OR IGNORE INTO t_dict_transfer_types (id, name, description) VALUES
(0, 'none', '无处理'),
(1, 'linear_scaling', '线性缩放'),
(2, 'advanced_algorithm', '高级算法');

-- 7. 字典：报警方法
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
(5, 'fixed_value', '定值', '等于某值时报警');

-- 8. 字典：点位数据类型（新增）
CREATE TABLE IF NOT EXISTS t_dict_data_types (
    code TEXT PRIMARY KEY,          -- 与 t_points.datatype 值一致
    cname TEXT NOT NULL,            -- 中文名
    description TEXT
);
INSERT OR IGNORE INTO t_dict_data_types (code, cname, description) VALUES
('bool',   '布尔型',   'true/false 或 0/1'),
('int',    '整型',     '32位有符号整数'),
('float',  '浮点型',   '32位单精度浮点数'),
('string', '字符串型', '文本数据');

----联动配置---

-- 1. 事件类型表（语义层 + 生产者）
CREATE TABLE IF NOT EXISTS t_trigger_type (
    id INTEGER PRIMARY KEY,
    name VARCHAR(64) NOT NULL UNIQUE,
    cname VARCHAR(64),
    desc VARCHAR(256),
    producer VARCHAR(64) NOT NULL
);

-- 预置事件类型（使用 INSERT OR IGNORE 避免重复插入）
INSERT OR IGNORE INTO t_trigger_type (id, name, cname, desc, producer) VALUES
(1, 'alarm-produce', '报警产生', '由点位越限、故障等产生的报警事件', 'alarm-service'),
(2, 'expr-totrue', '表达式为真', '用户定义的表达式计算结果为真时触发', 'expr-engine'),
(3, 'custom-event', '自定义事件', '由用户算法插件产生的自定义事件', 'custom-plugin');

-- 2. 触发源表（实现层）
CREATE TABLE IF NOT EXISTS t_trigger_source (
    id INTEGER PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    eventtype_name VARCHAR(64) NOT NULL,
    enable INTEGER DEFAULT 1,
    description VARCHAR(256),
    param1 VARCHAR(256),
    param2 VARCHAR(256),
    param3 VARCHAR(256),
    param4 VARCHAR(256),
    FOREIGN KEY(eventtype_name) REFERENCES t_trigger_type(name)
);

-- 3. 动作类型表（动作模板）
CREATE TABLE IF NOT EXISTS t_action_type (
    id INTEGER PRIMARY KEY,
    name VARCHAR(32) NOT NULL UNIQUE,
    libname VARCHAR(64) NOT NULL,
    cname VARCHAR(64),
    param1_name VARCHAR(256),
    param1_tip VARCHAR(256),
    param2_name VARCHAR(256),
    param2_tip VARCHAR(256),
    param3_name VARCHAR(256),
    param3_tip VARCHAR(256),
    param4_name VARCHAR(256),
    param4_tip VARCHAR(256)
);

-- 预置动作类型
INSERT OR IGNORE INTO t_action_type (
    id, name, libname, cname,
    param1_name, param1_tip,
    param2_name, param2_tip,
    param3_name, param3_tip,
    param4_name, param4_tip
) VALUES
-- 单变量控制
(1, 'writetag_single', 'linkact_writetag_single', '单变量控制',
 '变量名称', 'tag点变量的名称',
 '变量值', '要控制的变量值',
 NULL, NULL,
 NULL, NULL),

-- 多变量循环控制（依次写入同一值）
(2, 'writetag_inturn', 'linkact_writetag_inturn', '多变量循环控制',
 '变量组名称', '以逗号隔开的变量组列表，如 tag1,tag2,tag3',
 '变量值', '一个要写入的变量值（所有变量写相同值）',
 NULL, NULL,
 NULL, NULL),

-- 多变量同时控制（不同值）
(3, 'writetag_multiple', 'linkact_writetag_multiple', '多变量同时控制',
 '变量名值对', '格式：tag1,val1;tag2,val2;tag3,val3',
 NULL, NULL,
 NULL, NULL,
 NULL, NULL);

-- 4. 动作实例表（具体配置的动作）
CREATE TABLE IF NOT EXISTS t_action_instance (
    id INTEGER PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    actiontype_id INTEGER NOT NULL,
    param1 VARCHAR(256),
    param2 VARCHAR(256),
    param3 VARCHAR(256),
    param4 VARCHAR(256),
    needconfirm INTEGER DEFAULT 0,
    FOREIGN KEY(actiontype_id) REFERENCES t_action_type(id)
);

-- 5. 联动规则组表
CREATE TABLE IF NOT EXISTS t_linkage_rule (
    id INTEGER PRIMARY KEY,
    name VARCHAR(64) NOT NULL UNIQUE,
    action_id INTEGER NOT NULL,
    logic_type VARCHAR(16) DEFAULT 'AND',
    enabled INTEGER DEFAULT 1,
    description VARCHAR(256),
    FOREIGN KEY(action_id) REFERENCES t_action_instance(id)
);

-- 6. 规则-触发源绑定表（支持多事件触发）
CREATE TABLE IF NOT EXISTS t_rule_trigger (
    rule_id INTEGER NOT NULL,
    trigger_id INTEGER NOT NULL,
    PRIMARY KEY(rule_id, trigger_id),
    FOREIGN KEY(rule_id) REFERENCES t_linkage_rule(id) ON DELETE CASCADE,
    FOREIGN KEY(trigger_id) REFERENCES t_trigger_source(id) ON DELETE CASCADE
);

-- 7. 联动日志表
CREATE TABLE IF NOT EXISTS t_linkage_log (
    id INTEGER PRIMARY KEY,
    rule_id INTEGER,
    rule_name VARCHAR(64),
    triggered_triggers TEXT,            -- JSON: [{"trigger_id":1,"value":"35"}]
    action_id INTEGER,
    action_name VARCHAR(64),
    result INTEGER,                     -- 0=失败, 1=成功
    trigger_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    duration_ms INTEGER,
    raw_payload TEXT                    -- 原始事件数据（可选）
);