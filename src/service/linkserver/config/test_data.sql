-- 启用外键约束
PRAGMA foreign_keys = ON;

-- ================
-- 1. 触发源配置
-- ================

-- 报警类触发源（引用已存在的告警规则ID）
INSERT INTO t_trigger_source (id, name, eventtype_name, description, param1) VALUES
(1, '温度过高告警', 'alarm-produce', '温度点 temp 超过 40℃', 'temp.high'),
(2, '门未关闭告警', 'alarm-produce', '门状态点 door_status 为开启', 'door.open'),
(3, '电压缺相', 'alarm-produce', '三相电压中某一相缺失', 'voltage.phase_loss');

-- 表达式类触发源
INSERT INTO t_trigger_source (id, name, eventtype_name, description, param1) VALUES
(4, '高温干燥', 'expr-totrue', '温度>30且湿度<20', 'temp > 30 && hum < 20'),
(5, '夜间无人值守', 'expr-totrue', '时间在22:00-6:00且无人员刷卡', 'hour() >= 22 || hour() < 6 && !has_card_swipe');

-- 自定义事件触发源（例如AI算法检测到异常）
INSERT INTO t_trigger_source (id, name, eventtype_name, description, param1, param2) VALUES
(6, '火焰识别告警', 'custom-event', '视频分析检测到火焰', 'fire_detection', 'zone_a');

-- ================
-- 2. 动作实例配置
-- ================

-- 动作1：关闭风机（单变量）
INSERT INTO t_action_instance (id, name, actiontype_id, param1, param2) VALUES
(1, '关闭风机', 1, 'fan_on', '0');

-- 动作2：开启排风（单变量）
INSERT INTO t_action_instance (id, name, actiontype_id, param1, param2) VALUES
(2, '开启排风', 1, 'exhaust_fan', '1');

-- 动作3：多变量同时控制（加湿+关空调）
INSERT INTO t_action_instance (id, name, actiontype_id, param1) VALUES
(3, '启动加湿并关闭空调', 3, 'humidifier,1;ac,0');

-- 动作4：发送短信通知（用单变量动作模拟，实际可扩展）
INSERT INTO t_action_instance (id, name, actiontype_id, param1, param2) VALUES
(4, '发送高温告警短信', 1, 'sms_notify', '高温告警：请检查设备！');

-- ================
-- 3. 联动规则配置
-- ================

-- 规则1：温度过高 → 关风机
INSERT INTO t_linkage_rule (id, name, action_id, logic_type, enabled, description) VALUES
(1, '高温自动关风机', 1, 'AND', 1, '当温度过高时自动关闭风机');

-- 规则2：高温干燥 → 启动加湿并关空调
INSERT INTO t_linkage_rule (id, name, action_id, logic_type, enabled, description) VALUES
(2, '高温干燥联动', 3, 'AND', 1, '高温且干燥时启动加湿、关闭空调');

-- 规则3：门未关 + 夜间 → 发短信
INSERT INTO t_linkage_rule (id, name, action_id, logic_type, enabled, description) VALUES
(3, '夜间门未关告警', 4, 'AND', 1, '夜间门未关闭时发送告警短信');

-- 规则4：火焰识别 → 开启排风
INSERT INTO t_linkage_rule (id, name, action_id, logic_type, enabled, description) VALUES
(4, '火焰应急排风', 2, 'AND', 1, '检测到火焰时立即开启排风');

-- ================
-- 4. 规则-触发源绑定
-- ================

-- 规则1：仅绑定温度过高
INSERT INTO t_rule_trigger (rule_id, trigger_id) VALUES (1, 1);

-- 规则2：仅绑定高温干燥表达式
INSERT INTO t_rule_trigger (rule_id, trigger_id) VALUES (2, 4);

-- 规则3：绑定两个触发源（门未关 + 夜间）
INSERT INTO t_rule_trigger (rule_id, trigger_id) VALUES (3, 2), (3, 5);

-- 规则4：绑定火焰识别
INSERT INTO t_rule_trigger (rule_id, trigger_id) VALUES (4, 6);

-- ================
-- 5. 模拟联动日志（可选，用于测试查询）
-- ================

INSERT INTO t_linkage_log (
    rule_id, rule_name, triggered_triggers,
    action_id, action_name, result, duration_ms, raw_payload
) VALUES
-- 高温触发
(1, '高温自动关风机', '[{"trigger_id":1,"value":"42.5"}]', 1, '关闭风机', 1, 15, '{"point":"temp","value":42.5,"alarm_id":"temp.high"}'),

-- 高温干燥触发
(2, '高温干燥联动', '[{"trigger_id":4,"value":"true"}]', 3, '启动加湿并关闭空调', 1, 22, '{"expr":"temp > 30 && hum < 20","result":true}'),

-- 夜间门未关触发
(3, '夜间门未关告警', '[{"trigger_id":2,"value":"1"},{"trigger_id":5,"value":"true"}]', 4, '发送高温告警短信', 1, 8, '{"door_status":1,"time":"23:15","card_swipe":false}'),

-- 火焰识别触发
(4, '火焰应急排风', '[{"trigger_id":6,"value":"detected"}]', 2, '开启排风', 1, 30, '{"event":"fire","zone":"zone_a","confidence":0.96}');