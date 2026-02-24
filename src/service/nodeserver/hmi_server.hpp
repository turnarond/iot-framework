/*
 * @Author: yanchaodong
 * @Date: 2026-02-09 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-09 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/nodeserver/hmi_server.hpp
 * @Description: HMI服务器头文件，实现单例模式
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef HMI_SERVER_HPP
#define HMI_SERVER_HPP

#include "AppComponent.hpp"
#include "oatpp/network/Server.hpp"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include <memory>
#include <thread>

/**
 * HMI服务器类
 * 用于启动和配置web服务器，注册HMI点位控制器
 * 实现单例模式
 */
class HmiServer {
private:
    std::shared_ptr<vsoa::network::Server> server;
    std::shared_ptr<vsoa::parser::json::mapping::ObjectMapper> objectMapper;
    std::thread serverThread;
    
    AppComponent components; // 创建应用组件
    
    // 单例实例
    static HmiServer instance;
    
    // 私有化构造函数
    HmiServer();
    
    // 私有化析构函数
    ~HmiServer();
    
    // 防止拷贝和赋值
    HmiServer(const HmiServer&) = delete;
    HmiServer& operator=(const HmiServer&) = delete;

public:
    /**
     * 获取单例实例
     * @return HmiServer实例
     */
    static HmiServer* getInstance();
    
    /**
     * 启动HMI服务器
     * @param port 服务器端口
     * @return 启动是否成功
     */
    bool start(uint16_t port = 8080);
    
    /**
     * 停止HMI服务器
     */
    void stop();
    
    /**
     * 运行HMI服务器（阻塞）
     */
    void run();
    
    /**
     * 停止并重置单例实例
     */
    static void stopInstance();
};

/**
 * 启动HMI服务器
 * @param port 服务器端口
 * @return 启动是否成功
 */
bool StartHmiServer(uint16_t port);

/**
 * 停止HMI服务器
 */
void StopHmiServer();

/**
 * 运行HMI服务器（阻塞）
 */
void RunHmiServer();

#endif // HMI_SERVER_HPP
