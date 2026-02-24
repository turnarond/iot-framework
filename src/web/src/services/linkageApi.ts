// 联动规则相关API
import { request } from './apiClient';

// 联动规则类型定义
export interface LinkageRule {
  id: string;
  name: string;
  description: string;
  enabled: boolean;
  triggers?: Trigger[];
  actions?: Action[];
  [key: string]: any;
}

// 触发器类型定义
export interface Trigger {
  id: string;
  source_id: string;
  [key: string]: any;
}

// 动作类型定义
export interface Action {
  id: string;
  name: string;
  type: string;
  action_type_id?: string;
  params?: {
    param1?: string;
    param2?: string;
    param3?: any;
    param4?: any;
  };
  need_confirm?: boolean;
  [key: string]: any;
}

// 事件类型定义
export interface EventType {
  id: string;
  name: string;
  cname?: string;
  description: string;
  [key: string]: any;
}

// 触发器类型定义
export interface TriggerType {
  id: string;
  name: string;
  cname: string;
  description: string;
  [key: string]: any;
}

// 动作类型定义
export interface ActionType {
  id: string;
  name: string;
  cname: string;
  description: string;
  param1_name?: string;
  param2_name?: string;
  param3_name?: string;
  param4_name?: string;
  param1_tip?: string;
  param2_tip?: string;
  param3_tip?: string;
  param4_tip?: string;
  [key: string]: any;
}

// 触发器类型列表响应类型
export interface TriggerTypeListResponse {
  items: TriggerType[];
  counts: number;
  page: number;
  size: number;
  pages: number;
}

// 联动规则列表响应类型
export interface LinkageRuleListResponse {
  items: LinkageRule[];
  counts: number;
  page: number;
  size: number;
  pages: number;
}

// 触发器列表响应类型
export interface TriggerListResponse {
  items: Trigger[];
  counts: number;
  page: number;
  size: number;
  pages: number;
}

// 动作列表响应类型
export interface ActionListResponse {
  items: Action[];
  counts: number;
  page: number;
  size: number;
  pages: number;
}

// 事件类型列表响应类型
export interface EventTypeListResponse {
  items: EventType[];
  counts: number;
  page: number;
  size: number;
  pages: number;
}

// 动作类型列表响应类型
export interface ActionTypeListResponse {
  items: ActionType[];
  counts: number;
  page: number;
  size: number;
  pages: number;
}

// 表达式类型定义
export interface Expression {
  id: string;
  name: string;
  expression: string;
  enable: boolean;
  description?: string;
  [key: string]: any;
}

// 表达式列表响应类型
export interface ExpressionListResponse {
  items: Expression[];
  counts: number;
  page: number;
  size: number;
  pages: number;
}

// 联动规则相关API
export const linkageApi = {
  // 获取联动规则列表（分页）
  getLinkages: async (page: number = 1, size: number = 10): Promise<LinkageRuleListResponse> => {
    // Use the detailed list if available
    return await request<LinkageRuleListResponse>(`/linkage-rules?page=${page}&size=${size}`);
  },

  // 根据ID获取联动规则
  getLinkageById: async (id: string): Promise<LinkageRule> => {
    // Try to fetch full details first
    try {
      return await request<LinkageRule>(`/linkage-rules/full/${id}`);
    } catch (e) {
      return await request<LinkageRule>(`/linkage-rules/${id}`);
    }
  },

  // 创建联动规则
  createLinkage: async (linkageData: Partial<LinkageRule>): Promise<LinkageRule> => {
    const payload = { ...linkageData };
    if (payload.hasOwnProperty('id')) delete payload.id;

    // 如果包含 triggers 或 actions，调用后端一并创建/关联的接口
    const hasTriggers = Array.isArray(payload.triggers) && payload.triggers.length > 0;
    const hasActions = Array.isArray(payload.actions) && payload.actions.length > 0;

    if (hasTriggers || hasActions) {
      // 规范化 actions 为后端需要的 ActionInstanceCreateDto 格式
      if (hasActions) {
        payload.actions = payload.actions.map(a => {
          const params = (a.params && typeof a.params === 'object') ? a.params : {};
          return {
            name: a.name || '',
            action_type_id: a.type || a.action_type_id || 0,
            param1: params.param1 || '',
            param2: params.param2 || '',
            param3: params.param3 || null,
            param4: params.param4 || null,
            need_confirm: !!a.need_confirm
          };
        });
      }

      // triggers 前端常用字段为 source_id
      if (hasTriggers) {
        payload.triggers = payload.triggers.map(t => ({ source_id: t.source_id || t.id || 0 }));
      }

      return await request<LinkageRule>('/linkage-rules/full', {
        method: 'POST',
        body: JSON.stringify(payload)
      });
    }

    return await request<LinkageRule>('/linkage-rules', {
      method: 'POST',
      body: JSON.stringify(payload)
    });
  },

  // 更新联动规则
  updateLinkage: async (linkageData: LinkageRule): Promise<LinkageRule> => {
    const payload = { ...linkageData };
    if (payload.hasOwnProperty('id')) delete payload.id;
    return await request<LinkageRule>('/linkage-rules', {
      method: 'PUT',
      body: JSON.stringify(payload)
    });
  },

  // 删除联动规则
  deleteLinkage: async (id: string): Promise<void> => {
    return await request<void>(`/linkage-rules/${id}`, {
      method: 'DELETE'
    });
  },

  // ---------- 触发器 CRUD (t_linkage_trigger) ----------
  getTriggers: async (page: number = 1, size: number = 100): Promise<TriggerListResponse> => {
    return await request<TriggerListResponse>(`/trigger-sources?page=${page}&size=${size}`);
  },

  getTriggerById: async (id: string): Promise<Trigger> => {
    return await request<Trigger>(`/trigger-sources/${id}`);
  },

  createTrigger: async (triggerData: Partial<Trigger>): Promise<Trigger> => {
    const payload = { ...triggerData };
    if (payload.hasOwnProperty('id')) delete payload.id;
    return await request<Trigger>('/trigger-sources', {
      method: 'POST',
      body: JSON.stringify(payload)
    });
  },

  updateTrigger: async (triggerData: Trigger): Promise<Trigger> => {
    const payload = { ...triggerData };
    if (payload.hasOwnProperty('id')) delete payload.id;
    return await request<Trigger>('/trigger-sources', {
      method: 'PUT',
      body: JSON.stringify(payload)
    });
  },

  deleteTrigger: async (id: string): Promise<void> => {
    return await request<void>(`/trigger-sources/${id}`, { method: 'DELETE' });
  },

  // ---------- 动作 CRUD (t_linkage_action) ----------
  getActions: async (page: number = 1, size: number = 100): Promise<ActionListResponse> => {
    return await request<ActionListResponse>(`/action-instances?page=${page}&size=${size}`);
  },

  getActionById: async (id: string): Promise<Action> => {
    return await request<Action>(`/action-instances/${id}`);
  },

  createAction: async (actionData: Partial<Action>): Promise<Action> => {
    const payload = { ...actionData };
    if (payload.hasOwnProperty('id')) delete payload.id;
    return await request<Action>('/action-instances', {
      method: 'POST',
      body: JSON.stringify(payload)
    });
  },

  updateAction: async (actionData: Action): Promise<Action> => {
    const payload = { ...actionData };
    if (payload.hasOwnProperty('id')) delete payload.id;
    return await request<Action>('/action-instances', {
      method: 'PUT',
      body: JSON.stringify(payload)
    });
  },

  deleteAction: async (id: string): Promise<void> => {
    return await request<void>(`/action-instances/${id}`, { method: 'DELETE' });
  },

  // ---------- 事件类型与动作类型 ----------
  getEventTypes: async (page: number = 1, size: number = 100): Promise<EventTypeListResponse> => {
    return await request<EventTypeListResponse>(`/event-types?page=${page}&size=${size}`);
  },

  // 获取触发器类型列表
  getTriggerTypes: async (page: number = 1, size: number = 100): Promise<TriggerTypeListResponse> => {
    return await request<TriggerTypeListResponse>(`/trigger-types?page=${page}&size=${size}`);
  },

  getActionTypes: async (page: number = 1, size: number = 100): Promise<ActionTypeListResponse> => {
    return await request<ActionTypeListResponse>(`/action-types?page=${page}&size=${size}`);
  },

  // ---------- 表达式 CRUD ----------
  getExpressions: async (page: number = 1, size: number = 100): Promise<ExpressionListResponse> => {
    return await request<ExpressionListResponse>(`/expressions?page=${page}&size=${size}`);
  },

  getExpressionById: async (id: string): Promise<Expression> => {
    return await request<Expression>(`/expressions/${id}`);
  },

  createExpression: async (expressionData: Partial<Expression>): Promise<Expression> => {
    const payload = { ...expressionData };
    if (payload.hasOwnProperty('id')) delete payload.id;
    return await request<Expression>('/expressions', {
      method: 'POST',
      body: JSON.stringify(payload)
    });
  },

  updateExpression: async (expressionData: Expression): Promise<Expression> => {
    const payload = { ...expressionData };
    if (payload.hasOwnProperty('id')) delete payload.id;
    return await request<Expression>('/expressions', {
      method: 'PUT',
      body: JSON.stringify(payload)
    });
  },

  deleteExpression: async (id: string): Promise<void> => {
    return await request<void>(`/expressions/${id}`, { method: 'DELETE' });
  }
};