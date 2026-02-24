<template>
  <el-dialog
    v-model="dialogVisible"
    :title="actionData.id ? '编辑动作' : '创建动作'"
    width="600px"
    @close="handleClose"
  >
    <el-form :model="form" label-width="120px">
      <!-- 动作类型选择 -->
      <el-form-item label="动作类型" required>
        <el-select
          v-model="form.action_type_id"
          placeholder="请选择动作类型"
          @change="handleActionTypeChange"
        >
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

      <!-- 动作名称 -->
      <el-form-item label="动作名称" required>
        <el-input v-model="form.name" placeholder="请输入动作名称" />
      </el-form-item>

      <!-- 动作描述 -->
      <el-form-item label="描述">
        <el-input
          v-model="form.description"
          type="textarea"
          placeholder="请输入动作描述"
          rows="3"
        />
      </el-form-item>

      <!-- 基于选择的动作类型显示不同的参数选项 -->
      <div v-if="selectedActionType" class="action-params-config">
        <h4 class="params-title">动作参数</h4>
        
        <!-- 参数1 -->
        <el-form-item
          v-if="selectedActionType.param1_name"
          :label="selectedActionType.param1_name"
          :required="true"
        >
          <el-input
            v-model="form.param1"
            :placeholder="selectedActionType.param1_tip || '请输入参数值'"
          >
            <template #append v-if="selectedActionType.param1_tip">
              <el-tooltip :content="selectedActionType.param1_tip" placement="top">
                <el-icon class="info-icon"><QuestionFilled /></el-icon>
              </el-tooltip>
            </template>
          </el-input>
        </el-form-item>

        <!-- 参数2 -->
        <el-form-item
          v-if="selectedActionType.param2_name"
          :label="selectedActionType.param2_name"
          :required="true"
        >
          <el-input
            v-model="form.param2"
            :placeholder="selectedActionType.param2_tip || '请输入参数值'"
          >
            <template #append v-if="selectedActionType.param2_tip">
              <el-tooltip :content="selectedActionType.param2_tip" placement="top">
                <el-icon class="info-icon"><QuestionFilled /></el-icon>
              </el-tooltip>
            </template>
          </el-input>
        </el-form-item>

        <!-- 参数3 -->
        <el-form-item
          v-if="selectedActionType.param3_name"
          :label="selectedActionType.param3_name"
        >
          <el-input
            v-model="form.param3"
            :placeholder="selectedActionType.param3_tip || '请输入参数值'"
          >
            <template #append v-if="selectedActionType.param3_tip">
              <el-tooltip :content="selectedActionType.param3_tip" placement="top">
                <el-icon class="info-icon"><QuestionFilled /></el-icon>
              </el-tooltip>
            </template>
          </el-input>
        </el-form-item>

        <!-- 参数4 -->
        <el-form-item
          v-if="selectedActionType.param4_name"
          :label="selectedActionType.param4_name"
        >
          <el-input
            v-model="form.param4"
            :placeholder="selectedActionType.param4_tip || '请输入参数值'"
          >
            <template #append v-if="selectedActionType.param4_tip">
              <el-tooltip :content="selectedActionType.param4_tip" placement="top">
                <el-icon class="info-icon"><QuestionFilled /></el-icon>
              </el-tooltip>
            </template>
          </el-input>
        </el-form-item>

        <!-- 是否需要确认 -->
        <el-form-item label="需要确认">
          <el-switch v-model="form.need_confirm" />
          <span class="switch-description">执行此动作前是否需要用户确认</span>
        </el-form-item>
      </div>
    </el-form>

    <template #footer>
      <span class="dialog-footer">
        <el-button @click="handleClose">取消</el-button>
        <el-button type="primary" @click="handleSave">保存</el-button>
      </span>
    </template>
  </el-dialog>
</template>

<script>
import { ref, computed, watch, onMounted } from 'vue';
import { useLinkageApi } from '../../hooks/useApi';
import { QuestionFilled } from '@element-plus/icons-vue';

export default {
  name: 'ActionForm',
  props: {
    visible: {
      type: Boolean,
      default: false
    },
    actionData: {
      type: Object,
      default: () => ({})
    }
  },
  emits: ['close', 'saved'],
  setup(props, { emit }) {
    const dialogVisible = computed({
      get: () => props.visible,
      set: (value) => {
        if (!value) {
          emit('close');
        }
      }
    });

    const { getActionTypes, createAction, updateAction } = useLinkageApi();

    // 表单数据
    const form = ref({
      id: '',
      name: '',
      description: '',
      action_type_id: '',
      param1: '',
      param2: '',
      param3: null,
      param4: null,
      need_confirm: false
    });

    // 动作类型列表
    const actionTypes = ref([]);
    // 当前选择的动作类型
    const selectedActionType = computed(() => {
      if (!form.value.action_type_id) return null;
      return actionTypes.value.find(type => type.id === form.value.action_type_id);
    });

    // 加载动作类型列表
    const loadActionTypes = async () => {
      try {
        const response = await getActionTypes.call(1, 100);
        actionTypes.value = response.items || [];
      } catch (error) {
        console.error('加载动作类型失败:', error);
        actionTypes.value = [];
      }
    };

    // 处理动作类型变化
    const handleActionTypeChange = () => {
      // 重置参数值
      if (selectedActionType.value) {
        form.value.param1 = '';
        form.value.param2 = '';
        form.value.param3 = null;
        form.value.param4 = null;
      }
    };

    // 处理表单保存
    const handleSave = async () => {
      // 表单验证
      if (!form.value.action_type_id) {
        alert('请选择动作类型');
        return;
      }
      if (!form.value.name) {
        alert('请输入动作名称');
        return;
      }

      // 验证必填参数
      if (selectedActionType.value) {
        if (selectedActionType.value.param1_name && !form.value.param1) {
          alert(`请输入${selectedActionType.value.param1_name}`);
          return;
        }
        if (selectedActionType.value.param2_name && !form.value.param2) {
          alert(`请输入${selectedActionType.value.param2_name}`);
          return;
        }
      }

      try {
        const actionData = {
          id: form.value.id || undefined,
          name: form.value.name,
          description: form.value.description,
          action_type_id: form.value.action_type_id,
          param1: form.value.param1 || '',
          param2: form.value.param2 || '',
          param3: form.value.param3 !== null ? form.value.param3 : null,
          param4: form.value.param4 !== null ? form.value.param4 : null,
          need_confirm: !!form.value.need_confirm
        };

        let result;
        if (form.value.id) {
          result = await updateAction.call(actionData);
        } else {
          result = await createAction.call(actionData);
        }

        emit('saved', result);
        handleClose();
      } catch (error) {
        console.error('保存动作失败:', error);
        alert('保存失败，请检查日志');
      }
    };

    // 处理关闭
    const handleClose = () => {
      emit('close');
    };

    // 监听actionData变化
    watch(() => props.actionData, (newData) => {
      if (newData && Object.keys(newData).length) {
        form.value = {
          id: newData.id || '',
          name: newData.name || '',
          description: newData.description || '',
          action_type_id: newData.action_type_id || newData.type || '',
          param1: newData.param1 !== undefined ? newData.param1 : (newData.params && newData.params.param1) || '',
          param2: newData.param2 !== undefined ? newData.param2 : (newData.params && newData.params.param2) || '',
          param3: newData.param3 !== undefined ? newData.param3 : (newData.params && newData.params.param3) || null,
          param4: newData.param4 !== undefined ? newData.param4 : (newData.params && newData.params.param4) || null,
          need_confirm: !!((newData.need_confirm !== undefined ? newData.need_confirm : newData.needConfirm))
        };
      }
    }, { immediate: true });

    // 初始化
    onMounted(() => {
      loadActionTypes();
    });

    return {
      dialogVisible,
      form,
      actionTypes,
      selectedActionType,
      handleSave,
      handleClose,
      handleActionTypeChange,
      QuestionFilled
    };
  }
};
</script>

<style scoped>
.type-description {
  font-size: 12px;
  color: #666;
  margin-left: 8px;
}

.action-params-config {
  margin-top: 20px;
  padding-top: 20px;
  border-top: 1px solid #eee;
}

.params-title {
  font-size: 16px;
  font-weight: bold;
  margin-bottom: 15px;
  color: #333;
}

.switch-description {
  font-size: 12px;
  color: #666;
  margin-left: 10px;
}

.info-icon {
  color: #1890ff;
  cursor: help;
}

.dialog-footer {
  display: flex;
  justify-content: flex-end;
  gap: 10px;
}
</style>