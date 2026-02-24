import { servermgrApi } from './servermgrApi'

// 服务信息接口
export interface ServiceInfo {
  id: string
  name: string
  status: string
  startTime: string
  restartCount: number
  exePath: string
  args: string[]
  env: Record<string, string>
  workingDir: string
}

// 服务管理服务
export const servermgrService = {
  // 获取服务列表
  async getServices(): Promise<ServiceInfo[]> {
    try {
      const services = await servermgrApi.getServices()
      return services
    } catch (error) {
      console.error('获取服务列表失败:', error)
      return []
    }
  },

  // 启动服务
  async startService(serviceName: string): Promise<boolean> {
    try {
      await servermgrApi.startService(serviceName)
      return true
    } catch (error) {
      console.error(`启动服务 ${serviceName} 失败:`, error)
      return false
    }
  },

  // 停止服务
  async stopService(serviceName: string): Promise<boolean> {
    try {
      await servermgrApi.stopService(serviceName)
      return true
    } catch (error) {
      console.error(`停止服务 ${serviceName} 失败:`, error)
      return false
    }
  },

  // 重启服务
  async restartService(serviceName: string): Promise<boolean> {
    try {
      await servermgrApi.restartService(serviceName)
      return true
    } catch (error) {
      console.error(`重启服务 ${serviceName} 失败:`, error)
      return false
    }
  },

  // 获取服务状态文本
  getServiceStatusText(status: string): string {
    const statusMap: Record<string, string> = {
      'running': '运行中',
      'stopped': '已停止',
      'starting': '启动中',
      'stopping': '停止中',
      'failed': '失败'
    }
    return statusMap[status] || status
  },

  // 获取服务状态类型（用于Element Plus的标签类型）
  getServiceStatusType(status: string): string {
    const typeMap: Record<string, string> = {
      'running': 'success',
      'stopped': 'info',
      'starting': 'warning',
      'stopping': 'warning',
      'failed': 'danger'
    }
    return typeMap[status] || 'info'
  }
}
