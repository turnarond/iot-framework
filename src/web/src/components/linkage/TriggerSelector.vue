<template>
  <div class="trigger-selector">
    <h4>可用触发器</h4>
    
    <!-- 触发器类型选择（用于筛选） -->
    <div class="trigger-type-selector">
      <el-form label-width="100px">
        <el-form-item label="按类型筛选">
          <el-select v-model="selectedTriggerType" placeholder="请选择触发器类型（可选）">
            <el-option value="" label="全部触发器" />
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
      </el-form>
    </div>

    <!-- 显示所有触发器 -->
    <div class="trigger-category">
      <div v-if="loading" class="loading">加载中...</div>
      <div v-else>
        <!-- 报警触发类型 -->
        <div v-if="!selectedTriggerType || selectedEventType?.id === 1 || selectedEventType?.id === 2">
          <h5>报警触发</h5>
          <div v-if="loading" class="loading">加载中...</div>
          <div v-else-if="alarmRules.length === 0" class="empty">暂无报警规则</div>
          <ul v-else>
            <li v-for="rule in alarmRules" :key="rule.id">
              <div class="rule-info">
                <div class="rule-name">{{ rule.name }}</div>
                <div class="rule-detail">
                  <span>点位: {{ rule.point_name }}</span>
                  <span>设备: {{ rule.device_name }}</span>
                </div>
              </div>
              <div class="trigger-actions">
                <el-button size="small" @click="addAlarmTrigger(rule, 'alarm', 1)">
                  报警产生
                </el-button>
                <el-button size="small" @click="addAlarmTrigger(rule, 'restore', 2)">
                  报警恢复
                </el-button>
              </div>
            </li>
          </ul>
        </div>

        <!-- 表达式触发类型 -->
        <div v-if="!selectedTriggerType || selectedEventType?.id === 3">
          <h5>表达式触发</h5>
          <div v-if="loadingExpressions" class="loading">加载中...</div>
          <div v-else-if="expressions.length === 0" class="empty">暂无表达式，请先创建</div>
          <ul v-else>
            <li v-for="expr in expressions" :key="expr.id">
              <div class="expr-info">
                <div class="expr-name">{{ expr.name }}</div>
                <div class="expr-detail">{{ expr.expression }}</div>
                <div class="expr-description" v-if="expr.description">{{ expr.description }}</div>
              </div>
              <div class="trigger-actions">
                <el-button size="small" @click="addExpressionTrigger(expr, 3)">
                  选择
                </el-button>
              </div>
            </li>
          </ul>
          <div style="margin-top: 20px;">
            <el-button type="primary" size="small" @click="openExpressionModal">
              创建新表达式
            </el-button>
          </div>
        </div>

        <!-- 自定义事件触发类型 -->
        <div v-if="!selectedTriggerType || selectedEventType?.id === 4">
          <h5>自定义事件触发</h5>
          <div class="custom-trigger-section">
            <el-button type="primary" size="small" @click="addCustomTrigger(4)">
              添加自定义事件触发器
            </el-button>
          </div>
        </div>
      </div>
    </div>

    <!-- 表达式触发器创建模态框 -->
    <el-dialog
      v-model="expressionModalVisible"
      title="创建表达式触发器"
      width="600px"
    >
      <el-form :model="expressionForm" label-width="100px">
        <el-form-item label="触发器名称" required>
          <el-input v-model="expressionForm.name" placeholder="请输入触发器名称" />
        </el-form-item>
        <el-form-item label="表达式" required>
          <el-input
            v-model="expressionForm.expression"
            placeholder="请输入表达式，例如: temperature > 100"
            type="textarea"
            rows="3"
          />
        </el-form-item>
        <el-form-item label="描述">
          <el-input
            v-model="expressionForm.description"
            type="textarea"
            placeholder="请输入触发器描述"
            rows="2"
          />
        </el-form-item>
      </el-form>
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="expressionModalVisible = false">取消</el-button>
          <el-button type="primary" @click="createExpressionTrigger">创建</el-button>
        </span>
      </template>
    </el-dialog>
  </div>
</template>

<script>
import { ref, onMounted, watch } from 'vue';
import { linkageApi } from '../../services/linkageApi';
import { alarmApi } from '../../services/alarmApi';

export default {
  name: 'TriggerSelector',
  emits: ['add'],
  setup(props, { emit }) {
    const selectedTriggerType = ref('');
    const eventTypes = ref([]);
    const alarmRules = ref([]);
    const expressions = ref([]);
    const loading = ref(false);
    const loadingExpressions = ref(false);
    
    // 表达式触发器表单
    const expressionModalVisible = ref(false);
    const expressionForm = ref({
      name: '',
      expression: '',
      description: ''
    });

    // 获取事件类型列表
    const loadEventTypes = async () => {
      loading.value = true;
      try {
        const response = await linkageApi.getEventTypes(1, 100);
        eventTypes.value = response.items || [];
      } catch (e) {
        console.error('获取事件类型失败', e);
        eventTypes.value = [];
      } finally {
        loading.value = false;
      }
    };

    // 获取报警规则列表
    const fetchAlarmRules = async () => {
      loading.value = true;
      try {
        const response = await alarmApi.getAlarmRulesWithPoint(1, 100);
        alarmRules.value = response.items || [];
      } catch (e) {
        console.error('获取报警规则失败', e);
        alarmRules.value = [];
      } finally {
        loading.value = false;
      }
    };

    // 获取表达式列表
    const fetchExpressions = async () => {
      loadingExpressions.value = true;
      try {
        const response = await linkageApi.getExpressions(1, 100);
        expressions.value = response.items || [];
      } catch (e) {
        console.error('获取表达式失败', e);
        expressions.value = [];
      } finally {
        loadingExpressions.value = false;
      }
    };

    // 添加报警触发器
    const addAlarmTrigger = (rule, triggerType, eventTypeId) => {
      const trigger = {
        _localId: Math.random().toString(36).slice(2, 9),
        type: triggerType === 'alarm' ? 'alarm_trigger' : 'alarm_restore',
        event_type_id: eventTypeId,
        name: `${rule.name}${triggerType === 'alarm' ? ' 产生' : ' 恢复'}`,
        source: {
          id: rule.id,
          name: rule.name,
          point_name: rule.point_name,
          device_name: rule.device_name
        },
        alarm_rule_id: rule.id,
        description: `${rule.name}的${triggerType === 'alarm' ? '报警产生' : '报警恢复'}事件`
      };
      emit('add', trigger);
    };

    // 打开表达式触发器创建模态框
    const openExpressionModal = () => {
      expressionForm.value = {
        name: '',
        expression: '',
        description: ''
      };
      expressionModalVisible.value = true;
    };

    // 创建表达式触发器
    const createExpressionTrigger = () => {
      if (!expressionForm.value.name || !expressionForm.value.expression) {
        alert('名称和表达式为必填项');
        return;
      }

      const trigger = {
        _localId: Math.random().toString(36).slice(2, 9),
        type: 'expression',
        event_type_id: 3,
        name: expressionForm.value.name,
        expression: expressionForm.value.expression,
        description: expressionForm.value.description
      };
      emit('add', trigger);
      expressionModalVisible.value = false;
    };

    // 当前选择的事件类型
    const selectedEventType = ref(null);
    
    // 监听触发器类型变化，更新选中的事件类型
    watch(selectedTriggerType, (newType) => {
      if (newType) {
        selectedEventType.value = eventTypes.value.find(type => type.id === newType) || null;
      } else {
        selectedEventType.value = null;
      }
    });

    // 添加表达式触发器
    const addExpressionTrigger = (expr, eventTypeId) => {
      const trigger = {
        _localId: Math.random().toString(36).slice(2, 9),
        type: 'expression',
        event_type_id: eventTypeId,
        expr_id: expr.id,
        name: expr.name,
        expression: expr.expression,
        description: expr.description || `${expr.name}的表达式触发事件`
      };
      emit('add', trigger);
    };

    // 添加自定义触发器
    const addCustomTrigger = (eventTypeId) => {
      const name = prompt('触发器名称');
      if (!name) return;

      const trigger = {
        _localId: Math.random().toString(36).slice(2, 9),
        type: 'custom',
        event_type_id: eventTypeId,
        name: name,
        description: `自定义事件触发的联动规则`
      };
      emit('add', trigger);
    };

    // 初始化
    onMounted(() => {
      loadEventTypes();
      fetchAlarmRules();
      fetchExpressions();
    });

    return {
      selectedTriggerType,
      eventTypes,
      selectedEventType,
      alarmRules,
      expressions,
      loading,
      loadingExpressions,
      expressionModalVisible,
      expressionForm,
      addAlarmTrigger,
      addExpressionTrigger,
      openExpressionModal,
      createExpressionTrigger,
      addCustomTrigger
    };
  }
};
</script>

<style scoped>
.trigger-selector {
  padding: 10px;
}

.trigger-type-selector {
  margin-bottom: 20px;
  padding: 15px;
  background-color: #f9f9f9;
  border-radius: 4px;
}

.trigger-category {
  margin-top: 20px;
}

.trigger-category h5 {
  margin-bottom: 15px;
  color: #333;
  border-bottom: 1px solid #eee;
  padding-bottom: 8px;
}

.rule-info {
  flex: 1;
}

.rule-name {
  font-weight: bold;
  margin-bottom: 5px;
}

.rule-detail {
  font-size: 12px;
  color: #666;
  display: flex;
  gap: 15px;
}

.expr-info {
  flex: 1;
}

.expr-name {
  font-weight: bold;
  margin-bottom: 5px;
}

.expr-detail {
  font-size: 12px;
  color: #666;
  margin-bottom: 5px;
  font-family: monospace;
  background-color: #f5f5f5;
  padding: 4px 8px;
  border-radius: 3px;
}

.expr-description {
  font-size: 12px;
  color: #999;
  margin-top: 3px;
}

.trigger-actions {
  display: flex;
  gap: 8px;
}

ul {
  list-style: none;
  padding: 0;
  margin: 0;
}

li {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px;
  border: 1px solid #eee;
  border-radius: 4px;
  margin-bottom: 10px;
  background-color: #fafafa;
}

.loading, .empty {
  padding: 20px;
  text-align: center;
  color: #666;
}

.dialog-footer {
  display: flex;
  justify-content: flex-end;
  gap: 10px;
}
</style>