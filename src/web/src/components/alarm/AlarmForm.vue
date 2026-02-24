<template>
  <div v-if="visible" class="modal" @click="handleModalClick">
    <div class="modal-content" @click.stop>
      <div class="modal-header">
        <h3>{{ editMode ? '编辑报警规则' : '添加报警规则' }}</h3>
        <button class="close-btn" @click="$emit('close')">&times;</button>
      </div>
      <div class="modal-body">
        <form @submit.prevent="handleSubmit">
          <!-- 基本信息 -->
          <div class="form-section">
            <h4>基本信息</h4>
            
            <div class="form-group">
              <label for="point_id">点位 *</label>
              <select 
                id="point_id" 
                v-model="ruleForm.point_id" 
                class="form-control" 
                required
                :disabled="editMode"
              >
                <option value="">请选择点位</option>
                <option v-for="point in points" :key="point.id" :value="point.id">
                  {{ point.name || point.point_name || '未知点位' }} (设备: {{point.device_name || '未知设备' }})
                </option>
              </select>
              <small v-if="editMode" class="form-text text-muted">编辑模式下点位不可修改</small>
            </div>
            
            <div class="form-group">
              <label for="name">规则名称 *</label>
              <input 
                type="text" 
                id="name" 
                v-model="ruleForm.name" 
                class="form-control" 
                required
              >
            </div>
            
            <div class="form-group">
            <label for="method">报警方法 *</label>
            <select 
              id="method" 
              v-model="ruleForm.method" 
              class="form-control" 
              required
            >
              <option value="">请选择报警方法</option>
              <option v-for="method in alarmMethods" :key="method.id" :value="method.id">
                {{ method.cname }}
              </option>
            </select>
          </div>
          </div>
          
          <!-- 阈值设置 -->
          <div v-if="isThresholdMethod" class="form-section">
            <h4>阈值设置</h4>
            
            <div class="form-group">
              <label for="threshold">报警阈值 *</label>
              <input 
                type="number" 
                id="threshold" 
                v-model.number="ruleForm.threshold" 
                class="form-control" 
                step="0.01" 
                required
              >
            </div>
            
            <div class="form-group">
              <label for="restore_threshold">恢复阈值</label>
              <input 
                type="number" 
                id="restore_threshold" 
                v-model.number="ruleForm.restore_threshold" 
                class="form-control" 
                step="0.01"
              >
              <small class="form-text text-muted">可选，不设置则使用回差值计算恢复阈值</small>
            </div>
            
            <div class="form-group">
              <label for="hysteresis">回差值</label>
              <input 
                type="number" 
                id="hysteresis" 
                v-model.number="ruleForm.hysteresis" 
                class="form-control" 
                step="0.01"
                min="0"
              >
              <small class="form-text text-muted">当恢复阈值未设置时使用，默认为0</small>
            </div>
          </div>
          
          <!-- 定值设置 -->
          <div v-else-if="isFixedValueMethod" class="form-section">
            <h4>定值设置</h4>
            
            <div class="form-group">
              <label for="threshold">报警值 *</label>
              <input 
                type="number" 
                id="threshold" 
                v-model.number="ruleForm.threshold" 
                class="form-control" 
                step="0.01" 
                required
              >
            </div>
          </div>
          
          <!-- 高级设置 -->
          <div class="form-section">
            <h4>高级设置</h4>
            
            <div class="form-group checkbox-group">
              <label>
                <input type="checkbox" v-model="ruleForm.enable">
                启用规则
              </label>
            </div>
            
            <div class="form-group checkbox-group">
              <label>
                <input type="checkbox" v-model="ruleForm.enable_restore">
                启用恢复事件
              </label>
            </div>
          </div>
          
          <!-- 操作按钮 -->
          <div class="form-actions">
            <button type="button" class="btn btn-secondary" @click="$emit('close')">取消</button>
            <button type="submit" class="btn btn-primary">保存</button>
          </div>
        </form>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'AlarmForm',
  props: {
    visible: {
      type: Boolean,
      default: false
    },
    editMode: {
      type: Boolean,
      default: false
    },
    ruleData: {
      type: Object,
      default: () => ({})
    },
    points: {
      type: Array,
      default: () => []
    },
    alarmMethods: {
      type: Array,
      default: () => []
    }
  },
  emits: ['close', 'submit'],
  data() {
    return {
      ruleForm: {
        id: '',
        point_id: '',
        name: '',
        method: '',
        threshold: 0,
        restore_threshold: null,
        hysteresis: 0,
        enable: true,
        enable_restore: true
      }
    };
  },
  computed: {
    // 判断是否为阈值类型的报警方法（高高限、高限、低限、低低限）
    isThresholdMethod() {
      const method = String(this.ruleForm.method);
      const thresholdMethods = ['1', '2', '3', '4']; // 对应高高限、高限、低限、低低限
      return thresholdMethods.includes(method);
    },
    // 判断是否为定值类型的报警方法
    isFixedValueMethod() {
      const method = String(this.ruleForm.method);
      return method === '5'; // 对应定值
    }
  },
  watch: {
    ruleData: {
      handler(newData) {
        if (newData) {
          // 处理字段映射，确保pointId正确映射到point_id
          const formData = {
            ...this.ruleForm,
            ...newData
          };
          // 如果有pointId字段，将其映射到point_id
          if (newData.pointId) {
            formData.point_id = newData.pointId;
          }
          this.ruleForm = formData;
        }
      },
      deep: true,
      immediate: true
    }
  },
  methods: {
    /**
     * 处理表单提交
     * 说明：需要进行类型转换是因为前端表单中的值可能为字符串类型，
     * 而后端API期望接收正确的数值类型，确保数据类型一致性避免后端处理错误
     */
    handleSubmit() {
      // 确保数值类型字段正确
      const formattedForm = {
        ...this.ruleForm,
        // 将字符串转换为整数类型
        point_id: parseInt(this.ruleForm.point_id),
        method: parseInt(this.ruleForm.method),
        // 将字符串转换为浮点数类型
        threshold: parseFloat(this.ruleForm.threshold),
        // 恢复阈值可能为空，需要进行空值判断后再转换
        restore_threshold: this.ruleForm.restore_threshold !== null && this.ruleForm.restore_threshold !== '' ? parseFloat(this.ruleForm.restore_threshold) : null,
        // 回差值默认为0
        hysteresis: this.ruleForm.hysteresis !== null && this.ruleForm.hysteresis !== '' ? parseFloat(this.ruleForm.hysteresis) : 0,
        // 将值转换为布尔类型
        enable: Boolean(this.ruleForm.enable),
        enable_restore: Boolean(this.ruleForm.enable_restore)
      };
      this.$emit('submit', formattedForm);
    },
    // 处理点击模态框外部关闭
    handleModalClick() {
      this.$emit('close');
    }
  }
};
</script>

<style scoped>
.modal {
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background-color: rgba(0, 0, 0, 0.5);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 1000;
}

.modal-content {
  background-color: white;
  border-radius: 8px;
  width: 90%;
  max-width: 600px;
  max-height: 80vh;
  overflow-y: auto;
  box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
}

.modal-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20px;
  border-bottom: 1px solid #ddd;
}

.modal-header h3 {
  margin: 0;
}

.close-btn {
  background: none;
  border: none;
  font-size: 24px;
  cursor: pointer;
  color: #666;
}

.modal-body {
  padding: 20px;
}

.form-section {
  margin-bottom: 30px;
  padding: 20px;
  border: 1px solid #eee;
  border-radius: 6px;
  background-color: #f9f9f9;
}

.form-section h4 {
  margin-top: 0;
  margin-bottom: 20px;
  border-bottom: 1px solid #ddd;
  padding-bottom: 10px;
}

.form-group {
  margin-bottom: 15px;
}

.form-group label {
  display: block;
  margin-bottom: 5px;
  font-weight: 500;
}

.form-control {
  width: 100%;
  padding: 8px 12px;
  border: 1px solid #ddd;
  border-radius: 4px;
  font-size: 14px;
}

.form-control:focus {
  border-color: #007bff;
  outline: none;
  box-shadow: 0 0 0 0.2rem rgba(0, 123, 255, 0.25);
}

.checkbox-group {
  display: flex;
  align-items: center;
  margin-bottom: 10px;
}

.checkbox-group label {
  display: flex;
  align-items: center;
  gap: 8px;
  cursor: pointer;
  font-weight: normal;
  width: 100%;
}

.checkbox-group input[type="checkbox"] {
  margin: 0;
  flex-shrink: 0;
  width: 16px;
  height: 16px;
  vertical-align: middle;
}

.checkbox-group input[type="checkbox"] + span {
  vertical-align: middle;
}

.form-text {
  display: block;
  margin-top: 5px;
  color: #6c757d;
  font-size: 12px;
}

.form-actions {
  margin-top: 30px;
  display: flex;
  justify-content: flex-end;
  gap: 10px;
}

.btn {
  padding: 10px 20px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
  font-weight: 500;
}

.btn-primary {
  background-color: #007bff;
  color: white;
}

.btn-secondary {
  background-color: #6c757d;
  color: white;
}

.btn-primary:hover {
  background-color: #0069d9;
}

.btn-secondary:hover {
  background-color: #5a6268;
}

@media (max-width: 768px) {
  .modal-content {
    width: 95%;
    margin: 20px;
  }
  
  .form-actions {
    flex-direction: column;
  }
  
  .btn {
    width: 100%;
  }
}
</style>