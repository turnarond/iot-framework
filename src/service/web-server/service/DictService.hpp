#pragma once

#include "db/DictDb.hpp"
#include "dto/DictDto.hpp"
#include "dto/StatusDto.hpp"
#include "vsoa_dto/core/Types.hpp"

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"

/**
 * 字典数据服务类
 */
class DictService {
private:
    typedef vsoa::web::protocol::http::Status Status;

private:
    VSOA_COMPONENT(std::shared_ptr<DictDb>, dictDb);

public:
    /**
     * 获取所有数据类型
     */
    vsoa::Vector<vsoa::Object<DictDataTypeDto>> getDataTypes();

    /**
     * 获取所有点位类型
     */
    vsoa::Vector<vsoa::Object<DictPointTypeDto>> getPointTypes();

    /**
     * 获取所有传输方法
     */
    vsoa::Vector<vsoa::Object<DictTransferTypeDto>> getTransferMethods();

    /**
     * 获取所有报警方法
     */
    vsoa::Vector<vsoa::Object<DictAlarmMethodDto>> getAlarmMethods();

    /**
     * 获取所有驱动类型
     */
    vsoa::Vector<vsoa::Object<DictDriverTypeDto>> getDriverTypes();
    
    /**
     * 获取所有连接类型
     */
    vsoa::Vector<vsoa::Object<DictConnTypeDto>> getConnTypes();

    /**
     * 获取所有取流协议
     */
    vsoa::Vector<vsoa::Object<DictStreamProtocolDto>> getStreamProtocols();

    /**
     * 获取所有控制协议
     */
    vsoa::Vector<vsoa::Object<DictControlProtocolDto>> getControlProtocols();
};

