// 基础API服务类
import { request } from './apiClient';
import type { PaginatedResponse } from '../types';

// 基础API服务类
export class BaseApiService {
  protected basePath: string;

  constructor(basePath: string) {
    this.basePath = basePath;
  }

  /**
   * 获取分页列表
   */
  protected async getList<T>(page: number = 1, size: number = 10): Promise<PaginatedResponse<T>> {
    return await request<PaginatedResponse<T>>(`${this.basePath}?page=${page}&size=${size}`);
  }

  /**
   * 根据ID获取详情
   */
  protected async getById<T>(id: string): Promise<T> {
    return await request<T>(`${this.basePath}/${id}?id=${id}`);
  }

  /**
   * 创建新资源
   */
  protected async create<T>(data: Partial<T>, customPath?: string): Promise<T> {
    const path = customPath || this.basePath;
    return await request<T>(path, {
      method: 'POST',
      body: JSON.stringify(data)
    });
  }

  /**
   * 更新资源
   */
  protected async update<T>(data: T): Promise<T> {
    return await request<T>(this.basePath, {
      method: 'PUT',
      body: JSON.stringify(data)
    });
  }

  /**
   * 删除资源
   */
  protected async delete(id: string): Promise<void> {
    return await request<void>(`${this.basePath}/${id}`, {
      method: 'DELETE'
    });
  }
}
