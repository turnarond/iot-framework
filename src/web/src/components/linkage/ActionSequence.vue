<template>
  <div class="action-sequence">
    <h4>动作序列</h4>
    <div class="actions-list">
      <div v-for="(action, idx) in actions" :key="action._localId" class="action-item">
        <div class="action-header">
          <strong>{{ idx+1 }}. {{ action.name || action.type }}</strong>
          <div class="action-controls">
            <el-button size="small" @click="$emit('move-up', idx)" :disabled="idx===0" style="margin-right: 8px">上移</el-button>
            <el-button size="small" @click="$emit('move-down', idx)" :disabled="idx===actions.length-1" style="margin-right: 8px">下移</el-button>
            <el-button size="small" type="danger" @click="$emit('remove', idx)" style="margin-right: 8px">删除</el-button>
            <el-button size="small" type="primary" @click="handleEditAction(idx)">编辑</el-button>
          </div>
        </div>
        <div class="action-body">
          <div v-if="action.params" v-for="(v,k) in action.params" :key="k">{{ k }}: {{ v }}</div>
          <div v-else-if="action.param1 || action.param2 || action.param3 || action.param4">
            <div v-if="action.param1">param1: {{ action.param1 }}</div>
            <div v-if="action.param2">param2: {{ action.param2 }}</div>
            <div v-if="action.param3">param3: {{ action.param3 }}</div>
            <div v-if="action.param4">param4: {{ action.param4 }}</div>
          </div>
          <div v-else>无参数</div>
        </div>
      </div>
    </div>

    <div class="action-add">
      <el-button type="primary" @click="handleAddAction">添加动作</el-button>
      <el-button @click="openReuseModal" style="margin-left: 8px">复用已有动作</el-button>
    </div>

    <!-- 复用动作模态 -->
    <el-dialog
      v-model="showModal"
      title="选择要复用的动作"
      width="800px"
    >
      <!-- 动作类型筛选 -->
      <div class="action-type-filter" style="margin-bottom: 20px">
        <el-form label-width="100px">
          <el-form-item label="动作类型">
            <el-select v-model="selectedActionType" placeholder="请选择动作类型（可选）">
              <el-option value="" label="全部动作" />
              <el-option
                v-for="type in actionTypes"
                :key="type.id"
                :value="type.id"
                :label="type.cname || type.name"
              >
                <span>{{ type.cname || type.name }}</span>
                <span v-if="type.description" class="type-description">({{ type.description }})</span>
              </el-option>
            </el-select>
          </el-form-item>
        </el-form>
      </div>

      <!-- 动作列表 -->
      <div v-if="loading" class="loading">加载中...</div>
      <div v-else>
        <el-table :data="filteredActions" style="width: 100%" stripe>
          <el-table-column prop="id" label="ID" width="80" />
          <el-table-column prop="name" label="名称" min-width="200" />
          <el-table-column label="动作类型" width="150">
            <template #default="scope">
              <span>{{ actionTypeMap[scope.row.action_type_id] ? (actionTypeMap[scope.row.action_type_id].cname || actionTypeMap[scope.row.action_type_id].name) : scope.row.action_type_id }}</span>
            </template>
          </el-table-column>
          <el-table-column prop="need_confirm" label="需确认" width="80" align="center">
            <template #default="scope">
              <span>{{ scope.row.need_confirm ? '是' : '否' }}</span>
            </template>
          </el-table-column>
          <el-table-column label="操作" width="100" align="center">
            <template #default="scope">
              <el-button type="primary" size="small" @click="selectItem(scope.row)">添加</el-button>
            </template>
          </el-table-column>
        </el-table>
        <div v-if="!filteredActions.length" class="empty">暂无动作实例</div>
      </div>
    </el-dialog>

    <!-- 动作表单 -->
    <ActionForm
      :visible="actionFormVisible"
      :actionData="currentAction"
      @close="handleActionFormClose"
      @saved="handleActionSaved"
    />
  </div>
</template>

<script>
import { ref, onMounted, computed } from 'vue';
import { linkageApi } from '../../services/linkageApi';
import ActionForm from './ActionForm.vue';

export default {
  name: 'ActionSequence',
  components: {
    ActionForm
  },
  props: {
    actions: { type: Array, default: () => [] }
  },
  emits: ['add', 'remove', 'move-up', 'move-down', 'edit'],
  setup(props, { emit }) {
    const uid = () => Math.random().toString(36).slice(2, 9);

    // 动作表单相关状态
    const actionFormVisible = ref(false);
    const currentAction = ref({});
    const editingIndex = ref(-1);

    // 复用动作模态相关状态
    const showModal = ref(false);
    const items = ref([]);
    const loading = ref(false);
    const actionTypes = ref([]);
    const actionTypeMap = ref({});
    const selectedActionType = ref('');

    // 筛选后的动作列表
    const filteredActions = computed(() => {
      if (!selectedActionType.value) {
        return items.value;
      }
      return items.value.filter(item => item.action_type_id === selectedActionType.value);
    });

    // 处理添加动作
    const handleAddAction = () => {
      editingIndex.value = -1;
      currentAction.value = {};
      actionFormVisible.value = true;
    };

    // 处理编辑动作
    const handleEditAction = (idx) => {
      editingIndex.value = idx;
      currentAction.value = { ...props.actions[idx] };
      actionFormVisible.value = true;
    };

    // 处理动作表单关闭
    const handleActionFormClose = () => {
      actionFormVisible.value = false;
      editingIndex.value = -1;
      currentAction.value = {};
    };

    // 处理动作保存
    const handleActionSaved = (actionData) => {
      const action = {
        _localId: uid(),
        id: actionData.id,
        name: actionData.name,
        action_type_id: actionData.action_type_id,
        param1: actionData.param1,
        param2: actionData.param2,
        param3: actionData.param3,
        param4: actionData.param4,
        need_confirm: actionData.need_confirm
      };

      if (editingIndex.value >= 0) {
        // 编辑现有动作
        emit('edit', editingIndex.value, action);
      } else {
        // 添加新动作
        emit('add', action);
      }

      handleActionFormClose();
    };

    // 获取动作类型列表
    const loadActionTypes = async () => {
      try {
        const response = await linkageApi.getActionTypes(1, 100);
        actionTypes.value = response.items || [];
        actionTypeMap.value = {};
        (actionTypes.value || []).forEach(x => actionTypeMap.value[x.id] = x);
      } catch (e) {
        console.error('获取动作类型失败', e);
        actionTypes.value = [];
      }
    };

    const openReuseModal = async () => {
      showModal.value = true;
      loading.value = true;
      try {
        // 并行加载动作列表和动作类型
        const [actionsResponse, typesResponse] = await Promise.all([
          linkageApi.getActions(1, 200),
          linkageApi.getActionTypes(1, 100)
        ]);
        items.value = actionsResponse.items || [];
        actionTypes.value = typesResponse.items || [];
        actionTypeMap.value = {};
        (actionTypes.value || []).forEach(x => actionTypeMap.value[x.id] = x);
      } catch (e) {
        console.error('获取动作列表失败', e);
        items.value = [];
        alert('获取动作失败');
      } finally {
        loading.value = false;
      }
    };

    const closeReuseModal = () => {
      showModal.value = false;
    };

    const selectItem = (chosen) => {
      if (!chosen) return;
      const action = {
        _localId: uid(),
        id: chosen.id,
        name: chosen.name,
        action_type_id: chosen.action_type_id,
        param1: chosen.param1,
        param2: chosen.param2,
        param3: chosen.param3,
        param4: chosen.param4,
        need_confirm: !!chosen.need_confirm
      };
      emit('add', action);
      closeReuseModal();
    };

    onMounted(() => {
      // 可选：预加载动作类型以加快筛选速度
      loadActionTypes();
    });

    return {
      handleAddAction,
      handleEditAction,
      handleActionFormClose,
      handleActionSaved,
      openReuseModal,
      closeReuseModal,
      selectItem,
      showModal,
      items,
      loading,
      actionFormVisible,
      currentAction,
      actionTypes,
      actionTypeMap,
      selectedActionType,
      filteredActions
    };
  }
};
</script>

<style scoped>
.action-item { border:1px solid #eee; padding:16px; margin-bottom:12px; border-radius:4px; }
.action-header { display:flex; justify-content:space-between; align-items:center; margin-bottom:12px; }
.action-body { padding-top:12px; border-top:1px solid #f0f0f0; }
.action-add { margin-top:20px; }

/* 模态样式 */
.type-description {
  font-size: 12px;
  color: #666;
  margin-left: 8px;
}

.loading, .empty {
  padding: 40px 0;
  text-align: center;
  color: #666;
}
</style>
