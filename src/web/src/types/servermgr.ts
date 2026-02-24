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
