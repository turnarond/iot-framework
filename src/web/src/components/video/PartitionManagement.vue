<template>
  <div class="partition-management">
    <div class="management-header">
      <div class="header-left">
        <h2 class="page-title">分区管理</h2>
        <p class="page-description">创建和管理摄像头分区</p>
      </div>
      <div class="header-right">
        <el-input v-model="searchQuery" placeholder="搜索分区" class="search-input">
          <template #prefix>
            <el-icon><Search /></el-icon>
          </template>
        </el-input>
        <el-button type="primary" @click="openAddPartitionDialog">
          <el-icon><Plus /></el-icon>
          添加分区
        </el-button>
      </div>
    </div>

    <el-table :data="filteredPartitions" style="width: 100%" class="partition-table">
      <el-table-column prop="name" label="分区名称" width="180" />
      <el-table-column label="摄像头数量" width="120">
        <template #default="{ row }">
          <span>{{ row.camera_count || row.cameraCount || 0 }}</span>
        </template>
      </el-table-column>
      <el-table-column prop="description" label="描述" />
      <el-table-column label="操作" width="180" fixed="right">
        <template #default="{ row }">
          <div class="action-buttons">
            <el-button size="small" @click="openEditPartitionDialog(row)">
              <el-icon><Edit /></el-icon>
              编辑
            </el-button>
            <el-button 
              size="small" 
              type="danger" 
              @click="deletePartition(row.id)"
              :disabled="(row.camera_count || row.cameraCount || 0) > 0"
            >
              <el-icon><Delete /></el-icon>
              删除
            </el-button>
          </div>
        </template>
      </el-table-column>
    </el-table>

    <!-- 添加/编辑分区对话框 -->
    <el-dialog
      v-model="dialogVisible"
      :title="isEditing ? '编辑分区' : '添加分区'"
      width="500px"
    >
      <el-form :model="partitionForm" :rules="partitionRules" ref="partitionFormRef" label-width="80px">
        <el-form-item label="分区名称" prop="name">
          <el-input v-model="partitionForm.name" placeholder="请输入分区名称" />
        </el-form-item>
        <el-form-item label="描述">
          <el-input v-model="partitionForm.description" type="textarea" placeholder="请输入分区描述" />
        </el-form-item>
      </el-form>
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="dialogVisible = false">取消</el-button>
          <el-button type="primary" @click="savePartition">保存</el-button>
        </span>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted } from 'vue'
import { Plus, Search, Edit, Delete } from '@element-plus/icons-vue'
import type { FormInstance } from 'element-plus'
import { partitionApi } from '@/services/videoApi'
import type { Partition } from '@/types'

const dialogVisible = ref(false)
const isEditing = ref(false)
const searchQuery = ref('')
const partitionFormRef = ref<FormInstance>()
const loading = ref(false)
const error = ref('')

const partitions = ref<Partition[]>([])

const partitionForm = ref<Partition>({
  id: '',
  name: '',
  cameraCount: 0,
  description: ''
})

const partitionRules = ref({
  name: [{ required: true, message: '请输入分区名称', trigger: 'blur' }]
})

const filteredPartitions = computed(() => {
  if (!searchQuery.value) {
    return partitions.value
  }
  const query = searchQuery.value.toLowerCase()
  return partitions.value.filter(partition => 
    partition.name.toLowerCase().includes(query) ||
    partition.description.toLowerCase().includes(query)
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

const openAddPartitionDialog = () => {
  isEditing.value = false
  partitionForm.value = {
    id: '',
    name: '',
    cameraCount: 0,
    description: ''
  }
  dialogVisible.value = true
}

const openEditPartitionDialog = (partition: Partition) => {
  isEditing.value = true
  partitionForm.value = { ...partition }
  dialogVisible.value = true
}

const savePartition = async () => {
  if (!partitionFormRef.value) return
  
  try {
    await partitionFormRef.value.validate()
    loading.value = true
    
    if (isEditing.value) {
      // 更新现有分区
      await partitionApi.updatePartition(partitionForm.value)
    } else {
      // 添加新分区
      await partitionApi.createPartition(partitionForm.value)
    }
    
    dialogVisible.value = false
    await loadPartitions()
  } catch (error) {
    console.error('保存分区失败:', error)
  } finally {
    loading.value = false
  }
}

const deletePartition = async (id: string) => {
  try {
    loading.value = true
    await partitionApi.deletePartition(id)
    await loadPartitions()
  } catch (error) {
    console.error('删除分区失败:', error)
  } finally {
    loading.value = false
  }
}

onMounted(() => {
  loadPartitions()
})
</script>

<style scoped>
.partition-management {
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

.partition-table {
  background-color: #ffffff;
  border-radius: 12px;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.08);
  overflow: hidden;
}

.dark-mode .partition-table {
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
  .partition-management {
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