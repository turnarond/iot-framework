/*
 * @Author: yanchaodong
 * @Date: 2026-02-09 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-02-09 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/nodeserver/service/HmiPointService.hpp
 * @Description: HMI点位服务，用于处理HMI接口的业务逻辑
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#ifndef HMI_POINT_SERVICE_HPP
#define HMI_POINT_SERVICE_HPP

#include "dto/HmiPointDto.hpp"
#include "data_center.h"
#include "driver_collector.h"

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"

class HmiPointService {
private:
    typedef vsoa::web::protocol::http::Status Status;

public:
    HmiPointService() = default;

    /**
     * 批量点位查询
     * @param requestDto 批量查询请求
     * @return 批量查询响应
     */
    vsoa::Object<HmiBatchPointsResponseDto> batchQueryPoints(vsoa::Object<HmiBatchPointsRequestDto> requestDto);

    /**
     * 前缀查询点位
     * @param prefix 点位前缀
     * @return 符合前缀的点位值映射
     */
    vsoa::Object<HmiBatchPointsResponseDto> queryPointsByPrefix(vsoa::String prefix);

    /**
     * 下发控制命令
     * @param controlDto 控制命令
     * @return 控制响应
     */
    vsoa::Object<HmiControlResponseDto> sendControlCommand(vsoa::Object<HmiControlCommandDto> controlDto);

private:
    /**
     * 从RTDB获取点位值
     * @param pointId 点位ID
     * @return 点位值DTO
     */
    vsoa::Object<HmiPointValueDto> getPointValueFromRtdb(const std::string& pointId);
};

#endif // HMI_POINT_SERVICE_HPP