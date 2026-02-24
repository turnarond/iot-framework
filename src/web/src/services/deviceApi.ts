// 设备相关API
import { BaseApiService } from './baseApi';
import type { PaginatedResponse } from '../types';

// 设备类型定义
export interface Device {
  id: string;
  name: string;
  driverId: string;
  driverName: string;
  address: string;
  status: string;
  [key: string]: any;
}

// 设备API服务类
class DeviceApiService extends BaseApiService {
  constructor() {
    super('/devices');
  }

  // 获取设备列表（分页）
  async getDevices(page: number = 1, size: number = 10): Promise<PaginatedResponse<Device>> {
    return await this.getList<Device>(page, size);
  }

  // 根据ID获取设备
  async getDeviceById(id: string): Promise<Device> {
    return await this.getById<Device>(id);
  }

  // 创建设备
  async createDevice(deviceData: Partial<Device>): Promise<Device> {
    return await this.create<Device>(deviceData);
  }

  // 更新设备
  async updateDevice(deviceData: Device): Promise<Device> {
    return await this.update<Device>(deviceData);
  }

  // 删除设备
  async deleteDevice(id: string): Promise<void> {
    return await this.delete(id);
  }
}

// 导出设备API实例
export const deviceApi = new DeviceApiService();
