<template>
  <div class="servermgr-container">
    <el-card class="servermgr-card">
      <template #header>
        <div class="card-header">
          <span>基础信息</span>
          <el-button type="primary" size="small" @click="refreshServices">
            刷新数据
          </el-button>
        </div>
      </template>

      <div class="license-info">
        <el-collapse>
          <el-collapse-item title="许可状态" name="1">
            <el-row :gutter="20">
              <el-col :span="12">
                <el-table :data="licenseData" style="width: 100%">
                  <el-table-column prop="item" label="项目" width="180" />
                  <el-table-column prop="content" label="内容" />
                </el-table>
              </el-col>
            </el-row>
          </el-collapse-item>
          <el-collapse-item title="服务信息" name="2">
            <el-row :gutter="20">
              <el-col :span="6">
                <el-card class="service-info-card">
                  <div class="service-info-content">
                    <div class="service-info-value">{{ services.length }}</div>
                    <div class="service-info-label">服务总数</div>
                  </div>
                </el-card>
              </el-col>
              <el-col :span="6">
                <el-card class="service-info-card running">
                  <div class="service-info-content">
                    <div class="service-info-value">{{ runningServices }}</div>
                    <div class="service-info-label">运行中</div>
                  </div>
                </el-card>
              </el-col>
              <el-col :span="6">
                <el-card class="service-info-card stopped">
                  <div class="service-info-content">
                    <div class="service-info-value">{{ stoppedServices }}</div>
                    <div class="service-info-label">已停止</div>
                  </div>
                </el-card>
              </el-col>
              <el-col :span="6">
                <el-card class="service-info-card failed">
                  <div class="service-info-content">
                    <div class="service-info-value">{{ failedServices }}</div>
                    <div class="service-info-label">失败</div>
                  </div>
                </el-card>
              </el-col>
            </el-row>
          </el-collapse-item>
        </el-collapse>
      </div>

      <div class="process-management" style="margin-top: 30px;">
        <h3>进程管理</h3>
        <el-table :data="services" style="width: 100%" stripe>
          <el-table-column prop="name" label="服务名称" width="180" />
          <el-table-column prop="status" label="状态" width="100">
            <template #default="scope">
              <el-tag :type="getServiceStatusType(scope.row.status)">
                {{ getServiceStatusText(scope.row.status) }}
              </el-tag>
            </template>
          </el-table-column>
          <el-table-column prop="startTime" label="启动时间">
            <template #default="scope">
              {{ scope.row.startTime || '--' }}
            </template>
          </el-table-column>
          <el-table-column prop="restartCount" label="启动次数" width="100" />
          <el-table-column label="操作" width="180">
            <template #default="scope">
              <el-button 
                size="small" 
                type="primary" 
                @click="startService(scope.row.name)"
                :disabled="scope.row.status === 'running' || scope.row.status === 'starting'"
                style="margin-right: 5px;"
              >
                运行
              </el-button>
              <el-button 
                size="small" 
                type="danger" 
                @click="stopService(scope.row.name)"
                :disabled="scope.row.status === 'stopped' || scope.row.status === 'stopping'"
              >
                停止
              </el-button>
            </template>
          </el-table-column>
        </el-table>
      </div>

      <!-- 服务详情对话框 -->
      <el-dialog
        v-model="detailDialogVisible"
        :title="`服务详情: ${currentService?.name}`"
        width="600px"
      >
        <div class="service-detail">
          <el-descriptions :column="1" border>
            <el-descriptions-item label="服务ID">{{ currentService?.id }}</el-descriptions-item>
            <el-descriptions-item label="服务名称">{{ currentService?.name }}</el-descriptions-item>
            <el-descriptions-item label="状态">{{ currentService?.status ? getServiceStatusText(currentService.status) : '--' }}</el-descriptions-item>
            <el-descriptions-item label="启动时间">{{ currentService?.startTime || '--' }}</el-descriptions-item>
            <el-descriptions-item label="启动次数">{{ currentService?.restartCount || 0 }}</el-descriptions-item>
            <el-descriptions-item label="可执行路径">{{ currentService?.exePath || '--' }}</el-descriptions-item>
            <el-descriptions-item label="工作目录">{{ currentService?.workingDir || '--' }}</el-descriptions-item>
            <el-descriptions-item label="命令行参数">
              <div v-if="currentService?.args && currentService.args.length > 0">
                <span v-for="(arg, index) in currentService.args" :key="index" class="arg-item">
                  {{ arg }}
                </span>
              </div>
              <span v-else>--</span>
            </el-descriptions-item>
            <el-descriptions-item label="环境变量">
              <div v-if="currentService?.env && Object.keys(currentService.env).length > 0">
                <div v-for="(value, key) in currentService.env" :key="key" class="env-item">
                  <span class="env-key">{{ key }}:</span>
                  <span class="env-value">{{ value }}</span>
                </div>
              </div>
              <span v-else>--</span>
            </el-descriptions-item>
          </el-descriptions>
        </div>
        <template #footer>
          <span class="dialog-footer">
            <el-button @click="detailDialogVisible = false">关闭</el-button>
          </span>
        </template>
      </el-dialog>
    </el-card>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted } from 'vue'
import { ElMessage } from 'element-plus'
import { servermgrService } from '@/services/servermgrService'

// 服务信息接口
interface ServiceInfo {
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

// 服务列表
const services = ref<ServiceInfo[]>([])
const loading = ref(false)

// 详情对话框
const detailDialogVisible = ref(false)
const currentService = ref<ServiceInfo | null>(null)

// 许可数据
const licenseData = ref([
  { item: '许可标识图', content: '永久' },
  { item: '许可项目', content: 'ycd' },
  { item: '许可最大天数', content: '49' },
  { item: '许可可部署数', content: '49' },
  { item: '产品版本', content: '6.0.0' },
  { item: '产品-当前运行时间', content: new Date().toLocaleString() }
])

// 计算运行中、已停止和失败的服务数量
const runningServices = computed(() => {
  return services.value.filter(service => service.status === 'running').length
})

const stoppedServices = computed(() => {
  return services.value.filter(service => service.status === 'stopped').length
})

const failedServices = computed(() => {
  return services.value.filter(service => service.status === 'failed').length
})

// 获取服务状态文本
const getServiceStatusText = (status: string) => {
  return servermgrService.getServiceStatusText(status)
}

// 获取服务状态类型
const getServiceStatusType = (status: string) => {
  return servermgrService.getServiceStatusType(status)
}

// 刷新服务列表
const refreshServices = async () => {
  loading.value = true
  try {
    const serviceList = await servermgrService.getServices()
    services.value = serviceList
  } catch (error) {
    console.error('刷新服务列表失败:', error)
  } finally {
    loading.value = false
  }
}

// 启动服务
const startService = async (serviceName: string) => {
  try {
    const success = await servermgrService.startService(serviceName)
    if (success) {
      // 刷新服务列表
      await refreshServices()
      // 显示成功消息
      ElMessage.success('服务启动成功')
    } else {
      ElMessage.error('服务启动失败')
    }
  } catch (error) {
    console.error('启动服务失败:', error)
    ElMessage.error('服务启动失败')
  }
}

// 停止服务
const stopService = async (serviceName: string) => {
  try {
    const success = await servermgrService.stopService(serviceName)
    if (success) {
      // 刷新服务列表
      await refreshServices()
      // 显示成功消息
      ElMessage.success('服务停止成功')
    } else {
      ElMessage.error('服务停止失败')
    }
  } catch (error) {
    console.error('停止服务失败:', error)
    ElMessage.error('服务停止失败')
  }
}

// 显示服务详情
const showServiceDetail = (service: ServiceInfo) => {
  currentService.value = service
  detailDialogVisible.value = true
}

onMounted(() => {
  // 初始加载服务列表
  refreshServices()
})
</script>

<style scoped>
.servermgr-container {
  padding: 20px;
  height: 100vh;
  overflow-y: auto;
  box-sizing: border-box;
}

.servermgr-container::-webkit-scrollbar {
  width: 8px;
}

.servermgr-container::-webkit-scrollbar-track {
  background: #f1f1f1;
  border-radius: 4px;
}

.servermgr-container::-webkit-scrollbar-thumb {
  background: #c1c1c1;
  border-radius: 4px;
}

.servermgr-container::-webkit-scrollbar-thumb:hover {
  background: #a8a8a8;
}

:global(.dark-mode) .servermgr-container::-webkit-scrollbar-track {
  background: #333;
}

:global(.dark-mode) .servermgr-container::-webkit-scrollbar-thumb {
  background: #555;
}

:global(.dark-mode) .servermgr-container::-webkit-scrollbar-thumb:hover {
  background: #666;
}

.servermgr-card {
  margin-bottom: 20px;
  transition: all 0.3s ease;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.license-info {
  margin: 20px 0;
}

.service-info-card {
  height: 100px;
  border-radius: 8px;
  overflow: hidden;
  transition: all 0.3s ease;
  border-left: 4px solid #409EFF;
}

.service-info-card.running {
  border-left-color: #67c23a;
}

.service-info-card.stopped {
  border-left-color: #909399;
}

.service-info-card.failed {
  border-left-color: #f56c6c;
}

.service-info-content {
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  height: 100%;
}

.service-info-value {
  font-size: 28px;
  font-weight: bold;
  margin-bottom: 8px;
  transition: all 0.3s ease;
}

.service-info-label {
  font-size: 14px;
  color: #606266;
  transition: all 0.3s ease;
}

.process-management {
  margin-top: 30px;
}

.process-management h3 {
  margin-bottom: 15px;
  font-size: 16px;
  font-weight: bold;
  color: #303133;
  transition: all 0.3s ease;
}

.service-detail {
  margin-top: 20px;
}

.arg-item {
  display: block;
  padding: 2px 0;
  font-family: monospace;
  background-color: #f5f7fa;
  padding: 4px 8px;
  border-radius: 4px;
  margin: 2px 0;
}

.env-item {
  display: flex;
  margin: 4px 0;
}

.env-key {
  font-weight: bold;
  margin-right: 8px;
}

.env-value {
  font-family: monospace;
  background-color: #f5f7fa;
  padding: 2px 6px;
  border-radius: 4px;
  flex: 1;
}

/* 深色模式 */
:global(.dark-mode) .service-info-value {
  color: #e4e7ed;
}

:global(.dark-mode) .service-info-label {
  color: #c0c4cc;
}

:global(.dark-mode) .process-management h3 {
  color: #e4e7ed;
}

:global(.dark-mode) .arg-item {
  background-color: #1f2d3d;
  color: #e4e7ed;
}

:global(.dark-mode) .env-value {
  background-color: #1f2d3d;
  color: #e4e7ed;
}
</style>
