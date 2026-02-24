/*
 * @Author: yanchaodong
 * @Date: 2026-02-14 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-14 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/service/VideoService.cpp
 * @Description: Video Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "VideoService.hpp"
#include "dto/StatusDto.hpp"
#include "vsoa_dto/core/Types.hpp"
#include "oatpp-sqlite/Utils.hpp"
#include <chrono>

// 单例模式实现
VideoService& VideoService::getInstance()
{
    static VideoService instance;
    return instance;
}

// 初始化视频服务
void VideoService::initialize(const std::shared_ptr<vsoa::orm::Executor>& executor)
{
    
}

// 获取分区列表
vsoa::Object<VideoPartitionPageDTO> VideoService::getPartitions(vsoa::UInt32 page, vsoa::UInt32 size) {
    // 获取总页数
    auto dbResult = videoDb->getPartitionCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto count = totalCounts["count"];

    vsoa::UInt32 pages = (count + size - 1) / size;
    if (page > pages) {
        page = pages;
    }

    // 计算偏移量
    vsoa::UInt32 offset = (page - 1) * size;
    vsoa::UInt32 limit = size;
    
    // 获取分区列表
    dbResult = videoDb->getPartitionsWithPagination(offset, limit);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    // 解析结果
    auto partitions = dbResult->fetch<vsoa::Vector<vsoa::Object<VideoPartitionDTO>>>();
    
    // 创建分页对象
    auto response = VideoPartitionPageDTO::createShared();
    response->items = partitions;
    response->counts = partitions->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    
    return response;
}

// 根据ID获取分区
vsoa::Object<VideoPartitionDTO> VideoService::getPartitionById(vsoa::UInt32 id)
{
    auto dbResult = videoDb->getPartitionById(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto partitions = dbResult->fetch<vsoa::Vector<vsoa::Object<VideoPartitionDTO>>>();
    if (partitions->empty())
    {
        return nullptr;
    }
    
    return partitions->at(0);
}

// 创建分区
vsoa::Object<VideoPartitionDTO> VideoService::createPartition(vsoa::Object<VideoPartitionCreateDTO> partitionCreateDto)
{
    // 创建分区DTO
    auto partitionDto = vsoa::Object<VideoPartitionDTO>::createShared();
    partitionDto->name = partitionCreateDto->name;
    partitionDto->description = partitionCreateDto->description;
    partitionDto->create_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    // 插入分区
    auto dbResult = videoDb->createPartition(partitionDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    // 获取新插入的分区ID
    auto newId = vsoa::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
    
    // 重新获取分区信息
    auto newPartition = getPartitionById((vsoa::UInt32)newId);
    return newPartition;
}

// 更新分区
vsoa::Object<VideoPartitionDTO> VideoService::updatePartition(vsoa::Object<VideoPartitionDTO> partitionDto)
{
    // 更新分区
    auto dbResult = videoDb->updatePartition(partitionDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    return getPartitionById(partitionDto->id);
}

// 删除分区
vsoa::Object<StatusDto> VideoService::deletePartition(vsoa::UInt32 id)
{
    auto status = vsoa::Object<StatusDto>::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // 检查分区是否存在
    auto partition = getPartitionById(id);
    if (!partition)
    {
        status->code = 404;
        status->message = "分区不存在";
        return status;
    }

    // 检查分区下是否有摄像头
    auto dbResult = videoDb->getCameraCountByPartitionId(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto cameraCount = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0]["count"];
    if (cameraCount > 0)
    {
        status->code = 400;
        status->message = "分区下存在摄像头，无法删除";
        return status;
    }

    // 删除分区
    dbResult = videoDb->deletePartition(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    status->code = 200;
    status->message = "删除成功";
    return status;
}

// 获取摄像头列表
vsoa::Object<VideoCameraPageDTO> VideoService::getCameras(vsoa::UInt32 page, vsoa::UInt32 size, vsoa::UInt32 partitionId)
{
    auto response = VideoCameraPageDTO::createShared();
    // 直接返回所有摄像头，忽略partitionId参数
    // 获取总页数
    auto dbResult = videoDb->getCameraCount();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto totalCounts = dbResult->fetch<vsoa::Vector<vsoa::Fields<vsoa::UInt32>>>()[0];
    OATPP_ASSERT_HTTP(totalCounts["count"] >= 0, Status::CODE_500, "Unknown error");
    auto pages = (totalCounts["count"] + size - 1) / size;

    if (page > pages)
    {
        page = pages;
    }
    // 计算偏移量
    vsoa::UInt32 offset = (page - 1) * size;
    dbResult = videoDb->getCamerasWithPagination(offset, size);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto cameras = dbResult->fetch<vsoa::Vector<vsoa::Object<VideoCameraDTO>>>();
    response->counts = cameras->size();
    response->page = page;
    response->size = size;
    response->pages = pages;
    response->items = cameras;
    return response;
}

// 根据ID获取摄像头
vsoa::Object<VideoCameraDTO> VideoService::getCameraById(vsoa::UInt32 id)
{
    auto dbResult = videoDb->getCameraById(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto cameras = dbResult->fetch<vsoa::Vector<vsoa::Object<VideoCameraDTO>>>();
    if (cameras->empty())
    {
        return nullptr;
    }
    
    return cameras->at(0);
}

// 创建摄像头
vsoa::Object<VideoCameraDTO> VideoService::createCamera(vsoa::Object<VideoCameraCreateDTO> cameraCreateDto)
{
    // 创建摄像头DTO
    cameraCreateDto->create_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    // 插入摄像头
    auto dbResult = videoDb->createCamera(cameraCreateDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    // 获取新插入的摄像头ID
    auto newId = vsoa::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());
    
    // 重新获取摄像头信息
    auto newCamera = getCameraById((vsoa::UInt32)newId);
    return newCamera;
}

// 更新摄像头
vsoa::Object<VideoCameraDTO> VideoService::updateCamera(vsoa::Object<VideoCameraDTO> cameraDto)
{
    // 更新摄像头
    auto dbResult = videoDb->updateCamera(cameraDto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    return getCameraById(cameraDto->id);
}

// 删除摄像头
vsoa::Object<StatusDto> VideoService::deleteCamera(vsoa::UInt32 id)
{
    // 检查摄像头是否存在
    auto camera = getCameraById(id);
    auto status = vsoa::Object<StatusDto>::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (!camera)
    {
        status->code = 404;
        status->message = "摄像头不存在";
        return status;
    }

    // 删除摄像头
    auto dbResult = videoDb->deleteCamera(id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    status->code = 200;
    status->message = "删除成功";
    return status;
}

// 控制摄像头
vsoa::Object<StatusDto> VideoService::controlCamera(vsoa::Object<VideoCameraControlDTO> controlDto)
{
    // 检查摄像头是否存在
    auto camera = getCameraById(controlDto->camera_id);
    auto status = vsoa::Object<StatusDto>::createShared();
    status->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (!camera)
    {
        status->code = 404;
        status->message = "摄像头不存在";
        return status;
    }

    // 检查摄像头是否在线
    if (camera->status != "online")
    {
        status->code = 400;
        status->message = "摄像头离线，无法控制";
        return status;
    }

    // 这里可以添加具体的控制逻辑
    // 例如，根据不同的action执行不同的控制命令

    status->code = 200;
    status->message = "控制成功";
    return status;
}

// 更新摄像头状态
vsoa::Object<StatusDto> VideoService::updateCameraStatus(vsoa::UInt32 id, vsoa::String status)
{
    // 检查摄像头是否存在
    auto camera = getCameraById(id);
    auto statusDto = vsoa::Object<StatusDto>::createShared();
    statusDto->timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (!camera)
    {
        statusDto->code = 404;
        statusDto->message = "摄像头不存在";
        return statusDto;
    }

    // 更新摄像头状态
    auto dbResult = videoDb->updateCameraStatus(status, id);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    
    statusDto->code = 200;
    statusDto->message = "状态更新成功";
    return statusDto;
}

// // 转换数据库行到分区DTO
// vsoa::Object<VideoPartitionDTO> VideoService::convertToPartitionDTO(const vsoa::orm::Row& row) {
//     auto partition = vsoa::Object<VideoPartitionDTO>::createShared();
//     partition->id = row.get<vsoa::Int32>("id");
//     partition->name = row.get<vsoa::String>("name");
//     partition->description = row.get<vsoa::String>("description");
//     partition->create_time = row.get<vsoa::Int64>("create_time");
//     return partition;
// }

// // 转换数据库行到摄像头DTO
// vsoa::Object<VideoCameraDTO> VideoService::convertToCameraDTO(const vsoa::orm::Row& row) {
//     auto camera = vsoa::Object<VideoCameraDTO>::createShared();
//     camera->id = row.get<vsoa::Int32>("id");
//     camera->name = row.get<vsoa::String>("name");
//     camera->model = row.get<vsoa::String>("model");
//     camera->ip = row.get<vsoa::String>("ip");
//     camera->port = row.get<vsoa::Int32>("port");
//     camera->username = row.get<vsoa::String>("username");
//     camera->password = row.get<vsoa::String>("password");
//     camera->stream_protocol = row.get<vsoa::String>("stream_protocol");
//     camera->control_protocol = row.get<vsoa::String>("control_protocol");
//     camera->partition_id = row.get<vsoa::Int32>("partition_id");
//     camera->partition_name = row.get<vsoa::String>("partition_name");
//     camera->status = row.get<vsoa::String>("status");
//     camera->description = row.get<vsoa::String>("description");
//     camera->create_time = row.get<vsoa::Int64>("create_time");
//     return camera;
// }
