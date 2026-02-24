<template>
  <div class="device-health">
    <el-card class="health-card">
      <template #header>
        <div class="card-header">
          <span>设备健康状态监控</span>
          <el-button type="primary" size="small" @click="refreshDevices">
            刷新设备状态
          </el-button>
        </div>
      </template>

      <div class="health-overview">
        <el-row :gutter="20">
          <el-col :span="8">
            <el-card class="health-status-card online">
              <div class="status-content">
                <div class="status-value">{{ onlineDevices.length }}</div>
                <div class="status-label">在线设备</div>
              </div>
            </el-card>
          </el-col>
          <el-col :span="8">
            <el-card class="health-status-card offline">
              <div class="status-content">
                <div class="status-value">{{ offlineDevices.length }}</div>
                <div class="status-label">离线设备</div>
              </div>
            </el-card>
          </el-col>
          <el-col :span="8">
            <el-card class="health-status-card error">
              <div class="status-content">
                <div class="status-value">{{ errorDevices.length }}</div>
                <div class="status-label">异常设备</div>
              </div>
            </el-card>
          </el-col>
        </el-row>
      </div>

      <div class="device-list" style="margin-top: 30px;">
        <h3>设备状态列表</h3>
        <el-table :data="allDevices" style="width: 100%" border>
          <el-table-column prop="deviceId" label="设备ID" width="180" />
          <el-table-column prop="status" label="状态" width="120">
            <template #default="scope">
              <el-tag :type="getStatusType(scope.row.status)">
                {{ scope.row.status }}
              </el-tag>
            </template>
          </el-table-column>
          <el-table-column prop="responseTime" label="响应时间 (ms)" width="150">
            <template #default="scope">
              <div>
                {{ scope.row.responseTime }}
                <el-progress 
                  :percentage="Math.min(scope.row.responseTime, 100)" 
                  :color="getResponseTimeColor(scope.row.responseTime)"
                  :stroke-width="4"
                  style="margin-top: 5px;"
                />
              </div>
            </template>
          </el-table-column>
          <el-table-column prop="timestamp" label="更新时间">
            <template #default="scope">
              {{ formatTime(scope.row.timestamp) }}
            </template>
          </el-table-column>
        </el-table>
      </div>

      <div class="response-time-chart" style="margin-top: 30px;">
        <h3>设备响应时间趋势</h3>
        <div ref="chartRef" class="chart-container"></div>
      </div>
    </el-card>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, watch, onUnmounted, reactive } from 'vue'
import { useMonitorStore } from '@/stores/monitor'
import * as echarts from 'echarts'

const monitorStore = useMonitorStore()
const chartRef = ref<HTMLElement>()
const chart = ref<echarts.ECharts>()
const responseTimeHistory = reactive<Record<string, Array<{ timestamp: number; value: number }>>>({})

const allDevices = computed(() => monitorStore.allDevices)
const onlineDevices = computed(() => monitorStore.onlineDevices)
const offlineDevices = computed(() => monitorStore.offlineDevices)
const errorDevices = computed(() => monitorStore.errorDevices)

// 获取状态类型
const getStatusType = (status: string) => {
  switch (status) {
    case 'online':
      return 'success'
    case 'offline':
      return 'info'
    case 'error':
      return 'danger'
    default:
      return 'info'
  }
}

// 获取响应时间颜色
const getResponseTimeColor = (time: number) => {
  if (time > 500) return '#f56c6c'
  if (time > 200) return '#e6a23c'
  return '#67c23a'
}

// 格式化时间
const formatTime = (timestamp: number) => {
  const date = new Date(timestamp)
  return date.toLocaleString()
}

// 刷新设备状态
const refreshDevices = () => {
  console.log('Refreshing device status...')
  // 这里可以添加刷新逻辑
  // monitorStore.simulateDataUpdate()
}

// 初始化图表
const initChart = () => {
  if (chartRef.value) {
    chart.value = echarts.init(chartRef.value)
    updateChart()
  }
}

// 更新图表
const updateChart = () => {
  if (!chart.value) return

  const series = Object.entries(responseTimeHistory).map(([deviceId, data]) => {
    return {
      name: deviceId,
      type: 'line',
      data: data.map(item => [item.timestamp, item.value]),
      smooth: true,
      symbol: 'none',
      lineStyle: {
        width: 2
      }
    }
  })

  chart.value.setOption({
    tooltip: {
      trigger: 'axis',
      formatter: function(params: any) {
        let result = new Date(params[0].value[0]).toLocaleString() + '<br/>'
        params.forEach((item: any) => {
          result += `${item.marker}${item.seriesName}: ${item.value[1]}ms<br/>`
        })
        return result
      }
    },
    legend: {
      data: Object.keys(responseTimeHistory),
      bottom: 0
    },
    xAxis: {
      type: 'time',
      boundaryGap: false
    },
    yAxis: {
      type: 'value',
      name: '响应时间 (ms)',
      min: 0
    },
    grid: {
      left: '3%',
      right: '4%',
      bottom: '10%',
      top: '3%',
      containLabel: true
    },
    series
  })
}

// 监听设备状态变化
watch(
  () => monitorStore.devices,
  (newDevices) => {
    // 更新响应时间历史
    Object.entries(newDevices).forEach(([deviceId, device]) => {
      if (!responseTimeHistory[deviceId]) {
        responseTimeHistory[deviceId] = []
      }
      responseTimeHistory[deviceId].push({ timestamp: device.timestamp, value: device.responseTime })

      // 限制数据量
      const maxDataPoints = 100
      if (responseTimeHistory[deviceId].length > maxDataPoints) {
        responseTimeHistory[deviceId].shift()
      }
    })

    updateChart()
  },
  { deep: true }
)

// 监听窗口大小变化
const handleResize = () => {
  chart.value?.resize()
}

onMounted(() => {
  initChart()
  window.addEventListener('resize', handleResize)
  
  // 初始化设备状态
  if (allDevices.value.length === 0) {
    // 模拟设备数据
    // monitorStore.simulateDataUpdate()
  }
})

onUnmounted(() => {
  window.removeEventListener('resize', handleResize)
  chart.value?.dispose()
})
</script>

<style scoped>
.device-health {
  padding: 20px;
}

.health-card {
  margin-bottom: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.health-overview {
  margin: 20px 0;
}

.health-status-card {
  height: 120px;
  border-radius: 8px;
  overflow: hidden;
}

.health-status-card.online {
  border-left: 4px solid #67c23a;
}

.health-status-card.offline {
  border-left: 4px solid #409EFF;
}

.health-status-card.error {
  border-left: 4px solid #f56c6c;
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
}

.status-label {
  font-size: 14px;
  color: #606266;
}

.device-list {
  margin-top: 30px;
}

.device-list h3,
.response-time-chart h3 {
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
  .el-col {
    flex: 1;
    min-width: 200px;
  }
  
  .chart-container {
    height: 300px;
  }
}
</style>