// 用户相关API
import { request } from './apiClient';

// 用户类型定义
export interface User {
  id: string;
  username: string;
  name: string;
  email: string;
  role: string;
  status: string;
  [key: string]: any;
}

// 用户列表响应类型
export interface UserListResponse {
  data: User[];
  total: number;
  page: number;
  size: number;
}

// 用户相关API
export const userApi = {
  // 获取用户列表（分页）
  getUsers: async (page: number = 1, size: number = 10): Promise<UserListResponse> => {
    return await request<UserListResponse>(`/users?page=${page}&size=${size}`);
  },

  // 根据ID获取用户
  getUserById: async (id: string): Promise<User> => {
    return await request<User>(`/users/${id}`);
  },

  // 创建用户
  createUser: async (userData: Partial<User>): Promise<User> => {
    return await request<User>('/users', {
      method: 'POST',
      body: JSON.stringify(userData)
    });
  },

  // 更新用户
  updateUser: async (userData: User): Promise<User> => {
    return await request<User>('/users', {
      method: 'PUT',
      body: JSON.stringify(userData)
    });
  },

  // 删除用户
  deleteUser: async (id: string): Promise<void> => {
    return await request<void>(`/users?id=${id}`, {
      method: 'DELETE'
    });
  }
};