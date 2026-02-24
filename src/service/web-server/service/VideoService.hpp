/*
 * @Author: yanchaodong
 * @Date: 2026-02-14 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-14 10:00:00
 * @FilePath:
 * /acoinfo/edge-framework/src/service/web-server/service/VideoService.hpp
 * @Description: Video Service
 *
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved.
 */

#ifndef VIDEO_SERVICE_HPP
#define VIDEO_SERVICE_HPP

#include "common/Logger.hpp"
#include "db/VideoDb.hpp"
#include "dto/StatusDto.hpp"
#include "dto/VideoDTO.hpp"

#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/protocol/http/Http.hpp"

#include <memory>
#include <vector>

/**
 * VideoService class.
 * Provides video-related services.
 */
class VideoService {
private:
    VSOA_COMPONENT(std::shared_ptr<VideoDb>, videoDb);

private:
  typedef vsoa::web::protocol::http::Status Status;
  VideoService() = default;
  ~VideoService() = default;

public:
  /**
   * Get instance of VideoService (singleton).
   * @return VideoService instance.
   */
  static VideoService &getInstance();

  /**
   * Initialize VideoService.
   * @param executor Database executor.
   */
  void initialize(const std::shared_ptr<vsoa::orm::Executor> &executor);

  /**
   * Get all partitions with pagination.
   * @param page Page number.
   * @param size Page size.
   * @return Paginated response of partitions.
   */
  vsoa::Object<VideoPartitionPageDTO> getPartitions(vsoa::UInt32 page, vsoa::UInt32 size);

  /**
   * Get partition by ID.
   * @param id Partition ID.
   * @return Partition DTO.
   */
  vsoa::Object<VideoPartitionDTO> getPartitionById(vsoa::UInt32 id);

  /**
   * Create partition.
   * @param partition Partition create DTO.
   * @return Created partition DTO.
   */
  vsoa::Object<VideoPartitionDTO>
  createPartition(vsoa::Object<VideoPartitionCreateDTO> partition);

  /**
   * Update partition.
   * @param partition Partition DTO.
   * @return Updated partition DTO.
   */
  vsoa::Object<VideoPartitionDTO>
  updatePartition(vsoa::Object<VideoPartitionDTO> partition);

  /**
   * Delete partition.
   * @param id Partition ID.
   * @return Status DTO.
   */
  vsoa::Object<StatusDto> deletePartition(vsoa::UInt32 id);

  /**
   * Get all cameras with pagination.
   * @param page Page number.
   * @param size Page size.
   * @param partitionId Partition ID (optional).
   * @return Paginated response of cameras.
   */
  vsoa::Object<VideoCameraPageDTO> getCameras(vsoa::UInt32 page, vsoa::UInt32 size, vsoa::UInt32 partitionId);

  /**
   * Get camera by ID.
   * @param id Camera ID.
   * @return Camera DTO.
   */
  vsoa::Object<VideoCameraDTO> getCameraById(vsoa::UInt32 id);

  /**
   * Create camera.
   * @param camera Camera create DTO.
   * @return Created camera DTO.
   */
  vsoa::Object<VideoCameraDTO>
  createCamera(vsoa::Object<VideoCameraCreateDTO> camera);

  /**
   * Update camera.
   * @param camera Camera DTO.
   * @return Updated camera DTO.
   */
  vsoa::Object<VideoCameraDTO>
  updateCamera(vsoa::Object<VideoCameraDTO> camera);

  /**
   * Delete camera.
   * @param id Camera ID.
   * @return Status DTO.
   */
  vsoa::Object<StatusDto> deleteCamera(vsoa::UInt32 id);

  /**
   * Control camera.
   * @param controlDto Camera control DTO.
   * @return Status DTO.
   */
  vsoa::Object<StatusDto>
  controlCamera(vsoa::Object<VideoCameraControlDTO> controlDto);

  /**
   * Update camera status.
   * @param id Camera ID.
   * @param status Camera status.
   * @return Status DTO.
   */
  vsoa::Object<StatusDto> updateCameraStatus(vsoa::UInt32 id,
                                             vsoa::String status);

private:
  // /**
  //  * Convert database row to partition DTO.
  //  * @param row Database row.
  //  * @return Partition DTO.
  //  */
  // vsoa::Object<VideoPartitionDTO> convertToPartitionDTO(const vsoa::orm::Row&
  // row);

  // /**
  //  * Convert database row to camera DTO.
  //  * @param row Database row.
  //  * @return Camera DTO.
  //  */
  // vsoa::Object<VideoCameraDTO> convertToCameraDTO(const vsoa::orm::Row& row);
};

#endif // VIDEO_SERVICE_HPP