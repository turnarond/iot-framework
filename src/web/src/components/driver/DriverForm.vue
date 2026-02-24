<template>
  <div class="modal" v-if="visible" @click="handleModalClick">
    <div class="modal-content" @click.stop>
      <div class="modal-header">
        <h3>{{ editMode ? '编辑驱动' : '添加驱动' }}</h3>
        <button class="modal-close" @click="$emit('close')">&times;</button>
      </div>
      <form @submit.prevent="handleSubmit">
        <div class="form-group">
          <label for="driverName">驱动名称 *</label>
          <input
            type="text"
            id="driverName"
            v-model="driverForm.name"
            :disabled="editMode"
            required
          />
        </div>
        <div class="form-group">
          <label for="driverType">驱动类型</label>
          <select
            id="driverType"
            v-model="driverForm.type"
            required
          >
            <option value="">请选择驱动类型</option>
            <option v-for="type in driverTypes" :key="type.id" :value="type.id">
              {{ type.cname }}
            </option>
          </select>
        </div>
        <div class="form-group">
          <label for="driverVersion">版本</label>
          <input
            type="text"
            id="driverVersion"
            v-model="driverForm.version"
          />
        </div>
        <div class="form-group">
          <label for="driverDescription">驱动描述</label>
          <textarea
            id="driverDescription"
            v-model="driverForm.description"
            placeholder="请简要描述驱动功能或用途"
          ></textarea>
        </div>
        <div class="form-group">
          <label>参数配置</label>
          <table>
            <thead>
              <tr>
                <th>序号</th>
                <th>参数名称</th>
                <th>参数描述</th>
              </tr>
            </thead>
            <tbody>
              <tr>
                <td>1</td>
                <td>
                  <input
                    type="text"
                    v-model="driverForm.param1_name"
                    placeholder="参数1名称"
                  />
                </td>
                <td>
                  <input
                    type="text"
                    v-model="driverForm.param1_desc"
                    placeholder="参数1描述"
                  />
                </td>
              </tr>
              <tr>
                <td>2</td>
                <td>
                  <input
                    type="text"
                    v-model="driverForm.param2_name"
                    placeholder="参数2名称"
                  />
                </td>
                <td>
                  <input
                    type="text"
                    v-model="driverForm.param2_desc"
                    placeholder="参数2描述"
                  />
                </td>
              </tr>
              <tr>
                <td>3</td>
                <td>
                  <input
                    type="text"
                    v-model="driverForm.param3_name"
                    placeholder="参数3名称"
                  />
                </td>
                <td>
                  <input
                    type="text"
                    v-model="driverForm.param3_desc"
                    placeholder="参数3描述"
                  />
                </td>
              </tr>
              <tr>
                <td>4</td>
                <td>
                  <input
                    type="text"
                    v-model="driverForm.param4_name"
                    placeholder="参数4名称"
                  />
                </td>
                <td>
                  <input
                    type="text"
                    v-model="driverForm.param4_desc"
                    placeholder="参数4描述"
                  />
                </td>
              </tr>
            </tbody>
          </table>
        </div>
        <div class="form-actions">
          <button type="button" class="btn" @click="$emit('close')">
            取消
          </button>
          <button type="submit" class="btn btn-primary">
            确定
          </button>
        </div>
      </form>
    </div>
  </div>
</template>

<script>
export default {
  name: 'DriverForm',
  props: {
    visible: {
      type: Boolean,
      default: false
    },
    editMode: {
      type: Boolean,
      default: false
    },
    driverData: {
      type: Object,
      default: () => ({
        id: '',
        name: '',
        type: '',
        version: '',
        description: '',
        param1_name: '',
        param1_desc: '',
        param2_name: '',
        param2_desc: '',
        param3_name: '',
        param3_desc: '',
        param4_name: '',
        param4_desc: ''
      })
    },
    driverTypes: {
      type: Array,
      default: () => []
    }
  },
  data() {
    return {
      driverForm: {
        id: '',
        name: '',
        type: '',
        version: '',
        description: '',
        param1_name: '',
        param1_desc: '',
        param2_name: '',
        param2_desc: '',
        param3_name: '',
        param3_desc: '',
        param4_name: '',
        param4_desc: ''
      }
    }
  },
  watch: {
    driverData: {
      handler(newData) {
        this.driverForm = { ...newData }
      },
      deep: true,
      immediate: true
    }
  },
  emits: ['close', 'submit'],
  methods: {
    handleSubmit() {
      this.$emit('submit', { ...this.driverForm })
    },
    handleModalClick() {
      this.$emit('close')
    }
  }
}
</script>

<style scoped>
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
  z-index: 1000;
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

.modal-header h3 {
  margin: 0;
}

.modal-close {
  font-size: 24px;
  background: none;
  border: none;
  cursor: pointer;
}

form {
  padding: 20px;
}

.form-group {
  margin-bottom: 15px;
}

.form-group label {
  display: block;
  margin-bottom: 5px;
  font-weight: bold;
}

.form-group input,
.form-group select,
.form-group textarea {
  width: 100%;
  padding: 8px;
  border: 1px solid #ddd;
  border-radius: 4px;
}

.form-group textarea {
  resize: vertical;
  min-height: 80px;
}

.form-group table {
  width: 100%;
  border-collapse: collapse;
  margin-top: 10px;
}

.form-group table th,
.form-group table td {
  padding: 8px;
  border: 1px solid #ddd;
  text-align: left;
}

.form-group table th {
  background-color: #f5f5f5;
}

.form-actions {
  margin-top: 20px;
  display: flex;
  gap: 10px;
  justify-content: flex-end;
}

.btn {
  padding: 8px 16px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
}

.btn-primary {
  background-color: #409eff;
  color: white;
}

.btn-primary:hover {
  background-color: #66b1ff;
}

.btn:hover {
  opacity: 0.9;
}

.tag {
  display: inline-block;
  padding: 2px 8px;
  border-radius: 12px;
  font-size: 12px;
  margin-right: 8px;
  margin-bottom: 4px;
}

.tag-success {
  background-color: #f0f9eb;
  color: #67c23a;
  border: 1px solid #e1f5dc;
}

.tag-warning {
  background-color: #fdf6ec;
  color: #e6a23c;
  border: 1px solid #faecd8;
}
</style>
