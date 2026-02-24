/*
 * @Author: yanchaodong
 * @Date: 2026-02-09 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-09 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/nodeserver/hmi_server.cpp
 * @Description: HMI服务器实现，用于启动和配置web服务器，注册HMI点位控制器
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "hmi_server.hpp"

#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
// #include "oatpp-swagger/Controller.hpp"
#include "controller/HmiPointController.hpp"

#include <lwlog/lwlog.h>

// 初始化静态实例
HmiServer HmiServer::instance;

extern CLWLog g_logger;

/**
 * 获取单例实例
 * @return HmiServer实例
 */
HmiServer* HmiServer::getInstance() 
{
    return &instance;
}

/**
 * 构造函数
 */
HmiServer::HmiServer()
{
    // 初始化对象映射器
    auto serializerConfig = vsoa::parser::json::mapping::Serializer::Config::createShared();
    serializerConfig->includeNullFields = false;
    
    auto deserializerConfig = vsoa::parser::json::mapping::Deserializer::Config::createShared();
    deserializerConfig->allowUnknownFields = false;

    objectMapper = vsoa::parser::json::mapping::ObjectMapper::createShared(serializerConfig, deserializerConfig);
}

/**
 * 析构函数
 */
HmiServer::~HmiServer()
{
    stop();
}

/**
 * 启动HMI服务器
 * @param port 服务器端口
 * @return 启动是否成功
 */
bool HmiServer::start(uint16_t port) 
{
    try
    {
        // 创建路由器
        VSOA_COMPONENT(std::shared_ptr<vsoa::web::server::HttpRouter>, router);

        vsoa::web::server::api::Endpoints docEndpoints;

        // 创建控制器并注册路由
        docEndpoints.append(router->addController(HmiPointController::createShared())->getEndpoints());
        
        // 注册swagger
        // router->addController(vsoa::swagger::Controller::createShared(docEndpoints));
        
        // 创建连接处理器
        VSOA_COMPONENT(std::shared_ptr<vsoa::network::ConnectionHandler>, connectionHandler);
        
        // 创建网络连接提供者
        VSOA_COMPONENT(std::shared_ptr<vsoa::network::ServerConnectionProvider>, connectionProvider);
        
        // 创建服务器
        server = vsoa::network::Server::createShared(connectionProvider, connectionHandler);
        
        // 在单独的线程中启动服务器
        serverThread = std::thread(
            [this]
            {
                server->run();
            }
        );

        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[HmiServer] HMI server started on port %u", 
            port);
        return true;
    }
    catch (const std::exception& e)
    {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[HmiServer] Failed to start HMI server: %s", 
            e.what());
        return false;
    }
}

/**
 * 停止HMI服务器
 */
void HmiServer::stop() 
{
    if (server)
    {
        server->stop();
        if (serverThread.joinable())
        {
            serverThread.join();
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[HmiServer] HMI server stopped");
    }
}

/**
 * 停止并重置单例实例
 */
void HmiServer::stopInstance() 
{
    instance.stop();
}

/**
 * 运行HMI服务器（阻塞）
 */
void HmiServer::run() 
{
    if (server)
    {
        server->run();
    }
}

/**
 * 启动HMI服务器
 * @param port 服务器端口
 * @return 启动是否成功
 */
bool StartHmiServer(uint16_t port) 
{
    return HmiServer::getInstance()->start(port);
}

/**
 * 停止HMI服务器
 */
void StopHmiServer() 
{
    HmiServer::stopInstance();
}

/**
 * 运行HMI服务器（阻塞）
 */
void RunHmiServer() 
{
    auto server = HmiServer::getInstance();
    if (server)
    {
        server->run();
    }
}
