// 字典相关API
import { request } from './apiClient';

// 字典类型定义
export interface Dict {
  id: string;
  code: string;
  name: string;
  value: string;
  description: string;
  [key: string]: any;
}

// 驱动类型字典
interface DriverTypeDict {
  id: string;
  cname: string;
  [key: string]: any;
}

// 连接类型字典
interface ConnTypeDict {
  id: string;
  cname: string;
  [key: string]: any;
}

// 数据类型字典
interface DataTypeDict {
  id: string;
  cname: string;
  [key: string]: any;
}

// 点位类型字典
interface PointTypeDict {
  id: string;
  cname: string;
  [key: string]: any;
}

// 传输方法字典
interface TransferMethodDict {
  id: string;
  cname: string;
  [key: string]: any;
}

// 报警方法字典
interface AlarmMethodDict {
  id: string;
  cname: string;
  [key: string]: any;
}

// 协议字典
interface ProtocolDict {
  id: string;
  name: string;
  params: string;
  [key: string]: any;
}

// 字典列表响应类型
export interface DictListResponse {
  data: Dict[];
  total: number;
  page: number;
  size: number;
}

// 字典相关API
export const dictApi = {
  // 获取字典列表（分页）
  getDicts: async (page: number = 1, size: number = 10): Promise<DictListResponse> => {
    return await request<DictListResponse>(`/dict?page=${page}&size=${size}`);
  },

  // 根据代码获取字典列表
  getDictsByCode: async (code: string): Promise<Dict[]> => {
    return await request<Dict[]>(`/dict?code=${code}`);
  },

  // 根据ID获取字典
  getDictById: async (id: string): Promise<Dict> => {
    return await request<Dict>(`/dict/${id}`);
  },

  // 创建字典
  createDict: async (dictData: Partial<Dict>): Promise<Dict> => {
    return await request<Dict>('/dict', {
      method: 'POST',
      body: JSON.stringify(dictData)
    });
  },

  // 更新字典
  updateDict: async (dictData: Dict): Promise<Dict> => {
    return await request<Dict>('/dict', {
      method: 'PUT',
      body: JSON.stringify(dictData)
    });
  },

  // 删除字典
  deleteDict: async (id: string): Promise<void> => {
    return await request<void>(`/dict/${id}`, {
      method: 'DELETE'
    });
  },

  // 获取所有驱动类型
  getDriverTypes: async (): Promise<DriverTypeDict[]> => {
    return await request<DriverTypeDict[]>('/dict/driver-types');
  },

  // 获取所有连接类型
  getConnTypes: async (): Promise<ConnTypeDict[]> => {
    return await request<ConnTypeDict[]>('/dict/conn-types');
  },

  // 获取所有数据类型
  getDataTypes: async (): Promise<DataTypeDict[]> => {
    return await request<DataTypeDict[]>('/dict/data-types');
  },

  // 获取所有点位类型
  getPointTypes: async (): Promise<PointTypeDict[]> => {
    return await request<PointTypeDict[]>('/dict/point-types');
  },

  // 获取所有传输方法
  getTransferMethods: async (): Promise<TransferMethodDict[]> => {
    return await request<TransferMethodDict[]>('/dict/transfer-methods');
  },

  // 获取所有报警方法
  getAlarmMethods: async (): Promise<AlarmMethodDict[]> => {
    return await request<AlarmMethodDict[]>('/dict/alarm-methods');
  },

  // 获取所有取流协议
  getStreamProtocols: async (): Promise<ProtocolDict[]> => {
    return await request<ProtocolDict[]>('/dict/stream-protocols');
  },

  // 获取所有控制协议
  getControlProtocols: async (): Promise<ProtocolDict[]> => {
    return await request<ProtocolDict[]>('/dict/control-protocols');
  }
};