<template>
  <div class="point-view">
    <el-page-header>
      <template #title>点位管理</template>
      <template #subtitle>管理系统中的点位</template>
    </el-page-header>

    <el-card class="point-card">
      <template #header>
        <div class="card-header">
          <span>点位列表</span>
          <el-button type="primary" size="small" @click="handleAddPoint">
            <el-icon><Plus /></el-icon>
            添加点位
          </el-button>
        </div>
      </template>

      <!-- 使用PointList组件 -->
      <PointList
        :points="points"
        :currentPage="currentPage"
        :totalPages="totalPages"
        :totalItems="totalItems"
        @edit="handleEditPoint"
        @view="handleViewPoint"
        @delete="handleDeletePoint"
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

    <!-- 使用PointForm组件 -->
    <PointForm
      :visible="dialogVisible"
      :editMode="editMode"
      :pointData="pointForm"
      :devices="devices"
      :dataTypes="dataTypes"
      :pointTypes="pointTypes"
      :transferMethods="transferMethods"
      @close="handleCloseForm"
      @submit="handleSubmitPoint"
    />

    <!-- 查看点位对话框 -->
    <el-dialog
      v-model="viewDialogVisible"
      title="点位详情"
      width="500px"
    >
      <el-descriptions :column="1" border>
        <el-descriptions-item label="点位ID">{{ viewPointData.id }}</el-descriptions-item>
        <el-descriptions-item label="点位名称">{{ viewPointData.name }}</el-descriptions-item>
        <el-descriptions-item label="设备名称">{{ viewPointData.deviceName }}</el-descriptions-item>
        <el-descriptions-item label="点位地址">{{ viewPointData.address }}</el-descriptions-item>
        <el-descriptions-item label="数据类型">{{ viewPointData.type }}</el-descriptions-item>
        <el-descriptions-item label="当前值">{{ viewPointData.value }}</el-descriptions-item>
        <el-descriptions-item label="单位">{{ viewPointData.unit }}</el-descriptions-item>
        <el-descriptions-item label="点位描述">{{ viewPointData.description }}</el-descriptions-item>
      </el-descriptions>
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="viewDialogVisible = false">关闭</el-button>
        </span>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, reactive, watch } from 'vue'
import { Plus } from '@element-plus/icons-vue'
import PointList from '@/components/point/PointList.vue'
import PointForm from '@/components/point/PointForm.vue'
import type { Point } from '@/services/pointApi'
import { pointApi } from '@/services/pointApi'
import { deviceApi } from '@/services/deviceApi'
import type { Device } from '@/services/deviceApi'
import { dictApi } from '@/services/dictApi'

// 点位列表数据
const points = ref<Point[]>([])
const loading = ref(false)
const currentPage = ref(1)
const pageSize = ref(10)
const totalItems = ref(0)
const totalPages = ref(1)

// 设备列表（用于点位绑定）
const devices = ref<Device[]>([])

// 类型列表（用于点位表单）
const dataTypes = ref<any[]>([])
const pointTypes = ref<any[]>([])
const transferMethods = ref<any[]>([])

// 对话框相关
const dialogVisible = ref(false)
const viewDialogVisible = ref(false)
const editMode = ref(false)
const pointForm = reactive({
  id: '',
  name: '',
  deviceId: '',
  address: '',
  type: 'float',
  unit: '',
  description: ''
})

// 查看点位数据
const viewPointData = reactive({
  id: '',
  name: '',
  deviceName: '',
  address: '',
  type: '',
  value: '',
  unit: '',
  description: ''
})

// 初始化数据
onMounted(() => {
  fetchPoints()
  fetchDevices()
  fetchDataTypes()
  fetchPointTypes()
  fetchTransferMethods()
})

// 获取点位列表
const fetchPoints = async () => {
  loading.value = true
  try {
    const response = await pointApi.getPointsWithDevice(currentPage.value, pageSize.value)
    points.value = response.items
    totalItems.value = response.counts
    totalPages.value = Math.ceil(response.counts / pageSize.value)
  } catch (error) {
    console.error('获取点位列表失败:', error)
  } finally {
    loading.value = false
  }
}

// 获取设备列表
const fetchDevices = async () => {
  try {
    const response = await deviceApi.getDevices()
    devices.value = response.items
  } catch (error) {
    console.error('获取设备列表失败:', error)
  }
}

// 获取数据类型
const fetchDataTypes = async () => {
  try {
    const types = await dictApi.getDataTypes()
    dataTypes.value = types
  } catch (error) {
    console.error('获取数据类型失败:', error)
  }
}

// 获取点位类型
const fetchPointTypes = async () => {
  try {
    const types = await dictApi.getPointTypes()
    pointTypes.value = types
  } catch (error) {
    console.error('获取点位类型失败:', error)
  }
}

// 获取传输方法
const fetchTransferMethods = async () => {
  try {
    const methods = await dictApi.getTransferMethods()
    transferMethods.value = methods
  } catch (error) {
    console.error('获取传输方法失败:', error)
  }
}

// 分页处理
const handlePageChange = (page: number) => {
  currentPage.value = page
  fetchPoints()
}

// 每页条数变化处理
const handlePageSizeChange = (size: number) => {
  pageSize.value = size
  fetchPoints()
}

// 点位操作
const handleAddPoint = () => {
  editMode.value = false
  // 重置表单
  Object.assign(pointForm, {
    id: '',
    name: '',
    deviceId: '',
    address: '',
    type: 'float',
    unit: '',
    description: ''
  })
  dialogVisible.value = true
}

const handleEditPoint = (point: Point) => {
  editMode.value = true
  // 填充表单数据
  Object.assign(pointForm, point)
  dialogVisible.value = true
}

const handleViewPoint = (point: Point) => {
  // 填充查看数据
  Object.assign(viewPointData, point)
  viewDialogVisible.value = true
}

const handleSubmitPoint = async (pointData: any) => {
  try {
    if (editMode.value) {
      await pointApi.updatePoint(pointData)
    } else {
      await pointApi.createPoint(pointData)
    }
    dialogVisible.value = false
    fetchPoints()
  } catch (error) {
    console.error('保存点位失败:', error)
  }
}

const handleCloseForm = () => {
  dialogVisible.value = false
}

const handleDeletePoint = async (id: string) => {
  try {
    await pointApi.deletePoint(id)
    fetchPoints()
  } catch (error) {
    console.error('删除点位失败:', error)
  }
}
</script>

<style scoped>
.point-view {
  padding: 20px;
  background-color: #f5f7fa;
  min-height: 100vh;
}

.point-card {
  margin-top: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
</style>