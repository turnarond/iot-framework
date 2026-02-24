#pragma once

#include <dto/DictDto.hpp>
#include "oatpp-sqlite/orm.hpp"
#include "lwcomm/lwcomm.h"

#include VSOA_CODEGEN_BEGIN(DbClient) //<- Begin Codegen

/**
 * 字典数据数据库操作类
 */
class DictDb : public vsoa::orm::DbClient {
public:
    DictDb(const std::shared_ptr<vsoa::orm::Executor>& executor) 
    : vsoa::orm::DbClient(executor) 
    {}

    /**
     * 获取所有数据类型
     */
    QUERY(getDataTypes,
          "SELECT id, code, cname, description FROM t_dict_data_types ORDER BY id")

    /**
     * 获取所有点位类型
     */
    QUERY(getPointTypes,
          "SELECT id, name, cname FROM t_dict_point_types ORDER BY id")

    /**
     * 获取所有传输方法
     */
    QUERY(getTransferMethods,
          "SELECT id, name, cname FROM t_dict_transfer_types ORDER BY id")

    /**
     * 获取所有报警方法
     */
    QUERY(getAlarmMethods,
          "SELECT id, name, cname, description FROM t_dict_alarm_methods ORDER BY id")

    /**
     * 获取所有驱动类型
     */
    QUERY(getDriverTypes,
          "SELECT id, name, cname FROM t_dict_driver_types ORDER BY id")

    /**
     * 获取所有连接类型
     */
    QUERY(getConnTypes,
          "SELECT id, code, cname, description FROM t_dict_conn_types ORDER BY id")

    /**
     * 获取所有取流协议
     */
    QUERY(getStreamProtocols,
          "SELECT id, name, params, params_desc FROM t_dict_stream_protocols ORDER BY id")

    /**
     * 获取所有控制协议
     */
    QUERY(getControlProtocols,
          "SELECT id, name, params, params_desc FROM t_dict_control_protocols ORDER BY id")

};

#include VSOA_CODEGEN_END(DbClient) //<- End Codegen
