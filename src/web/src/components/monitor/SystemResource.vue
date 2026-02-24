<template>
  <div class="system-resource">
    <el-card class="resource-card">
      <template #header>
        <div class="card-header">
          <span>系统资源监控</span>
          <el-select v-model="timeRange" placeholder="时间范围" style="width: 120px;">
            <el-option label="5分钟" value="300000" />
            <el-option label="15分钟" value="900000" />
            <el-option label="30分钟" value="1800000" />
            <el-option label="1小时" value="3600000" />
          </el-select>
        </div>
      </template>

      <div class="resource-overview">
        <el-row :gutter="20">
          <el-col :span="6">
            <el-card class="resource-item">
              <div class="resource-content">
                <div class="resource-label">CPU使用率</div>
                <div class="resource-value">{{ systemStatus.cpu }}%</div>
                <el-progress 
                  :percentage="systemStatus.cpu" 
                  :color="getProgressColor(systemStatus.cpu)"
                  :stroke-width="8"
                />
              </div>
            </el-card>
          </el-col>
          <el-col :span="6">
            <el-card class="resource-item">
              <div class="resource-content">
                <div class="resource-label">内存使用率</div>
                <div class="resource-value">{{ systemStatus.memory }}%</div>
                <el-progress 
                  :percentage="systemStatus.memory" 
                  :color="getProgressColor(systemStatus.memory)"
                  :stroke-width="8"
                />
              </div>
            </el-card>
          </el-col>
          <el-col :span="6">
            <el-card class="resource-item">
              <div class="resource-content">
                <div class="resource-label">磁盘使用率</div>
                <div class="resource-value">{{ systemStatus.disk }}%</div>
                <el-progress 
                  :percentage="systemStatus.disk" 
                  :color="getProgressColor(systemStatus.disk)"
                  :stroke-width="8"
                />
              </div>
            </el-card>
          </el-col>
          <el-col :span="6">
            <el-card class="resource-item">
              <div class="resource-content">
                <div class="resource-label">网络连接数</div>
                <div class="resource-value">{{ systemStatus.network }}</div>
                <el-progress 
                  :percentage="Math.min(systemStatus.network, 100)" 
                  :color="getProgressColor(Math.min(systemStatus.network, 100))"
                  :stroke-width="8"
                />
              </div>
            </el-card>
          </el-col>
        </el-row>
      </div>

      <div class="resource-charts" style="margin-top: 30px;">
        <el-row :gutter="20">
          <el-col :span="12">
            <el-card class="chart-card">
              <template #header>
                <div class="chart-header">
                  <span>CPU使用率趋势</span>
                </div>
              </template>
              <div ref="cpuChartRef" class="chart-container"></div>
            </el-card>
          </el-col>
          <el-col :span="12">
            <el-card class="chart-card">
              <template #header>
                <div class="chart-header">
                  <span>内存使用率趋势</span>
                </div>
              </template>
              <div ref="memoryChartRef" class="chart-container"></div>
            </el-card>
          </el-col>
        </el-row>

        <el-row :gutter="20" style="margin-top: 20px;">
          <el-col :span="12">
            <el-card class="chart-card">
              <template #header>
                <div class="chart-header">
                  <span>磁盘使用率趋势</span>
                </div>
              </template>
              <div ref="diskChartRef" class="chart-container"></div>
            </el-card>
          </el-col>
          <el-col :span="12">
            <el-card class="chart-card">
              <template #header>
                <div class="chart-header">
                  <span>网络连接数趋势</span>
                </div>
              </template>
              <div ref="networkChartRef" class="chart-container"></div>
            </el-card>
          </el-col>
        </el-row>
      </div>
    </el-card>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, watch, onUnmounted, reactive } from 'vue'
import { useMonitorStore } from '@/stores/monitor'
import * as echarts from 'echarts'

const monitorStore = useMonitorStore()
const timeRange = ref('300000') // 默认5分钟
const cpuChartRef = ref<HTMLElement>()
const memoryChartRef = ref<HTMLElement>()
const diskChartRef = ref<HTMLElement>()
const networkChartRef = ref<HTMLElement>()

const cpuChart = ref<echarts.ECharts>()
const memoryChart = ref<echarts.ECharts>()
const diskChart = ref<echarts.ECharts>()
const networkChart = ref<echarts.ECharts>()

const systemStatus = computed(() => monitorStore.systemStatus)
const resourceHistory = reactive({
  cpu: [] as Array<{ timestamp: number; value: number }>,
  memory: [] as Array<{ timestamp: number; value: number }>,
  disk: [] as Array<{ timestamp: number; value: number }>,
  network: [] as Array<{ timestamp: number; value: number }>
})

// 获取进度条颜色
const getProgressColor = (percentage: number) => {
  if (percentage > 80) return '#f56c6c'
  if (percentage > 60) return '#e6a23c'
  return '#67c23a'
}

// 初始化图表
const initCharts = () => {
  if (cpuChartRef.value) {
    cpuChart.value = echarts.init(cpuChartRef.value)
  }
  if (memoryChartRef.value) {
    memoryChart.value = echarts.init(memoryChartRef.value)
  }
  if (diskChartRef.value) {
    diskChart.value = echarts.init(diskChartRef.value)
  }
  if (networkChartRef.value) {
    networkChart.value = echarts.init(networkChartRef.value)
  }
  updateCharts()
}

// 更新图表
const updateCharts = () => {
  const now = Date.now()
  const range = parseInt(timeRange.value)
  const startTime = now - range

  // 过滤数据
  const filteredCpuData = resourceHistory.cpu.filter(item => item.timestamp >= startTime)
  const filteredMemoryData = resourceHistory.memory.filter(item => item.timestamp >= startTime)
  const filteredDiskData = resourceHistory.disk.filter(item => item.timestamp >= startTime)
  const filteredNetworkData = resourceHistory.network.filter(item => item.timestamp >= startTime)

  // 更新CPU图表
  if (cpuChart.value) {
    cpuChart.value.setOption({
      tooltip: {
        trigger: 'axis',
        formatter: function(params: any) {
          const data = params[0]
          return `${new Date(data.value[0]).toLocaleString()}<br/>CPU: ${data.value[1]}%`
        }
      },
      xAxis: {
        type: 'time',
        boundaryGap: false
      },
      yAxis: {
        type: 'value',
        name: '使用率 (%)',
        min: 0,
        max: 100
      },
      series: [{
        data: filteredCpuData.map(item => [item.timestamp, item.value]),
        type: 'line',
        smooth: true,
        symbol: 'none',
        areaStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: 'rgba(245, 108, 108, 0.5)' },
            { offset: 1, color: 'rgba(245, 108, 108, 0.1)' }
          ])
        },
        itemStyle: { color: '#f56c6c' }
      }]
    })
  }

  // 更新内存图表
  if (memoryChart.value) {
    memoryChart.value.setOption({
      tooltip: {
        trigger: 'axis',
        formatter: function(params: any) {
          const data = params[0]
          return `${new Date(data.value[0]).toLocaleString()}<br/>内存: ${data.value[1]}%`
        }
      },
      xAxis: {
        type: 'time',
        boundaryGap: false
      },
      yAxis: {
        type: 'value',
        name: '使用率 (%)',
        min: 0,
        max: 100
      },
      series: [{
        data: filteredMemoryData.map(item => [item.timestamp, item.value]),
        type: 'line',
        smooth: true,
        symbol: 'none',
        areaStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: 'rgba(64, 158, 255, 0.5)' },
            { offset: 1, color: 'rgba(64, 158, 255, 0.1)' }
          ])
        },
        itemStyle: { color: '#409EFF' }
      }]
    })
  }

  // 更新磁盘图表
  if (diskChart.value) {
    diskChart.value.setOption({
      tooltip: {
        trigger: 'axis',
        formatter: function(params: any) {
          const data = params[0]
          return `${new Date(data.value[0]).toLocaleString()}<br/>磁盘: ${data.value[1]}%`
        }
      },
      xAxis: {
        type: 'time',
        boundaryGap: false
      },
      yAxis: {
        type: 'value',
        name: '使用率 (%)',
        min: 0,
        max: 100
      },
      series: [{
        data: filteredDiskData.map(item => [item.timestamp, item.value]),
        type: 'line',
        smooth: true,
        symbol: 'none',
        areaStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: 'rgba(103, 194, 58, 0.5)' },
            { offset: 1, color: 'rgba(103, 194, 58, 0.1)' }
          ])
        },
        itemStyle: { color: '#67c23a' }
      }]
    })
  }

  // 更新网络图表
  if (networkChart.value) {
    networkChart.value.setOption({
      tooltip: {
        trigger: 'axis',
        formatter: function(params: any) {
          const data = params[0]
          return `${new Date(data.value[0]).toLocaleString()}<br/>连接数: ${data.value[1]}`
        }
      },
      xAxis: {
        type: 'time',
        boundaryGap: false
      },
      yAxis: {
        type: 'value',
        name: '连接数'
      },
      series: [{
        data: filteredNetworkData.map(item => [item.timestamp, item.value]),
        type: 'line',
        smooth: true,
        symbol: 'none',
        areaStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: 'rgba(194, 129, 249, 0.5)' },
            { offset: 1, color: 'rgba(194, 129, 249, 0.1)' }
          ])
        },
        itemStyle: { color: '#c281f9' }
      }]
    })
  }
}

// 监听时间范围变化
watch(timeRange, updateCharts)

// 监听系统状态变化
watch(
  () => systemStatus.value,
  (newStatus) => {
    // 添加新数据
    resourceHistory.cpu.push({ timestamp: newStatus.timestamp, value: newStatus.cpu })
    resourceHistory.memory.push({ timestamp: newStatus.timestamp, value: newStatus.memory })
    resourceHistory.disk.push({ timestamp: newStatus.timestamp, value: newStatus.disk })
    resourceHistory.network.push({ timestamp: newStatus.timestamp, value: newStatus.network })

    // 限制数据量
    const maxDataPoints = 1000
    if (resourceHistory.cpu.length > maxDataPoints) {
      resourceHistory.cpu.shift()
    }
    if (resourceHistory.memory.length > maxDataPoints) {
      resourceHistory.memory.shift()
    }
    if (resourceHistory.disk.length > maxDataPoints) {
      resourceHistory.disk.shift()
    }
    if (resourceHistory.network.length > maxDataPoints) {
      resourceHistory.network.shift()
    }

    updateCharts()
  },
  { deep: true }
)

// 监听窗口大小变化
const handleResize = () => {
  cpuChart.value?.resize()
  memoryChart.value?.resize()
  diskChart.value?.resize()
  networkChart.value?.resize()
}

onMounted(() => {
  initCharts()
  window.addEventListener('resize', handleResize)
  
  // 初始化历史数据
  resourceHistory.cpu.push({ timestamp: Date.now(), value: systemStatus.value.cpu })
  resourceHistory.memory.push({ timestamp: Date.now(), value: systemStatus.value.memory })
  resourceHistory.disk.push({ timestamp: Date.now(), value: systemStatus.value.disk })
  resourceHistory.network.push({ timestamp: Date.now(), value: systemStatus.value.network })
})

onUnmounted(() => {
  window.removeEventListener('resize', handleResize)
  cpuChart.value?.dispose()
  memoryChart.value?.dispose()
  diskChart.value?.dispose()
  networkChart.value?.dispose()
})
</script>

<style scoped>
.system-resource {
  padding: 20px;
}

.resource-card {
  margin-bottom: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.resource-overview {
  margin: 20px 0;
}

.resource-item {
  border-radius: 8px;
  overflow: hidden;
}

.resource-content {
  padding: 15px;
}

.resource-label {
  font-size: 14px;
  color: #606266;
  margin-bottom: 8px;
}

.resource-value {
  font-size: 24px;
  font-weight: bold;
  margin-bottom: 12px;
}

.resource-charts {
  margin-top: 30px;
}

.chart-card {
  margin-bottom: 20px;
}

.chart-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.chart-container {
  width: 100%;
  height: 200px;
}

/* 响应式设计 */
@media screen and (max-width: 768px) {
  .el-col {
    flex: 1;
    min-width: 100%;
  }
  
  .chart-container {
    height: 150px;
  }
}
</style>