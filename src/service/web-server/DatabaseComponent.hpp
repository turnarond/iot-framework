/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath:
 * /acoinfo/edge-framework/src/service/web-server/src/DatabaseComponent.hpp
 * @Description: Database Component for Web Server
 *
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved.
 */

#ifndef DATABASE_COMPONENT_HPP
#define DATABASE_COMPONENT_HPP

#include "db/AlarmDb.hpp"
#include "db/DeviceDb.hpp"
#include "db/DictDb.hpp"
#include "db/DriverDb.hpp"
#include "db/LinkageDb.hpp"
#include "db/PointDb.hpp"
#include "db/UserDb.hpp"
#include "db/VideoDb.hpp"
#include "lwcomm/lwcomm.h"

#include "lwlog/lwlog.h"
#include "oatpp/core/macro/component.hpp"

#include "common/Logger.hpp"

/**
 *  Database Component
 */
class DatabaseComponent {
public:
    /**
    * Create database connection provider component
    */
    OATPP_CREATE_COMPONENT(std::shared_ptr<vsoa::provider::Provider<vsoa::sqlite::Connection>>, dbConnectionProvider)([] {
        /* Create database-specific ConnectionProvider */
        std::string datapath = LWComm::GetDataPath();
        std::string database_file =
            datapath + LW_OS_DIR_SEPARATOR + "bas-business.db";
        auto connectionProvider =
            std::make_shared<vsoa::sqlite::ConnectionProvider>(
                database_file);

        g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Create connection componment");
        /* Create database-specific ConnectionPool */
        return vsoa::sqlite::ConnectionPool::createShared(
            connectionProvider, 10 /* max-connections */,
            std::chrono::seconds(5) /* connection TTL */);
    }());

    /**
    * Create Driver database client
    */
    OATPP_CREATE_COMPONENT(std::shared_ptr<DriverDb>, driverDb)([] {
        /* Get database ConnectionProvider component */
        VSOA_COMPONENT(std::shared_ptr<vsoa::provider::Provider<vsoa::sqlite::Connection>>, connectionProvider);
        /* Create database-specific Executor */
        auto executor =
            std::make_shared<vsoa::sqlite::Executor>(connectionProvider);

        g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Create driver db exector %p", 
            executor.get());
        /* Create DriverDb database client */
        return std::make_shared<DriverDb>(executor);
    }());

    /**
     * Create Device database client
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<DeviceDb>, deviceDb)([] {
        /* Get database ConnectionProvider component */
        VSOA_COMPONENT(std::shared_ptr<vsoa::provider::Provider<vsoa::sqlite::Connection>>, connectionProvider);
        /* Create database-specific Executor */
        auto executor =
            std::make_shared<vsoa::sqlite::Executor>(connectionProvider);

        g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Create device db exector %p", 
            executor.get());
        /* Create DriverDb database client */
        return std::make_shared<DeviceDb>(executor);
    }());

    /**
    * Create Point database client
    */
    OATPP_CREATE_COMPONENT(std::shared_ptr<PointDb>, pointDb)([] {
        /* Get database ConnectionProvider component */
        VSOA_COMPONENT(std::shared_ptr<vsoa::provider::Provider<vsoa::sqlite::Connection>>, connectionProvider);
        /* Create database-specific Executor */
        auto executor =
            std::make_shared<vsoa::sqlite::Executor>(connectionProvider);

        /* Create DriverDb database client */
        return std::make_shared<PointDb>(executor);
    }());

    /**
    * Create Alarm database client
    */
    OATPP_CREATE_COMPONENT(std::shared_ptr<AlarmDb>, alarmDb)([] {
        /* Get database ConnectionProvider component */
        VSOA_COMPONENT(std::shared_ptr<vsoa::provider::Provider<vsoa::sqlite::Connection>>, connectionProvider);
        /* Create database-specific Executor */
        auto executor =
            std::make_shared<vsoa::sqlite::Executor>(connectionProvider);

        /* Create DriverDb database client */
        return std::make_shared<AlarmDb>(executor);
    }());

    /**
    * Create Linkage database client
    */
    OATPP_CREATE_COMPONENT(std::shared_ptr<LinkageDb>, linkageDb)([] {
        /* Get database ConnectionProvider component */
        VSOA_COMPONENT(std::shared_ptr<vsoa::provider::Provider<vsoa::sqlite::Connection>>, connectionProvider);
        /* Create database-specific Executor */
        auto executor =
            std::make_shared<vsoa::sqlite::Executor>(connectionProvider);

        /* Create DriverDb database client */
        return std::make_shared<LinkageDb>(executor);
    }());

    /**
    * Create Dict database client
    */
    OATPP_CREATE_COMPONENT(std::shared_ptr<DictDb>, dictDb)([] {
        /* Get database ConnectionProvider component */
        VSOA_COMPONENT(std::shared_ptr<vsoa::provider::Provider<vsoa::sqlite::Connection>>, connectionProvider);
        /* Create database-specific Executor */
        auto executor =
            std::make_shared<vsoa::sqlite::Executor>(connectionProvider);

        /* Create DriverDb database client */
        return std::make_shared<DictDb>(executor);
    }());

    /**
    * Create User database client
    */
    OATPP_CREATE_COMPONENT(std::shared_ptr<UserDb>, userDb)([] {
        /* Get database ConnectionProvider component */
        VSOA_COMPONENT(std::shared_ptr<vsoa::provider::Provider<vsoa::sqlite::Connection>>, connectionProvider);
        /* Create database-specific Executor */
        auto executor =
            std::make_shared<vsoa::sqlite::Executor>(connectionProvider);

        g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Create user db exector %p", 
            executor.get());
        /* Create UserDb database client */
        return std::make_shared<UserDb>(executor);
    }());

    /**
    * Create Video database client
    */
    OATPP_CREATE_COMPONENT(std::shared_ptr<VideoDb>, videoDb)([] {
        /* Get database ConnectionProvider component */
        VSOA_COMPONENT(std::shared_ptr<vsoa::provider::Provider<vsoa::sqlite::Connection>>, connectionProvider);
        /* Create database-specific Executor */
        auto executor =
            std::make_shared<vsoa::sqlite::Executor>(connectionProvider);

        g_logger.LogMessage(LW_LOGLEVEL_DEBUG, "Create video db exector %p", 
            executor.get());
        /* Create VideoDb database client */
        return std::make_shared<VideoDb>(executor);
    }());



//    /**
//     * Create database connection provider component
//     */
//     std::shared_ptr<vsoa::provider::Provider<vsoa::sqlite::Connection>>
//         dbConnectionProvider =
//             std::shared_ptr<vsoa::provider::Provider<vsoa::sqlite::Connection>>([] {
//                 /* Create database-specific ConnectionProvider */
//                 std::string datapath = LWComm::GetDataPath();
//                 std::string database_file =
//                     datapath + LW_OS_DIR_SEPARATOR + "bas-business.db";
//                 auto connectionProvider =
//                     std::make_shared<vsoa::sqlite::ConnectionProvider>(
//                         database_file);

//                 /* Create database-specific ConnectionPool */
//                 return vsoa::sqlite::ConnectionPool::createShared(
//                     connectionProvider, 10 /* max-connections */,
//                     std::chrono::seconds(1) /* connection TTL */);
//             }());

//     /**
//     * Create Driver database client
//     */
//     std::shared_ptr<DriverDb> driverDb =
//         std::shared_ptr<DriverDb>([&] {
//             /* Create database-specific Executor */
//             auto executor =
//                 std::make_shared<vsoa::sqlite::Executor>(dbConnectionProvider);

//             /* Create DriverDb database client */
//             return std::make_shared<DriverDb>(executor);
//         }());

//     /**
//     * Create Device database client
//     */
//     std ::shared_ptr<DeviceDb> deviceDb =
//         std ::shared_ptr<DeviceDb>([&] {

//             /* Create database-specific Executor */
//             auto executor =
//                 std::make_shared<vsoa::sqlite::Executor>(dbConnectionProvider);

//             /* Create DeviceDb database client */
//             return std::make_shared<DeviceDb>(executor);
//         }());

//     /**
//     * Create Point database client
//     */
//     std ::shared_ptr<PointDb> pointDb =
//         std ::shared_ptr<PointDb>([&] {
//             /* Create database-specific Executor */
//             auto executor =
//                 std::make_shared<vsoa::sqlite::Executor>(dbConnectionProvider);

//             /* Create PointDb database client */
//             return std::make_shared<PointDb>(executor);
//         }());

//     /**
//     * Create Alarm database client
//     */
//     std ::shared_ptr<AlarmDb> alarmDb =
//         std ::shared_ptr<AlarmDb>([&] {

//             /* Create database-specific Executor */
//             auto executor =
//                 std::make_shared<vsoa::sqlite::Executor>(dbConnectionProvider);

//             /* Create AlarmDb database client */
//             return std::make_shared<AlarmDb>(executor);
//         }());

//     /**
//     * Create Linkage database client
//     */
//     std ::shared_ptr<LinkageDb> linkageDb =
//         std ::shared_ptr<LinkageDb>([&] {

//             /* Create database-specific Executor */
//             auto executor =
//                 std::make_shared<vsoa::sqlite::Executor>(dbConnectionProvider);

//             /* Create LinkageDb database client */
//             return std::make_shared<LinkageDb>(executor);
//         }());

//     /**
//     * Create Dict database client
//     */
//     std ::shared_ptr<DictDb> dictDb =
//         std ::shared_ptr<DictDb>([&] {
        
//             /* Create database-specific Executor */
//             auto executor =
//                 std::make_shared<vsoa::sqlite::Executor>(dbConnectionProvider);

//             /* Create DictDb database client */
//             return std::make_shared<DictDb>(executor);
//         }());
};

#endif /* DATABASE_COMPONENT_HPP */
