// 点位相关API
import { request } from './apiClient';

// 点位类型定义
export interface Point {
  id: string;
  name: string;
  deviceId: string;
  deviceName?: string;
  address: string;
  type: string;
  value: any;
  unit: string;
  description: string;
  [key: string]: any;
}

// 点位列表响应类型
export interface PointListResponse {
  items: Point[];
  counts: number;
  page: number;
  size: number;
  pages: number;
}

// 点位值响应类型
export interface PointValueResponse {
  value: any;
  timestamp: string;
}

// 点位相关API
export const pointApi = {
  // 获取点位列表（分页）
  getPoints: async (page: number = 1, size: number = 10): Promise<PointListResponse> => {
    return await request<PointListResponse>(`/points?page=${page}&size=${size}`);
  },

  // 获取所有点位（包含设备信息）
  getPointsWithDevice: async (page: number = 1, size: number = 10): Promise<PointListResponse> => {
    return await request<PointListResponse>(`/points-with-device?page=${page}&size=${size}`);
  },

  // 根据ID获取点位
  getPointById: async (id: string): Promise<Point> => {
    return await request<Point>(`/points/${id}`);
  },

  // 根据ID获取点位（包含设备信息）
  getPointWithDeviceById: async (id: string): Promise<Point> => {
    return await request<Point>(`/points-with-device/${id}`);
  },

  // 创建点位
  createPoint: async (pointData: Partial<Point>): Promise<Point> => {
    return await request<Point>('/points', {
      method: 'POST',
      body: JSON.stringify(pointData)
    });
  },

  // 更新点位
  updatePoint: async (pointData: Point): Promise<Point> => {
    return await request<Point>(`/points/${pointData.id}`, {
      method: 'PUT',
      body: JSON.stringify(pointData)
    });
  },

  // 删除点位
  deletePoint: async (id: string): Promise<void> => {
    return await request<void>(`/points/${id}`, {
      method: 'DELETE'
    });
  },

  // 获取点位值
  getPointValue: async (name: string): Promise<PointValueResponse> => {
    return await request<PointValueResponse>(`/points/value?name=${name}`);
  },

  // 根据ID获取点位值
  getPointValueById: async (id: string): Promise<PointValueResponse> => {
    return await request<PointValueResponse>(`/points/value/by-id/${id}`);
  },

  // 设置点位值
  setPointValue: async (name: string, value: any): Promise<PointValueResponse> => {
    return await request<PointValueResponse>(`/points/value?name=${name}`, {
      method: 'POST',
      body: JSON.stringify({ value })
    });
  },

  // 根据ID设置点位值
  setPointValueById: async (id: string, value: any): Promise<PointValueResponse> => {
    return await request<PointValueResponse>(`/points/value/by-id/${id}`, {
      method: 'POST',
      body: JSON.stringify({ value })
    });
  }
};