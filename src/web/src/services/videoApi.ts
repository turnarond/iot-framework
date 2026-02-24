// 视频监控相关API服务
import { BaseApiService } from './baseApi';
import type { Camera, Partition } from '../types';

// 摄像头API服务
export class CameraApiService extends BaseApiService {
  constructor() {
    super('/video-cameras');
  }

  /**
   * 获取摄像头列表
   */
  async getCameras(page: number = 1, size: number = 10, partitionId?: string) {
    const params: Record<string, any> = { page, size };
    if (partitionId) {
      params.partition_id = partitionId;
    }
    return await this.getList<Camera>(page, size, params);
  }

  /**
   * 获取摄像头详情
   */
  async getCameraById(id: string) {
    return await this.getById<Camera>(id);
  }

  /**
   * 创建摄像头
   */
  async createCamera(data: Partial<Camera>) {
    return await this.create<Camera>(data);
  }

  /**
   * 更新摄像头
   */
  async updateCamera(data: Camera) {
    return await this.update<Camera>(data);
  }

  /**
   * 删除摄像头
   */
  async deleteCamera(id: string) {
    return await this.delete(id);
  }

  /**
   * 获取摄像头状态
   */
  async getCameraStatus(id: string) {
    // 假设后端提供了获取状态的接口
    return await this.getById<{ status: 'online' | 'offline' }>(`${id}/status`);
  }

  /**
   * 控制摄像头
   */
  async controlCamera(id: string, action: string, params?: any) {
    // 与后端接口匹配，使用 camera_id 字段
    return await this.create<{ success: boolean }>({
      camera_id: id,
      action,
      params
    }, `${this.basePath}/control`);
  }
}

// 分区API服务
export class PartitionApiService extends BaseApiService {
  constructor() {
    super('/video-partitions');
  }

  /**
   * 获取分区列表
   */
  async getPartitions(page: number = 1, size: number = 10) {
    return await this.getList<Partition>(page, size);
  }

  /**
   * 获取分区详情
   */
  async getPartitionById(id: string) {
    return await this.getById<Partition>(id);
  }

  /**
   * 创建分区
   */
  async createPartition(data: Partial<Partition>) {
    return await this.create<Partition>(data);
  }

  /**
   * 更新分区
   */
  async updatePartition(data: Partition) {
    return await this.update<Partition>(data);
  }

  /**
   * 删除分区
   */
  async deletePartition(id: string) {
    return await this.delete(id);
  }

  /**
   * 获取分区下的摄像头数量
   */
  async getPartitionCameraCount(id: string) {
    // 假设后端提供了获取摄像头数量的接口
    return await this.getById<{ count: number }>(`${id}/camera-count`);
  }
}

// 导出实例
export const cameraApi = new CameraApiService();
export const partitionApi = new PartitionApiService();