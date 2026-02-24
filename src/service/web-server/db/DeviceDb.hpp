/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/src/db/DeviceDb.hpp
 * @Description: Device Database Client
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef DEVICE_DB_HPP
#define DEVICE_DB_HPP

#include "dto/DeviceDto.hpp"
#include "dto/DriverDto.hpp"
#include "oatpp-sqlite/orm.hpp"
#include "lwcomm/lwcomm.h"

#include VSOA_CODEGEN_BEGIN(DbClient) //<- Begin Codegen

/**
 * DeviceDb client definitions.
 */
class DeviceDb : public vsoa::orm::DbClient {
public:

  DeviceDb(const std::shared_ptr<vsoa::orm::Executor>& executor)
    : vsoa::orm::DbClient(executor)
  {

    vsoa::orm::SchemaMigration migration(executor);
    std::string database_migrations = LWComm::GetConfigPath();
    migration.addFile(1 /* start from version 1 */, database_migrations + LW_OS_DIR_SEPARATOR + "bas-business.sql");
    // TODO - Add more migrations here.
    migration.migrate(); // <-- run migrations. This guy will throw on error.

    auto version = executor->getSchemaVersion();
    VSOA_LOGD("DeviceDb", "Migration - OK. Version=%ld.", version);

  }

  // Device related queries
  QUERY(getDevices,
        "SELECT d.*, dr.name as driver_name FROM t_devices d "
        "LEFT JOIN t_drivers dr ON d.driver_id = dr.id "
        "ORDER BY d.id;")

  QUERY(getDevicesWithPagination,
        "SELECT d.*, dr.name as driver_name FROM t_devices d "
        "LEFT JOIN t_drivers dr ON d.driver_id = dr.id "
        "ORDER BY d.id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getDeviceById,
        "SELECT d.*, dr.name as driver_name FROM t_devices d "
        "LEFT JOIN t_drivers dr ON d.driver_id = dr.id "
        "WHERE d.id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(createDevice,
        "INSERT INTO t_devices "
        "(name, driver_id, conn_type, connparam, description, "
        "param1, param2, param3, param4) VALUES "
        "(:device.name, :device.driver_id, "
        ":device.conn_type, :device.connparam, "
        ":device.description, "
        ":device.param1, :device.param2, "
        ":device.param3, :device.param4);",
        PARAM(vsoa::Object<DeviceCreateDto>, device))

  QUERY(updateDevice,
        "UPDATE t_devices SET "
        "name = :device.name, "
        "driver_id = :device.driver_id, "
        "conn_type = :device.conn_type, "
        "connparam = :device.connparam, "
        "description = :device.description, "
        "param1 = :device.param1, "
        "param2 = :device.param2, "
        "param3 = :device.param3, "
        "param4 = :device.param4 "
        "WHERE id = :device.id;",
        PARAM(vsoa::Object<DeviceDto>, device))

  QUERY(deleteDevice,
        "DELETE FROM t_devices "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(getDeviceCount,
        "SELECT COUNT(*) as count FROM t_devices;")

  QUERY(getDevicesByDriverId,
        "SELECT d.*, dr.name as driver_name FROM t_devices d "
        "LEFT JOIN t_drivers dr ON d.driver_id = dr.id "
        "WHERE d.driver_id = :driver_id "
        "ORDER BY d.id;",
        PARAM(vsoa::UInt32, driver_id))

};

#include VSOA_CODEGEN_END(DbClient) //<- End Codegen

#endif /* DEVICE_DB_HPP */
