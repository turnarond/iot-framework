// 报警相关API
import { request } from './apiClient';

// 报警规则类型定义
export interface AlarmRule {
  id: string;
  pointId: string;
  pointName?: string;
  deviceId?: string;
  deviceName?: string;
  condition: string;
  threshold: number;
  priority: string;
  message: string;
  [key: string]: any;
}

// 报警类型定义
export interface Alarm {
  id: string;
  ruleId: string;
  pointId: string;
  pointName: string;
  deviceId: string;
  deviceName: string;
  value: number;
  threshold: number;
  priority: string;
  message: string;
  status: string;
  timestamp: string;
  [key: string]: any;
}

// 报警规则列表响应类型
export interface AlarmRuleListResponse {
  items: AlarmRule[];
  counts: number;
  page: number;
  size: number;
  pages: number;
}

// 报警列表响应类型
export interface AlarmListResponse {
  items: Alarm[];
  counts: number;
  page: number;
  size: number;
  pages: number;
}

// 报警相关API
export const alarmApi = {
  // 获取报警规则列表（分页）
  getAlarmRules: async (page: number = 1, size: number = 10): Promise<AlarmRuleListResponse> => {
    return await request<AlarmRuleListResponse>(`/alarm-rules?page=${page}&size=${size}`);
  },

  // 获取报警规则列表（包含点位信息，分页）
  getAlarmRulesWithPoint: async (page: number = 1, size: number = 10): Promise<AlarmRuleListResponse> => {
    return await request<AlarmRuleListResponse>(`/alarm-rules/with-point?page=${page}&size=${size}`);
  },

  // 根据ID获取报警规则
  getAlarmRuleById: async (id: string): Promise<AlarmRule> => {
    return await request<AlarmRule>(`/alarm-rules/${id}`);
  },

  // 根据ID获取报警规则（包含点位信息）
  getAlarmRuleWithPointById: async (id: string): Promise<AlarmRule> => {
    return await request<AlarmRule>(`/alarm-rules/with-point/${id}`);
  },

  // 创建报警规则
  createAlarmRule: async (alarmRuleData: Partial<AlarmRule>): Promise<AlarmRule> => {
    // 在保存时移除 `id` 字段，仅在查询返回时包含 `id`
    const payload = { ...alarmRuleData };
    if (payload.hasOwnProperty('id')) {
      delete payload.id;
    }
    return await request<AlarmRule>('/alarm-rules', {
      method: 'POST',
      body: JSON.stringify(payload)
    });
  },

  // 更新报警规则
  updateAlarmRule: async (alarmRuleData: AlarmRule): Promise<AlarmRule> => {
    // 在保存/更新时移除 `id` 字段，避免将ID作为规则内容保存
    const payload = { ...alarmRuleData };
    if (payload.hasOwnProperty('id')) {
      delete payload.id;
    }
    return await request<AlarmRule>('/alarm-rules', {
      method: 'PUT',
      body: JSON.stringify(payload)
    });
  },

  // 删除报警规则
  deleteAlarmRule: async (id: string): Promise<void> => {
    return await request<void>(`/alarm-rules/${id}`, {
      method: 'DELETE'
    });
  },

  // 获取所有报警（分页）
  getAlarms: async (page: number = 1, size: number = 10): Promise<AlarmListResponse> => {
    return await request<AlarmListResponse>(`/alarms?page=${page}&size=${size}`);
  },

  // 获取活跃报警（分页）
  getActiveAlarms: async (page: number = 1, size: number = 10): Promise<AlarmListResponse> => {
    return await request<AlarmListResponse>(`/alarms/active?page=${page}&size=${size}`);
  },

  // 根据点位ID获取报警规则
  getAlarmRulesByPointId: async (pointId: string): Promise<AlarmRule[]> => {
    return await request<AlarmRule[]>(`/alarm-rules/by-point/${pointId}`);
  }
};