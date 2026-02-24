/*
 * @Author: yanchaodong
 * @Date: 2026-02-14 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-14 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/controller/VideoController.hpp
 * @Description: 视频监控控制器，用于处理视频分区和摄像头相关的API请求
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef VIDEO_CONTROLLER_HPP
#define VIDEO_CONTROLLER_HPP

#include "dto/StatusDto.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "vsoa_dto/parser/json/mapping/ObjectMapper.hpp"
#include "vsoa_dto/core/macro/codegen.hpp"

#include "dto/VideoDTO.hpp"
#include "service/VideoService.hpp"
#include "common/Logger.hpp"

#include VSOA_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

/**
 * 视频监控控制器
 * 处理视频分区和摄像头相关的API请求，包括CRUD操作和摄像头控制
 */
class VideoController : public vsoa::web::server::api::ApiController {
private:
    // 使用单例模式获取视频服务实例
    VideoService& getVideoService() { return VideoService::getInstance(); }

public:
    /**
     * 构造函数
     * @param objectMapper 对象映射器，用于JSON序列化和反序列化
     */
    VideoController(VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : vsoa::web::server::api::ApiController(objectMapper)
    {
    }

public:
    /**
     * 创建控制器实例
     * @param objectMapper 对象映射器
     * @return 控制器实例的智能指针
     */
    static std::shared_ptr<VideoController> createShared(
        VSOA_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    {
        return std::make_shared<VideoController>(objectMapper);
    }

    // 分区相关端点
    /**
     * 获取所有视频分区
     * @return 分区列表
     */
    ENDPOINT_INFO(getPartitions)
    {
        info->summary = "获取所有视频分区";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->addResponse<vsoa::Object<VideoPartitionPageDTO>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/video-partitions", getPartitions, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] GET /api/video-partitions called with page=%u, size=%u",
            page.getValue(0), size.getValue(0));
        auto response = getVideoService().getPartitions(page, size);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] GET /api/video-partitions returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 根据ID获取分区
     * @param id 分区ID
     * @return 分区信息
     */
    ENDPOINT_INFO(getPartitionById)
    {
        info->summary = "根据ID获取分区";
        info->pathParams.add<vsoa::UInt32>("id").description = "分区ID";
        info->pathParams["id"].required = "true";   
        info->addResponse<vsoa::Object<VideoPartitionDTO>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/video-partitions/{id}", getPartitionById, 
        PATH(vsoa::UInt32, id))
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] GET /api/video-partitions/{id} called with id=%u", 
            id.getValue(0));
        auto partition = getVideoService().getPartitionById(id);
        if (!partition)
        {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[VideoController] Partition not found with id=%u", 
                id.getValue(0));
            return createResponse(Status::CODE_404, "分区不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] GET /api/video-partitions/{id} returned successfully for id=%u", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_200, partition);
    }

    /**
     * 创建新分区
     * @param partitionDto 分区创建数据
     * @return 创建的分区信息
     */
    ENDPOINT_INFO(createPartition)
    {
        info->summary = "创建新分区";
        info->addConsumes<vsoa::Object<VideoPartitionCreateDTO>>("application/json");
        info->addResponse<vsoa::Object<VideoPartitionDTO>>(Status::CODE_201, "application/json");
        info->addResponse<String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/video-partitions", createPartition, BODY_DTO(vsoa::Object<VideoPartitionCreateDTO>, partitionDto))
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] POST /api/video-partitions called to create partition");
        auto partition = getVideoService().createPartition(partitionDto);
        if (!partition)
        {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[VideoController] Failed to create partition");
            return createResponse(Status::CODE_400, "创建分区失败");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] Partition created successfully");
        return createDtoResponse(Status::CODE_201, partition);
    }

    /**
     * 更新分区
     * @param id 分区ID
     * @param partitionDto 分区更新数据
     * @return 更新后的分区信息
     */
    ENDPOINT_INFO(updatePartition)
    {
        info->summary = "更新分区";
        info->addConsumes<vsoa::Object<VideoPartitionDTO>>("application/json");
        info->addResponse<vsoa::Object<VideoPartitionDTO>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("PUT", "/api/video-partitions/", updatePartition, 
        BODY_DTO(vsoa::Object<VideoPartitionDTO>, partitionDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] PUT /api/video-partitions/ called to update partition");
        auto partition = getVideoService().updatePartition(partitionDto);
        if (!partition)
        {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[VideoController] Partition not found for update");
            return createResponse(Status::CODE_404, "分区不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] Partition updated successfully");
        return createDtoResponse(Status::CODE_200, partition);
    }

    /**
     * 删除分区
     * @param id 分区ID
     * @return 删除结果
     */
    ENDPOINT_INFO(deletePartition)
    {
        info->summary = "删除分区";
        info->pathParams.add<vsoa::UInt32>("id").description = "分区ID";
        info->pathParams["id"].required = "true";   
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_204, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("DELETE", "/api/video-partitions/{id}", deletePartition, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] DELETE /api/video-partitions/{id} called with id=%u", 
            id.getValue(0));
        auto status = getVideoService().deletePartition(id);
        if (status->code != 200)
        {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[VideoController] Failed to delete partition with id=%u: %s", 
                id.getValue(0), status->message->c_str());
            return createResponse(Status::CODE_404, status->message);
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] Partition deleted successfully with id=%u", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_204, status);
    }

    // 摄像头相关端点
    /**
     * 获取所有摄像头（带分区）
     * @param page 页码
     * @param size 每页数量
     * @param partitionId 分区ID（可选）
     * @return 摄像头列表
     */
    ENDPOINT_INFO(getCamerasWithPartition)
    {
        info->summary = "获取所有摄像头（带分区）";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->queryParams.add<vsoa::UInt32>("partitionId").description = "分区ID";
        info->queryParams["partitionId"].required = false;
        info->addResponse<vsoa::Object<VideoCameraPageDTO>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/video-cameras", getCamerasWithPartition, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size),
        QUERY(vsoa::UInt32, partitionId, "partitionId", false)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] GET /api/video-cameras called with page=%u, size=%u, partitionId=%u",
            page.getValue(0), size.getValue(0), partitionId.getValue(0));
        auto response = getVideoService().getCameras(page, size, partitionId);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] GET /api/video-cameras returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }
    
    ENDPOINT_INFO(getCamerasWithoutPartition)
    {
        info->summary = "获取所有摄像头（无分区）";
        info->queryParams.add<vsoa::UInt32>("page").description = "页码";
        info->queryParams.add<vsoa::UInt32>("size").description = "每页数量";
        info->addResponse<vsoa::Object<VideoCameraPageDTO>>(Status::CODE_200, "application/json");
    }
    ENDPOINT("GET", "/api/video-cameras", getCamerasWithoutPartition, 
        QUERY(vsoa::UInt32, page),
        QUERY(vsoa::UInt32, size)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] GET /api/video-cameras called with page=%u, size=%u (no partitionId)",
            page.getValue(0), size.getValue(0));
        auto response = getVideoService().getCameras(page, size, page);
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] GET /api/video-cameras returned successfully");
        return createDtoResponse(Status::CODE_200, response);
    }

    /**
     * 根据ID获取摄像头
     * @param id 摄像头ID
     * @return 摄像头信息
     */
    ENDPOINT_INFO(getCameraById)
    {
        info->summary = "根据ID获取摄像头";
        info->pathParams.add<vsoa::UInt32>("id").description = "摄像头ID";
        info->pathParams["id"].required = "true";   
        info->addResponse<vsoa::Object<VideoCameraDTO>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("GET", "/api/video-cameras/{id}", getCameraById, 
        PATH(vsoa::UInt32, id))
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] GET /api/video-cameras/{id} called with id=%u", 
            id.getValue(0));
        auto camera = getVideoService().getCameraById(id);
        if (!camera)
        {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[VideoController] Camera not found with id=%u", 
                id.getValue(0));
            return createResponse(Status::CODE_404, "摄像头不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] GET /api/video-cameras/{id} returned successfully for id=%u", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_200, camera);
    }

    /**
     * 创建新摄像头
     * @param cameraDto 摄像头创建数据
     * @return 创建的摄像头信息
     */
    ENDPOINT_INFO(createCamera) 
    {
        info->summary = "创建新摄像头";
        info->addConsumes<vsoa::Object<VideoCameraCreateDTO>>("application/json");
        info->addResponse<vsoa::Object<VideoCameraDTO>>(Status::CODE_201, "application/json");
        info->addResponse<String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/video-cameras", createCamera, 
        BODY_DTO(vsoa::Object<VideoCameraCreateDTO>, cameraDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] POST /api/video-cameras called to create camera");
        auto camera = getVideoService().createCamera(cameraDto);
        if (!camera)
        {
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[VideoController] Failed to create camera");
            return createResponse(Status::CODE_400, "创建摄像头失败");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] Camera created successfully");
        return createDtoResponse(Status::CODE_201, camera);
    }

    /**
     * 更新摄像头
     * @param cameraDto 摄像头更新数据
     * @return 更新后的摄像头信息
     */
    ENDPOINT_INFO(updateCamera)
    {
        info->summary = "更新摄像头";
        info->addConsumes<vsoa::Object<VideoCameraDTO>>("application/json");
        info->addResponse<vsoa::Object<VideoCameraDTO>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("PUT", "/api/video-cameras/", updateCamera, 
        BODY_DTO(vsoa::Object<VideoCameraDTO>, cameraDto)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] PUT /api/video-cameras/ called to update camera");
        auto camera = getVideoService().updateCamera(cameraDto);
        if (!camera)
        {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[VideoController] Camera not found for update");
            return createResponse(Status::CODE_404, "摄像头不存在");
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] Camera updated successfully");
        return createDtoResponse(Status::CODE_200, camera);
    }

    /**
     * 删除摄像头
     * @param id 摄像头ID
     * @return 删除结果
     */
    ENDPOINT_INFO(deleteCamera)
    {
        info->summary = "删除摄像头";
        info->pathParams.add<vsoa::UInt32>("id").description = "摄像头ID";
        info->pathParams["id"].required = "true";   
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_204, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT("DELETE", "/api/video-cameras/{id}", deleteCamera, 
        PATH(vsoa::UInt32, id)) 
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] DELETE /api/video-cameras/{id} called with id=%u", 
            id.getValue(0));
        auto status = getVideoService().deleteCamera(id);
        if (status->code != 200)
        {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[VideoController] Failed to delete camera with id=%u: %s", 
                id.getValue(0), status->message->c_str());
            return createResponse(Status::CODE_404, status->message);
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] Camera deleted successfully with id=%u", 
            id.getValue(0));
        return createDtoResponse(Status::CODE_204, status);
    }

    /**
     * 控制摄像头
     * @param controlDto 控制数据
     * @return 控制结果
     */
    ENDPOINT_INFO(controlCamera)
    {
        info->summary = "控制摄像头";
        info->addConsumes<vsoa::Object<VideoCameraControlDTO>>("application/json");
        info->addResponse<vsoa::Object<StatusDto>>(Status::CODE_200, "application/json");
        info->addResponse<vsoa::String>(Status::CODE_404, "text/plain");
        info->addResponse<vsoa::String>(Status::CODE_400, "text/plain");
    }
    ENDPOINT("POST", "/api/video-cameras/control", controlCamera, BODY_DTO(vsoa::Object<VideoCameraControlDTO>, controlDto))
    {
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] POST /api/video-cameras/control called with camera_id=%u, action=%s",
            controlDto->camera_id.getValue(0), controlDto->action->c_str());
        auto status = getVideoService().controlCamera(controlDto);
        if (status->code != 200)
        {
            g_logger.LogMessage(LW_LOGLEVEL_WARN, "[VideoController] Failed to control camera with id=%u: %s", 
                controlDto->camera_id.getValue(0), status->message->c_str());
            return createResponse(Status::CODE_400, status->message);
        }
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "[VideoController] Camera controlled successfully with id=%u", 
            controlDto->camera_id.getValue(0));
        return createDtoResponse(Status::CODE_200, status);
    }
};

#include VSOA_CODEGEN_END(ApiController) //<- End Codegen

#endif // VIDEO_CONTROLLER_HPP