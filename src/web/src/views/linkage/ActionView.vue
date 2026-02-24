<template>
  <div class="action-view">
    <el-page-header>
      <template #title>执行器管理</template>
      <template #subtitle>管理系统中的执行器实例</template>
    </el-page-header>

    <el-card class="action-card">
      <template #header>
        <div class="card-header">
          <span>执行器列表</span>
          <el-button type="primary" size="small" @click="handleAddAction">
            <el-icon><Plus /></el-icon>
            添加执行器
          </el-button>
        </div>
      </template>

      <!-- 使用ActionList组件 -->
      <ActionList
        :actions="actions"
        :currentPage="currentPage"
        :totalPages="totalPages"
        :totalItems="totalItems"
        @edit="handleEditAction"
        @delete="handleDeleteAction"
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

    <!-- 使用ActionForm组件 -->
    <ActionForm
      :visible="dialogVisible"
      :actionData="actionForm"
      @close="handleCloseForm"
      @saved="handleSubmitAction"
    />
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, reactive } from 'vue'
import { Plus } from '@element-plus/icons-vue'
import ActionList from '@/components/linkage/ActionList.vue'
import ActionForm from '@/components/linkage/ActionForm.vue'
import { linkageApi } from '@/services/linkageApi'

// 执行器列表数据
const actions = ref<any[]>([])
const loading = ref(false)
const currentPage = ref(1)
const pageSize = ref(10)
const totalItems = ref(0)
const totalPages = ref(1)

// 对话框相关
const dialogVisible = ref(false)
const editMode = ref(false)
const actionForm = reactive({
  id: '',
  name: '',
  description: '',
  action_type_id: '',
  param1: '',
  param2: '',
  param3: null,
  param4: null,
  need_confirm: false
})

// 初始化数据
onMounted(() => {
  fetchActions()
})

// 获取执行器列表
const fetchActions = async () => {
  loading.value = true
  try {
    const response = await linkageApi.getActions(currentPage.value, pageSize.value)
    actions.value = response.items
    totalItems.value = response.counts
    totalPages.value = response.pages
  } catch (error) {
    console.error('获取执行器列表失败:', error)
  } finally {
    loading.value = false
  }
}

// 分页处理
const handlePageChange = (page: number) => {
  currentPage.value = page
  fetchActions()
}

// 每页条数变化处理
const handlePageSizeChange = (size: number) => {
  pageSize.value = size
  fetchActions()
}

// 执行器操作
const handleAddAction = () => {
  editMode.value = false
  // 重置表单
  Object.assign(actionForm, {
    id: '',
    name: '',
    description: '',
    action_type_id: '',
    param1: '',
    param2: '',
    param3: null,
    param4: null,
    need_confirm: false
  })
  dialogVisible.value = true
}

const handleEditAction = (action: any) => {
  editMode.value = true
  // 填充表单数据
  Object.assign(actionForm, action)
  dialogVisible.value = true
}

const handleSubmitAction = async () => {
  try {
    dialogVisible.value = false
    fetchActions()
  } catch (error) {
    console.error('保存执行器失败:', error)
  }
}

const handleCloseForm = () => {
  dialogVisible.value = false
}

const handleDeleteAction = async (id: string) => {
  try {
    await linkageApi.deleteAction(id)
    fetchActions()
  } catch (error) {
    console.error('删除执行器失败:', error)
  }
}
</script>

<style scoped>
.action-view {
  padding: 20px;
  background-color: #f5f7fa;
  min-height: 100vh;
}

.action-card {
  margin-top: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
</style>