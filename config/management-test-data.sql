-- 字典表数据
INSERT INTO dict_config_source (source_id, source_name, description) VALUES
(0, 'system', '系统自动生成的配置'),
(1, 'cloud', '从云端同步的配置'),
(2, 'user', '用户手动输入的配置');

INSERT INTO dict_connection_status (status_id, status_name, description) VALUES
(0, '断开', '连接已断开'),
(1, '连接中', '正在尝试连接'),
(2, '已连接', '连接已建立');

-- 注册Broker配置
INSERT INTO registration_broker (broker_name, protocol, endpoint, port, username, password, ssl_enable, connection_timeout, keepalive, reconnect_interval, is_active, created_time) VALUES
('Primary Broker', 'mqtt', '47.92.110.53', 8883, 'device_reg', 'securepass123', 1, 10, 60, 60, 1, strftime('%s', 'now') * 1000),
('Backup Broker', 'mqtt', 'mqtt-backup.example.com', 1883, 'device_reg', 'securepass123', 1, 10, 60, 60, 0, strftime('%s', 'now') * 1000);

-- 服务凭证
INSERT INTO service_credential (service_broker_endpoint, service_broker_port, service_username, service_token, service_type_id, issued_at, expires_at, refresh_token, is_active, created_time) VALUES
('mqtt-service.example.com', 8883, 'gateway001', 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9', 1, strftime('%s', 'now') * 1000, strftime('%s', 'now', '+30 days') * 1000, 'refresh123456', 1, strftime('%s', 'now') * 1000),
('mqtt-service.example.com', 8883, 'gateway001', 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9', 2, strftime('%s', 'now') * 1000, strftime('%s', 'now', '+30 days') * 1000, 'refresh123456', 1, strftime('%s', 'now') * 1000);

-- 云服务连接
INSERT INTO cloud_connection (service_type_id, credential_id, endpoint, port, protocol, qos_level, reconnect_interval, connection_status, failure_count) VALUES
(1, 1, 'mqtt-service.example.com', 8883, 'mqtt', 1, 5000, 2, 0),
(2, 2, 'mqtt-service.example.com', 8883, 'mqtt', 1, 5000, 2, 0);

-- 设备基本信息
INSERT INTO device_identity (device_id, device_name, device_type, device_model, manufacturer, project_id, location, registration_broker_id, registration_phase, firmware_version, last_heartbeat, is_active) VALUES
('SN123456789', 'EdgeGateway-01', 1, 'Gateway Model X', 'IoT Solutions Inc.', 'PROJ-001', 'Building A, Floor 2', 1, 3, 'v2.3.1', strftime('%s', 'now') * 1000, 1);

-- 系统事件日志
INSERT INTO system_event_log (event_type, severity, message, details) VALUES
('DEVICE_REGISTRATION', 0, '设备成功注册到平台', '{"device_id": "SN123456789", "registration_id": "REG-98765"}'),
('CONNECTION_LOST', 1, '与云服务的连接断开', '{"service": "status", "reason": "network_timeout"}'),
('HEARTBEAT_TIMEOUT', 2, '设备心跳超时', '{"device_id": "SN987654321", "last_seen": "' || strftime('%s', 'now', '-10 minutes') * 1000 || '"}');

-- 配置历史
INSERT INTO config_log (source_type, source_id, metadata) VALUES
(1, 'TASK-12345', '{"operation": "UPDATE", "target": "network_settings", "changes": {"wifi_ssid": "new_ssid"}}'),
(0, NULL, '{"operation": "INITIAL_SETUP", "components": ["network", "mqtt", "security"]}');

-- 网络接口
INSERT INTO network_interfaces (interface_name, interface_type, mac_address, description, operational_status, physical_status) VALUES
('eth0', 1, '00:11:22:33:44:55', '主以太网接口', 1, 4),
('wlan0', 2, 'aa:bb:cc:dd:ee:ff', 'WiFi接口', 1, 4),
('lo', 5, NULL, '回环接口', 1, 3);

-- IP配置
INSERT INTO ip_configurations (interface_id, ip_version, address_type, ip_address, subnet_mask, gateway, dns_servers, active) VALUES
(1, 4, 1, '192.168.1.100', '255.255.255.0', '192.168.1.1', '["8.8.8.8", "8.8.4.4"]', 1),
(1, 6, 3, 'fe80::1', '64', NULL, '["2001:4860:4860::8888"]', 1),
(2, 4, 2, '10.0.0.15', '255.255.255.0', '10.0.0.1', '["1.1.1.1"]', 1),
(3, 4, 1, '127.0.0.1', '255.0.0.0', NULL, NULL, 1);