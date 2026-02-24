<template>
  <div class="camera-management">
    <div class="management-header">
      <div class="header-left">
        <h2 class="page-title">摄像头管理</h2>
        <p class="page-description">添加、编辑和管理摄像头设备</p>
      </div>
      <div class="header-right">
        <el-input v-model="searchQuery" placeholder="搜索摄像头" class="search-input">
          <template #prefix>
            <el-icon>
              <Search />
            </el-icon>
          </template>
        </el-input>
        <el-button type="primary" @click="openAddCameraDialog">
          <el-icon>
            <Plus />
          </el-icon>
          添加摄像头
        </el-button>
      </div>
    </div>

    <el-table :data="filteredCameras" style="width: 100%" class="camera-table">
      <el-table-column prop="name" label="摄像头名称" width="180" />
      <el-table-column prop="model" label="型号" width="120" />
      <el-table-column prop="ip" label="IP地址" width="150" />
      <el-table-column prop="partition_name" label="所属分区" width="120" />
      <el-table-column prop="stream_protocol" label="取流协议" width="100" />
      <el-table-column prop="control_protocol" label="控制协议" width="100" />
      <el-table-column prop="status" label="状态" width="80">
        <template #default="{ row }">
          <el-tag :type="row.status === 'online' ? 'success' : 'danger'">
            {{ row.status === 'online' ? '在线' : '离线' }}
          </el-tag>
        </template>
      </el-table-column>
      <el-table-column label="操作" width="180" fixed="right">
        <template #default="{ row }">
          <div class="action-buttons">
            <el-button size="small" @click="openEditCameraDialog(row)">
              <el-icon>
                <Edit />
              </el-icon>
              编辑
            </el-button>
            <el-button size="small" type="danger" @click="deleteCamera(row.id)">
              <el-icon>
                <Delete />
              </el-icon>
              删除
            </el-button>
          </div>
        </template>
      </el-table-column>
    </el-table>

    <!-- 添加/编辑摄像头对话框 -->
    <el-dialog v-model="dialogVisible" :title="isEditing ? '编辑摄像头' : '添加摄像头'" width="600px">
      <el-form :model="cameraForm" :rules="cameraRules" ref="cameraFormRef" label-width="100px">
        <el-form-item label="摄像头名称" prop="name">
          <el-input v-model="cameraForm.name" placeholder="请输入摄像头名称" />
        </el-form-item>
        <el-form-item label="所属分区" prop="partition_id">
          <el-select v-model="cameraForm.partition_id" placeholder="请选择分区">
            <el-option v-for="partition in partitions" :key="partition.id" :label="partition.name"
              :value="partition.id" />
          </el-select>
        </el-form-item>
        <el-form-item label="摄像头型号" prop="model">
          <el-input v-model="cameraForm.model" placeholder="请输入摄像头型号" />
        </el-form-item>
        <el-form-item label="IP地址" prop="ip">
          <el-input v-model="cameraForm.ip" placeholder="请输入摄像头IP地址" />
        </el-form-item>
        <el-form-item label="端口" prop="port">
          <el-input-number v-model="cameraForm.port" :min="1" :max="65535" />
        </el-form-item>
        <el-form-item label="用户名" prop="username">
          <el-input v-model="cameraForm.username" placeholder="请输入用户名" />
        </el-form-item>
        <el-form-item label="密码" prop="password">
          <el-input v-model="cameraForm.password" type="password" placeholder="请输入密码" />
        </el-form-item>
        <el-form-item label="取流协议" prop="stream_protocol">
          <el-select v-model="cameraForm.stream_protocol" placeholder="请选择取流协议" @change="handleStreamProtocolChange">
            <el-option v-for="protocol in streamProtocols" :key="protocol.id" :label="protocol.name"
              :value="protocol.name" />
          </el-select>
        </el-form-item>
        <!-- 动态显示取流协议URL输入框 -->
        <el-form-item v-if="selectedStreamProtocol" :label="selectedStreamProtocol.params">
          <el-input v-model="cameraForm.stream_protocol_param" :placeholder="selectedStreamProtocol.params_desc || '请输入URL'" />
        </el-form-item>
        <el-form-item label="控制协议" prop="control_protocol">
          <el-select v-model="cameraForm.control_protocol" placeholder="请选择控制协议" @change="handleControlProtocolChange">
            <el-option v-for="protocol in controlProtocols" :key="protocol.id" :label="protocol.name"
              :value="protocol.name" />
          </el-select>
        </el-form-item>
        <!-- 动态显示控制协议URL输入框 -->
        <el-form-item v-if="selectedControlProtocol" :label="selectedControlProtocol.params">
          <el-input v-model="cameraForm.control_protocol_param" :placeholder="selectedControlProtocol.params_desc || '请输入URL'" />
        </el-form-item>
        <el-form-item label="备注">
          <el-input v-model="cameraForm.description" type="textarea" placeholder="请输入备注信息" />
        </el-form-item>
      </el-form>
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="dialogVisible = false">取消</el-button>
          <el-button type="primary" @click="saveCamera">保存</el-button>
        </span>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted } from 'vue'
import { Plus, Search, Edit, Delete } from '@element-plus/icons-vue'
import type { FormInstance } from 'element-plus'
import { cameraApi, partitionApi } from '@/services/videoApi'
import { dictApi } from '@/services/dictApi'
import type { Camera, Partition, DictProtocol } from '@/types'

const emit = defineEmits(['cameraAdded'])

const dialogVisible = ref(false)
const isEditing = ref(false)
const searchQuery = ref('')
const cameraFormRef = ref<FormInstance>()
const loading = ref(false)
const error = ref('')

const partitions = ref<Partition[]>([])
const cameras = ref<Camera>([])
const streamProtocols = ref<DictProtocol[]>([])
const controlProtocols = ref<DictProtocol[]>([])
const selectedStreamProtocol = ref<DictProtocol | null>(null)
const selectedControlProtocol = ref<DictProtocol | null>(null)

const cameraForm = ref<Camera>({
  id: '',
  name: '',
  model: '',
  ip: '',
  port: 554,
  username: '',
  password: '',
  stream_protocol: '',
  stream_protocol_id: 0,
  stream_protocol_param: '',
  control_protocol: '',
  control_protocol_id: 0,
  control_protocol_param: '',
  partition_id: '',
  partition_name: '',
  status: 'offline',
  description: ''
})

const cameraRules = ref({
  name: [{ required: true, message: '请输入摄像头名称', trigger: 'blur' }],
  partition_id: [{ required: true, message: '请选择所属分区', trigger: 'change' }],
  ip: [{ required: true, message: '请输入IP地址', trigger: 'blur' }],
  model: [{ required: true, message: '请输入摄像头型号', trigger: 'blur' }],
  stream_protocol_id: [{ required: true, message: '请选择取流协议', trigger: 'change' }],
  control_protocol_id: [{ required: true, message: '请选择控制协议', trigger: 'change' }]
})

const filteredCameras = computed(() => {
  if (!searchQuery.value) {
    return cameras.value
  }
  const query = searchQuery.value.toLowerCase()
  return cameras.value.filter(camera =>
    camera.name.toLowerCase().includes(query) ||
    camera.ip.toLowerCase().includes(query) ||
    camera.model.toLowerCase().includes(query) ||
    camera.partition_name.toLowerCase().includes(query) ||
    camera.stream_protocol.toLowerCase().includes(query) ||
    camera.control_protocol.toLowerCase().includes(query)
  )
})

const loadPartitions = async () => {
  try {
    loading.value = true
    error.value = ''
    const response = await partitionApi.getPartitions()
    partitions.value = response.items
  } catch (err) {
    error.value = '加载分区失败'
    console.error('Failed to load partitions:', err)
  } finally {
    loading.value = false
  }
}

const loadCameras = async () => {
  try {
    loading.value = true
    error.value = ''
    const response = await cameraApi.getCameras()
    cameras.value = response.items
  } catch (err) {
    error.value = '加载摄像头失败'
    console.error('Failed to load cameras:', err)
  } finally {
    loading.value = false
  }
}

const loadStreamProtocols = async () => {
  try {
    const response = await dictApi.getStreamProtocols()
    streamProtocols.value = response
  } catch (err) {
    console.error('Failed to load stream protocols:', err)
  }
}

const loadControlProtocols = async () => {
  try {
    const response = await dictApi.getControlProtocols()
    controlProtocols.value = response
  } catch (err) {
    console.error('Failed to load control protocols:', err)
  }
}

const handleStreamProtocolChange = (protocolName: string) => {
  selectedStreamProtocol.value = streamProtocols.value.find(protocol => protocol.name === protocolName) || null
  if (selectedStreamProtocol.value) {
    cameraForm.value.stream_protocol_id = parseInt(selectedStreamProtocol.value.id)
  } else {
    cameraForm.value.stream_protocol_id = 0
  }
}

const handleControlProtocolChange = (protocolName: string) => {
  selectedControlProtocol.value = controlProtocols.value.find(protocol => protocol.name === protocolName) || null
  if (selectedControlProtocol.value) {
    cameraForm.value.control_protocol_id = parseInt(selectedControlProtocol.value.id)
  } else {
    cameraForm.value.control_protocol_id = 0
  }
}

const openAddCameraDialog = async () => {
  isEditing.value = false
  // 重新加载分区列表，确保包含最新添加的分区
  await loadPartitions()
  
  const defaultStreamProtocol = streamProtocols.value.length > 0 ? streamProtocols.value[0].name : ''
  const defaultControlProtocol = controlProtocols.value.length > 0 ? controlProtocols.value[0].name : ''
  
  cameraForm.value = {
    id: '',
    name: '',
    model: '',
    ip: '',
    port: 554,
    username: '',
    password: '',
    stream_protocol: defaultStreamProtocol,
    stream_protocol_id: 0,
    stream_protocol_param: '',
    control_protocol: defaultControlProtocol,
    control_protocol_id: 0,
    control_protocol_param: '',
    partition_id: '',
    partition_name: '',
    status: 'offline',
    description: ''
  }
  
  // 触发协议选择变化，显示对应的URL输入框并设置协议ID
  handleStreamProtocolChange(defaultStreamProtocol)
  handleControlProtocolChange(defaultControlProtocol)
  
  dialogVisible.value = true
}

const openEditCameraDialog = (camera: Camera) => {
  isEditing.value = true
  cameraForm.value = { ...camera }
  
  // 触发协议选择变化，显示对应的URL输入框并设置协议ID
  handleStreamProtocolChange(camera.stream_protocol)
  handleControlProtocolChange(camera.control_protocol)
  
  dialogVisible.value = true
}

const saveCamera = async () => {
  if (!cameraFormRef.value) return

  try {
    await cameraFormRef.value.validate()
    loading.value = true

    if (isEditing.value) {
      // 更新现有摄像头
      await cameraApi.updateCamera(cameraForm.value)
    } else {
      // 添加新摄像头
      await cameraApi.createCamera(cameraForm.value)
      // 通知父组件摄像头已添加，以便刷新视频预览
      emit('cameraAdded')
    }

    dialogVisible.value = false
    await loadCameras()
  } catch (error) {
    console.error('保存摄像头失败:', error)
  } finally {
    loading.value = false
  }
}

const deleteCamera = async (id: string) => {
  try {
    loading.value = true
    await cameraApi.deleteCamera(id)
    await loadCameras()
  } catch (error) {
    console.error('删除摄像头失败:', error)
  } finally {
    loading.value = false
  }
}

onMounted(() => {
  loadPartitions()
  loadCameras()
  loadStreamProtocols()
  loadControlProtocols()
})
</script>

<style scoped>
.camera-management {
  padding: 30px;
  min-height: 100vh;
}

.management-header {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  margin-bottom: 30px;
  padding-bottom: 20px;
  border-bottom: 1px solid #f0f0f0;
}

.dark-mode .management-header {
  border-bottom: 1px solid #333;
}

.header-left {
  flex: 1;
}

.page-title {
  font-size: 24px;
  font-weight: 700;
  margin: 0 0 8px 0;
  color: #303133;
}

.dark-mode .page-title {
  color: #ffffff;
}

.page-description {
  font-size: 14px;
  color: #606266;
  margin: 0;
}

.dark-mode .page-description {
  color: #909399;
}

.header-right {
  display: flex;
  align-items: center;
  gap: 16px;
}

.search-input {
  width: 300px;
}

.camera-table {
  background-color: #ffffff;
  border-radius: 12px;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.08);
  overflow: hidden;
}

.dark-mode .camera-table {
  background-color: #1f1f1f;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.3);
}

.action-buttons {
  display: flex;
  gap: 8px;
}

.dialog-footer {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
}

@media (max-width: 768px) {
  .camera-management {
    padding: 20px;
  }

  .management-header {
    flex-direction: column;
    align-items: flex-start;
    gap: 16px;
  }

  .header-right {
    width: 100%;
    flex-direction: column;
    align-items: flex-start;
    gap: 12px;
  }

  .search-input {
    width: 100%;
  }

  .action-buttons {
    flex-direction: column;
    gap: 4px;
  }
}
</style>