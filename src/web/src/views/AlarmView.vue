<template>
  <div class="alarm-view">
    <el-page-header>
      <template #title>报警管理</template>
      <template #subtitle>管理系统中的报警规则和报警记录</template>
    </el-page-header>

    <el-card class="alarm-card">
      <template #header>
        <div class="card-header">
          <span>报警规则列表</span>
          <el-button type="primary" size="small" @click="handleAddAlarmRule">
            <el-icon>
              <Plus />
            </el-icon>
            添加报警规则
          </el-button>
        </div>
      </template>

      <!-- 使用AlarmList组件 -->
      <AlarmList :alarmRules="alarmRules" :currentPage="ruleCurrentPage" :totalPages="ruleTotalPages"
        :totalItems="ruleTotalItems" @edit="handleEditAlarmRule" @toggle="handleToggleAlarmRule"
        @delete="handleDeleteAlarmRule" @page-change="handleRulePageChange" />

      <!-- 分页 -->
      <div class="pagination" style="margin-top: 20px; display: flex; justify-content: flex-end; align-items: center;">
        <el-pagination :current-page="ruleCurrentPage" :page-size="rulePageSize" :page-sizes="[10, 20, 50, 100]"
          layout="total, sizes, prev, pager, next, jumper" :total="ruleTotalItems"
          @current-change="handleRulePageChange" @size-change="handleRulePageSizeChange" />
      </div>
    </el-card>

    <el-card class="alarm-card" style="margin-top: 20px;">
      <template #header>
        <div class="card-header">
          <span>报警记录</span>
          <el-button type="primary" size="small" @click="fetchAlarms">
            <el-icon>
              <Refresh />
            </el-icon>
            刷新
          </el-button>
        </div>
      </template>

      <el-table :data="alarms" style="width: 100%" border v-loading="alarmLoading">
        <el-table-column prop="id" label="报警ID" width="100" />
        <el-table-column prop="pointName" label="点位名称" />
        <el-table-column prop="deviceName" label="设备名称" />
        <el-table-column prop="value" label="当前值" width="100" />
        <el-table-column prop="threshold" label="阈值" width="100" />
        <el-table-column prop="priority" label="优先级" width="100">
          <template #default="scope">
            <el-tag :type="getPriorityType(scope.row.priority)">
              {{ scope.row.priority }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="message" label="报警消息" />
        <el-table-column prop="timestamp" label="报警时间" width="180" />
        <el-table-column label="操作" width="100">
          <template #default="scope">
            <el-button type="success" size="small" @click="handleConfirmAlarm(scope.row.id)">确认</el-button>
          </template>
        </el-table-column>
      </el-table>

      <!-- 分页 -->
      <div class="pagination" style="margin-top: 20px; display: flex; justify-content: flex-end; align-items: center;">
        <el-pagination :current-page="alarmCurrentPage" :page-size="alarmPageSize" :page-sizes="[10, 20, 50, 100]"
          layout="total, sizes, prev, pager, next, jumper" :total="alarmTotalItems"
          @current-change="handleAlarmPageChange" @size-change="handleAlarmPageSizeChange" />
      </div>
    </el-card>

    <!-- 使用AlarmForm组件 -->
    <AlarmForm :visible="dialogVisible" :editMode="editMode" :ruleData="alarmRuleForm" :points="points"
      :alarmMethods="alarmMethods" @close="handleCloseForm" @submit="handleSubmitAlarmRule" />
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, reactive, watch } from 'vue'
import { Plus, Refresh } from '@element-plus/icons-vue'
import AlarmList from '@/components/alarm/AlarmList.vue'
import AlarmForm from '@/components/alarm/AlarmForm.vue'
import type { AlarmRule, Alarm } from '@/services/alarmApi'
import { alarmApi } from '@/services/alarmApi'
import { pointApi } from '@/services/pointApi'
import type { Point } from '@/services/pointApi'
import { dictApi } from '@/services/dictApi'

// 报警规则列表数据
const alarmRules = ref<AlarmRule[]>([])
const loading = ref(false)
const ruleCurrentPage = ref(1)
const rulePageSize = ref(10)
const ruleTotalItems = ref(0)
const ruleTotalPages = ref(1)

// 报警记录数据
const alarms = ref<Alarm[]>([])
const alarmLoading = ref(false)
const alarmCurrentPage = ref(1)
const alarmPageSize = ref(10)
const alarmTotalItems = ref(0)

// 点位列表（用于报警规则绑定）
const points = ref<Point[]>([])

// 报警方法列表（用于报警规则配置）
const alarmMethods = ref<any[]>([])

// 对话框相关
const dialogVisible = ref(false)
const editMode = ref(false)
const alarmRuleForm = reactive({
  id: '',
  pointId: '',
  condition: '>',
  threshold: 0,
  priority: 'medium',
  message: ''
})

// 获取报警方法列表
const fetchAlarmMethods = async () => {
  try {
    const response = await dictApi.getAlarmMethods()
    alarmMethods.value = response
  } catch (error) {
    console.error('获取报警方法列表失败:', error)
  }
}

// 初始化数据
onMounted(() => {
  fetchAlarmRules()
  fetchAlarms()
  fetchPoints()
  fetchAlarmMethods()
})

// 获取报警规则列表
const fetchAlarmRules = async () => {
  loading.value = true
  try {
    const response = await alarmApi.getAlarmRulesWithPoint(ruleCurrentPage.value, rulePageSize.value)
    alarmRules.value = response.items
    ruleTotalItems.value = response.counts
    ruleTotalPages.value = response.pages
  } catch (error) {
    console.error('获取报警规则列表失败:', error)
  } finally {
    loading.value = false
  }
}

// 获取报警记录
const fetchAlarms = async () => {
  alarmLoading.value = true
  try {
    const response = await alarmApi.getAlarms(alarmCurrentPage.value, alarmPageSize.value)
    alarms.value = response.items
    alarmTotalItems.value = response.counts
  } catch (error) {
    console.error('获取报警记录失败:', error)
  } finally {
    alarmLoading.value = false
  }
}

// 获取点位列表
const fetchPoints = async () => {
  try {
    const response = await pointApi.getPointsWithDevice(1, 100) // 获取更多点位，避免分页
    points.value = response.items
  } catch (error) {
    console.error('获取点位列表失败:', error)
  }
}

// 报警规则分页处理
const handleRulePageChange = (page: number) => {
  ruleCurrentPage.value = page
  fetchAlarmRules()
}

// 报警规则每页条数变化处理
const handleRulePageSizeChange = (size: number) => {
  rulePageSize.value = size
  fetchAlarmRules()
}

// 报警记录分页处理
const handleAlarmPageChange = (page: number) => {
  alarmCurrentPage.value = page
  fetchAlarms()
}

// 报警记录每页条数变化处理
const handleAlarmPageSizeChange = (size: number) => {
  alarmPageSize.value = size
  fetchAlarms()
}

// 报警规则操作
const handleAddAlarmRule = () => {
  editMode.value = false
  // 重置表单
  Object.assign(alarmRuleForm, {
    id: '',
    pointId: '',
    condition: '>',
    threshold: 0,
    priority: 'medium',
    message: ''
  })
  dialogVisible.value = true
}

const handleEditAlarmRule = (rule: AlarmRule) => {
  editMode.value = true
  // 填充表单数据
  Object.assign(alarmRuleForm, rule)
  dialogVisible.value = true
}

const handleSubmitAlarmRule = async (alarmRuleData: any) => {
  try {
    if (editMode.value) {
      await alarmApi.updateAlarmRule(alarmRuleData)
    } else {
      await alarmApi.createAlarmRule(alarmRuleData)
    }
    dialogVisible.value = false
    fetchAlarmRules()
  } catch (error) {
    console.error('保存报警规则失败:', error)
  }
}

const handleCloseForm = () => {
  dialogVisible.value = false
}

const handleToggleAlarmRule = async (rule: AlarmRule) => {
  try {
    // 这里假设alarmApi中有启用/停用报警规则的方法
    // 如果没有，可以根据实际情况调整
    console.log('切换报警规则状态:', rule.id)
  } catch (error) {
    console.error('切换报警规则状态失败:', error)
  }
}

const handleDeleteAlarmRule = async (id: string) => {
  try {
    await alarmApi.deleteAlarmRule(id)
    fetchAlarmRules()
  } catch (error) {
    console.error('删除报警规则失败:', error)
  }
}

// 报警操作
const handleConfirmAlarm = async (id: string) => {
  try {
    // 这里假设alarmApi中有确认报警的方法
    // 如果没有，可以根据实际情况调整
    console.log('确认报警:', id)
    // 模拟确认操作
    const alarm = alarms.value.find(a => a.id === id)
    if (alarm) {
      alarm.status = 'confirmed'
    }
  } catch (error) {
    console.error('确认报警失败:', error)
  }
}

// 获取优先级类型
const getPriorityType = (priority: string): string => {
  switch (priority) {
    case 'high':
      return 'danger'
    case 'medium':
      return 'warning'
    case 'low':
      return 'info'
    default:
      return 'info'
  }
}
</script>

<style scoped>
.alarm-view {
  padding: 20px;
  background-color: #f5f7fa;
  min-height: 100vh;
}

.alarm-card {
  margin-top: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
</style>