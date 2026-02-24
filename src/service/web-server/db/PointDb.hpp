/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/src/db/PointDb.hpp
 * @Description: Point Database Client
 *
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved.
 */

#ifndef POINT_DB_HPP
#define POINT_DB_HPP

#include "dto/PointDto.hpp"
#include "lwcomm/lwcomm.h"
#include "oatpp-sqlite/orm.hpp"

#include VSOA_CODEGEN_BEGIN(DbClient) //<- Begin Codegen

/**
 * PointDb client definitions.
 */
class PointDb : public vsoa::orm::DbClient {
public:
  PointDb(const std::shared_ptr<vsoa::orm::Executor> &executor)
      : vsoa::orm::DbClient(executor) {

    //     vsoa::orm::SchemaMigration migration(executor);
    //     std::string database_migrations = LWComm::GetDataPath();
    //     migration.addFile(1 /* start from version 1 */, database_migrations +
    //     LW_OS_DIR_SEPARATOR + "001_init.sql");
    //     // TODO - Add more migrations here.
    //     migration.migrate(); // <-- run migrations. This guy will throw on
    //     error.

    //     auto version = executor->getSchemaVersion();
    //     VSOA_LOGD("PointDb", "Migration - OK. Version=%ld.", version);
  }

  // Point related queries
  QUERY(getPoints, "SELECT * FROM t_points "
                   "ORDER BY id;")

  QUERY(getPointsWithPagination,
        "SELECT * FROM t_points "
        "ORDER BY id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset), PARAM(vsoa::UInt32, limit))

  QUERY(getPointsWithDevice,
        "SELECT p.id, p.name, p.address, p.device_id, d.name as device_name, "
        "p.datatype as datatype_id, dt.cname as datatype_cname, p.point_type "
        "as point_type_id, pt.cname as point_type_cname, p.enable_control, "
        "p.enable_history, p.description "
        "FROM t_points p "
        "JOIN t_devices d ON p.device_id = d.id "
        "LEFT JOIN t_dict_data_types dt ON p.datatype = dt.id "
        "LEFT JOIN t_dict_point_types pt ON p.point_type = pt.id "
        "ORDER BY p.id;")

  /**
   * 获取所有点位（包含设备信息），分页查询
   * @param offset 偏移量
   * @param limit 每页数量
   * @return 点位列表
   */
  QUERY(getPointsWithDeviceWithPagination,
        "SELECT p.id, p.name, p.address, p.device_id, d.name as device_name, "
        "p.datatype as datatype_id, dt.cname as datatype_cname, p.point_type "
        "as point_type_id, pt.cname as point_type_cname, p.enable_control, "
        "p.enable_history, p.description "
        "FROM t_points p "
        "JOIN t_devices d ON p.device_id = d.id "
        "LEFT JOIN t_dict_data_types dt ON p.datatype = dt.id "
        "LEFT JOIN t_dict_point_types pt ON p.point_type = pt.id "
        "ORDER BY p.id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset), PARAM(vsoa::UInt32, limit))

  QUERY(getPointById,
        "SELECT * FROM t_points "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(getPointWithDeviceById,
        "SELECT p.id, p.name, p.address, p.device_id, d.name as device_name, "
        "p.datatype as datatype_id, dt.cname as datatype_cname, p.point_type "
        "as point_type_id, pt.cname as point_type_cname, p.enable_control, "
        "p.enable_history, p.description "
        "FROM t_points p "
        "JOIN t_devices d ON p.device_id = d.id "
        "LEFT JOIN t_dict_data_types dt ON p.datatype = dt.id "
        "LEFT JOIN t_dict_point_types pt ON p.point_type = pt.id "
        "WHERE p.id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(createPoint,
        "INSERT INTO t_points "
        "(name, address, device_id, datatype, point_type, transfer_type, "
        "linear_raw_min, linear_raw_max, linear_eng_min, linear_eng_max, "
        "advanced_algo_lib, advanced_param1, advanced_param2, advanced_param3, "
        "advanced_param4, enable_control, enable_history, poll_rate, "
        "description) VALUES "
        "(:point.name, :point.address, :point.device_id, :point.datatype, "
        ":point.point_type, :point.transfer_type, :point.linear_raw_min, "
        ":point.linear_raw_max, :point.linear_eng_min, :point.linear_eng_max, "
        ":point.advanced_algo_lib, :point.advanced_param1, "
        ":point.advanced_param2, :point.advanced_param3, "
        ":point.advanced_param4, :point.enable_control, :point.enable_history, "
        ":point.poll_rate, :point.description);",
        PARAM(vsoa::Object<PointCreateDto>, point))

  QUERY(updatePoint,
        "UPDATE t_points SET "
        "name = :point.name, "
        "address = :point.address, "
        "device_id = :point.device_id, "
        "datatype = :point.datatype, "
        "point_type = :point.point_type, "
        "transfer_type = :point.transfer_type, "
        "linear_raw_min = :point.linear_raw_min, "
        "linear_raw_max = :point.linear_raw_max, "
        "linear_eng_min = :point.linear_eng_min, "
        "linear_eng_max = :point.linear_eng_max, "
        "advanced_algo_lib = :point.advanced_algo_lib, "
        "advanced_param1 = :point.advanced_param1, "
        "advanced_param2 = :point.advanced_param2, "
        "advanced_param3 = :point.advanced_param3, "
        "advanced_param4 = :point.advanced_param4, "
        "enable_control = :point.enable_control, "
        "enable_history = :point.enable_history, "
        "poll_rate = :point.poll_rate, "
        "description = :point.description "
        "WHERE id = :point.id;",
        PARAM(vsoa::Object<PointDto>, point))

  QUERY(deletePoint,
        "DELETE FROM t_points "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(getPointCount,
        "SELECT COUNT(*) as count FROM t_points;")

  QUERY(getPointsByDeviceId,
        "SELECT * FROM t_points "
        "WHERE device_id = :device_id "
        "ORDER BY id;",
        PARAM(vsoa::UInt32, device_id))

  // Point value related queries
  QUERY(getPointValues, "SELECT * FROM t_point_value "
                        "ORDER BY point_id, ts DESC;")

  QUERY(getPointValue,
        "SELECT * FROM t_point_value "
        "WHERE point_id = :point_id "
        "ORDER BY ts DESC LIMIT 1;",
        PARAM(vsoa::UInt32, point_id))

  QUERY(getPointValueById,
        "SELECT * FROM t_point_value "
        "WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(setPointValue,
        "INSERT INTO t_point_value "
        "(point_id, value, ts) VALUES "
        "(:point_id, :value, CURRENT_TIMESTAMP);",
        PARAM(vsoa::UInt32, point_id), PARAM(vsoa::Float64, value))
};

#include VSOA_CODEGEN_END(DbClient) //<- End Codegen
#endif /* POINT_DB_HPP */
