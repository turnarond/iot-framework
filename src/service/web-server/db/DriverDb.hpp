/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/db/DriverDb.hpp
 * @Description: Driver Database Client
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef DRIVER_DB_HPP
#define DRIVER_DB_HPP

#include "dto/DriverDto.hpp"
#include "oatpp-sqlite/orm.hpp"
#include "lwcomm/lwcomm.h"

#include VSOA_CODEGEN_BEGIN(DbClient) //<- Begin Codegen

/**
 * DriverDb client definitions.
 */
class DriverDb : public vsoa::orm::DbClient {
public:
      vsoa ::provider ::ResourceHandle<vsoa ::orm ::Connection> connection_ = getConnection();
public:

  DriverDb(const std::shared_ptr<vsoa::orm::Executor>& executor)
    : vsoa::orm::DbClient(executor)
  {

//     vsoa::orm::SchemaMigration migration(executor);
//     std::string database_migrations = LWComm::GetDataPath();
//     migration.addFile(1 /* start from version 1 */, database_migrations + LW_OS_DIR_SEPARATOR + "001_init.sql");
//     // TODO - Add more migrations here.
//     migration.migrate(); // <-- run migrations. This guy will throw on error.

//     auto version = executor->getSchemaVersion();
//     VSOA_LOGD("DriverDb", "Migration - OK. Version=%ld.", version);
    VSOA_LOGD("DriverDb", "exector is %p.", executor.get())
  }

  // Driver related queries
    const vsoa ::data ::share ::StringTemplate Z_QUERY_TEMPLATE_getDrivers =
        this->parseQueryTemplate("getDrivers",
                                "SELECT * FROM t_drivers "
                                "ORDER BY id;",
                                {}, false);

    std ::shared_ptr<vsoa ::orm ::QueryResult>
        getDrivers() 
    {
        std ::unordered_map<vsoa ::String, vsoa ::Void> __params;
        return this->execute(Z_QUERY_TEMPLATE_getDrivers, __params, connection_);
    }

    vsoa ::data ::share ::StringTemplate
        Z_QUERY_TEMPLATE_CREATOR_getDriversWithPagination() 
    {
        bool __prepare = false;
        vsoa ::orm ::Executor ::ParamsTypeMap map;
        map.insert({"offset", vsoa ::UInt32 ::Class ::getType()});
        map.insert({"limit", vsoa ::UInt32 ::Class ::getType()});
        return this->parseQueryTemplate("getDriversWithPagination",
                                        "SELECT * FROM t_drivers "
                                        "ORDER BY id "
                                        "LIMIT :limit OFFSET :offset;",
                                        map, __prepare);
    }

    const vsoa ::data ::share ::StringTemplate
        Z_QUERY_TEMPLATE_getDriversWithPagination =
            Z_QUERY_TEMPLATE_CREATOR_getDriversWithPagination();

    std ::shared_ptr<vsoa ::orm ::QueryResult> getDriversWithPagination(
        const vsoa ::UInt32 &offset, const vsoa ::UInt32 &limit)
    {
        std ::unordered_map<vsoa ::String, vsoa ::Void> __params;
        __params.insert({"offset", offset});
        __params.insert({"limit", limit});
        return this->execute(Z_QUERY_TEMPLATE_getDriversWithPagination, __params, connection_);
    }

    vsoa ::data ::share ::StringTemplate Z_QUERY_TEMPLATE_CREATOR_getDriverById() {
        bool __prepare = false;
        vsoa ::orm ::Executor ::ParamsTypeMap map;
        map.insert({"id", vsoa ::UInt32 ::Class ::getType()});
        return this->parseQueryTemplate("getDriverById",
                                        "SELECT * FROM t_drivers "
                                        "WHERE id = :id;",
                                        map, __prepare);
    }

    const vsoa ::data ::share ::StringTemplate Z_QUERY_TEMPLATE_getDriverById =
        Z_QUERY_TEMPLATE_CREATOR_getDriverById();

    std ::shared_ptr<vsoa ::orm ::QueryResult>
    getDriverById(const vsoa ::UInt32 &id) {
        std ::unordered_map<vsoa ::String, vsoa ::Void> __params;
        __params.insert({"id", id});
        return this->execute(Z_QUERY_TEMPLATE_getDriverById, __params, connection_);
    }


    vsoa ::data ::share ::StringTemplate Z_QUERY_TEMPLATE_CREATOR_createDriver() 
    {
        bool __prepare = false;
        vsoa ::orm ::Executor ::ParamsTypeMap map;
        map.insert({"driver", vsoa ::Object<DriverCreateDto>::Class ::getType()});
        map.insert({"create_time", vsoa ::UInt64 ::Class ::getType()});
        return this->parseQueryTemplate(
        "createDriver",
        "INSERT INTO t_drivers "
        "(name, type, version, description, "
        "param1_name, param1_desc, param2_name, "
        "param2_desc, param3_name, param3_desc, "
        "param4_name, param4_desc, create_time) VALUES "
        "(:driver.name, :driver.type, :driver.version, "
        ":driver.description, :driver.param1_name, "
        ":driver.param1_desc, :driver.param2_name, "
        ":driver.param2_desc, :driver.param3_name, "
        ":driver.param3_desc, :driver.param4_name, "
        ":driver.param4_desc, :create_time);",
        map, __prepare);
    }

    const vsoa ::data ::share ::StringTemplate Z_QUERY_TEMPLATE_createDriver =
        Z_QUERY_TEMPLATE_CREATOR_createDriver();

    std ::shared_ptr<vsoa ::orm ::QueryResult>
    createDriver(const vsoa ::Object<DriverCreateDto> &driver,
                const vsoa ::UInt64 &create_time) 
    {
        std ::unordered_map<vsoa ::String, vsoa ::Void> __params;
        __params.insert({"driver", driver});
        __params.insert({"create_time", create_time});
        return this->execute(Z_QUERY_TEMPLATE_createDriver, __params, connection_);
    }

//   QUERY(createDriver,
//         "INSERT INTO t_drivers "
//         "(name, type, version, description, "
//         "param1_name, param1_desc, param2_name, "
//         "param2_desc, param3_name, param3_desc, "
//         "param4_name, param4_desc, create_time) VALUES "
//         "(:driver.name, :driver.type, :driver.version, "
//         ":driver.description, :driver.param1_name, "
//         ":driver.param1_desc, :driver.param2_name, "
//         ":driver.param2_desc, :driver.param3_name, "
//         ":driver.param3_desc, :driver.param4_name, "
//         ":driver.param4_desc, :create_time);",
//       //   "SELECT last_insert_rowid() as id;",
//         PARAM(vsoa::Object<DriverCreateDto>, driver),
//         PARAM(vsoa::UInt64, create_time))

    vsoa ::data ::share ::StringTemplate Z_QUERY_TEMPLATE_CREATOR_updateDriver() {
        bool __prepare = false;
        vsoa ::orm ::Executor ::ParamsTypeMap map;
        map.insert({"driver", vsoa ::Object<DriverDto>::Class ::getType()});
        return this->parseQueryTemplate("updateDriver",
                                        "UPDATE t_drivers SET "
                                        "type = :driver.type, "
                                        "version = :driver.version, "
                                        "description = :driver.description, "
                                        "param1_name = :driver.param1_name, "
                                        "param1_desc = :driver.param1_desc, "
                                        "param2_name = :driver.param2_name, "
                                        "param2_desc = :driver.param2_desc, "
                                        "param3_name = :driver.param3_name, "
                                        "param3_desc = :driver.param3_desc, "
                                        "param4_name = :driver.param4_name, "
                                        "param4_desc = :driver.param4_desc "
                                        "WHERE id = :driver.id;",
                                        map, __prepare);
    }

    const vsoa ::data ::share ::StringTemplate Z_QUERY_TEMPLATE_updateDriver =
        Z_QUERY_TEMPLATE_CREATOR_updateDriver();

    std ::shared_ptr<vsoa ::orm ::QueryResult>
    updateDriver(const vsoa ::Object<DriverDto> &driver)
    {
        std ::unordered_map<vsoa ::String, vsoa ::Void> __params;
        __params.insert({"driver", driver});
        return this->execute(Z_QUERY_TEMPLATE_updateDriver, __params, connection_);
    }

    vsoa ::data ::share ::StringTemplate Z_QUERY_TEMPLATE_CREATOR_deleteDriver() 
    {
        bool __prepare = false;
        vsoa ::orm ::Executor ::ParamsTypeMap map;
        map.insert({"id", vsoa ::UInt32 ::Class ::getType()});
        return this->parseQueryTemplate("deleteDriver",
                                        "DELETE FROM t_drivers "
                                        "WHERE id = :id;",
                                        map, __prepare);
    }

    const vsoa ::data ::share ::StringTemplate Z_QUERY_TEMPLATE_deleteDriver = Z_QUERY_TEMPLATE_CREATOR_deleteDriver();

    std ::shared_ptr<vsoa ::orm ::QueryResult> deleteDriver(const vsoa ::UInt32 &id)
    {
        std ::unordered_map<vsoa ::String, vsoa ::Void> __params;
        __params.insert({"id", id});
        return this->execute(Z_QUERY_TEMPLATE_deleteDriver, __params, connection_);
    }

    const vsoa ::data ::share ::StringTemplate Z_QUERY_TEMPLATE_getDriverCount =
        this->parseQueryTemplate("getDriverCount",
                                "SELECT COUNT(*) as count FROM t_drivers;", {},
                                false);

    std ::shared_ptr<vsoa ::orm ::QueryResult> getDriverCount() {
        std ::unordered_map<vsoa ::String, vsoa ::Void> __params;
        return this->execute(Z_QUERY_TEMPLATE_getDriverCount, __params, connection_);
    }

};

#include VSOA_CODEGEN_END(DbClient) //<- End Codegen

#endif /* DRIVER_DB_HPP */