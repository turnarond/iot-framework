// API Hooks for Linkage and other services
import { linkageApi } from '../services/linkageApi';
import { alarmApi } from '../services/alarmApi';
import { dictApi } from '../services/dictApi';
import { pointApi } from '../services/pointApi';
import { deviceApi } from '../services/deviceApi';
import { driverApi } from '../services/driverApi';
import { userApi } from '../services/userApi';
import { systemApi } from '../services/systemApi';

// Linkage API Hook
export function useLinkageApi() {
  return {
    // Linkage Rules
    getLinkages: linkageApi.getLinkages,
    getLinkageById: linkageApi.getLinkageById,
    createLinkage: linkageApi.createLinkage,
    updateLinkage: linkageApi.updateLinkage,
    deleteLinkage: linkageApi.deleteLinkage,
    
    // Triggers
    getTriggers: linkageApi.getTriggers,
    getTriggerById: linkageApi.getTriggerById,
    createTrigger: linkageApi.createTrigger,
    updateTrigger: linkageApi.updateTrigger,
    deleteTrigger: linkageApi.deleteTrigger,
    
    // Actions
    getActions: linkageApi.getActions,
    getActionById: linkageApi.getActionById,
    createAction: linkageApi.createAction,
    updateAction: linkageApi.updateAction,
    deleteAction: linkageApi.deleteAction,
    
    // Event Types and Action Types
    getEventTypes: linkageApi.getEventTypes,
    getTriggerTypes: linkageApi.getTriggerTypes,
    getActionTypes: linkageApi.getActionTypes,
    
    // Expressions
    getExpressions: linkageApi.getExpressions,
    getExpressionById: linkageApi.getExpressionById,
    createExpression: linkageApi.createExpression,
    updateExpression: linkageApi.updateExpression,
    deleteExpression: linkageApi.deleteExpression
  };
}

// Alarm API Hook
export function useAlarmApi() {
  return alarmApi;
}

// Dict API Hook
export function useDictApi() {
  return dictApi;
}

// Point API Hook
export function usePointApi() {
  return pointApi;
}

// Device API Hook
export function useDeviceApi() {
  return deviceApi;
}

// Driver API Hook
export function useDriverApi() {
  return driverApi;
}

// User API Hook
export function useUserApi() {
  return userApi;
}

// System API Hook
export function useSystemApi() {
  return systemApi;
}
