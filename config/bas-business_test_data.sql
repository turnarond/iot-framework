PRAGMA foreign_keys = ON;

-- ===================================================================
-- 一、驱动与设备（保持不变）
-- ===================================================================

INSERT OR IGNORE INTO t_drivers (id, name, type, version, description, param1_name, param1_desc, create_time)
VALUES (1, 'libabc_driver', 1, '1.0.0', '通用采集驱动 libabc.so', 'params', '驱动初始化参数字符串', 1730123456789);

INSERT OR IGNORE INTO t_devices (id, name, driver_id, connparam, description, param1) VALUES
(2, 'XXX舱温度采集模块', 1, 'UDP:10.13.42.12:20001', '温度采集模块，msgId=257', 'xxxxx'),
(3, 'XXX舱油压采集模块', 1, 'UDP:10.13.42.12:20002', '油压采集模块，msgId=258', 'xxxxx'),
(4, 'XXX舱开关量采集模块', 1, 'UDP:10.13.42.12:20003', '开关量采集模块，msgId=259', 'xxxxx');

-- ===================================================================
-- 二、点位
-- ===================================================================

INSERT OR IGNORE INTO t_points (id, name, address, device_id, datatype, point_type, transfer_type, linear_raw_min, linear_raw_max, linear_eng_min, linear_eng_max, advanced_algo_lib, advanced_param1, enable_control, enable_history, poll_rate, description) VALUES
(1, '温度点位1', '0', 2, 3, 1, 2, NULL, NULL, NULL, NULL, 'libMyMath.so', 'xxxxx', 0, 1, 20000, '温度点位1，启用高级预处理'),
(2, '温度点位2', '1', 2, 3, 1, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 1, 20000, '温度点位2'),
(3, '温度点位3', '2', 2, 3, 1, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 1, 20000, '温度点位3'),
(4, '油压点位1', '0', 3, 3, 1, 1, 4.0, 20.0, -20.0, 20.0, NULL, NULL, 0, 1, 25000, '油压点位1'),
(5, '油压点位2', '1', 3, 3, 1, 1, 4.0, 20.0, -20.0, 20.0, NULL, NULL, 0, 1, 25000, '油压点位2'),
(8, '开关点位1', '0', 4, 1, 1, 0, NULL, NULL, NULL, NULL, NULL, NULL, 1, 1, 30000, '可控开关1'),
(9, '开关点位2', '1', 4, 1, 1, 0, NULL, NULL, NULL, NULL, NULL, NULL, 1, 1, 30000, '可控开关2，有报警'),
(15, '开关点位8', '7', 4, 1, 1, 0, NULL, NULL, NULL, NULL, NULL, NULL, 1, 1, 30000, '可控开关8，有报警');

-- ===================================================================
-- 三、报警规则
-- ===================================================================

INSERT OR IGNORE INTO t_alarm_rules (id, point_id, name, method, threshold, restore_threshold, hysteresis, enable, enable_restore) VALUES
(1, 1, '温度高限报警', 2, 45.0, 42.0, NULL, 1, 1),
(2, 2, '温度低限报警', 3, 30.0, 33.0, NULL, 1, 1),
(3, 4, '油压低限报警', 3, 300.0, 310.0, NULL, 1, 1),
(4, 9, '开关点位2状态报警', 5, 1.0, NULL, 0.5, 1, 1),
(5, 15, '开关点位8状态报警', 5, 0.0, NULL, 0.5, 1, 1);

-- ===================================================================
-- 四、联动配置（新结构）
-- ===================================================================

-- 表达式示例数据
INSERT OR IGNORE INTO t_expressions (id, name, expression, enable, description) VALUES
(1, '温度过高表达式', '温度点位1 > 45', 1, '温度点位1超过45度时触发'),
(2, '油压过低表达式', '油压点位1 < 300', 1, '油压点位1低于300时触发'),
(3, '多条件表达式', '温度点位1 > 40 AND 油压点位1 < 350', 1, '温度过高且油压过低时触发');

-- 触发器（绑定报警规则）
INSERT OR IGNORE INTO t_linkage_trigger (id, name, event_type_id, alarm_rule_id, description) VALUES
(1, '温度过高告警', 1, 1, '温度点位1 > 45℃'),
(2, '温度过低告警', 1, 2, '温度点位2 < 30℃'),
(3, '油压过低告警', 1, 3, '油压点位1 < 300'),
(4, '开关2闭合告警', 1, 4, '开关点位2 = 1'),
(5, '开关8断开告警', 1, 5, '开关点位8 = 0');

-- 触发器（绑定表达式）
INSERT OR IGNORE INTO t_linkage_trigger (id, name, event_type_id, expr_id, description) VALUES
(6, '温度过高表达式触发', 3, 1, '基于表达式的温度过高触发'),
(7, '油压过低表达式触发', 3, 2, '基于表达式的油压过低触发'),
(8, '多条件表达式触发', 3, 3, '基于多条件表达式的触发');


-- 动作实例
INSERT OR IGNORE INTO t_linkage_action (id, name, action_type_id, param1, param2) VALUES
(1, '启动加热器', 1, 'heater_on', '1'),
(2, '关闭加热器', 1, 'heater_on', '0'),
(3, '开启备用泵', 1, 'backup_pump', '1'),
(4, '发送短信通知', 1, 'sms_notify', '【告警】设备异常，请检查！');

-- 联动规则：支持多动作
INSERT OR IGNORE INTO t_linkage_rule (id, name, logic_type, enable, description) VALUES
(1, '温度过低自动加热', 'AND', 1, '温度低于30℃时启动加热器'),
(2, '温度过高联动处理', 'AND', 1, '先关加热器，再发短信'),
(3, '油压过低启备用泵', 'AND', 1, '油压低于300时启动备用泵'),
(4, '关键开关异常通知', 'AND', 1, '开关2或8异常时发短信（注：当前为AND，需拆分为两个规则）');

-- 规则-触发器绑定
INSERT OR IGNORE INTO t_linkage_rule_trigger (rule_id, trigger_id) VALUES
(1, 2),
(2, 1),
(3, 3),
(4, 4),
(4, 5);  -- 注意：此规则需两个触发器同时满足（若需OR，请拆成两个规则）

-- 规则-动作绑定（带执行顺序）
INSERT OR IGNORE INTO t_linkage_rule_action (rule_id, action_id, exec_order) VALUES
(1, 1, 1),                    -- 温度过低 → 启动加热器
(2, 2, 1), (2, 4, 2),        -- 温度过高 → 先关加热器，再发短信
(3, 3, 1),                    -- 油压过低 → 启泵
(4, 4, 1);                    -- 开关异常 → 发短信

-- ===================================================================
-- 五、模拟报警日志（可选）
-- ===================================================================

INSERT OR IGNORE INTO t_alarm_log (
    rule_id, point_id, point_name, event_type, value, threshold, restore_threshold,
    message, ack_status, start_time
) VALUES
(1, 1, '温度点位1', 1, 46.2, 45.0, 42.0, '温度超限(46.2℃ > 45℃)', 0, 1730123500000),
(2, 2, '温度点位2', 1, 28.5, 30.0, 33.0, '温度过低(28.5℃ < 30℃)', 0, 1730123510000);

-- ===================================================================
-- 六、用户测试数据
-- ===================================================================

-- 注意：密码哈希值为示例，实际系统中应使用安全的哈希算法
-- 这里使用简单的示例值，实际实现时需要替换为真实的哈希值
-- 密码规则：≥8位，含大小写+数字

INSERT OR IGNORE INTO t_users (id, username, password_hash, role_id, enable, created_at, updated_at) VALUES
(1, 'admin', 'Admin123!', 1, 1, 1730123456789, 1730123456789),  -- 管理员，密码：Admin123!
(2, 'operator', 'Operator123!', 2, 1, 1730123456789, 1730123456789),  -- 运维员，密码：Operator123!
(3, 'viewer', 'Viewer123!', 3, 1, 1730123456789, 1730123456789);  -- 普通用户，密码：Viewer123!

-- ===================================================================
-- 七、视频监控测试数据
-- ===================================================================

-- 视频分区示例数据
INSERT OR IGNORE INTO t_video_partitions (id, name, description, create_time) VALUES
(1, '大门口', '大门区域的摄像头', 1730123456789),
(2, '停车场', '停车场区域的摄像头', 1730123456789),
(3, '楼道', '各楼层楼道的摄像头', 1730123456789),
(4, '办公室', '办公室区域的摄像头', 1730123456789);

-- 取流协议测试数据
INSERT OR IGNORE INTO t_dict_stream_protocols (id, name, params) VALUES
(1, 'RTSP', '{"url": "rtsp://{ip}:{port}/{path}"}'),
(2, 'RTMP', '{"url": "rtmp://{ip}:{port}/{app}/{stream}"}'),
(3, 'HTTP-FLV', '{"url": "http://{ip}:{port}/{path}.flv"}'),
(4, 'HLS', '{"url": "http://{ip}:{port}/{path}/playlist.m3u8"}');

-- 控制协议测试数据
INSERT OR IGNORE INTO t_dict_control_protocols (id, name, params) VALUES
(1, 'ONVIF', '{"wsdl_url": "http://{ip}:{port}/onvif/device_service"}');

-- 摄像头示例数据
INSERT OR IGNORE INTO t_video_cameras (id, name, model, ip, port, username, password, stream_protocol_id, stream_protocol_param, control_protocol_id, control_protocol_param, partition_id, status, description, create_time) VALUES
(1, '大门摄像头1', 'Hikvision DS-2CD2T45FWD-I5', '192.168.1.101', 554, 'admin', '12345', 1, "rtsp://192.168.1.101:554/Streaming/Channels/101", 1, 'http://{ip}:{port}/onvif/device_service', 1, 'online', '主大门入口摄像头', 1730123456789),
(2, '大门摄像头2', 'Hikvision DS-2CD2T45FWD-I5', '192.168.1.102', 554, 'admin', '12345', 1, "rtsp://192.168.1.102:554/Streaming/Channels/101", 1, 'http://{ip}:{port}/onvif/device_service', 1, 'online', '侧门入口摄像头', 1730123456789),
(3, '停车场摄像头1', 'Dahua DH-IPC-HDW4433C-A', '192.168.1.103', 554, 'admin', '12345', 1, "rtsp://192.168.1.103:554/Streaming/Channels/101", 1, 'http://{ip}:{port}/onvif/device_service', 2, 'offline', '停车场入口摄像头', 1730123456789),
(4, '楼道摄像头1', 'UNV IPC3232LB-ADF28K-G', '192.168.1.104', 554, 'admin', '12345', 1, "rtsp://192.168.1.104:554/Streaming/Channels/101", 1, 'http://{ip}:{port}/onvif/device_service', 3, 'online', '一楼楼道摄像头', 1730123456789),
(5, '办公室摄像头1', 'Hikvision DS-2CD2345FWD-I', '192.168.1.105', 554, 'admin', '12345', 1, "rtsp://hik111:554/Streaming/Channels/101", 1, 'http://{ip}:{port}/onvif/device_service', 4, 'online', '总经理办公室摄像头', 1730123456789);
