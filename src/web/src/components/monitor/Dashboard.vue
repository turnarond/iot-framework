<template>
  <div class="dashboard-container">
    <el-card class="dashboard-card">
      <template #header>
        <div class="card-header">
          <span>系统状态监控</span>
          <el-button type="primary" size="small" @click="refreshData">
            刷新数据
          </el-button>
        </div>
      </template>

      <div class="status-overview">
        <el-row :gutter="20">
          <el-col :span="8">
            <el-card class="status-card normal">
              <div class="status-content">
                <div class="status-value">{{ normalPoints.length }}</div>
                <div class="status-label">正常点位</div>
              </div>
            </el-card>
          </el-col>
          <el-col :span="8">
            <el-card class="status-card warning">
              <div class="status-content">
                <div class="status-value">{{ warningPoints.length }}</div>
                <div class="status-label">警告点位</div>
              </div>
            </el-card>
          </el-col>
          <el-col :span="8">
            <el-card class="status-card error">
              <div class="status-content">
                <div class="status-value">{{ errorPoints.length }}</div>
                <div class="status-label">错误点位</div>
              </div>
            </el-card>
          </el-col>
        </el-row>

        <el-row :gutter="20" style="margin-top: 20px;">
          <el-col :span="6">
            <el-card class="status-card system">
              <div class="status-content">
                <div class="status-value">{{ systemStatus.cpu }}%</div>
                <div class="status-label">CPU使用率</div>
              </div>
            </el-card>
          </el-col>
          <el-col :span="6">
            <el-card class="status-card system">
              <div class="status-content">
                <div class="status-value">{{ systemStatus.memory }}%</div>
                <div class="status-label">内存使用率</div>
              </div>
            </el-card>
          </el-col>
          <el-col :span="6">
            <el-card class="status-card system">
              <div class="status-content">
                <div class="status-value">{{ systemStatus.disk }}%</div>
                <div class="status-label">磁盘使用率</div>
              </div>
            </el-card>
          </el-col>
          <el-col :span="6">
            <el-card class="status-card system">
              <div class="status-content">
                <div class="status-value">{{ systemStatus.network }}</div>
                <div class="status-label">网络连接数</div>
              </div>
            </el-card>
          </el-col>
        </el-row>
      </div>

      <div class="connection-status">
        <el-alert
          :title="connected ? 'WebSocket连接正常' : 'WebSocket连接断开'"
          :type="connected ? 'success' : 'error'"
          show-icon
          :closable="false"
        />
      </div>

      <!-- 点位状态监控 -->
      <div class="point-monitoring" style="margin-top: 30px;">
        <div class="point-monitoring-header">
          <h3>点位状态监控</h3>
          <el-button size="small" @click="togglePointList">
            {{ pointListVisible ? '隐藏点位列表' : '显示点位列表' }}
          </el-button>
        </div>

        <div class="point-monitoring-content">
          <!-- 左侧监控区域 -->
          <el-col :span="pointListVisible ? 16 : 24">
            <div class="monitoring-area">
              <div class="current-values" v-if="monitoredPoints.length > 0">
                <el-row :gutter="20">
                  <el-col :span="8" v-for="pointName in monitoredPoints" :key="pointName">
                    <el-card class="value-card" :class="getPointStatus(pointName)">
                      <div class="value-content">
                        <div class="value-label">{{ pointName }}</div>
                        <div class="value-number">{{ getPointValue(pointName) || '--' }}</div>
                        <div class="value-time">{{ getPointTime(pointName) || '--' }}</div>
                        <div class="value-actions">
                          <el-button 
                            size="small" 
                            type="danger" 
                            @click="unsubscribePoint(pointName)"
                            style="margin-right: 5px;"
                          >
                            取消监控
                          </el-button>
                          <el-button 
                            v-if="isControlPoint(pointName)"
                            size="small" 
                            type="primary"
                            @click="showControlDialog(pointName)"
                          >
                            控制
                          </el-button>
                        </div>
                      </div>
                    </el-card>
                  </el-col>
                </el-row>
              </div>
              <div v-else class="no-data">
                <el-empty description="暂无监控点位" />
              </div>
            </div>
          </el-col>

          <!-- 右侧点位列表 -->
          <el-col :span="8" v-if="pointListVisible">
            <div class="point-list-area">
              <div class="point-list-header">
                <h4>点位列表</h4>
                <el-input 
                  v-model="pointSearch" 
                  placeholder="搜索点位" 
                  clearable 
                  size="small"
                  style="width: 150px;"
                />
              </div>
              <div class="point-list-content">
                <el-table :data="filteredPoints" style="width: 100%" size="small">
                  <el-table-column prop="name" label="点位名称" />
                  <el-table-column prop="device_name" label="设备" width="120" />
                  <el-table-column prop="datatype_cname" label="数据类型" width="100" />
                  <el-table-column label="操作" width="120">
                    <template #default="scope">
                      <el-button 
                        size="small" 
                        :type="monitoredPoints.includes(scope.row.name) ? 'warning' : 'success'"
                        @click="toggleSubscribe(scope.row)"
                      >
                        {{ monitoredPoints.includes(scope.row.name) ? '已监控' : '监控' }}
                      </el-button>
                    </template>
                  </el-table-column>
                </el-table>
              </div>
            </div>
          </el-col>
        </div>
      </div>

      <div class="recent-updates">
        <h3>最近更新</h3>
        <el-table :data="recentUpdates" style="width: 100%">
          <el-table-column prop="name" label="点位名称" width="200" />
          <el-table-column prop="value" label="值" width="150" />
          <el-table-column prop="status" label="状态" width="100">
            <template #default="scope">
              <el-tag :type="getStatusType(scope.row.status)">
                {{ scope.row.status }}
              </el-tag>
            </template>
          </el-table-column>
          <el-table-column prop="timestamp" label="更新时间">
            <template #default="scope">
              {{ formatTime(scope.row.timestamp) }}
            </template>
          </el-table-column>
        </el-table>
      </div>

      <div class="recent-alarms" style="margin-top: 30px;">
        <h3>最近报警</h3>
        <el-table :data="recentAlarms" style="width: 100%">
          <el-table-column prop="level" label="级别" width="100">
            <template #default="scope">
              <el-tag :type="scope.row.level === 'error' ? 'danger' : scope.row.level === 'warning' ? 'warning' : 'info'">
                {{ scope.row.level }}
              </el-tag>
            </template>
          </el-table-column>
          <el-table-column prop="message" label="报警信息" />
          <el-table-column prop="deviceId" label="设备ID" width="120" />
          <el-table-column prop="timestamp" label="发生时间">
            <template #default="scope">
              {{ formatTime(scope.row.timestamp) }}
            </template>
          </el-table-column>
        </el-table>
      </div>
    </el-card>

    <!-- 控制对话框 -->
    <el-dialog
      v-model="controlDialogVisible"
      :title="`控制点位: ${currentControlPoint}`"
      width="400px"
    >
      <div class="control-form">
        <el-form label-width="80px">
          <el-form-item label="设置值">
            <el-input 
              v-model="controlValue"
              placeholder="请输入控制值"
            />
          </el-form-item>
        </el-form>
      </div>
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="controlDialogVisible = false">取消</el-button>
          <el-button type="primary" @click="sendControlCommand">发送</el-button>
        </span>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { computed, onMounted, ref } from 'vue'
import { useMonitorStore } from '@/stores/monitor'
import { pointService } from '@/services/pointService'
import { pointApi } from '@/services/pointApi'

const monitorStore = useMonitorStore()

// 系统状态
const normalPoints = computed(() => monitorStore.normalPoints)
const warningPoints = computed(() => monitorStore.warningPoints)
const errorPoints = computed(() => monitorStore.errorPoints)
const connected = computed(() => monitorStore.connected)
const systemStatus = computed(() => monitorStore.systemStatus)
const recentUpdates = computed(() => {
  return Object.values(monitorStore.points)
    .sort((a, b) => b.timestamp - a.timestamp)
    .slice(0, 10)
})
const recentAlarms = computed(() => {
  return monitorStore.alarms.slice(0, 5)
})

// 点位状态监控
const pointListVisible = ref(true)
const pointSearch = ref('')
const allPoints = ref<any[]>([])
const monitoredPoints = ref<string[]>([])
const loading = ref(false)

// 控制对话框
const controlDialogVisible = ref(false)
const currentControlPoint = ref('')
const controlValue = ref('')

// 过滤后的点位列表
const filteredPoints = computed(() => {
  if (!pointSearch.value) {
    return allPoints.value
  }
  return allPoints.value.filter(point => 
    point.name.toLowerCase().includes(pointSearch.value.toLowerCase())
  )
})

// 获取点位状态
const getPointStatus = (pointName: string) => {
  const point = monitorStore.points[pointName]
  return point?.status || 'normal'
}

// 获取点位值
const getPointValue = (pointName: string) => {
  const point = monitorStore.points[pointName]
  return point !== undefined ? point.value : null
}

// 获取点位更新时间
const getPointTime = (pointName: string) => {
  const point = monitorStore.points[pointName]
  return point ? formatTime(point.timestamp) : null
}

// 判断是否为控制点位
const isControlPoint = (pointName: string) => {
  const point = allPoints.value.find(p => p.name === pointName)
  return point?.enable_control === true
}

// 切换点位列表显示
const togglePointList = () => {
  pointListVisible.value = !pointListVisible.value
}

// 切换点位订阅状态
const toggleSubscribe = (point: any) => {
  if (monitoredPoints.value.includes(point.name)) {
    unsubscribePoint(point.name)
  } else {
    subscribePoint(point)
  }
}

// 订阅点位
const subscribePoint = (point: any) => {
  pointService.subscribePoint(point.name)
  monitoredPoints.value.push(point.name)
  console.log(`订阅点位: ${point.name}`)
}

// 取消订阅点位
const unsubscribePoint = (pointName: string) => {
  pointService.unsubscribePoint(pointName)
  monitoredPoints.value = monitoredPoints.value.filter(name => name !== pointName)
  console.log(`取消订阅点位: ${pointName}`)
}

// 显示控制对话框
const showControlDialog = (pointName: string) => {
  currentControlPoint.value = pointName
  controlValue.value = getPointValue(pointName) || ''
  controlDialogVisible.value = true
}

// 发送控制命令
const sendControlCommand = async () => {
  if (!currentControlPoint.value || !controlValue.value) {
    return
  }

  try {
    const success = await pointService.sendControlCommand(currentControlPoint.value, controlValue.value)
    if (success) {
      controlDialogVisible.value = false
      // 刷新数据
      refreshData()
    } else {
      alert('控制命令发送失败')
    }
  } catch (error) {
    console.error('控制命令发送失败:', error)
    alert('控制命令发送失败')
  }
}

// 获取状态类型
const getStatusType = (status: string) => {
  switch (status) {
    case 'normal':
      return 'success'
    case 'warning':
      return 'warning'
    case 'error':
      return 'danger'
    default:
      return 'info'
  }
}

// 格式化时间
const formatTime = (timestamp: number) => {
  const date = new Date(timestamp)
  return date.toLocaleString()
}

// 刷新数据
const refreshData = async () => {
  loading.value = true
  try {
    await loadAllPoints()
    monitorStore.refreshData()
    console.log('刷新数据')
  } catch (error) {
    console.error('刷新数据失败:', error)
  } finally {
    loading.value = false
  }
}

// 加载所有点位
const loadAllPoints = async () => {
  loading.value = true
  try {
    const response = await pointApi.getPointsWithDevice(1, 100) // 使用较大的size获取所有点位
    allPoints.value = response.items
  } catch (error) {
    console.error('加载点位列表失败:', error)
  } finally {
    loading.value = false
  }
}

onMounted(() => {
  monitorStore.initWebSocket()
  // 加载点位列表
  loadAllPoints()
  
  // 订阅一些默认的点位
  // monitorStore.subscribePoint('temperature')
  // monitorStore.subscribePoint('humidity')
  // monitorStore.subscribePoint('pressure')
  
  // // 模拟数据更新（用于开发和测试）
  // monitorStore.simulateDataUpdate()
  // // 每5秒模拟一次数据更新
  // setInterval(() => {
  //   monitorStore.simulateDataUpdate()
  // }, 5000)
})
</script>

<style scoped>
.dashboard-container {
  padding: 20px;
  min-height: 100vh;
  max-height: calc(100vh - 40px);
  overflow-y: auto;
}

.dashboard-container::-webkit-scrollbar {
  width: 8px;
}

.dashboard-container::-webkit-scrollbar-track {
  background: #f1f1f1;
  border-radius: 4px;
}

.dashboard-container::-webkit-scrollbar-thumb {
  background: #c1c1c1;
  border-radius: 4px;
}

.dashboard-container::-webkit-scrollbar-thumb:hover {
  background: #a8a8a8;
}

:global(.dark-mode) .dashboard-container::-webkit-scrollbar-track {
  background: #333;
}

:global(.dark-mode) .dashboard-container::-webkit-scrollbar-thumb {
  background: #555;
}

:global(.dark-mode) .dashboard-container::-webkit-scrollbar-thumb:hover {
  background: #666;
}

.dashboard-card {
  margin-bottom: 20px;
  transition: all 0.3s ease;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.status-overview {
  margin: 20px 0;
}

.status-card {
  height: 120px;
  border-radius: 8px;
  overflow: hidden;
  transition: all 0.3s ease;
}

.status-card.normal {
  border-left: 4px solid #67c23a;
}

.status-card.warning {
  border-left: 4px solid #e6a23c;
}

.status-card.error {
  border-left: 4px solid #f56c6c;
}

.status-card.system {
  border-left: 4px solid #409EFF;
}

.status-content {
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  height: 100%;
}

.status-value {
  font-size: 32px;
  font-weight: bold;
  margin-bottom: 8px;
  transition: all 0.3s ease;
}

.status-label {
  font-size: 14px;
  color: #606266;
  transition: all 0.3s ease;
}

.connection-status {
  margin: 20px 0;
}

.recent-updates {
  margin-top: 30px;
}

.recent-updates h3 {
  margin-bottom: 15px;
  font-size: 16px;
  font-weight: bold;
  color: #303133;
  transition: all 0.3s ease;
}

.recent-alarms h3 {
  margin-bottom: 15px;
  font-size: 16px;
  font-weight: bold;
  color: #303133;
  transition: all 0.3s ease;
}

/* 点位状态监控 */
.point-monitoring {
  margin-top: 30px;
}

.point-monitoring-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.point-monitoring-header h3 {
  margin: 0;
  font-size: 16px;
  font-weight: bold;
  color: #303133;
}

.point-monitoring-content {
  display: flex;
  gap: 20px;
}

.monitoring-area {
  flex: 2;
  max-height: 500px;
  overflow-y: auto;
  padding-right: 10px;
  scrollbar-width: thin;
  scrollbar-color: #409EFF #f0f2f5;
}

.monitoring-area::-webkit-scrollbar {
  width: 6px;
}

.monitoring-area::-webkit-scrollbar-track {
  background: #f0f2f5;
  border-radius: 3px;
}

.monitoring-area::-webkit-scrollbar-thumb {
  background: #409EFF;
  border-radius: 3px;
}

.monitoring-area::-webkit-scrollbar-thumb:hover {
  background: #66b1ff;
}

.point-list-area {
  flex: 1;
  border: 1px solid #ebeef5;
  border-radius: 8px;
  overflow: hidden;
}

.point-list-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 10px 15px;
  background-color: #f5f7fa;
  border-bottom: 1px solid #ebeef5;
}

.point-list-header h4 {
  margin: 0;
  font-size: 14px;
  font-weight: bold;
  color: #303133;
}

.point-list-content {
  max-height: 400px;
  overflow-y: auto;
}

/* 监控区域点位卡片 */
.current-values {
  margin-bottom: 30px;
}

.value-card {
  height: 180px;
  border-radius: 8px;
  overflow: hidden;
  transition: all 0.3s ease;
}

.value-card.normal {
  border-left: 4px solid #67c23a;
}

.value-card.warning {
  border-left: 4px solid #e6a23c;
}

.value-card.error {
  border-left: 4px solid #f56c6c;
}

.value-content {
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  height: 100%;
}

.value-label {
  font-size: 14px;
  color: #606266;
  margin-bottom: 8px;
}

.value-number {
  font-size: 36px;
  font-weight: bold;
  margin-bottom: 12px;
}

.value-time {
  font-size: 12px;
  color: #909399;
  margin-bottom: 15px;
}

.value-actions {
  display: flex;
  gap: 5px;
}

.no-data {
  padding: 40px 0;
  text-align: center;
}

/* 控制对话框 */
.control-form {
  margin-top: 20px;
}

.dialog-footer {
  display: flex;
  justify-content: flex-end;
  gap: 10px;
}

/* 深色模式 */
:global(.dark-mode) .status-value {
  color: #e4e7ed;
}

:global(.dark-mode) .status-label {
  color: #c0c4cc;
}

:global(.dark-mode) .recent-updates h3 {
  color: #e4e7ed;
}

:global(.dark-mode) .recent-alarms h3 {
  color: #e4e7ed;
}

:global(.dark-mode) .point-monitoring-header h3 {
  color: #e4e7ed;
}

:global(.dark-mode) .point-list-header {
  background-color: #1f2d3d;
  border-bottom: 1px solid #304156;
}

:global(.dark-mode) .point-list-header h4 {
  color: #e4e7ed;
}

:global(.dark-mode) .point-list-area {
  border: 1px solid #304156;
}

:global(.dark-mode) .value-label {
  color: #c0c4cc;
}

:global(.dark-mode) .value-time {
  color: #909399;
}
</style>
