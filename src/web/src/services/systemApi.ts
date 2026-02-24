// 系统配置相关API
import { request } from './apiClient';

// 系统配置类型定义
export interface SystemConfig {
  id: string;
  key: string;
  value: string;
  description: string;
  [key: string]: any;
}

// 网络配置类型定义
export interface NetworkConfig {
  id: string;
  interface: string;
  ipAddress: string;
  subnetMask: string;
  gateway: string;
  dns1: string;
  dns2: string;
  [key: string]: any;
}

// NTP配置类型定义
export interface NtpConfig {
  id: string;
  server: string;
  timezone: string;
  enabled: boolean;
  [key: string]: any;
}

// 系统配置列表响应类型
export interface SystemConfigListResponse {
  data: SystemConfig[];
  total: number;
  page: number;
  size: number;
}

// 系统配置相关API
export const systemApi = {
  // 获取系统配置列表
  getSystemConfigs: async (): Promise<SystemConfig[]> => {
    return await request<SystemConfig[]>('/system/configs');
  },

  // 根据键获取系统配置
  getSystemConfigByKey: async (key: string): Promise<SystemConfig> => {
    return await request<SystemConfig>(`/system/configs?key=${key}`);
  },

  // 更新系统配置
  updateSystemConfig: async (key: string, value: string): Promise<SystemConfig> => {
    return await request<SystemConfig>('/system/configs', {
      method: 'PUT',
      body: JSON.stringify({ key, value })
    });
  },

  // 获取网络配置
  getNetworkConfig: async (): Promise<NetworkConfig[]> => {
    return await request<NetworkConfig[]>('/system/network');
  },

  // 更新网络配置
  updateNetworkConfig: async (networkConfig: NetworkConfig): Promise<NetworkConfig> => {
    return await request<NetworkConfig>('/system/network', {
      method: 'PUT',
      body: JSON.stringify(networkConfig)
    });
  },

  // 获取NTP配置
  getNtpConfig: async (): Promise<NtpConfig> => {
    return await request<NtpConfig>('/system/ntp');
  },

  // 更新NTP配置
  updateNtpConfig: async (ntpConfig: NtpConfig): Promise<NtpConfig> => {
    return await request<NtpConfig>('/system/ntp', {
      method: 'PUT',
      body: JSON.stringify(ntpConfig)
    });
  }
};