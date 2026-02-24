<template>
  <div class="device-view">
    <el-page-header>
      <template #title>设备管理</template>
      <template #subtitle>管理系统中的设备</template>
    </el-page-header>

    <el-card class="device-card">
      <template #header>
        <div class="card-header">
          <span>设备列表</span>
          <el-button type="primary" size="small" @click="handleAddDevice">
            <el-icon><Plus /></el-icon>
            添加设备
          </el-button>
        </div>
      </template>

      <!-- 使用DeviceList组件 -->
      <DeviceList
        :devices="devices"
        :drivers="drivers"
        :currentPage="currentPage"
        :totalPages="totalPages"
        :totalItems="totalItems"
        @edit="handleEditDevice"
        @delete="handleDeleteDevice"
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

    <!-- 使用DeviceForm组件 -->
    <DeviceForm
      :visible="dialogVisible"
      :editMode="editMode"
      :deviceData="deviceForm"
      :drivers="drivers"
      :connTypes="connTypes"
      @close="handleCloseForm"
      @submit="handleSubmitDevice"
    />
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, reactive, watch } from 'vue'
import { Plus } from '@element-plus/icons-vue'
import DeviceList from '@/components/device/DeviceList.vue'
import DeviceForm from '@/components/device/DeviceForm.vue'
import type { Device } from '@/services/deviceApi'
import { deviceApi } from '@/services/deviceApi'
import { driverApi } from '@/services/driverApi'
import type { Driver } from '@/services/driverApi'
import { dictApi } from '@/services/dictApi'

// 设备列表数据
const devices = ref<Device[]>([])
const loading = ref(false)
const currentPage = ref(1)
const pageSize = ref(10)
const totalItems = ref(0)
const totalPages = ref(1)

// 驱动列表（用于设备绑定）
const drivers = ref<Driver[]>([])

// 连接类型列表（用于设备表单）
const connTypes = ref<any[]>([])

// 对话框相关
const dialogVisible = ref(false)
const editMode = ref(false)
const deviceForm = reactive({
  id: '',
  name: '',
  driverId: '',
  address: '',
  description: ''
})

// 初始化数据
onMounted(() => {
  fetchDevices()
  fetchDrivers()
  fetchConnTypes()
})

// 获取设备列表
const fetchDevices = async () => {
  loading.value = true
  try {
    const response = await deviceApi.getDevices(currentPage.value, pageSize.value)
    devices.value = response.items || []
    totalItems.value = response.counts || 0
    totalPages.value = response.pages || 1
  } catch (error) {
    console.error('获取设备列表失败:', error)
  } finally {
    loading.value = false
  }
}

// 获取驱动列表
const fetchDrivers = async () => {
  try {
    const response = await driverApi.getAllDrivers()
    drivers.value = response
  } catch (error) {
    console.error('获取驱动列表失败:', error)
  }
}

// 获取连接类型
const fetchConnTypes = async () => {
  try {
    const types = await dictApi.getConnTypes()
    connTypes.value = types
  } catch (error) {
    console.error('获取连接类型失败:', error)
  }
}

// 分页处理
const handlePageChange = (page: number) => {
  currentPage.value = page
  fetchDevices()
}

// 每页条数变化处理
const handlePageSizeChange = (size: number) => {
  pageSize.value = size
  fetchDevices()
}

// 设备操作
const handleAddDevice = () => {
  editMode.value = false
  // 重置表单
  Object.assign(deviceForm, {
    id: '',
    name: '',
    driverId: '',
    address: '',
    description: ''
  })
  dialogVisible.value = true
}

const handleEditDevice = (device: Device) => {
  editMode.value = true
  // 填充表单数据
  Object.assign(deviceForm, device)
  dialogVisible.value = true
}

const handleSubmitDevice = async (deviceData: any) => {
  try {
    if (editMode.value) {
      await deviceApi.updateDevice(deviceData)
    } else {
      await deviceApi.createDevice(deviceData)
    }
    dialogVisible.value = false
    fetchDevices()
  } catch (error) {
    console.error('保存设备失败:', error)
  }
}

const handleCloseForm = () => {
  dialogVisible.value = false
}

const handleDeleteDevice = async (id: string) => {
  try {
    await deviceApi.deleteDevice(id)
    fetchDevices()
  } catch (error) {
    console.error('删除设备失败:', error)
  }
}
</script>

<style scoped>
.device-view {
  padding: 20px;
  background-color: #f5f7fa;
  min-height: 100vh;
}

.device-card {
  margin-top: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
</style>