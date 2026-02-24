// 视频监控相关DTO
#ifndef VIDEO_DTO_HPP
#define VIDEO_DTO_HPP

#include "PageDto.hpp"

#include VSOA_CODEGEN_BEGIN(DTO)

// 分区DTO
class VideoPartitionDTO : public vsoa::DTO {
    DTO_INIT(VideoPartitionDTO, DTO);
    DTO_FIELD(vsoa::UInt32, id, "id"); ///< 分区ID
    DTO_FIELD(vsoa::String, name, "name"); ///< 分区名称
    DTO_FIELD(vsoa::String, description, "description"); ///< 分区描述
    DTO_FIELD(vsoa::Int64, create_time, "create_time"); ///< 创建时间
    DTO_FIELD(vsoa::UInt32, camera_count, "camera_count"); ///< 摄像头数量
};

// 分区分页响应DTO
class VideoPartitionPageDTO : public PageDto<vsoa::Object<VideoPartitionDTO>> {
    DTO_INIT(VideoPartitionPageDTO, PageDto<vsoa::Object<VideoPartitionDTO>>);
};

// 分区创建/更新DTO
class VideoPartitionCreateDTO : public vsoa::DTO {
    DTO_INIT(VideoPartitionCreateDTO, DTO);
    DTO_FIELD(vsoa::String, name, "name"); ///< 分区名称
    DTO_FIELD(vsoa::String, description, "description"); ///< 分区描述
};

// 摄像头DTO
class VideoCameraDTO : public vsoa::DTO {
    DTO_INIT(VideoCameraDTO, DTO);
    DTO_FIELD(vsoa::UInt32, id, "id"); ///< 摄像头ID
    DTO_FIELD(vsoa::String, name, "name"); ///< 摄像头名称
    DTO_FIELD(vsoa::String, model, "model"); ///< 摄像头模型
    DTO_FIELD(vsoa::String, ip, "ip"); ///< IP地址
    DTO_FIELD(vsoa::Int32, port, "port"); ///< 端口号
    DTO_FIELD(vsoa::String, username, "username"); ///< 用户名
    DTO_FIELD(vsoa::String, password, "password"); ///< 密码
    DTO_FIELD(vsoa::String, stream_protocol, "stream_protocol"); ///< 流协议
    DTO_FIELD(vsoa::UInt32, stream_protocol_id, "stream_protocol_id"); ///< 流协议ID
    DTO_FIELD(vsoa::String, stream_protocol_param, "stream_protocol_param"); ///< 流协议参数
    DTO_FIELD(vsoa::String, control_protocol, "control_protocol"); ///< 控制协议
    DTO_FIELD(vsoa::UInt32, control_protocol_id, "control_protocol_id"); ///< 控制协议ID
    DTO_FIELD(vsoa::String, control_protocol_param, "control_protocol_param"); ///< 控制协议URL
    DTO_FIELD(vsoa::UInt32, partition_id, "partition_id"); ///< 分区ID
    DTO_FIELD(vsoa::String, partition_name, "partition_name"); ///< 分区名称
    DTO_FIELD(vsoa::String, status, "status"); ///< 状态描述
    DTO_FIELD(vsoa::String, description, "description"); ///< 描述
    DTO_FIELD(vsoa::Int64, create_time, "create_time"); ///< 创建时间
};

// 摄像头分页响应DTO
class VideoCameraPageDTO : public PageDto<vsoa::Object<VideoCameraDTO>> {
    DTO_INIT(VideoCameraPageDTO, PageDto<vsoa::Object<VideoCameraDTO>>);
};

// 摄像头创建/更新DTO
class VideoCameraCreateDTO : public vsoa::DTO {
    DTO_INIT(VideoCameraCreateDTO, DTO);
    DTO_FIELD(vsoa::String, name, "name"); ///< 摄像头名称
    DTO_FIELD(vsoa::String, model, "model"); ///< 摄像头模型
    DTO_FIELD(vsoa::String, ip, "ip"); ///< IP地址
    DTO_FIELD(vsoa::Int32, port, "port"); ///< 端口号
    DTO_FIELD(vsoa::String, username, "username"); ///< 用户名
    DTO_FIELD(vsoa::String, password, "password"); ///< 密码
    DTO_FIELD(vsoa::String, stream_protocol, "stream_protocol"); ///< 流协议
    DTO_FIELD(vsoa::UInt32, stream_protocol_id, "stream_protocol_id"); ///< 流协议ID
    DTO_FIELD(vsoa::String, stream_protocol_param, "stream_protocol_param"); ///< 流协议URL
    DTO_FIELD(vsoa::String, control_protocol, "control_protocol"); ///< 控制协议
    DTO_FIELD(vsoa::UInt32, control_protocol_id, "control_protocol_id"); ///< 控制协议ID
    DTO_FIELD(vsoa::String, control_protocol_param, "control_protocol_param"); ///< 控制协议URL
    DTO_FIELD(vsoa::UInt32, partition_id, "partition_id"); ///< 分区ID
    DTO_FIELD(vsoa::String, description, "description"); ///< 描述
    DTO_FIELD(vsoa::Int64, create_time, "create_time"); ///< 创建时间
};

class VideoCameraControlDTO : public vsoa::DTO {
    DTO_INIT(VideoCameraControlDTO, DTO);
    DTO_FIELD(vsoa::UInt32, camera_id, "camera_id"); ///< 摄像头ID
    DTO_FIELD(vsoa::String, action, "action"); ///< 控制动作
    DTO_FIELD(vsoa::String, params, "params"); ///< 控制参数
};


#include VSOA_CODEGEN_END(DTO)

#endif // VIDEO_DTO_HPP