<template>
  <el-dialog
    v-model="dialogVisible"
    :title="triggerData.id ? '编辑触发器' : '创建触发器'"
    width="600px"
    @close="handleClose"
  >
    <el-form :model="form" label-width="120px">
      <!-- 触发器类型选择 -->
      <el-form-item label="触发器类型" required>
        <el-select
          v-model="form.event_type_id"
          placeholder="请选择触发器类型"
          @change="handleTriggerTypeChange"
        >
          <el-option
            v-for="type in eventTypes"
            :key="type.id"
            :value="type.id"
            :label="type.cname || type.name"
          >
            <span>{{ type.cname || type.name }}</span>
            <span v-if="type.description" class="type-description">({{ type.description }})</span>
          </el-option>
        </el-select>
      </el-form-item>

      <!-- 触发器名称 -->
      <el-form-item label="触发器名称" required>
        <el-input v-model="form.name" placeholder="请输入触发器名称" />
      </el-form-item>

      <!-- 触发器描述 -->
      <el-form-item label="描述">
        <el-input
          v-model="form.description"
          type="textarea"
          placeholder="请输入触发器描述"
          rows="3"
        />
      </el-form-item>

      <!-- 基于选择的触发器类型显示不同的配置选项 -->
      <div v-if="selectedEventType" class="trigger-specific-config">
        <!-- 这里可以根据不同的触发器类型显示不同的配置选项 -->
        <div v-if="selectedEventType.id === 1 || selectedEventType.id === 2" class="alarm-trigger-config">
          <el-form-item label="报警规则" required>
            <el-select v-model="form.alarm_rule_id" placeholder="请选择报警规则">
              <el-option
                v-for="rule in alarmRules"
                :key="rule.id"
                :value="rule.id"
                :label="rule.name"
              >
                <span>{{ rule.name }}</span>
                <span class="rule-description">
                  (点位: {{ rule.point_name }}, 设备: {{ rule.device_name }})
                </span>
              </el-option>
            </el-select>
          </el-form-item>
        </div>

        <div v-else-if="selectedEventType.id === 3" class="expression-trigger-config">
          <el-form-item label="表达式" required>
            <el-select v-model="form.expr_id" placeholder="请选择表达式">
              <el-option
                v-for="expr in expressions"
                :key="expr.id"
                :value="expr.id"
                :label="expr.name"
              >
                <span>{{ expr.name }}</span>
                <span class="rule-description">({{ expr.expression }})</span>
              </el-option>
            </el-select>
          </el-form-item>
        </div>

        <div v-else-if="selectedEventType.id === 4" class="manual-trigger-config">
          <el-form-item label="自定义事件键" required>
            <el-input v-model="form.custom_event_key" placeholder="请输入自定义事件键" />
          </el-form-item>
        </div>
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
import { useLinkageApi } from '../../hooks/useApi.js';
import { useAlarmApi } from '../../hooks/useApi.js';

export default {
  name: 'TriggerForm',
  props: {
    visible: {
      type: Boolean,
      default: false
    },
    triggerData: {
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

    const { getEventTypes, getExpressions, createTrigger, updateTrigger } = useLinkageApi();
    const { getAlarmRulesWithPoint } = useAlarmApi();

    // 表单数据
    const form = ref({
      id: '',
      name: '',
      description: '',
      event_type_id: '',
      alarm_rule_id: '',
      expr_id: '',
      custom_event_key: '',
      enable: true
    });

    // 事件类型列表
    const eventTypes = ref([]);
    // 报警规则列表
    const alarmRules = ref([]);
    // 表达式列表
    const expressions = ref([]);
    // 当前选择的事件类型
    const selectedEventType = computed(() => {
      if (!form.value.event_type_id) return null;
      return eventTypes.value.find(type => type.id === form.value.event_type_id);
    });

    // 加载事件类型列表
    const loadEventTypes = async () => {
      try {
        const response = await getEventTypes.call(1, 100);
        eventTypes.value = response.items || [];
      } catch (error) {
        console.error('加载事件类型失败:', error);
        eventTypes.value = [];
      }
    };

    // 加载报警规则列表
    const loadAlarmRules = async () => {
      try {
        const response = await getAlarmRulesWithPoint.call(1, 100);
        alarmRules.value = response.items || [];
      } catch (error) {
        console.error('加载报警规则失败:', error);
        alarmRules.value = [];
      }
    };

    // 加载表达式列表
    const loadExpressions = async () => {
      try {
        const response = await getExpressions.call(1, 100);
        expressions.value = response.items || [];
      } catch (error) {
        console.error('加载表达式失败:', error);
        expressions.value = [];
      }
    };

    // 处理触发器类型变化
    const handleTriggerTypeChange = () => {
      // 可以在这里根据选择的触发器类型重置相关表单字段
      if (selectedEventType.value) {
        if (selectedEventType.value.id === 1 || selectedEventType.value.id === 2) {
          loadAlarmRules();
        } else if (selectedEventType.value.id === 3) {
          loadExpressions();
        }
      }
    };

    // 处理表单保存
    const handleSave = async () => {
      // 表单验证
      if (!form.value.event_type_id) {
        alert('请选择触发器类型');
        return;
      }
      if (!form.value.name) {
        alert('请输入触发器名称');
        return;
      }

      try {
        const triggerData = {
          id: form.value.id || undefined,
          name: form.value.name,
          description: form.value.description,
          event_type_id: form.value.event_type_id,
          enable: form.value.enable,
          // 根据触发器类型添加不同的字段
          ...(form.value.alarm_rule_id && {
            alarm_rule_id: form.value.alarm_rule_id
          }),
          ...(form.value.expr_id && {
            expr_id: form.value.expr_id
          }),
          ...(form.value.custom_event_key && {
            custom_event_key: form.value.custom_event_key
          })
        };

        let result;
        if (form.value.id) {
          result = await updateTrigger.call(triggerData);
        } else {
          result = await createTrigger.call(triggerData);
        }

        emit('saved', result);
        handleClose();
      } catch (error) {
        console.error('保存触发器失败:', error);
        alert('保存失败，请检查日志');
      }
    };

    // 处理关闭
    const handleClose = () => {
      emit('close');
    };

    // 监听triggerData变化
    watch(() => props.triggerData, (newData) => {
      if (newData && Object.keys(newData).length) {
        form.value = {
          id: newData.id || '',
          name: newData.name || '',
          description: newData.description || '',
          event_type_id: newData.event_type_id || '',
          alarm_rule_id: newData.alarm_rule_id || '',
          expr_id: newData.expr_id || '',
          custom_event_key: newData.custom_event_key || '',
          enable: newData.enable !== undefined ? newData.enable : true
        };
      }
    }, { immediate: true });

    // 初始化
    onMounted(() => {
      loadEventTypes();
      if (form.value.event_type_id) {
        handleTriggerTypeChange();
      }
    });

    return {
      dialogVisible,
      form,
      eventTypes,
      alarmRules,
      expressions,
      selectedEventType,
      handleSave,
      handleClose,
      handleTriggerTypeChange
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

.trigger-specific-config {
  margin-top: 20px;
  padding-top: 20px;
  border-top: 1px solid #eee;
}

.alarm-trigger-config,
.expression-trigger-config,
.manual-trigger-config {
  padding: 10px;
  background-color: #f9f9f9;
  border-radius: 4px;
}

.rule-description {
  font-size: 12px;
  color: #666;
  margin-left: 8px;
}

.dialog-footer {
  display: flex;
  justify-content: flex-end;
  gap: 10px;
}
</style>