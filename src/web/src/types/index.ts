// 通用类型定义

// 导入服务管理相关类型
export * from './servermgr'

// 统一的分页响应类型
export interface PaginatedResponse<T> {
  items: T[];
  counts: number;
  page: number;
  size: number;
  pages: number;
}

// 摄像头类型定义
export interface Camera {
  id: string
  name: string
  model: string
  ip: string
  port: number
  username: string
  password: string
  stream_protocol: string
  stream_protocol_id: number
  stream_protocol_param?: string
  control_protocol: string
  control_protocol_id: number
  control_protocol_param?: string
  partition_id: string
  partition_name: string
  status: 'online' | 'offline'
  description: string
}

export interface DictProtocol {
  id: string
  name: string
  params: string
  params_desc?: string
}

// 分区类型定义
export interface Partition {
  id: string;
  name: string;
  camera_count: number;
  cameraCount?: number;
  description: string;
}
