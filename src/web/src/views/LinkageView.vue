<template>
  <div class="linkage-view">
    <el-page-header>
      <template #title>联动规则</template>
      <template #subtitle>管理系统中的联动规则</template>
    </el-page-header>

    <el-card class="linkage-card">
      <template #header>
        <div class="card-header">
          <span>联动规则列表</span>
          <el-button type="primary" size="small" @click="handleAddLinkageRule">
            <el-icon><Plus /></el-icon>
            添加联动规则
          </el-button>
        </div>
      </template>

      <!-- 使用LinkageList组件 -->
      <LinkageList
        :linkageRules="linkageRules"
        :currentPage="currentPage"
        :totalPages="totalPages"
        :totalItems="totalItems"
        @edit="handleEditLinkageRule"
        @toggle="handleToggleLinkageRule"
        @delete="handleDeleteLinkageRule"
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

    <!-- 使用LinkageForm组件 -->
    <LinkageForm
      :visible="dialogVisible"
      :linkageData="linkageRuleForm"
      @close="handleCloseForm"
      @saved="handleSubmitLinkageRule"
    />
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, reactive, watch } from 'vue'
import { Plus } from '@element-plus/icons-vue'
import LinkageList from '@/components/linkage/LinkageList.vue'
import LinkageForm from '@/components/linkage/LinkageFormWizard.vue'
import type { LinkageRule, Trigger, Action } from '@/services/linkageApi'
import { linkageApi } from '@/services/linkageApi'

// 联动规则列表数据
const linkageRules = ref<LinkageRule[]>([])
const loading = ref(false)
const currentPage = ref(1)
const pageSize = ref(10)
const totalItems = ref(0)
const totalPages = ref(1)

// 触发器和动作列表
const triggers = ref<Trigger[]>([])
const actions = ref<Action[]>([])

// 对话框相关
const dialogVisible = ref(false)
const editMode = ref(false)
const linkageRuleForm = reactive({
  id: '',
  name: '',
  description: '',
  enabled: true,
  triggers: [],
  actions: []
})

// 初始化数据
onMounted(() => {
  fetchLinkageRules()
  fetchTriggers()
  fetchActions()
})

// 获取联动规则列表
const fetchLinkageRules = async () => {
  loading.value = true
  try {
    const response = await linkageApi.getLinkages(currentPage.value, pageSize.value)
    linkageRules.value = response.items
    totalItems.value = response.counts
    totalPages.value = response.pages
  } catch (error) {
    console.error('获取联动规则列表失败:', error)
  } finally {
    loading.value = false
  }
}

// 获取触发器列表
const fetchTriggers = async () => {
  try {
    const response = await linkageApi.getTriggers()
    triggers.value = response.items
  } catch (error) {
    console.error('获取触发器列表失败:', error)
  }
}

// 获取动作列表
const fetchActions = async () => {
  try {
    const response = await linkageApi.getActions()
    actions.value = response.items
  } catch (error) {
    console.error('获取动作列表失败:', error)
  }
}

// 分页处理
const handlePageChange = (page: number) => {
  currentPage.value = page
  fetchLinkageRules()
}

// 每页条数变化处理
const handlePageSizeChange = (size: number) => {
  pageSize.value = size
  fetchLinkageRules()
}

// 联动规则操作
const handleAddLinkageRule = () => {
  editMode.value = false
  // 重置表单
  Object.assign(linkageRuleForm, {
    id: '',
    name: '',
    description: '',
    enabled: true,
    triggers: [],
    actions: []
  })
  dialogVisible.value = true
}

const handleEditLinkageRule = async (rule: LinkageRule) => {
  editMode.value = true
  loading.value = true
  try {
    // 获取完整的联动规则信息，包含触发源和动作详情
    const fullRule = await linkageApi.getLinkageById(rule.id)
    // 填充表单数据
    Object.assign(linkageRuleForm, {
      id: fullRule.id,
      name: fullRule.name,
      description: fullRule.description,
      enabled: fullRule.enable !== undefined ? fullRule.enable : fullRule.enabled,
      triggers: fullRule.triggers || [],
      actions: fullRule.actions || []
    })
    dialogVisible.value = true
  } catch (error) {
    console.error('获取联动规则详情失败:', error)
    alert('获取联动规则详情失败，请检查日志')
  } finally {
    loading.value = false
  }
}

const handleSubmitLinkageRule = async () => {
  try {
    dialogVisible.value = false
    fetchLinkageRules()
  } catch (error) {
    console.error('保存联动规则失败:', error)
  }
}

const handleCloseForm = () => {
  dialogVisible.value = false
}

const handleToggleLinkageRule = async (rule: LinkageRule) => {
  try {
    const updatedRule = {
      ...rule,
      enabled: !rule.enabled
    }
    await linkageApi.updateLinkage(updatedRule)
    fetchLinkageRules()
  } catch (error) {
    console.error('切换联动规则状态失败:', error)
  }
}

const handleDeleteLinkageRule = async (id: string) => {
  try {
    await linkageApi.deleteLinkage(id)
    fetchLinkageRules()
  } catch (error) {
    console.error('删除联动规则失败:', error)
  }
}
</script>

<style scoped>
.linkage-view {
  padding: 20px;
  background-color: #f5f7fa;
  min-height: 100vh;
}

.linkage-card {
  margin-top: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
</style>