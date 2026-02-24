<template>
  <div class="realtime-monitor">
    <el-card class="monitor-card">
      <template #header>
        <div class="card-header">
          <span>实时数据监控</span>
          <div class="header-controls">
            <el-select v-model="selectedPoints" multiple placeholder="选择点位" style="width: 200px; margin-right: 10px;" @change="handlePointSelectionChange">
              <el-option
                v-for="point in allPoints"
                :key="point.name"
                :label="point.name"
                :value="point.name"
              />
            </el-select>
            <el-select v-model="chartType" placeholder="图表类型" style="width: 120px; margin-right: 10px;">
              <el-option label="折线图" value="line" />
              <el-option label="柱状图" value="bar" />
              <el-option label="面积图" value="area" />
              <el-option label="散点图" value="scatter" />
            </el-select>
            <el-select v-model="timeRange" placeholder="时间范围" style="width: 120px;">
              <el-option label="5分钟" value="300000" />
              <el-option label="15分钟" value="900000" />
              <el-option label="30分钟" value="1800000" />
              <el-option label="1小时" value="3600000" />
            </el-select>
            <el-button type="primary" size="small" @click="refreshData" :loading="loading">
              刷新数据
            </el-button>
          </div>
        </div>
      </template>

      <div class="monitor-content">
        <!-- 加载状态 -->
        <el-skeleton :loading="loading" animated style="margin-bottom: 20px;">
          <template #template>
            <el-skeleton-item variant="p" style="width: 100%; height: 150px;" />
          </template>
        </el-skeleton>

        <!-- 错误提示 -->
        <el-alert
          v-if="error"
          :title="error"
          type="error"
          show-icon
          :closable="false"
          style="margin-bottom: 20px;"
        />

        <div class="current-values" v-if="selectedPoints.length > 0 && !loading">
          <el-row :gutter="20">
            <el-col :span="8" v-for="pointName in selectedPoints" :key="pointName">
              <el-card class="value-card" :class="getPointStatus(pointName)">
                <div class="value-content">
                  <div class="value-label">{{ pointName }}</div>
                  <div class="value-number">{{ getPointValue(pointName) || '--' }}</div>
                  <div class="value-time">{{ getPointTime(pointName) || '--' }}</div>
                  <div class="value-actions">
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

        <div class="trend-chart">
          <h3>数据趋势</h3>
          <div ref="chartRef" class="chart-container"></div>
        </div>
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
import { ref, computed, onMounted, watch, onUnmounted } from 'vue'
import { pointService } from '@/services/pointService'
import { chartService } from '@/services/chartService'
import { pointApi } from '@/services/pointApi'
import { type PointUpdate } from '@/services/websocket'

const chartRef = ref<HTMLElement>()
const selectedPoints = ref<string[]>([])
const chartType = ref('line')
const timeRange = ref('300000') // 默认5分钟
const loading = ref(false)
const error = ref('')
const allPoints = ref<any[]>([])
const pointData = ref<Record<string, { value: string; timestamp: number; status: string }>>({})
const CHART_ID = 'realtime-monitor-chart'

// 控制对话框
const controlDialogVisible = ref(false)
const currentControlPoint = ref('')
const controlValue = ref('')

// 初始化图表
const initChart = () => {
  if (chartRef.value) {
    chartService.initChart(chartRef.value, CHART_ID)
    updateChart()
  }
}

// 处理点位选择变化
const handlePointSelectionChange = (selected: string[]) => {
  // 订阅新选择的点位
  const newPoints = selected.filter(point => !selectedPoints.value.includes(point))
  newPoints.forEach(pointName => {
    pointService.subscribePoint(pointName)
  })
  
  // 取消订阅已移除的点位
  const removedPoints = selectedPoints.value.filter(point => !selected.includes(point))
  removedPoints.forEach(pointName => {
    pointService.unsubscribePoint(pointName)
    delete pointData.value[pointName]
  })
}

// 刷新数据
const refreshData = async () => {
  loading.value = true
  error.value = ''
  try {
    const response = await pointApi.getPointsWithDevice(1, 100) // 使用较大的size获取所有点位
    allPoints.value = response.items
  } catch (err) {
    error.value = `刷新数据失败: ${err instanceof Error ? err.message : '未知错误'}`
  } finally {
    loading.value = false
  }
}

// 获取点位状态
const getPointStatus = (pointName: string) => {
  return pointData.value[pointName]?.status || 'normal'
}

// 获取点位值
const getPointValue = (pointName: string) => {
  const point = pointData.value[pointName]
  return point !== undefined ? point.value : null
}

// 获取点位更新时间
const getPointTime = (pointName: string) => {
  const timestamp = pointData.value[pointName]?.timestamp
  return timestamp ? formatTime(timestamp) : null
}

// 更新图表
const updateChart = () => {
  chartService.updateChart(
    CHART_ID,
    selectedPoints.value,
    chartType.value,
    parseInt(timeRange.value)
  )
}

// 处理点位更新
const handlePointUpdate = (update: PointUpdate) => {
  // 更新点位数据
  const status = calculateStatus(update.value)
  pointData.value[update.name] = {
    value: update.value,
    timestamp: update.timestamp,
    status
  }
  
  // 更新图表数据
  const value = typeof update.value === 'string' ? parseFloat(update.value) : Number(update.value) || 0
  chartService.addDataPoint(update.name, update.timestamp, value)
  
  // 如果点位在选中列表中，更新图表
  if (selectedPoints.value.includes(update.name)) {
    updateChart()
  }
}

// 计算点位状态
const calculateStatus = (value: string | number): 'normal' | 'warning' | 'error' => {
  const numValue = typeof value === 'string' ? parseFloat(value) : Number(value)
  if (isNaN(numValue)) return 'normal'
  if (numValue > 80) return 'error'
  if (numValue > 60) return 'warning'
  return 'normal'
}

// 格式化时间
const formatTime = (timestamp: number) => {
  const date = new Date(timestamp)
  return date.toLocaleString()
}

// 判断是否为控制点位
const isControlPoint = (pointName: string) => {
  const point = allPoints.value.find(p => p.name === pointName)
  return point?.enable_control === true
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
      await refreshData()
    } else {
      alert('控制命令发送失败')
    }
  } catch (error) {
    console.error('控制命令发送失败:', error)
    alert('控制命令发送失败')
  }
}

// 监听点位变化
watch(
  selectedPoints,
  () => {
    updateChart()
  },
  { deep: true }
)

// 监听图表类型变化
watch(chartType, updateChart)

// 监听时间范围变化
watch(timeRange, updateChart)

// 监听窗口大小变化
const handleResize = () => {
  chartService.resizeChart(CHART_ID)
}

onMounted(async () => {
  // 初始化图表
  initChart()
  window.addEventListener('resize', handleResize)
  
  // 加载点位列表
  loading.value = true
  try {
    const response = await pointApi.getPointsWithDevice(1, 100) // 使用较大的size获取所有点位
    allPoints.value = response.items
    
    // 默认选择前两个点位（如果有）
    if (response.items.length > 0) {
      selectedPoints.value = response.items.slice(0, 2).map(point => point.name)
      // 订阅默认点位
      selectedPoints.value.forEach(pointName => {
        pointService.subscribePoint(pointName)
      })
    }
  } catch (err) {
    error.value = `加载点位列表失败: ${err instanceof Error ? err.message : '未知错误'}`
  } finally {
    loading.value = false
  }
  
  // 添加点位更新处理器
  selectedPoints.value.forEach(pointName => {
    pointService.addPointHandler(pointName, handlePointUpdate)
  })
})

onUnmounted(() => {
  // 移除事件监听器
  window.removeEventListener('resize', handleResize)
  
  // 销毁图表
  chartService.disposeChart(CHART_ID)
  
  // 取消订阅所有点位
  selectedPoints.value.forEach(pointName => {
    pointService.unsubscribePoint(pointName)
  })
})
</script>

<style scoped>
.realtime-monitor {
  padding: 20px;
}

.monitor-card {
  margin-bottom: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-wrap: wrap;
  gap: 10px;
}

.header-controls {
  display: flex;
  align-items: center;
  gap: 10px;
  flex-wrap: wrap;
}

.monitor-content {
  margin-top: 20px;
}

.current-values {
  margin-bottom: 30px;
  max-height: 400px;
  overflow-y: auto;
  padding-right: 10px;
  scrollbar-width: thin;
  scrollbar-color: #409EFF #f0f2f5;
}

.current-values::-webkit-scrollbar {
  width: 6px;
}

.current-values::-webkit-scrollbar-track {
  background: #f0f2f5;
  border-radius: 3px;
}

.current-values::-webkit-scrollbar-thumb {
  background: #409EFF;
  border-radius: 3px;
}

.current-values::-webkit-scrollbar-thumb:hover {
  background: #66b1ff;
}

.value-card {
  height: 150px;
  border-radius: 8px;
  overflow: hidden;
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
  margin-top: 10px;
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

.trend-chart {
  margin-top: 30px;
}

.trend-chart h3 {
  margin-bottom: 15px;
  font-size: 16px;
  font-weight: bold;
  color: #303133;
}

.chart-container {
  width: 100%;
  height: 400px;
}

/* 响应式设计 */
@media screen and (max-width: 768px) {
  .card-header {
    flex-direction: column;
    align-items: flex-start;
  }
  
  .header-controls {
    width: 100%;
    justify-content: space-between;
  }
  
  .el-select {
    flex: 1;
  }
  
  .el-col {
    flex: 1;
    min-width: 200px;
  }
}
</style>
