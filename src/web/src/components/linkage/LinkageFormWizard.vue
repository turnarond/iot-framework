<template>
  <el-dialog
    v-model="dialogVisible"
    title="联动规则配置"
    width="80%"
    @close="$emit('close')"
  >
    <div class="wizard-content">
      <el-steps :active="step" finish-status="success">
        <el-step title="基础信息" />
        <el-step title="触发条件" />
        <el-step title="动作序列" />
      </el-steps>

      <div v-if="step===1" class="step-pane" style="margin-top: 30px;">
        <el-form :model="form" label-width="120px">
          <el-form-item label="规则名称 *" required>
            <el-input v-model="form.name" placeholder="请输入规则名称" />
          </el-form-item>
          <el-form-item label="描述">
            <el-input
              v-model="form.description"
              type="textarea"
              placeholder="请输入规则描述"
              rows="3"
            />
          </el-form-item>
          <el-form-item label="启用">
            <el-switch v-model="form.enable" />
          </el-form-item>
          <el-form-item label="逻辑类型">
            <el-select v-model="form.logic_type" placeholder="请选择逻辑类型">
              <el-option value="AND" label="AND（所有条件同时满足）" />
              <el-option value="OR" label="OR（任一条件满足）" />
            </el-select>
          </el-form-item>
        </el-form>
      </div>

      <div v-if="step===2" class="step-pane" style="margin-top: 30px;">
        <el-row :gutter="20">
          <el-col :span="12">
            <el-card>
              <template #header>
                <div class="card-header">
                  <span>选择触发条件</span>
                </div>
              </template>
              <TriggerSelector @add="onAddTrigger" />
            </el-card>
          </el-col>
          <el-col :span="12">
            <el-card>
              <template #header>
                <div class="card-header">
                  <span>已选触发条件 ({{ form.logic_type }})</span>
                </div>
              </template>
              <div class="triggers-list">
                <div
                  v-for="(t,idx) in form.triggers"
                  :key="t._localId"
                  class="trigger-item"
                >
                  <div class="trigger-name">
                    {{ t.name || (t.source && t.source.name) || '未知触发器' }}
                  </div>
                  <div class="trigger-description" v-if="t.description">
                    {{ t.description }}
                  </div>
                  <div class="trigger-actions">
                    <el-button
                      size="small"
                      type="danger"
                      @click="removeTrigger(idx)"
                    >
                      移除
                    </el-button>
                  </div>
                </div>
              </div>
              <el-empty v-if="form.triggers.length === 0" description="暂无触发条件" />
            </el-card>
          </el-col>
        </el-row>
      </div>

      <div v-if="step===3" class="step-pane" style="margin-top: 30px;">
        <el-card>
          <template #header>
            <div class="card-header">
              <span>动作序列配置</span>
            </div>
          </template>
          <ActionSequence
            :actions="form.actions"
            @add="addAction"
            @remove="removeAction"
            @move-up="moveActionUp"
            @move-down="moveActionDown"
            @edit="editAction"
          />
        </el-card>
      </div>
    </div>

    <template #footer>
      <span class="dialog-footer">
        <el-button @click="$emit('close')">取消</el-button>
        <el-button
          type="primary"
          @click="prevStep"
          :disabled="step===1"
        >
          上一步
        </el-button>
        <el-button
          type="primary"
          @click="nextStep"
          :disabled="step===3"
        >
          下一步
        </el-button>
        <el-button
          type="success"
          @click="save"
          v-if="step===3"
        >
          保存
        </el-button>
      </span>
    </template>
  </el-dialog>
</template>

<script>
import { ref, watch, computed } from 'vue';
import TriggerSelector from './TriggerSelector.vue';
import ActionSequence from './ActionSequence.vue';
import { useLinkageApi } from '../../hooks/useApi';

export default {
  name: 'LinkageFormWizard',
  components: { TriggerSelector, ActionSequence },
  props: {
    visible: { type: Boolean, default: false },
    linkageData: { type: Object, default: () => ({}) }
  },
  emits: ['close', 'saved'],
  setup(props, { emit }) {
    const step = ref(1);
    const form = ref({
      id: '',
      name: '',
      description: '',
      enable: true,
      logic_type: 'AND',
      triggers: [],
      actions: []
    });
    
    const dialogVisible = computed({
      get: () => props.visible,
      set: (value) => {
        if (!value) {
          emit('close');
        }
      }
    });

    // 监听visible变化，当打开对话框时重置表单（如果是添加新规则）
    watch(() => props.visible, (newValue) => {
      if (newValue) {
        // 检查linkageData是否为空对象
        const isEmptyData = !props.linkageData || Object.keys(props.linkageData).length === 0;
        if (isEmptyData) {
          // 重置表单数据
          form.value = {
            id: '',
            name: '',
            description: '',
            enable: true,
            logic_type: 'AND',
            triggers: [],
            actions: []
          };
          // 重置步骤
          step.value = 1;
        }
      }
    });

    const { createLinkage, updateLinkage } = useLinkageApi();

    watch(() => props.linkageData, (v) => {
      if (v && Object.keys(v).length) {
        form.value = {
          id: v.id || '',
          name: v.name || '',
          description: v.description || '',
          enable: v.enable !== undefined ? v.enable : (v.enabled !== undefined ? v.enabled : true),
          logic_type: v.logic_type || 'AND',
          triggers: v.triggers || [],
          actions: v.actions || []
        };
      }
    }, { immediate: true });

    const onAddTrigger = (trigger) => {
      // 检查是否已经添加了相同的触发器
      const isDuplicate = form.value.triggers.some(t => {
        // 对于报警触发器，检查报警规则ID和类型
        if (t.type === 'alarm_trigger' || t.type === 'alarm_restore') {
          return t.alarm_rule_id === trigger.alarm_rule_id && t.type === trigger.type;
        }
        // 对于表达式触发器，检查表达式ID
        else if (t.type === 'expression') {
          return t.expr_id === trigger.expr_id;
        }
        // 对于其他触发器，检查名称
        return t.name === trigger.name;
      });

      if (isDuplicate) {
        alert('该触发器已经添加过了，请不要重复添加');
        return;
      }

      // create a new trigger object with unique local id
      const newTrigger = {
        ...trigger,
        _localId: Math.random().toString(36).slice(2,9)
      };
      form.value.triggers.push(newTrigger);
    };

    const removeTrigger = (idx) => form.value.triggers.splice(idx,1);

    const addAction = (action) => {
      form.value.actions.push(action);
    };

    const removeAction = (idx) => form.value.actions.splice(idx,1);

    const moveActionUp = (idx) => {
      if (idx<=0) return; const a=form.value.actions; [a[idx-1],a[idx]]=[a[idx],a[idx-1]];
    };
    const moveActionDown = (idx) => {
      const a=form.value.actions; if (idx>=a.length-1) return; [a[idx+1],a[idx]]=[a[idx],a[idx+1]];
    };
    const editAction = (idx) => {
      alert('请在动作列表中删除并重新添加以编辑（当前为简化实现）');
    };

    const prevStep = () => { if (step.value>1) step.value--; };
    const nextStep = () => {
      if (step.value===1) {
        if (!form.value.name) { alert('规则名称为必填'); return; }
      }
      if (step.value===2) {
        if (!form.value.triggers || form.value.triggers.length===0) { alert('请至少选择一个触发条件'); return; }
      }
      if (step.value<3) step.value++;
    };

    const save = async () => {
      if (!form.value.triggers || form.value.triggers.length===0) { alert('至少需要1个触发器'); return; }
      if (!form.value.actions || form.value.actions.length===0) { alert('至少需要1个动作'); return; }

      // prepare payload: map triggers/actions to backend expected shape
      const payload = {
        id: form.value.id || undefined,
        name: form.value.name,
        description: form.value.description,
        enable: !!form.value.enable,
        logic_type: form.value.logic_type || 'AND',
        triggers: form.value.triggers.map(t => ({ type: t.type || null, source_id: t.source ? t.source.id : (t.id || t.source_id || null) })),
        actions: form.value.actions.map(a => ({
          id: a.id || undefined,
          name: a.name || '',
          action_type_id: a.action_type_id || a.type || 0,
          param1: a.param1 !== undefined ? a.param1 : (a.params && a.params.param1) || '',
          param2: a.param2 !== undefined ? a.param2 : (a.params && a.params.param2) || '',
          param3: a.param3 !== undefined ? a.param3 : (a.params && a.params.param3) || null,
          param4: a.param4 !== undefined ? a.param4 : (a.params && a.params.param4) || null,
          need_confirm: !!(a.need_confirm || a.needConfirm)
        }))
      };

      try {
        if (form.value.id) {
          await updateLinkage(payload);
        } else {
          await createLinkage(payload);
        }
        emit('saved');
        emit('close');
      } catch (e) {
        console.error('保存联动规则失败', e);
        alert('保存失败，请检查日志');
      }
    };

    return { step, form, dialogVisible, onAddTrigger, removeTrigger, addAction, removeAction, moveActionUp, moveActionDown, editAction, prevStep, nextStep, save };
  }
};
</script>

<style scoped>
.wizard-content {
  padding: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.dialog-footer {
  display: flex;
  gap: 10px;
  justify-content: flex-end;
}

.triggers-list {
  margin-top: 10px;
}

.trigger-item {
  display: flex;
  flex-direction: column;
  padding: 12px;
  border: 1px solid #eee;
  border-radius: 4px;
  margin-bottom: 10px;
  background-color: #f9f9f9;
}

.trigger-name {
  font-weight: bold;
  margin-bottom: 5px;
  color: #333;
}

.trigger-description {
  font-size: 12px;
  color: #666;
  margin-bottom: 10px;
}

.trigger-actions {
  display: flex;
  justify-content: flex-end;
  margin-top: 5px;
}
</style>
