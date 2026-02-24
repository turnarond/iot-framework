<template>
  <div class="driver-view">
    <el-page-header>
      <template #title>驱动管理</template>
      <template #subtitle>管理系统中的驱动程序</template>
    </el-page-header>

    <el-card class="driver-card">
      <template #header>
        <div class="card-header">
          <span>驱动列表</span>
          <el-button type="primary" size="small" @click="handleAddDriver">
            <el-icon><Plus /></el-icon>
            添加驱动
          </el-button>
        </div>
      </template>

      <!-- 使用DriverList组件 -->
      <DriverList
        :drivers="drivers"
        :driverTypes="driverTypes"
        :currentPage="currentPage"
        :totalPages="totalPages"
        :totalItems="totalItems"
        @edit="handleEditDriver"
        @delete="handleDeleteDriver"
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

    <!-- 使用DriverForm组件 -->
    <DriverForm
      :visible="dialogVisible"
      :editMode="editMode"
      :driverData="driverForm"
      :driverTypes="driverTypes"
      @close="handleCloseForm"
      @submit="handleSubmitDriver"
    />
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, reactive, watch } from 'vue'
import { Plus } from '@element-plus/icons-vue'
import DriverList from '@/components/driver/DriverList.vue'
import DriverForm from '@/components/driver/DriverForm.vue'
import type { Driver } from '@/services/driverApi'
import { driverApi } from '@/services/driverApi'
import { dictApi } from '@/services/dictApi'

// 驱动列表数据
const drivers = ref<Driver[]>([])
const loading = ref(false)
const currentPage = ref(1)
const pageSize = ref(10)
const totalItems = ref(0)
const totalPages = ref(1)
const driverTypes = ref<any[]>([])

// 对话框相关
const dialogVisible = ref(false)
const editMode = ref(false)
const driverForm = reactive({
  id: '',
  name: '',
  type: '',
  version: '',
  description: '',
  param1_name: '',
  param1_desc: '',
  param2_name: '',
  param2_desc: '',
  param3_name: '',
  param3_desc: '',
  param4_name: '',
  param4_desc: ''
})

// 初始化数据
onMounted(() => {
  fetchDrivers()
  fetchDriverTypes()
})

// 获取驱动列表
const fetchDrivers = async () => {
  loading.value = true
  try {
    const response = await driverApi.getDrivers(currentPage.value, pageSize.value)
    drivers.value = response.items || []
    totalItems.value = response.counts || 0
    totalPages.value = response.pages || 1
  } catch (error) {
    console.error('获取驱动列表失败:', error)
  } finally {
    loading.value = false
  }
}

// 获取驱动类型
const fetchDriverTypes = async () => {
  try {
    const types = await dictApi.getDriverTypes()
    driverTypes.value = types
  } catch (error) {
    console.error('获取驱动类型失败:', error)
  }
}

// 分页处理
const handlePageChange = (page: number) => {
  currentPage.value = page
  fetchDrivers()
}

// 每页条数变化处理
const handlePageSizeChange = (size: number) => {
  pageSize.value = size
  fetchDrivers()
}

// 驱动操作
const handleAddDriver = () => {
  editMode.value = false
  // 重置表单
  Object.assign(driverForm, {
    id: '',
    name: '',
    type: '',
    version: '',
    description: '',
    param1_name: '',
    param1_desc: '',
    param2_name: '',
    param2_desc: '',
    param3_name: '',
    param3_desc: '',
    param4_name: '',
    param4_desc: ''
  })
  dialogVisible.value = true
}

const handleEditDriver = (driver: Driver) => {
  editMode.value = true
  // 填充表单数据
  Object.assign(driverForm, driver)
  dialogVisible.value = true
}

const handleSubmitDriver = async (driverData: any) => {
  try {
    if (editMode.value) {
      await driverApi.updateDriver(driverData)
    } else {
      await driverApi.createDriver(driverData)
    }
    dialogVisible.value = false
    fetchDrivers()
  } catch (error) {
    console.error('保存驱动失败:', error)
  }
}

const handleCloseForm = () => {
  dialogVisible.value = false
}

const handleDeleteDriver = async (id: string) => {
  try {
    await driverApi.deleteDriver(id)
    fetchDrivers()
  } catch (error) {
    console.error('删除驱动失败:', error)
  }
}
</script>

<style scoped>
.driver-view {
  padding: 20px;
  background-color: #f5f7fa;
  min-height: 100vh;
}

.driver-card {
  margin-top: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
</style>