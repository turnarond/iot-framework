/*
 * @Author: yanchaodong
 * @Date: 2026-02-14 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-14 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/db/VideoDb.hpp
 * @Description: Video Database Client
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef VIDEO_DB_HPP
#define VIDEO_DB_HPP

#include "dto/VideoDTO.hpp"
#include "oatpp-sqlite/orm.hpp"
#include "lwcomm/lwcomm.h"

#include VSOA_CODEGEN_BEGIN(DbClient) //<- Begin Codegen

/**
 * VideoDb client definitions.
 */
class VideoDb : public vsoa::orm::DbClient {
public:

  VideoDb(const std::shared_ptr<vsoa::orm::Executor>& executor)
    : vsoa::orm::DbClient(executor)
  {

    vsoa::orm::SchemaMigration migration(executor);
    std::string database_migrations = LWComm::GetConfigPath();
    migration.addFile(1 /* start from version 1 */, database_migrations + LW_OS_DIR_SEPARATOR + "bas-business.sql");
    // TODO - Add more migrations here.
    migration.migrate(); // <-- run migrations. This guy will throw on error.

    auto version = executor->getSchemaVersion();
    VSOA_LOGD("VideoDb", "Migration - OK. Version=%ld.", version);

  }

  // Partition related queries
  QUERY(getPartitions, "SELECT * FROM t_video_partitions ORDER BY id;")

  QUERY(getPartitionsWithPagination,
        "SELECT * FROM t_video_partitions ORDER BY id LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getPartitionById,
        "SELECT * FROM t_video_partitions WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(createPartition,
        "INSERT INTO t_video_partitions (name, description, create_time) VALUES "
        "(:partition.name, :partition.description, :partition.create_time);",
        PARAM(vsoa::Object<VideoPartitionDTO>, partition))

  QUERY(updatePartition,
        "UPDATE t_video_partitions SET name = :partition.name, description = :partition.description "
        "WHERE id = :partition.id;",
        PARAM(vsoa::Object<VideoPartitionDTO>, partition))

  QUERY(deletePartition,
        "DELETE FROM t_video_partitions WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(getPartitionCount,
        "SELECT COUNT(*) as count FROM t_video_partitions;")

  QUERY(getPartitionWithCameraCount,
        "SELECT p.*, COUNT(c.id) as camera_count FROM t_video_partitions p LEFT JOIN t_video_cameras c "
        "ON p.id = c.partition_id GROUP BY p.id;")

  // Camera related queries
  QUERY(getCameras,
        "SELECT c.*, p.name as partition_name FROM t_video_cameras c LEFT JOIN t_video_partitions p "
        "ON c.partition_id = p.id ORDER BY c.id;")

  QUERY(getCamerasWithPagination,
        "SELECT c.*, p.name as partition_name FROM t_video_cameras c LEFT JOIN t_video_partitions p "
        "ON c.partition_id = p.id ORDER BY c.id LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit))

  QUERY(getCamerasByPartitionId,
        "SELECT c.*, p.name as partition_name "
        "FROM t_video_cameras c LEFT JOIN t_video_partitions p "
        "ON c.partition_id = p.id WHERE c.partition_id = :partition_id ORDER BY c.id "
        "LIMIT :limit OFFSET :offset;",
        PARAM(vsoa::UInt32, offset),
        PARAM(vsoa::UInt32, limit),
        PARAM(vsoa::UInt32, partition_id))

  QUERY(getCameraById,
        "SELECT c.*, p.name as partition_name FROM t_video_cameras c LEFT JOIN t_video_partitions p "
        "ON c.partition_id = p.id WHERE c.id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(createCamera,
        "INSERT INTO t_video_cameras "
        "(name, model, ip, port, username, password, stream_protocol_id, stream_protocol_param, "
        "control_protocol_id, control_protocol_param, partition_id, status, description, create_time) "
        "VALUES "
        "(:camera.name, :camera.model, :camera.ip, :camera.port, :camera.username, :camera.password, "
        ":camera.stream_protocol_id, :camera.stream_protocol_param, "
        ":camera.control_protocol_id, :camera.control_protocol_param, "
        ":camera.partition_id, 'offline', :camera.description, :camera.create_time);",
        PARAM(vsoa::Object<VideoCameraCreateDTO>, camera))

  QUERY(updateCamera,
        "UPDATE t_video_cameras SET "
        "name = :camera.name, model = :camera.model, ip = :camera.ip, port = :camera.port, "
        "username = :camera.username, password = :camera.password, "
        "stream_protocol_id = :camera.stream_protocol_id, stream_protocol_param = :camera.stream_protocol_param, "
        "control_protocol_id = :camera.control_protocol_id, control_protocol_param = :camera.control_protocol_param, "
        "partition_id = :camera.partition_id, status = :camera.status, description = :camera.description "
        "WHERE id = :camera.id;",
        PARAM(vsoa::Object<VideoCameraDTO>, camera))

  QUERY(deleteCamera,
        "DELETE FROM t_video_cameras WHERE id = :id;",
        PARAM(vsoa::UInt32, id))

  QUERY(getCameraCount,
        "SELECT COUNT(*) as count FROM t_video_cameras;")

  QUERY(getCameraCountByPartitionId,
        "SELECT COUNT(*) as count FROM t_video_cameras WHERE partition_id = :partition_id;",
        PARAM(vsoa::UInt32, partition_id))

  QUERY(updateCameraStatus,
        "UPDATE t_video_cameras SET status = :status WHERE id = :id;",
        PARAM(vsoa::String, status),
        PARAM(vsoa::UInt32, id))

};

#include VSOA_CODEGEN_END(DbClient) //<- End Codegen

#endif /* VIDEO_DB_HPP */