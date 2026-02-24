/*
 * @Author: yanchaodong
 * @Date: 2026-01-28 10:00:00
 * @LastEditors: yanchaodong
 * @LastEditTime: 2026-01-28 10:00:00
 * @FilePath: /acoinfo/edge-framework/src/service/web-server/service/DictService.cpp
 * @Description: Dict Service Implementation
 * 
 * Copyright (c) 2026 by ACOINFO, All Rights Reserved. 
 */

#include "DictService.hpp"
#include "vsoa_dto/core/Types.hpp"
#include <chrono>

/**
* 获取所有数据类型
*/
vsoa::Vector<vsoa::Object<DictDataTypeDto>> DictService::getDataTypes() 
{
    auto dbResult = dictDb->getDataTypes();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    return dbResult->fetch<vsoa::Vector<vsoa::Object<DictDataTypeDto>>>();
}

/**
* 获取所有点位类型
*/
vsoa::Vector<vsoa::Object<DictPointTypeDto>> DictService::getPointTypes()
{
    auto dbResult = dictDb->getPointTypes();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    return dbResult->fetch<vsoa::Vector<vsoa::Object<DictPointTypeDto>>>();
}

/**
* 获取所有传输方法
*/
vsoa::Vector<vsoa::Object<DictTransferTypeDto>> DictService::getTransferMethods()
{
    auto dbResult = dictDb->getTransferMethods();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    return dbResult->fetch<vsoa::Vector<vsoa::Object<DictTransferTypeDto>>>();
}

/**
* 获取所有报警方法
*/
vsoa::Vector<vsoa::Object<DictAlarmMethodDto>> DictService::getAlarmMethods()
{
    auto dbResult = dictDb->getAlarmMethods();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    return dbResult->fetch<vsoa::Vector<vsoa::Object<DictAlarmMethodDto>>>();
}

/**
* 获取所有驱动类型
*/
vsoa::Vector<vsoa::Object<DictDriverTypeDto>> DictService::getDriverTypes()
{
    auto dbResult = dictDb->getDriverTypes();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    return dbResult->fetch<vsoa::Vector<vsoa::Object<DictDriverTypeDto>>>();
}

/**
* 获取所有连接类型
*/
vsoa::Vector<vsoa::Object<DictConnTypeDto>> DictService::getConnTypes()
{
    auto dbResult = dictDb->getConnTypes();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    return dbResult->fetch<vsoa::Vector<vsoa::Object<DictConnTypeDto>>>();
}

/**
* 获取所有取流协议
*/
vsoa::Vector<vsoa::Object<DictStreamProtocolDto>> DictService::getStreamProtocols()
{
    auto dbResult = dictDb->getStreamProtocols();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    return dbResult->fetch<vsoa::Vector<vsoa::Object<DictStreamProtocolDto>>>();
}

/**
* 获取所有控制协议
*/
vsoa::Vector<vsoa::Object<DictControlProtocolDto>> DictService::getControlProtocols()
{
    auto dbResult = dictDb->getControlProtocols();
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    return dbResult->fetch<vsoa::Vector<vsoa::Object<DictControlProtocolDto>>>();
}
