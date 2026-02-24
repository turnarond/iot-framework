<template>
  <div class="trigger-view">
    <el-page-header>
      <template #title>触发器管理</template>
      <template #subtitle>管理系统中的触发器实例</template>
    </el-page-header>

    <el-card class="trigger-card">
      <template #header>
        <div class="card-header">
          <span>触发器列表</span>
          <el-button type="primary" size="small" @click="handleAddTrigger">
            <el-icon><Plus /></el-icon>
            添加触发器
          </el-button>
        </div>
      </template>

      <!-- 使用TriggerList组件 -->
      <TriggerList
        :triggers="triggers"
        :currentPage="currentPage"
        :totalPages="totalPages"
        :totalItems="totalItems"
        @edit="handleEditTrigger"
        @delete="handleDeleteTrigger"
        @page-change="handlePageChange"
      />

      <!-- 分页 -->
      <div class="pagination" style="margin-top: 20px; display: flex; justify-content: flex-end; align-items: center;">
        <el-pagination
          :current-page="currentPage"
          :page-size="pageSize"
          :page-sizes="[10, 20, 50, 100]"
          layout="total, sizes, prev, pager, next, jumper"
          :total="totalItems"
          @current-change="handlePageChange"
          @size-change="handlePageSizeChange"
        />
      </div>
    </el-card>

    <!-- 使用TriggerForm组件 -->
    <TriggerForm
      :visible="dialogVisible"
      :triggerData="triggerForm"
      @close="handleCloseForm"
      @saved="handleSubmitTrigger"
    />
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, reactive } from 'vue'
import { Plus } from '@element-plus/icons-vue'
import TriggerList from '@/components/linkage/TriggerList.vue'
import TriggerForm from '@/components/linkage/TriggerForm.vue'
import { linkageApi } from '@/services/linkageApi'

// 触发器列表数据
const triggers = ref<any[]>([])
const loading = ref(false)
const currentPage = ref(1)
const pageSize = ref(10)
const totalItems = ref(0)
const totalPages = ref(1)

// 对话框相关
const dialogVisible = ref(false)
const editMode = ref(false)
const triggerForm = reactive({
  id: '',
  name: '',
  description: '',
  event_type_id: '',
  alarm_rule_id: '',
  expr_id: '',
  custom_event_key: '',
  enable: true
})

// 初始化数据
onMounted(() => {
  fetchTriggers()
})

// 获取触发器列表
const fetchTriggers = async () => {
  loading.value = true
  try {
    const response = await linkageApi.getTriggers(currentPage.value, pageSize.value)
    triggers.value = response.items
    totalItems.value = response.counts
    totalPages.value = response.pages
  } catch (error) {
    console.error('获取触发器列表失败:', error)
  } finally {
    loading.value = false
  }
}

// 分页处理
const handlePageChange = (page: number) => {
  currentPage.value = page
  fetchTriggers()
}

// 每页条数变化处理
const handlePageSizeChange = (size: number) => {
  pageSize.value = size
  fetchTriggers()
}

// 触发器操作
const handleAddTrigger = () => {
  editMode.value = false
  // 重置表单
  Object.assign(triggerForm, {
    id: '',
    name: '',
    description: '',
    event_type_id: '',
    alarm_rule_id: '',
    expr_id: '',
    custom_event_key: '',
    enable: true
  })
  dialogVisible.value = true
}

const handleEditTrigger = (trigger: any) => {
  editMode.value = true
  // 填充表单数据
  Object.assign(triggerForm, trigger)
  dialogVisible.value = true
}

const handleSubmitTrigger = async () => {
  try {
    dialogVisible.value = false
    fetchTriggers()
  } catch (error) {
    console.error('保存触发器失败:', error)
  }
}

const handleCloseForm = () => {
  dialogVisible.value = false
}

const handleDeleteTrigger = async (id: string) => {
  try {
    await linkageApi.deleteTrigger(id)
    fetchTriggers()
  } catch (error) {
    console.error('删除触发器失败:', error)
  }
}
</script>

<style scoped>
.trigger-view {
  padding: 20px;
  background-color: #f5f7fa;
  min-height: 100vh;
}

.trigger-card {
  margin-top: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
</style>