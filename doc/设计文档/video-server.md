查看当前项目结构，了解已经实现的功能和模块，为编写详细设计文档做准备。
        
            
toolName: view_folder
            
status: success
          
/home/yanchaodong/work/acoinfo/edge-framework/src/service/video-server
          
查看video-server目录的详细结构，了解所有实现的文件和模块。
        
            
toolName: file_search
            
status: success
          
file_pattern: **/*
          
# 基于ZLMediaKit的流媒体服务器详细设计

## 1. 系统架构

### 1.1 架构概述

本流媒体服务器基于ZLMediaKit开发，采用分层模块化架构，实现了多协议接入、多协议输出、插件系统和高性能流处理能力。系统架构分为以下几层：

| 层级 | 组件 | 职责 |
|------|------|------|
| 应用层 | VideoServer | 主服务器，协调各服务组件 |
| 服务层 | LogService、ConfigService、PluginManager、StreamManager、DeviceManager、DatabaseManager | 核心服务组件 |
| 适配层 | ZLMediaKitAdapter | ZLMediaKit适配，提供流处理能力 |
| 插件层 | StreamProtocolPlugin、ControlProtocolPlugin | 协议处理插件 |
| 设备层 | 摄像机设备 | 视频源设备 |

### 1.2 模块关系图

```
+---------------------+
|    VideoServer      | 主服务器
+---------------------+
          |
+----------+----------+
|                     |
+----------+----------+
|                     |
v                     v
+---------------+     +---------------+
| StreamManager |     | DeviceManager |
+---------------+     +---------------+
       |                     |
       v                     v
+---------------+     +---------------+
| PluginManager |     | DatabaseManager |
+---------------+     +---------------+
       |                     |
       v                     v
+---------------+     +---------------+
|  ZLMediaKit   |     |    SQLite     |
|   Adapter     |     |    Database   |
+---------------+     +---------------+
       |
       v
+---------------+
|  协议插件     |
|  (RTSP、海康等) |
+---------------+
```

## 2. 核心模块设计

### 2.1 VideoServer

**职责：**
- 服务器生命周期管理
- 各服务组件协调
- 插件加载与管理
- 流处理调度

**核心方法：**
- `initialize()` - 初始化服务器
- `start()` - 启动服务器
- `stop()` - 停止服务器
- `loadPlugin()` - 加载插件
- `getStreamUrl()` - 获取流URL

### 2.2 LogService

**职责：**
- 日志管理与输出
- 日志级别控制
- 日志文件管理

**核心方法：**
- `initialize()` - 初始化日志服务
- `logDebug()/logInfo()/logWarn()/logError()/logCritical()` - 不同级别日志输出
- `setLogLevel()` - 设置日志级别
- `setLogDirectory()` - 设置日志目录

### 2.3 ConfigService

**职责：**
- 配置文件管理
- 配置项读写
- 配置变更通知

**核心方法：**
- `initialize()` - 初始化配置服务
- `get()/getInt()/getBool()/getDouble()` - 获取配置项
- `set()/setInt()/setBool()/setDouble()` - 设置配置项
- `addListener()` - 添加配置变更监听器

### 2.4 PluginManager

**职责：**
- 插件加载与卸载
- 插件生命周期管理
- 插件查询与获取

**核心方法：**
- `initialize()` - 初始化插件管理器
- `loadPlugin()` - 加载单个插件
- `loadAllPlugins()` - 加载所有插件
- `getStreamProtocolPlugin()` - 获取流协议插件
- `getControlProtocolPlugin()` - 获取控制协议插件

### 2.5 StreamManager

**职责：**
- 流生命周期管理
- 流状态监控
- 流URL生成
- 客户端连接管理

**核心方法：**
- `initialize()` - 初始化流管理器
- `startStream()` - 启动流
- `stopStream()` - 停止流
- `getStreamUrl()` - 获取流URL
- `incrementClientCount()/decrementClientCount()` - 客户端计数管理

### 2.6 DeviceManager

**职责：**
- 设备管理与状态监控
- 设备连接管理
- 设备命令下发
- 设备发现

**核心方法：**
- `initialize()` - 初始化设备管理器
- `addDevice()` - 添加设备
- `removeDevice()` - 移除设备
- `connectDevice()` - 连接设备
- `sendDeviceCommand()` - 发送设备命令
- `discoverDevices()` - 发现设备

### 2.7 DatabaseManager

**职责：**
- 数据库连接管理
- 摄像机信息管理
- 分区信息管理
- 协议信息管理

**核心方法：**
- `initialize()` - 初始化数据库连接
- `addCamera()` - 添加摄像机
- `removeCamera()` - 移除摄像机
- `getAllCameras()` - 获取所有摄像机
- `getAllPartitions()` - 获取所有分区
- `getAllStreamProtocols()` - 获取所有流协议

### 2.8 ZLMediaKitAdapter

**职责：**
- ZLMediaKit服务器管理
- 流推送与拉取
- 配置管理
- 状态监控

**核心方法：**
- `initialize()` - 初始化ZLMediaKit适配器
- `startServer()` - 启动ZLMediaKit服务器
- `stopServer()` - 停止ZLMediaKit服务器
- `pushStream()` - 推送流
- `stopPushStream()` - 停止推送流
- `generateStreamUrl()` - 生成流URL

## 3. 插件系统设计

### 3.1 插件接口

**PluginInterface** - 基础插件接口：
- `getName()` - 获取插件名称
- `getVersion()` - 获取插件版本
- `getDescription()` - 获取插件描述
- `initialize()` - 初始化插件
- `uninitialize()` - 卸载插件
- `isInitialized()` - 检查插件是否初始化

**StreamProtocolPlugin** - 流协议插件接口：
- `startStream()` - 启动流
- `stopStream()` - 停止流
- `getStreamStatus()` - 获取流状态
- `generateStreamUrl()` - 生成流URL
- `isProtocolSupported()` - 检查协议是否支持
- `getSupportedProtocols()` - 获取支持的协议

**ControlProtocolPlugin** - 控制协议插件接口：
- `connect()` - 连接设备
- `disconnect()` - 断开连接
- `sendCommand()` - 发送命令
- `getDeviceStatus()` - 获取设备状态

### 3.2 插件实现

**RTSPPlugin**：
- 实现RTSP协议接入
- 支持标准RTSP流处理
- 提供RTSP流URL生成

**HikvisionPlugin**：
- 实现海康设备专用协议
- 支持海康摄像机接入
- 提供海康设备流URL生成

### 3.3 插件加载机制

1. 插件以动态库(.so)形式存在于plugins目录
2. PluginManager通过dlopen加载插件
3. 通过函数指针获取插件创建函数
4. 调用createPlugin()创建插件实例
5. 调用initialize()初始化插件
6. 插件卸载时调用destroyPlugin()释放资源

## 4. 数据流设计

### 4.1 输入数据流

1. 摄像机设备 → 插件层（协议处理）
2. 插件层 → ZLMediaKitAdapter（流推送）
3. ZLMediaKitAdapter → ZLMediaKit（流处理）

### 4.2 输出数据流

1. ZLMediaKit → ZLMediaKitAdapter（流拉取）
2. ZLMediaKitAdapter → StreamManager（流管理）
3. StreamManager → 客户端（多协议输出）

### 4.3 协议转换流程

```
摄像机 → RTSP/海康协议 → ZLMediaKit → RTSP/RTMP/HLS/FLV/WebRTC → 客户端
```

## 5. 配置管理

### 5.1 配置文件结构

```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8080,
    "rtsp_port": 554,
    "rtmp_port": 1935,
    "http_port": 80
  },
  "zlmediakit": {
    "enabled": true,
    "path": "/usr/bin/MediaServer",
    "config_path": "/etc/zlmediakit/config.ini"
  },
  "ffmpeg": {
    "path": "/usr/bin/ffmpeg"
  },
  "plugins": {
    "directory": "./plugins",
    "enabled": ["rtsp", "hikvision", "dahua", "gb28181"]
  },
  "logging": {
    "level": "info",
    "directory": "./logs"
  },
  "devices": {
    "discovery_interval": 60,
    "status_check_interval": 30
  }
}
```

### 5.2 配置项说明

| 配置项 | 说明 | 默认值 |
|--------|------|--------|
| server.host | 服务器主机地址 | 0.0.0.0 |
| server.port | 服务器端口 | 8080 |
| server.rtsp_port | RTSP服务端口 | 554 |
| server.rtmp_port | RTMP服务端口 | 1935 |
| server.http_port | HTTP服务端口 | 80 |
| zlmediakit.enabled | 是否启用ZLMediaKit | true |
| zlmediakit.path | ZLMediaKit可执行文件路径 | /usr/bin/MediaServer |
| ffmpeg.path | FFmpeg可执行文件路径 | /usr/bin/ffmpeg |
| plugins.directory | 插件目录 | ./plugins |
| plugins.enabled | 启用的插件 | ["rtsp", "hikvision", "dahua", "gb28181"] |
| logging.level | 日志级别 | info |
| logging.directory | 日志目录 | ./logs |
| devices.discovery_interval | 设备发现间隔(秒) | 60 |
| devices.status_check_interval | 状态检查间隔(秒) | 30 |

## 6. 数据库设计

### 6.1 数据库表结构

**t_video_cameras** - 摄像机表：
- id: 摄像机ID
- name: 摄像机名称
- model: 摄像机型号
- ip: IP地址
- port: 端口
- username: 用户名
- password: 密码
- stream_protocol_id: 流协议ID
- stream_protocol_param: 流协议参数
- control_protocol_id: 控制协议ID
- control_protocol_param: 控制协议参数
- partition_id: 分区ID
- status: 状态
- description: 描述
- create_time: 创建时间

**t_video_partitions** - 分区表：
- id: 分区ID
- name: 分区名称
- description: 分区描述
- create_time: 创建时间

**t_dict_stream_protocols** - 流协议字典表：
- id: 协议ID
- name: 协议名称
- description: 协议描述

**t_dict_control_protocols** - 控制协议字典表：
- id: 协议ID
- name: 协议名称
- description: 协议描述

### 6.2 数据库操作

- **摄像机管理**：添加、删除、更新、查询摄像机
- **分区管理**：添加、删除、更新、查询分区
- **协议管理**：查询支持的协议

## 7. 部署与集成方案

### 7.1 部署架构

**单机部署：**
- 单台服务器运行video-server
- 直接连接本地摄像机设备
- 提供流媒体服务

**分布式部署：**
- 多台服务器部署video-server
- 通过网络连接远程摄像机设备
- 负载均衡管理

### 7.2 集成方案

**与Web服务器集成：**
- 共享数据库：使用同一SQLite数据库
- 统一认证：共享用户认证系统
- 统一管理界面：Web界面管理流媒体服务

**与其他系统集成：**
- REST API：提供HTTP接口
- 消息队列：支持消息通知
- WebSocket：支持实时状态更新

## 8. 性能优化

### 8.1 内存优化

- 流缓存管理：合理设置缓存大小
- 连接池：复用数据库连接
- 内存分配：避免频繁内存分配和释放

### 8.2 CPU优化

- 线程池：合理设置线程数量
- 异步处理：非阻塞I/O
- 批量处理：批量数据库操作
- 缓存机制：减少重复计算

### 8.3 网络优化

- 连接复用：长连接减少连接建立开销
- 数据压缩：减少网络传输数据量
- 协议优化：选择高效协议
- 负载均衡：合理分配网络负载

## 9. 可靠性设计

### 9.1 错误处理

- 异常捕获：捕获并处理所有可能的异常
- 错误恢复：自动恢复故障
- 错误日志：详细的错误信息记录
- 错误通知：及时通知错误状态

### 9.2 故障转移

- 流备份：主备流切换
- 设备重连：自动重连故障设备
- 服务重启：自动重启故障服务
- 负载转移：故障时转移负载

### 9.3 监控与告警

- 状态监控：实时监控服务状态
- 性能监控：监控CPU、内存、网络等指标
- 告警机制：异常情况及时告警
- 日志分析：定期分析日志发现问题

## 10. 扩展性设计

### 10.1 插件扩展

- 协议插件：支持新的协议接入
- 设备插件：支持新的设备类型
- 功能插件：扩展新的功能
- 集成插件：与第三方系统集成

### 10.2 配置扩展

- 动态配置：运行时修改配置
- 配置模板：预设配置模板
- 配置验证：配置有效性检查
- 配置版本：配置版本管理

### 10.3 功能扩展

- 录制功能：支持流录制
- 截图功能：支持设备截图
- 智能分析：集成AI分析功能
- 多租户：支持多租户隔离

## 11. 安全设计

### 11.1 认证与授权

- 设备认证：摄像机设备认证
- 用户认证：访问控制认证
- 权限管理：细粒度权限控制
- 加密传输：HTTPS、WSS加密传输

### 11.2 防护措施

- 访问控制：IP白名单、黑名单
- 速率限制：防止DoS攻击
- 输入验证：防止注入攻击
- 日志审计：记录所有操作日志

## 12. 开发与维护

### 12.1 开发环境

- 操作系统：Linux
- 编译工具：CMake、GCC
- 开发语言：C++17
- 依赖库：ZLMediaKit、SQLite、lwlog、lwcomm

### 12.2 构建流程

1. 配置CMake：`cmake ..`
2. 编译项目：`make`
3. 构建插件：`make plugins`
4. 运行服务：`./video-server`

### 12.3 维护指南

- 日志管理：定期清理日志
- 配置管理：版本控制配置文件
- 插件管理：定期更新插件
- 性能监控：定期检查性能指标
- 安全更新：及时更新安全补丁

## 13. 总结

本流媒体服务器基于ZLMediaKit开发，实现了多协议接入、多协议输出、插件系统和高性能流处理能力。系统采用模块化设计，具有良好的扩展性和可维护性。通过插件系统，可以方便地扩展支持新的设备类型和协议。系统与现有Web服务器无缝集成，共享数据库和管理界面，提供了完整的流媒体解决方案。

### 13.1 技术优势

- 高性能：基于ZLMediaKit的高效流处理
- 灵活性：插件系统支持多种设备和协议
- 可靠性：完善的错误处理和故障恢复
- 扩展性：模块化设计，易于扩展
- 集成性：与现有系统无缝集成

### 13.2 应用场景

- 安防监控系统
- 视频会议系统
- 直播平台
- 视频点播系统
- 智能分析系统

本设计文档详细说明了流媒体服务器的架构、模块设计、插件系统、数据流、配置管理、数据库设计、部署方案、性能优化、可靠性设计、扩展性设计、安全设计和开发维护指南，为系统的开发、部署和维护提供了全面的参考。