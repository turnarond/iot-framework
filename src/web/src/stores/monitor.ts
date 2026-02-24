import { defineStore } from 'pinia'
import { wsClient, type PointUpdate, type SystemStatusUpdate, type DeviceStatusUpdate, type AlarmUpdate } from '@/services/websocket'
import axios from 'axios'

export interface PointState {
  name: string
  value: string
  timestamp: number
  driver?: string
  device?: string
  status: 'normal' | 'warning' | 'error'
}

export interface SystemState {
  cpu: number
  memory: number
  disk: number
  network: number
  timestamp: number
}

export interface DeviceState {
  deviceId: string
  status: 'online' | 'offline' | 'error'
  responseTime: number
  timestamp: number
}

export interface AlarmState {
  alarmId: string
  type: string
  level: 'info' | 'warning' | 'error'
  message: string
  timestamp: number
  deviceId?: string
  pointName?: string
}

export const useMonitorStore = defineStore('monitor', {
  state: () => ({
    points: {} as Record<string, PointState>,
    systemStatus: {
      cpu: 0,
      memory: 0,
      disk: 0,
      network: 0,
      timestamp: Date.now()
    } as SystemState,
    devices: {} as Record<string, DeviceState>,
    alarms: [] as AlarmState[],
    connected: false,
    loading: false,
    error: '',
    // 配置项
    config: {
      autoRefresh: true,
      refreshInterval: 5000,
      showSystemStatus: true,
      showDeviceStatus: true,
      showAlarms: true,
      alarmHistoryLimit: 100
    }
  }),

  getters: {
    allPoints: (state) => Object.values(state.points),
    normalPoints: (state) => Object.values(state.points).filter(p => p.status === 'normal'),
    warningPoints: (state) => Object.values(state.points).filter(p => p.status === 'warning'),
    errorPoints: (state) => Object.values(state.points).filter(p => p.status === 'error'),
    pointCount: (state) => Object.keys(state.points).length,
    
    allDevices: (state) => Object.values(state.devices),
    onlineDevices: (state) => Object.values(state.devices).filter(d => d.status === 'online'),
    offlineDevices: (state) => Object.values(state.devices).filter(d => d.status === 'offline'),
    errorDevices: (state) => Object.values(state.devices).filter(d => d.status === 'error'),
    deviceCount: (state) => Object.keys(state.devices).length,
    
    allAlarms: (state) => state.alarms,
    infoAlarms: (state) => state.alarms.filter(a => a.level === 'info'),
    warningAlarms: (state) => state.alarms.filter(a => a.level === 'warning'),
    errorAlarms: (state) => state.alarms.filter(a => a.level === 'error'),
    recentAlarms: (state) => state.alarms.slice(0, 20),
    
    // 系统资源使用率
    cpuUsage: (state) => state.systemStatus.cpu,
    memoryUsage: (state) => state.systemStatus.memory,
    diskUsage: (state) => state.systemStatus.disk,
    networkConnections: (state) => state.systemStatus.network,
    
    // 统计信息
    statusSummary: (state) => {
      return {
        points: {
          total: Object.keys(state.points).length,
          normal: Object.values(state.points).filter(p => p.status === 'normal').length,
          warning: Object.values(state.points).filter(p => p.status === 'warning').length,
          error: Object.values(state.points).filter(p => p.status === 'error').length
        },
        devices: {
          total: Object.keys(state.devices).length,
          online: Object.values(state.devices).filter(d => d.status === 'online').length,
          offline: Object.values(state.devices).filter(d => d.status === 'offline').length,
          error: Object.values(state.devices).filter(d => d.status === 'error').length
        },
        alarms: {
          total: state.alarms.length,
          info: state.alarms.filter(a => a.level === 'info').length,
          warning: state.alarms.filter(a => a.level === 'warning').length,
          error: state.alarms.filter(a => a.level === 'error').length
        }
      }
    }
  },

  actions: {
    initWebSocket() {
      // 添加不同类型的事件处理器
      wsClient.addHandler('POINT_UPDATE', this.handlePointUpdate.bind(this))
      wsClient.addHandler('SYSTEM_STATUS_UPDATE', this.handleSystemStatusUpdate.bind(this))
      wsClient.addHandler('DEVICE_STATUS_UPDATE', this.handleDeviceStatusUpdate.bind(this))
      wsClient.addHandler('ALARM_UPDATE', this.handleAlarmUpdate.bind(this))
      this.connected = true
      
      // 订阅系统状态
      wsClient.subscribeSystemStatus()
      // 订阅设备状态
      wsClient.subscribeDeviceStatus()
      // 订阅报警
      wsClient.subscribeAlarms()
      
      // 获取点位列表
      this.fetchPoints()
    },

    handlePointUpdate(update: PointUpdate) {
      const status = this.calculateStatus(update.value)
      this.points[update.name] = {
        name: update.name,
        value: update.value,
        timestamp: update.timestamp,
        driver: update.driver,
        device: update.device,
        status
      }
    },

    handleSystemStatusUpdate(update: SystemStatusUpdate) {
      this.systemStatus = {
        cpu: update.cpu,
        memory: update.memory,
        disk: update.disk,
        network: update.network,
        timestamp: update.timestamp
      }
    },

    handleDeviceStatusUpdate(update: DeviceStatusUpdate) {
      this.devices[update.deviceId] = {
        deviceId: update.deviceId,
        status: update.status,
        responseTime: update.responseTime,
        timestamp: update.timestamp
      }
    },

    handleAlarmUpdate(update: AlarmUpdate) {
      // 添加新报警到开头
      this.alarms.unshift({
        alarmId: update.alarmId,
        type: update.type,
        level: update.level,
        message: update.message,
        timestamp: update.timestamp,
        deviceId: update.deviceId,
        pointName: update.pointName
      })
      
      // 限制报警历史记录数量
      if (this.alarms.length > this.config.alarmHistoryLimit) {
        this.alarms = this.alarms.slice(0, this.config.alarmHistoryLimit)
      }
    },

    calculateStatus(value: string | number): 'normal' | 'warning' | 'error' {
      // 根据实际业务逻辑计算状态
      // 这里只是简单示例，实际需要根据点位类型和阈值判断
      const numValue = typeof value === 'string' ? parseFloat(value) : Number(value)
      if (isNaN(numValue)) return 'normal'
      if (numValue > 80) return 'error'
      if (numValue > 60) return 'warning'
      return 'normal'
    },

    subscribePoint(pointId: string) {
      wsClient.subscribePoint(pointId)
    },

    unsubscribePoint(pointId: string) {
      wsClient.unsubscribePoint(pointId)
      // 从本地状态中移除
      delete this.points[pointId]
    },

    subscribeAllPoints(pointIds: string[]) {
      pointIds.forEach(id => this.subscribePoint(id))
    },

    subscribeDeviceStatus(deviceId?: string) {
      wsClient.subscribeDeviceStatus(deviceId)
    },

    subscribeSystemStatus() {
      wsClient.subscribeSystemStatus()
    },

    subscribeAlarms(level?: 'info' | 'warning' | 'error') {
      wsClient.subscribeAlarms(level)
    },

    clearPoints() {
      this.points = {}
    },

    clearDevices() {
      this.devices = {}
    },

    clearAlarms() {
      this.alarms = []
    },

    setError(error: string) {
      this.error = error
    },

    clearError() {
      this.error = ''
    },

    updateConfig(newConfig: Partial<typeof this.config>) {
      this.config = { ...this.config, ...newConfig }
    },

    refreshData() {
      // 手动刷新数据
      console.log('Refreshing data...')
      this.fetchPoints()
    },

    // 从HTTP API获取点位列表
    async fetchPoints(prefix: string = '') {
      this.loading = true
      this.error = ''
      try {
        const response = await axios.get(`http://localhost:8081/api/v1/points?prefix=${prefix}`)
        if (response.data && response.data.points) {
          // 订阅所有获取到的点位
          const pointNames = Object.keys(response.data.points)
          this.subscribeAllPoints(pointNames)
          console.log(`Fetched and subscribed to ${pointNames.length} points`)
        }
      } catch (error) {
        this.error = `获取点位列表失败: ${error instanceof Error ? error.message : '未知错误'}`
        console.error('Error fetching points:', error)
        // 移除模拟数据逻辑，直接使用后端数据
      } finally {
        this.loading = false
      }
    }
  }
})
