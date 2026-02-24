<template>
  <div>
    <!-- 设备列表 -->
    <div class="table-container">
      <table>
        <thead>
          <tr>
            <th>ID</th>
            <th>名称</th>
            <th>驱动名</th>
            <th>参数</th>
            <th>描述</th>
            <th>创建时间</th>
            <th>操作</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="device in devices" :key="device.id">
            <td>{{ device.id }}</td>
            <td>{{ device.name }}</td>
            <td>{{ device.driver_name  }}</td>
            <td>
              <span class="tag tag-warning">设备编号: {{ device.id }}</span>
              <span class="tag tag-info">通信参数: {{ device.connparam }}</span>
              <template v-for="(param, index) in getDeviceParams(device)" :key="index">
                <span class="tag tag-default">{{ param.name }}: {{ param.value }}</span>
              </template>
            </td>
            <td>{{ device.description }}</td>
            <td>{{ formatDateTime(device.create_time) }}</td>
            <td>
              <button class="btn btn-primary" @click="$emit('edit', device)">
                编辑
              </button>
              <button class="btn btn-danger" @click="$emit('delete', device.id)">
                删除
              </button>
            </td>
          </tr>
        </tbody>
      </table>
    </div>

    <!-- 分页 -->
    <div class="pagination">
      <button :disabled="currentPage === 1" @click="$emit('page-change', currentPage - 1)">上一页</button>
      <span>{{ currentPage }} / {{ totalPages }}</span>
      <button :disabled="currentPage === totalPages" @click="$emit('page-change', currentPage + 1)">下一页</button>
      <span>共 {{ totalItems }} 条</span>
    </div>
  </div>
</template>

<script>
export default {
  name: 'DeviceList',
  props: {
    devices: {
      type: Array,
      default: () => []
    },
    drivers: {
      type: Array,
      default: () => []
    },
    currentPage: {
      type: Number,
      default: 1
    },
    totalPages: {
      type: Number,
      default: 1
    },
    totalItems: {
      type: Number,
      default: 0
    }
  },
  emits: ['edit', 'delete', 'page-change'],
  methods: {
    // 获取设备参数
    getDeviceParams(device) {
      const params = [];
      // 找到对应的驱动
      const driver = this.drivers.find(d => d.id === device.driver_id || d.name === device.driver_name);
      
      // 使用驱动定义的参数名称
      if (device.param1) {
        const paramName = driver?.param1_name || '参数1';
        params.push({ name: paramName, value: device.param1 });
      }
      if (device.param2) {
        const paramName = driver?.param2_name || '参数2';
        params.push({ name: paramName, value: device.param2 });
      }
      if (device.param3) {
        const paramName = driver?.param3_name || '参数3';
        params.push({ name: paramName, value: device.param3 });
      }
      if (device.param4) {
        const paramName = driver?.param4_name || '参数4';
        params.push({ name: paramName, value: device.param4 });
      }
      return params;
    },
    // 格式化时间戳为年月日时格式
    formatDateTime(timestamp) {
      if (!timestamp) return '';
      const date = new Date(timestamp);
      return date.toLocaleString('zh-CN', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit'
      });
    }
  }
}
</script>

<style scoped>
.table-container {
  width: 100%;
  overflow-x: auto;
}

table {
  width: 100%;
  border-collapse: collapse;
  margin-bottom: 20px;
}

th, td {
  padding: 12px;
  text-align: left;
  border-bottom: 1px solid #e8e8e8;
}

th {
  background-color: #fafafa;
  font-weight: 600;
  color: #303133;
}

tr:hover {
  background-color: #f5f7fa;
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

.tag-info {
  background-color: #ecf5ff;
  color: #409eff;
  border: 1px solid #d9ecff;
}

.tag-default {
  background-color: #f5f7fa;
  color: #909399;
  border: 1px solid #ebeef5;
}

.btn {
  padding: 6px 12px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
  margin-right: 8px;
}

.btn-primary {
  background-color: #409eff;
  color: white;
}

.btn-primary:hover {
  background-color: #66b1ff;
}

.btn-danger {
  background-color: #f56c6c;
  color: white;
}

.btn-danger:hover {
  background-color: #f78989;
}

.pagination {
  display: flex;
  justify-content: flex-end;
  align-items: center;
  margin-top: 20px;
  gap: 10px;
}

.pagination button {
  padding: 6px 12px;
  border: 1px solid #dcdfe6;
  border-radius: 4px;
  background-color: white;
  cursor: pointer;
  font-size: 14px;
}

.pagination button:hover {
  border-color: #c6e2ff;
  color: #409eff;
}

.pagination button:disabled {
  cursor: not-allowed;
  opacity: 0.6;
}

.pagination button:disabled:hover {
  border-color: #dcdfe6;
  color: #c0c4cc;
}
</style>
