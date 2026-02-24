#pragma once

#include "PageDto.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

/**
 * 数据类型字典DTO
 */
class DictDataTypeDto : public vsoa::DTO {
    DTO_INIT(DictDataTypeDto, DTO)

    DTO_FIELD(UInt32, id, "id");                    ///< 数据类型ID
    DTO_FIELD(String, code, "code");                ///< 数据类型代码
    DTO_FIELD(String, cname, "cname");              ///< 数据类型中文名称
    DTO_FIELD(String, description, "description");  ///< 数据类型描述
};

/**
 * 点位类型字典DTO
 */
class DictPointTypeDto : public vsoa::DTO {
    DTO_INIT(DictPointTypeDto, DTO)

    DTO_FIELD(UInt32, id, "id");                    ///< 点位类型ID
    DTO_FIELD(String, name, "name");                ///< 点位类型名称
    DTO_FIELD(String, cname, "cname");              ///< 点位类型中文名称
};

/**
 * 传输方法字典DTO
 */
class DictTransferTypeDto : public vsoa::DTO {
    DTO_INIT(DictTransferTypeDto, DTO)

    DTO_FIELD(UInt32, id, "id");                    ///< 传输方法ID
    DTO_FIELD(String, name, "name");                ///< 传输方法名称
    DTO_FIELD(String, cname, "cname");              ///< 传输方法中文名称
};

/**
 * 报警方法字典DTO
 */
class DictAlarmMethodDto : public vsoa::DTO {
    DTO_INIT(DictAlarmMethodDto, DTO)

    DTO_FIELD(UInt32, id, "id");                    ///< 报警方法ID
    DTO_FIELD(String, name, "name");                ///< 报警方法名称
    DTO_FIELD(String, cname, "cname");              ///< 报警方法中文名称
    DTO_FIELD(String, description, "description");  ///< 报警方法描述
};

/**
 * 驱动类型字典DTO
 */
class DictDriverTypeDto : public vsoa::DTO {
    DTO_INIT(DictDriverTypeDto, DTO)

    DTO_FIELD(UInt32, id, "id");                    ///< 驱动类型ID
    DTO_FIELD(String, name, "name");                ///< 驱动类型名称
    DTO_FIELD(String, cname, "cname");              ///< 驱动类型中文名称
};

/**
 * 连接类型字典DTO
 */
class DictConnTypeDto : public vsoa::DTO {
    DTO_INIT(DictConnTypeDto, DTO)

    DTO_FIELD(UInt32, id, "id");                    ///< 连接类型ID
    DTO_FIELD(String, code, "code");                ///< 连接类型代码
    DTO_FIELD(String, cname, "cname");              ///< 连接类型中文名称
    DTO_FIELD(String, description, "description");  ///< 连接类型描述
};

/**
 * 取流协议字典DTO
 */
class DictStreamProtocolDto : public vsoa::DTO {
    DTO_INIT(DictStreamProtocolDto, DTO)

    DTO_FIELD(UInt32, id, "id");                    ///< 取流协议ID
    DTO_FIELD(String, name, "name");                ///< 取流协议名称
    DTO_FIELD(String, params, "params");            ///< 取流协议参数
    DTO_FIELD(String, params_desc, "params_desc");  ///< 取流协议参数描述
};

/**
 * 控制协议字典DTO
 */
class DictControlProtocolDto : public vsoa::DTO {
    DTO_INIT(DictControlProtocolDto, DTO)

    DTO_FIELD(UInt32, id, "id");                    ///< 控制协议ID
    DTO_FIELD(String, name, "name");                ///< 控制协议名称
    DTO_FIELD(String, params, "params");            ///< 控制协议参数
    DTO_FIELD(String, params_desc, "params_desc");  ///< 控制协议参数描述
};

#include VSOA_CODEGEN_END(DTO) //<- End Codegen

