// 驱动相关API
import { BaseApiService } from './baseApi';
import type { PaginatedResponse } from '../types';

// 驱动类型定义
export interface Driver {
  id: string;
  name: string;
  type: string;
  version: string;
  status: 'running' | 'stopped';
  [key: string]: any;
}

// 驱动API服务类
class DriverApiService extends BaseApiService {
  constructor() {
    super('/drivers');
  }

  // 获取驱动列表（分页）
  async getDrivers(page: number = 1, size: number = 10): Promise<PaginatedResponse<Driver>> {
    return await this.getList<Driver>(page, size);
  }

  // 获取所有驱动（用于设备绑定）
  async getAllDrivers(): Promise<Driver[]> {
    const response = await this.getList<Driver>(1, 100);
    return response.items || [];
  }

  // 根据ID获取驱动
  async getDriverById(id: string): Promise<Driver> {
    return await this.getById<Driver>(id);
  }

  // 创建驱动
  async createDriver(driverData: Partial<Driver>): Promise<Driver> {
    return await this.create<Driver>(driverData);
  }

  // 更新驱动
  async updateDriver(driverData: Driver): Promise<Driver> {
    return await this.update<Driver>(driverData);
  }

  // 删除驱动
  async deleteDriver(id: string): Promise<void> {
    return await this.delete(id);
  }
}

// 导出驱动API实例
export const driverApi = new DriverApiService();
