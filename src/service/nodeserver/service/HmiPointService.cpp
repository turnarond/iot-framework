/*
 * @Author: yanchaodong
 * @Date: 2026-02-09 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-09 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/nodeserver/service/HmiPointService.cpp
 * @Description: HMI点位服务实现，处理HMI接口的业务逻辑
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "HmiPointService.hpp"
#include "rtdb.hpp"

#include <lwlog/lwlog.h>

extern CLWLog g_logger;

vsoa::Object<HmiBatchPointsResponseDto> HmiPointService::batchQueryPoints(vsoa::Object<HmiBatchPointsRequestDto> requestDto)
{
    auto response = HmiBatchPointsResponseDto::createShared();
    response->points = vsoa::Vector<vsoa::Object<HmiPointValueDto>>::createShared();

    if (requestDto->pointIds) {
        for (const auto& pointId : *requestDto->pointIds) {
            auto pointValue = getPointValueFromRtdb(*pointId);
            if (pointValue) {
                response->points->push_back(pointValue);
            }
        }
    }

    return response;
}

vsoa::Object<HmiBatchPointsResponseDto> HmiPointService::queryPointsByPrefix(vsoa::String prefix)
{
    auto response = HmiBatchPointsResponseDto::createShared();
    response->points = vsoa::Vector<vsoa::Object<HmiPointValueDto>>::createShared();

    // 这里需要实现前缀查询逻辑
    // 由于RTDB目前没有提供前缀查询的API，我们需要遍历所有点位并检查前缀
    // 注意：这种实现方式在点位数量很多时可能会影响性能
    // 后续可以考虑在RTDB中添加前缀查询的API

    // 暂时返回空结果，需要根据实际RTDB实现来修改
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "Prefix query not fully implemented yet for prefix: %s", prefix ? prefix->c_str() : "null");

    return response;
}

vsoa::Object<HmiControlResponseDto> HmiPointService::sendControlCommand(vsoa::Object<HmiControlCommandDto> controlDto)
{
    auto response = HmiControlResponseDto::createShared();

    if (!controlDto->pointId || !controlDto->value) {
        response->success = false;
        response->message = "Invalid control command: missing pointId or value";
        return response;
    }

    // 使用DRIVER_COLLECTOR下发控制命令
    int success = DRIVER_COLLECTOR->SetTagControl(*controlDto->pointId, *controlDto->value);

    if (success == 0) {
        response->success = true;
        response->message = "Control command sent successfully";
        g_logger.LogMessage(LW_LOGLEVEL_INFO, "Control command sent successfully for point %s with value %s", 
            controlDto->pointId->c_str(), controlDto->value->c_str());
    } else {
        response->success = false;
        response->message = "Failed to send control command";
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "Failed to send control command for point %s with value %s", 
            controlDto->pointId->c_str(), controlDto->value->c_str());
    }

    return response;
}

vsoa::Object<HmiPointValueDto> HmiPointService::getPointValueFromRtdb(const std::string& pointId)
{
    nodeserver::TagRecord rec;
    bool ok = DATA_CENTER->GetRTDB()->getTag(pointId.c_str(), rec);

    if (!ok) {
        g_logger.LogMessage(LW_LOGLEVEL_WARN, "Point not found in RTDB: %s", pointId.c_str());
        return nullptr;
    }

    auto pointValue = HmiPointValueDto::createShared();
    pointValue->value = rec.value;
    pointValue->quality = "GOOD"; // 暂时硬编码为GOOD，后续可以根据实际情况修改
    pointValue->ts = rec.timestamp_ms;

    return pointValue;
}