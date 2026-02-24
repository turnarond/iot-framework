<template>
  <div>
    <!-- 驱动列表 -->
    <div class="table-container">
      <table>
        <thead>
          <tr>
            <th>名称</th>
            <th>参数</th>
            <th>描述</th>
            <th>创建时间</th>
            <th>操作</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="driver in drivers" :key="driver.id">
            <td>{{ driver.name }}</td>
            <td>
              <span class="tag tag-success">类型: {{ getDriverTypeName(driver.type) }}</span>
              <span class="tag tag-warning">版本: {{ driver.version }}</span>
            </td>
            <td>{{ driver.description }}</td>
            <td>{{ formatDateTime(driver.create_time) }}</td>
            <td>
              <button class="btn btn-primary" @click="$emit('edit', driver)">
                编辑
              </button>
              <button class="btn btn-danger" @click="$emit('delete', driver.id)">
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
  name: 'DriverList',
  props: {
    drivers: {
      type: Array,
      default: () => []
    },
    driverTypes: {
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
  methods: {
    getDriverTypeName(typeId) {
      const type = this.driverTypes.find(t => t.id === typeId);
      return type ? type.cname : typeId;
    },
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
  },
  emits: ['edit', 'delete', 'page-change']
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
