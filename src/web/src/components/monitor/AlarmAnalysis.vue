<template>
  <div class="alarm-analysis">
    <el-card class="analysis-card">
      <template #header>
        <div class="card-header">
          <span>报警统计分析</span>
          <el-select v-model="timeRange" placeholder="时间范围" style="width: 120px;">
            <el-option label="1小时" value="3600000" />
            <el-option label="6小时" value="21600000" />
            <el-option label="12小时" value="43200000" />
            <el-option label="24小时" value="86400000" />
          </el-select>
        </div>
      </template>

      <div class="analysis-overview">
        <el-row :gutter="20">
          <el-col :span="8">
            <el-card class="overview-card">
              <div class="overview-content">
                <div class="overview-label">总报警数</div>
                <div class="overview-value">{{ allAlarms.length }}</div>
              </div>
            </el-card>
          </el-col>
          <el-col :span="8">
            <el-card class="overview-card warning">
              <div class="overview-content">
                <div class="overview-label">警告数</div>
                <div class="overview-value">{{ warningAlarms.length }}</div>
              </div>
            </el-card>
          </el-col>
          <el-col :span="8">
            <el-card class="overview-card error">
              <div class="overview-content">
                <div class="overview-label">错误数</div>
                <div class="overview-value">{{ errorAlarms.length }}</div>
              </div>
            </el-card>
          </el-col>
        </el-row>
      </div>

      <div class="analysis-charts">
        <el-row :gutter="20">
          <el-col :span="12">
            <el-card class="chart-card">
              <template #header>
                <div class="chart-header">
                  <span>报警级别分布</span>
                </div>
              </template>
              <div ref="levelChartRef" class="chart-container"></div>
            </el-card>
          </el-col>
          <el-col :span="12">
            <el-card class="chart-card">
              <template #header>
                <div class="chart-header">
                  <span>报警类型分布</span>
                </div>
              </template>
              <div ref="typeChartRef" class="chart-container"></div>
            </el-card>
          </el-col>
        </el-row>

        <el-row :gutter="20" style="margin-top: 20px;">
          <el-col :span="24">
            <el-card class="chart-card">
              <template #header>
                <div class="chart-header">
                  <span>报警趋势</span>
                </div>
              </template>
              <div ref="trendChartRef" class="chart-container"></div>
            </el-card>
          </el-col>
        </el-row>
      </div>

      <div class="recent-alarms" style="margin-top: 30px;">
        <h3>最近报警</h3>
        <el-table :data="recentAlarms" style="width: 100%" border>
          <el-table-column prop="level" label="级别" width="100">
            <template #default="scope">
              <el-tag :type="scope.row.level === 'error' ? 'danger' : scope.row.level === 'warning' ? 'warning' : 'info'">
                {{ scope.row.level }}
              </el-tag>
            </template>
          </el-table-column>
          <el-table-column prop="type" label="类型" width="150" />
          <el-table-column prop="message" label="报警信息" />
          <el-table-column prop="deviceId" label="设备ID" width="120" />
          <el-table-column prop="pointName" label="点位名称" width="120" />
          <el-table-column prop="timestamp" label="发生时间">
            <template #default="scope">
              {{ formatTime(scope.row.timestamp) }}
            </template>
          </el-table-column>
        </el-table>
      </div>
    </el-card>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, watch, onUnmounted, reactive } from 'vue'
import { useMonitorStore } from '@/stores/monitor'
import * as echarts from 'echarts'

const monitorStore = useMonitorStore()
const timeRange = ref('86400000') // 默认24小时
const levelChartRef = ref<HTMLElement>()
const typeChartRef = ref<HTMLElement>()
const trendChartRef = ref<HTMLElement>()
const levelChart = ref<echarts.ECharts>()
const typeChart = ref<echarts.ECharts>()
const trendChart = ref<echarts.ECharts>()

const allAlarms = computed(() => monitorStore.alarms)
const infoAlarms = computed(() => monitorStore.alarms.filter(a => a.level === 'info'))
const warningAlarms = computed(() => monitorStore.alarms.filter(a => a.level === 'warning'))
const errorAlarms = computed(() => monitorStore.alarms.filter(a => a.level === 'error'))
const recentAlarms = computed(() => monitorStore.alarms.slice(0, 10))

// 按类型分组报警
const alarmsByType = computed(() => {
  const grouped = allAlarms.value.reduce((acc, alarm) => {
    if (!acc[alarm.type]) {
      acc[alarm.type] = 0
    }
    acc[alarm.type]++
    return acc
  }, {} as Record<string, number>)
  return Object.entries(grouped).map(([type, count]) => ({ type, count }))
})

// 格式化时间
const formatTime = (timestamp: number) => {
  const date = new Date(timestamp)
  return date.toLocaleString()
}

// 初始化图表
const initCharts = () => {
  if (levelChartRef.value) {
    levelChart.value = echarts.init(levelChartRef.value)
  }
  if (typeChartRef.value) {
    typeChart.value = echarts.init(typeChartRef.value)
  }
  if (trendChartRef.value) {
    trendChart.value = echarts.init(trendChartRef.value)
  }
  updateCharts()
}

// 更新图表
const updateCharts = () => {
  updateLevelChart()
  updateTypeChart()
  updateTrendChart()
}

// 更新报警级别分布图表
const updateLevelChart = () => {
  if (!levelChart.value) return

  levelChart.value.setOption({
    tooltip: {
      trigger: 'item',
      formatter: '{a} <br/>{b}: {c} ({d}%)'
    },
    legend: {
      orient: 'vertical',
      left: 'left',
      data: ['info', 'warning', 'error']
    },
    series: [
      {
        name: '报警级别',
        type: 'pie',
        radius: '60%',
        center: ['50%', '50%'],
        data: [
          { value: infoAlarms.value.length, name: 'info', itemStyle: { color: '#909399' } },
          { value: warningAlarms.value.length, name: 'warning', itemStyle: { color: '#e6a23c' } },
          { value: errorAlarms.value.length, name: 'error', itemStyle: { color: '#f56c6c' } }
        ],
        emphasis: {
          itemStyle: {
            shadowBlur: 10,
            shadowOffsetX: 0,
            shadowColor: 'rgba(0, 0, 0, 0.5)'
          }
        }
      }
    ]
  })
}

// 更新报警类型分布图表
const updateTypeChart = () => {
  if (!typeChart.value) return

  const typeData = alarmsByType.value.slice(0, 10) // 只显示前10种类型

  typeChart.value.setOption({
    tooltip: {
      trigger: 'axis',
      axisPointer: {
        type: 'shadow'
      }
    },
    grid: {
      left: '3%',
      right: '4%',
      bottom: '3%',
      containLabel: true
    },
    xAxis: {
      type: 'category',
      data: typeData.map(item => item.type),
      axisLabel: {
        rotate: 45
      }
    },
    yAxis: {
      type: 'value',
      name: '数量'
    },
    series: [
      {
        name: '报警数量',
        type: 'bar',
        data: typeData.map(item => item.count),
        itemStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: '#409EFF' },
            { offset: 1, color: '#69c0ff' }
          ])
        }
      }
    ]
  })
}

// 更新报警趋势图表
const updateTrendChart = () => {
  if (!trendChart.value) return

  const now = Date.now()
  const range = parseInt(timeRange.value)
  const startTime = now - range

  // 过滤时间范围内的报警
  const filteredAlarms = allAlarms.value.filter(alarm => alarm.timestamp >= startTime)

  // 按时间分组报警
  const timeGroups = Math.min(Math.floor(range / 3600000), 24) // 最多24个时间点
  const interval = range / timeGroups
  const trendData = []

  for (let i = 0; i < timeGroups; i++) {
    const groupStart = startTime + i * interval
    const groupEnd = groupStart + interval
    const groupAlarms = filteredAlarms.filter(alarm => alarm.timestamp >= groupStart && alarm.timestamp < groupEnd)
    trendData.push([groupStart, groupAlarms.length])
  }

  trendChart.value.setOption({
    tooltip: {
      trigger: 'axis',
      formatter: function(params: any) {
        const data = params[0]
        return `${new Date(data.value[0]).toLocaleString()}<br/>报警数: ${data.value[1]}`
      }
    },
    grid: {
      left: '3%',
      right: '4%',
      bottom: '3%',
      containLabel: true
    },
    xAxis: {
      type: 'time',
      boundaryGap: false
    },
    yAxis: {
      type: 'value',
      name: '报警数量',
      min: 0
    },
    series: [
      {
        name: '报警数量',
        type: 'line',
        data: trendData,
        smooth: true,
        symbol: 'none',
        areaStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: 'rgba(245, 108, 108, 0.5)' },
            { offset: 1, color: 'rgba(245, 108, 108, 0.1)' }
          ])
        },
        itemStyle: {
          color: '#f56c6c'
        }
      }
    ]
  })
}

// 监听时间范围变化
watch(timeRange, updateCharts)

// 监听报警数据变化
watch(
  () => monitorStore.alarms,
  () => {
    updateCharts()
  },
  { deep: true }
)

// 监听窗口大小变化
const handleResize = () => {
  levelChart.value?.resize()
  typeChart.value?.resize()
  trendChart.value?.resize()
}

onMounted(() => {
  initCharts()
  window.addEventListener('resize', handleResize)
  
  // 初始化报警数据
  if (allAlarms.value.length === 0) {
    // 模拟报警数据
    // monitorStore.simulateDataUpdate()
  }
})

onUnmounted(() => {
  window.removeEventListener('resize', handleResize)
  levelChart.value?.dispose()
  typeChart.value?.dispose()
  trendChart.value?.dispose()
})
</script>

<style scoped>
.alarm-analysis {
  padding: 20px;
}

.analysis-card {
  margin-bottom: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.analysis-overview {
  margin: 20px 0;
}

.overview-card {
  border-radius: 8px;
  overflow: hidden;
}

.overview-card.warning {
  border-left: 4px solid #e6a23c;
}

.overview-card.error {
  border-left: 4px solid #f56c6c;
}

.overview-content {
  padding: 15px;
  text-align: center;
}

.overview-label {
  font-size: 14px;
  color: #606266;
  margin-bottom: 8px;
}

.overview-value {
  font-size: 32px;
  font-weight: bold;
  margin-bottom: 8px;
}

.analysis-charts {
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
  height: 300px;
}

.recent-alarms {
  margin-top: 30px;
}

.recent-alarms h3 {
  margin-bottom: 15px;
  font-size: 16px;
  font-weight: bold;
  color: #303133;
}

/* 响应式设计 */
@media screen and (max-width: 768px) {
  .el-col {
    flex: 1;
    min-width: 100%;
  }
  
  .chart-container {
    height: 250px;
  }
}
</style>