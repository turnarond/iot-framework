import { request } from './apiClient'

// 服务管理API
export const servermgrApi = {
  // 直接请求servermgr服务的基础URL
  baseUrl: 'http://localhost:8082',

  // 构建完整URL
  buildUrl(path: string) {
    if (path.startsWith('/')) {
      return `${this.baseUrl}${path}`
    }
    return `${this.baseUrl}/${path}`
  },

  // 获取服务列表
  async getServices() {
    try {
      // 直接请求servermgr服务的/services路径
      const url = this.buildUrl('/services')
      const response = await fetch(url, {
        method: 'GET',
        headers: {
          'Content-Type': 'application/json'
        }
      })

      if (!response.ok) {
        throw new Error(`HTTP ${response.status} ${response.statusText}`)
      }

      return await response.json()
    } catch (error) {
      console.error('获取服务列表失败:', error)
      throw error
    }
  },

  // 启动服务
  async startService(serviceName: string) {
    try {
      const url = this.buildUrl(`/services/start/${serviceName}`)
      const response = await fetch(url, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        }
      })

      if (!response.ok) {
        throw new Error(`HTTP ${response.status} ${response.statusText}`)
      }

      return await response.json()
    } catch (error) {
      console.error(`启动服务 ${serviceName} 失败:`, error)
      throw error
    }
  },

  // 停止服务
  async stopService(serviceName: string) {
    try {
      const url = this.buildUrl(`/services/stop/${serviceName}`)
      const response = await fetch(url, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        }
      })

      if (!response.ok) {
        throw new Error(`HTTP ${response.status} ${response.statusText}`)
      }

      return await response.json()
    } catch (error) {
      console.error(`停止服务 ${serviceName} 失败:`, error)
      throw error
    }
  },

  // 重启服务
  async restartService(serviceName: string) {
    try {
      const url = this.buildUrl(`/services/restart/${serviceName}`)
      const response = await fetch(url, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        }
      })

      if (!response.ok) {
        throw new Error(`HTTP ${response.status} ${response.statusText}`)
      }

      return await response.json()
    } catch (error) {
      console.error(`重启服务 ${serviceName} 失败:`, error)
      throw error
    }
  }
}
