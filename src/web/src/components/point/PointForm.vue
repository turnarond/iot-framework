<template>
  <div v-if="visible" class="modal" @click="handleModalClick">
    <div class="modal-content" @click.stop>
      <div class="modal-header">
        <h3>{{ editMode ? '编辑点位' : '新建点位' }}</h3>
        <button class="close-btn" @click="$emit('close')">&times;</button>
      </div>
      <div class="modal-body">
        <form @submit.prevent="handleSubmit">
          <!-- 基础信息 -->
          <div class="form-group">
            <label for="name">点位名称 *</label>
            <input 
              type="text" 
              id="name" 
              v-model="pointForm.name" 
              class="form-control" 
              :class="{ 'is-invalid': errors.name }"
              required
            >
            <div v-if="errors.name" class="invalid-feedback">{{ errors.name }}</div>
          </div>
          
          <div class="form-group">
            <label for="address">点位地址 *</label>
            <input 
              type="text" 
              id="address" 
              v-model="pointForm.address" 
              class="form-control" 
              :class="{ 'is-invalid': errors.address }"
              required
            >
            <div v-if="errors.address" class="invalid-feedback">{{ errors.address }}</div>
          </div>
          
          <div class="form-group">
            <label for="device_id">设备 *</label>
            <select 
              id="device_id" 
              v-model="pointForm.device_id" 
              class="form-control" 
              :class="{ 'is-invalid': errors.device_id }"
              required
            >
              <option value="">请选择设备</option>
              <option v-for="device in devices" :key="device.id" :value="device.id">
                {{ device.name }}
              </option>
            </select>
            <div v-if="errors.device_id" class="invalid-feedback">{{ errors.device_id }}</div>
          </div>
          
          <!-- 数据与类型 -->
          <div class="form-group">
            <label for="datatype">数据类型 *</label>
            <select 
              id="datatype" 
              v-model="pointForm.datatype" 
              class="form-control" 
              :class="{ 'is-invalid': errors.datatype }"
              required
            >
              <option value="">请选择数据类型</option>
              <option v-for="type in dataTypes" :key="type.id" :value="type.id">
                {{ type.cname }}
              </option>
            </select>
            <div v-if="errors.datatype" class="invalid-feedback">{{ errors.datatype }}</div>
          </div>
          
          <div class="form-group">
            <label for="point_type">点位类型 *</label>
            <select 
              id="point_type" 
              v-model="pointForm.point_type" 
              class="form-control" 
              :class="{ 'is-invalid': errors.point_type }"
              required
            >
              <option value="">请选择点位类型</option>
              <option v-for="type in pointTypes" :key="type.id" :value="type.id">
                {{ type.cname }}
              </option>
            </select>
            <div v-if="errors.point_type" class="invalid-feedback">{{ errors.point_type }}</div>
          </div>
          
          <!-- 数据处理方法 -->
          <div class="form-group">
            <label>点位数据处理方法 *</label>
            <div class="radio-group">
              <label v-for="method in transferMethods" :key="method.id">
                <input 
                  type="radio" 
                  :value="method.id" 
                  v-model="pointForm.transfer_type" 
                  required
                >
                {{ method.cname }}
              </label>
            </div>
          </div>
          
          <!-- 线性缩放配置 -->
          <div v-if="showLinearConfig" class="form-section">
            <h4>线性缩放配置</h4>
            <div class="form-row">
              <div class="form-group col-md-6">
                <label for="linear_raw_min">原始最小值</label>
                <input 
                  type="number" 
                  id="linear_raw_min" 
                  v-model.number="pointForm.linear_raw_min" 
                  class="form-control"
                  :class="{ 'is-invalid': errors.linear_raw_min }"
                >
                <div v-if="errors.linear_raw_min" class="invalid-feedback">{{ errors.linear_raw_min }}</div>
              </div>
              <div class="form-group col-md-6">
                <label for="linear_raw_max">原始最大值</label>
                <input 
                  type="number" 
                  id="linear_raw_max" 
                  v-model.number="pointForm.linear_raw_max" 
                  class="form-control"
                >
              </div>
            </div>
            <div class="form-row">
              <div class="form-group col-md-6">
                <label for="linear_eng_min">工程最小值</label>
                <input 
                  type="number" 
                  id="linear_eng_min" 
                  v-model.number="pointForm.linear_eng_min" 
                  class="form-control"
                  :class="{ 'is-invalid': errors.linear_eng_min }"
                >
                <div v-if="errors.linear_eng_min" class="invalid-feedback">{{ errors.linear_eng_min }}</div>
              </div>
              <div class="form-group col-md-6">
                <label for="linear_eng_max">工程最大值</label>
                <input 
                  type="number" 
                  id="linear_eng_max" 
                  v-model.number="pointForm.linear_eng_max" 
                  class="form-control"
                >
              </div>
            </div>
          </div>
          
          <!-- 高级算法配置 -->
          <div v-if="showAdvancedConfig" class="form-section">
            <h4>高级算法配置</h4>
            <div class="form-group">
              <label for="advanced_algo_lib">算法库</label>
              <input 
                type="text" 
                id="advanced_algo_lib" 
                v-model="pointForm.advanced_algo_lib" 
                class="form-control"
              >
            </div>
            <div class="form-row">
              <div class="form-group col-md-6">
                <label for="advanced_param1">参数1</label>
                <input 
                  type="text" 
                  id="advanced_param1" 
                  v-model="pointForm.advanced_param1" 
                  class="form-control"
                >
              </div>
              <div class="form-group col-md-6">
                <label for="advanced_param2">参数2</label>
                <input 
                  type="text" 
                  id="advanced_param2" 
                  v-model="pointForm.advanced_param2" 
                  class="form-control"
                >
              </div>
            </div>
            <div class="form-row">
              <div class="form-group col-md-6">
                <label for="advanced_param3">参数3</label>
                <input 
                  type="text" 
                  id="advanced_param3" 
                  v-model="pointForm.advanced_param3" 
                  class="form-control"
                >
              </div>
              <div class="form-group col-md-6">
                <label for="advanced_param4">参数4</label>
                <input 
                  type="text" 
                  id="advanced_param4" 
                  v-model="pointForm.advanced_param4" 
                  class="form-control"
                >
              </div>
            </div>
          </div>
          
          <!-- 其他设置 -->
          <div class="form-group">
            <label for="poll_rate">上报频率 (Hz) *</label>
            <input 
              type="number" 
              id="poll_rate" 
              v-model.number="pointForm.poll_rate" 
              class="form-control" 
              :class="{ 'is-invalid': errors.poll_rate }"
              required
              min="1"
            >
            <div v-if="errors.poll_rate" class="invalid-feedback">{{ errors.poll_rate }}</div>
          </div>
          
          <div class="checkbox-group-box">
            <div class="form-group checkbox-group">
              <label>
                <input type="checkbox" v-model="pointForm.enable_control" />
                启用控制
              </label>
            </div>
            <div class="form-group checkbox-group">
              <label>
                <input type="checkbox" v-model="pointForm.enable_history" />
                启用历史记录
              </label>
            </div>
          </div>
          
          <div class="form-group">
            <label for="description">描述</label>
            <textarea 
              id="description" 
              v-model="pointForm.description" 
              class="form-control" 
              rows="3"
            ></textarea>
          </div>
          
          <div class="form-actions">
            <button type="button" class="btn btn-secondary" @click="$emit('close')">取消</button>
            <button type="submit" class="btn btn-primary">确定</button>
          </div>
        </form>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, watch } from 'vue';
import { usePointForm } from '../../hooks/usePointForm.js';
import { DATATYPES, POINT_TYPES, TRANSFER_METHODS } from '../../constants/index.js';

// 定义组件属性
const props = defineProps({
  visible: {
    type: Boolean,
    default: false
  },
  editMode: {
    type: Boolean,
    default: false
  },
  pointData: {
    type: Object,
    default: () => ({})
  },
  devices: {
    type: Array,
    default: () => []
  },
  dataTypes: {
    type: Array,
    default: () => []
  },
  pointTypes: {
    type: Array,
    default: () => []
  },
  transferMethods: {
    type: Array,
    default: () => []
  }
});

// 定义事件
const emit = defineEmits(['close', 'submit']);

// 使用自定义Hook
const {
  pointForm,
  errors,
  showLinearConfig,
  showAdvancedConfig,
  handleSubmit: handleFormSubmit,
  resetForm
} = usePointForm(props.editMode, props.pointData);

// 监听pointData变化，确保编辑不同点位时表单数据更新
watch(() => props.pointData, (newData) => {
  if (newData && props.editMode) {
    resetForm();
    Object.assign(pointForm, newData);
  }
}, { deep: true, immediate: true });

// 处理表单提交
const handleSubmit = () => {
  const formData = handleFormSubmit();
  if (formData) {
    emit('submit', formData);
  }
};

// 处理点击模态框外部关闭
const handleModalClick = () => {
  emit('close');
};
</script>

<style scoped>
/* 样式保持不变，略 */
.modal {
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background: rgba(0,0,0,0.5);
  display: flex;
  justify-content: center;
  align-items: center;
}
.modal-content {
  background: white;
  border-radius: 8px;
  width: 90%;
  max-width: 800px;
  max-height: 90vh;
  overflow-y: auto;
}
.modal-header {
  padding: 16px;
  border-bottom: 1px solid #eee;
  display: flex;
  justify-content: space-between;
  align-items: center;
}
.close-btn {
  font-size: 24px;
  background: none;
  border: none;
  cursor: pointer;
}
.modal-body {
  padding: 20px;
}
.form-group {
  margin-bottom: 15px;
}
.form-section {
  margin: 20px 0;
  padding: 15px;
  border: 1px solid #ddd;
  border-radius: 4px;
}
.form-row {
  display: flex;
  gap: 15px;
  margin-bottom: 15px;
}
.form-row .form-group {
  flex: 1;
  margin-bottom: 0;
}
.form-actions {
  margin-top: 20px;
  display: flex;
  gap: 10px;
  justify-content: flex-end;
}
.form-control {
  width: 100%;
  padding: 8px;
  border: 1px solid #ddd;
  border-radius: 4px;
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
.btn {
  padding: 8px 16px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
}
.btn-primary {
  background-color: #007bff;
  color: white;
}
.btn-secondary {
  background-color: #6c757d;
  color: white;
}
.checkbox-group-box {
  display: flex;
  flex-direction: row;
  flex-wrap: wrap;
  gap: 15px;
}
.radio-group {
  display: flex;
  gap: 20px;
  margin-top: 5px;
}
.radio-group label {
  display: flex;
  align-items: center;
  gap: 5px;
  cursor: pointer;
}
</style>