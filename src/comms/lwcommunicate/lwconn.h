/*
* Copyright (c) 2026 ACOAUTO Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: lwconn.h .
*
* Date: 2026-02-05
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
*/

#ifndef LWCONN_H
#define LWCONN_H

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>

// 连接类型
enum class LWConnType {
    TCP_CLIENT,    // TCP客户端
    TCP_SERVER,    // TCP服务端
    UDP,           // UDP
    SERIAL         // 串口
};

// 连接状态
enum class LWConnStatus {
    DISCONNECTED,  // 断开连接
    CONNECTING,    // 连接中
    CONNECTED,     // 已连接
    ERROR          // 错误
};

// 错误码
enum class LWConnError {
    SUCCESS = 0,           // 成功
    INVALID_PARAM = -1,    // 参数无效
    CONNECTION_FAILED = -2, // 连接失败
    SEND_FAILED = -3,       // 发送失败
    RECEIVE_FAILED = -4,    // 接收失败
    TIMEOUT = -5,           // 超时
    NOT_CONNECTED = -6,     // 未连接
    INTERNAL_ERROR = -7     // 内部错误
};

// 连接事件回调类型
typedef std::function<void(bool connected, const std::string& extra_info)> LWConnEventHandler;

// 数据接收回调类型（仅用于异步操作）
typedef std::function<void(const char* data, size_t length, const std::string& extra_info)> LWConnDataHandler;

// 基础连接类
class LWConnBase {
public:
    LWConnBase(LWConnType type, const std::string& name);
    virtual ~LWConnBase();

    // 启动连接
    virtual LWConnError start() = 0;

    // 停止连接
    virtual void stop() = 0;

    // 发送数据（同步）
    virtual LWConnError send(const char* data, size_t length, int timeout_ms = 0) = 0;

    // 接收数据（同步）
    virtual LWConnError receive(char* buffer, size_t buffer_size, size_t& received_size, int timeout_ms = 0) = 0;

    // 发送数据到指定地址（仅用于UDP）
    virtual LWConnError sendTo(const std::string& address, const char* data, size_t length, int timeout_ms = 0);

    // 从指定地址接收数据（仅用于UDP）
    virtual LWConnError receiveFrom(std::string& address, char* buffer, size_t buffer_size, size_t& received_size, int timeout_ms = 0);

    // 断开连接
    virtual LWConnError disconnect() = 0;

    // 重新连接
    virtual LWConnError reconnect(int timeout_ms = 0) = 0;

    // 检查连接状态
    virtual bool isConnected() const = 0;

    // 获取连接状态
    LWConnStatus getStatus() const;

    // 获取连接类型
    LWConnType getType() const;

    // 获取连接名称
    const std::string& getName() const;

    // 设置事件回调
    void setEventHandler(LWConnEventHandler handler);

    // 清理接收缓冲区
    virtual void clearReceiveBuffer() = 0;

protected:
    // 通知连接状态变化
    void notifyConnChange(bool connected, const std::string& extra_info = "");

    // 更新连接状态
    void updateStatus(LWConnStatus status);

protected:
    LWConnType conn_type_;
    LWConnStatus conn_status_;
    std::string conn_name_;
    LWConnEventHandler event_handler_;
    mutable std::mutex status_mutex_;
};

// TCP客户端连接类
class LWTcpClient : public LWConnBase {
public:
    LWTcpClient(const std::string& name, const std::string& host, int port, int reconnect_interval = 5000);
    ~LWTcpClient() override;

    LWConnError start() override;
    void stop() override;
    LWConnError send(const char* data, size_t length, int timeout_ms = 0) override;
    LWConnError receive(char* buffer, size_t buffer_size, size_t& received_size, int timeout_ms = 0) override;
    LWConnError disconnect() override;
    LWConnError reconnect(int timeout_ms = 0) override;
    bool isConnected() const override;
    void clearReceiveBuffer() override;

private:
    std::string host_;
    int port_;
    int reconnect_interval_;
    void* socket_; // 内部socket指针
    std::vector<char> recv_buffer_;
    mutable std::mutex socket_mutex_;
};

// TCP服务端连接类
class LWTcpServer : public LWConnBase {
public:
    LWTcpServer(const std::string& name, const std::string& listen_address, int port);
    ~LWTcpServer() override;

    LWConnError start() override;
    void stop() override;
    LWConnError send(const char* data, size_t length, int timeout_ms = 0) override;
    LWConnError receive(char* buffer, size_t buffer_size, size_t& received_size, int timeout_ms = 0) override;
    LWConnError disconnect() override;
    LWConnError reconnect(int timeout_ms = 0) override;
    bool isConnected() const override;
    void clearReceiveBuffer() override;

    // 发送数据到指定客户端
    LWConnError sendToClient(const std::string& client_address, const char* data, size_t length, int timeout_ms = 0);

    // 获取连接的客户端列表
    std::vector<std::string> getConnectedClients() const;

private:
    std::string listen_address_;
    int port_;
    void* server_socket_; // 内部服务器socket指针
    std::map<std::string, void*> client_sockets_; // 客户端地址到socket的映射
    mutable std::mutex clients_mutex_;
};

// UDP连接类
class LWUdp : public LWConnBase {
public:
    LWUdp(const std::string& name, int local_port, const std::string& remote_address = "", int remote_port = 0);
    ~LWUdp() override;

    LWConnError start() override;
    void stop() override;
    LWConnError send(const char* data, size_t length, int timeout_ms = 0) override;
    LWConnError receive(char* buffer, size_t buffer_size, size_t& received_size, int timeout_ms = 0) override;
    LWConnError sendTo(const std::string& address, const char* data, size_t length, int timeout_ms = 0) override;
    LWConnError receiveFrom(std::string& address, char* buffer, size_t buffer_size, size_t& received_size, int timeout_ms = 0) override;
    LWConnError disconnect() override;
    LWConnError reconnect(int timeout_ms = 0) override;
    bool isConnected() const override;
    void clearReceiveBuffer() override;

private:
    int local_port_;
    std::string remote_address_;
    int remote_port_;
    void* socket_; // 内部socket指针
    mutable std::mutex socket_mutex_;
};

// 串口连接类
class LWSerial : public LWConnBase {
public:
    LWSerial(const std::string& name, const std::string& port, int baudrate, 
             char parity = 'N', int databits = 8, int stopbits = 1);
    ~LWSerial() override;

    LWConnError start() override;
    void stop() override;
    LWConnError send(const char* data, size_t length, int timeout_ms = 0) override;
    LWConnError receive(char* buffer, size_t buffer_size, size_t& received_size, int timeout_ms = 0) override;
    LWConnError disconnect() override;
    LWConnError reconnect(int timeout_ms = 0) override;
    bool isConnected() const override;
    void clearReceiveBuffer() override;

private:
    std::string port_;
    int baudrate_;
    char parity_;
    int databits_;
    int stopbits_;
    void* serial_handle_; // 内部串口句柄
    mutable std::mutex serial_mutex_;
};

// 连接池类
class LWConnPool {
public:
    static LWConnPool& instance();

    // 添加连接到池
    bool addConn(std::shared_ptr<LWConnBase> conn);

    // 从池获取连接
    std::shared_ptr<LWConnBase> getConn(const std::string& name);

    // 移除连接
    bool removeConn(const std::string& name);

    // 获取所有连接
    std::vector<std::shared_ptr<LWConnBase>> getAllConns() const;

    // 启动所有连接
    void startAll();

    // 停止所有连接
    void stopAll();

    // 清理池
    void clear();

private:
    LWConnPool();
    ~LWConnPool();

    std::map<std::string, std::shared_ptr<LWConnBase>> conn_map_;
    mutable std::mutex pool_mutex_;
};

// 连接管理器
class LWConnManager {
public:
    static LWConnManager& instance();

    // 初始化
    bool init();

    // 反初始化
    void uninit();

    // 创建TCP客户端连接
    std::shared_ptr<LWTcpClient> createTcpClient(const std::string& name, 
                                                const std::string& host, 
                                                int port, 
                                                int reconnect_interval = 5000);

    // 创建TCP服务端连接
    std::shared_ptr<LWTcpServer> createTcpServer(const std::string& name, 
                                                const std::string& listen_address, 
                                                int port);

    // 创建UDP连接
    std::shared_ptr<LWUdp> createUdp(const std::string& name, 
                                    int local_port, 
                                    const std::string& remote_address = "", 
                                    int remote_port = 0);

    // 创建串口连接
    std::shared_ptr<LWSerial> createSerial(const std::string& name, 
                                          const std::string& port, 
                                          int baudrate, 
                                          char parity = 'N', 
                                          int databits = 8, 
                                          int stopbits = 1);

    // 获取连接池
    LWConnPool& getConnPool();

private:
    LWConnManager();
    ~LWConnManager();

    bool initialized_;
    std::shared_ptr<LWConnPool> conn_pool_;
};

// 工具函数
std::string lwconnErrorToString(LWConnError error);
std::string lwconnStatusToString(LWConnStatus status);
std::string lwconnTypeToString(LWConnType type);

#endif // LWCONN_H
