<template>
  <div class="monitor-view">
    <el-page-header>
      <template #title>状态监控</template>
      <template #subtitle>实时监控系统状态和点位数据</template>
    </el-page-header>

    <Dashboard />
    <SystemResource />
    <DeviceHealth />
    <AlarmAnalysis />
    <RealTimeMonitor />
  </div>
</template>

<script setup lang="ts">
import Dashboard from '@/components/monitor/Dashboard.vue'
import RealTimeMonitor from '@/components/monitor/RealTimeMonitor.vue'
import SystemResource from '@/components/monitor/SystemResource.vue'
import DeviceHealth from '@/components/monitor/DeviceHealth.vue'
import AlarmAnalysis from '@/components/monitor/AlarmAnalysis.vue'
</script>

<style scoped>
.monitor-view {
  padding: 20px;
  background-color: #f5f7fa;
  min-height: 100vh;
}
</style>
