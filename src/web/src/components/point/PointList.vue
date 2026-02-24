<template>
  <div class="point-list">
    <div class="point-list-content">
      <table class="table table-striped">
        <thead>
          <tr>
            <th>点位名称</th>
            <th>点位地址</th>
            <th>设备</th>
            <th>数据类型</th>
            <th>点位类型</th>
            <th>操作</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="point in points" :key="point.id">
            <td>{{ point.name }}</td>
            <td>{{ point.address }}</td>
            <td>{{ point.device_name }}</td>
            <td>
              <span class="tag tag-success">{{ point.datatype_cname }}</span>
            </td>
            <td>
              <span class="tag tag-warning">{{ point.point_type_cname }}</span>
            </td>
            <td>
              <button class="btn btn-sm btn-info" @click="handleEdit(point)">编辑</button>
              <button class="btn btn-sm btn-danger" @click="confirmDelete(point.id, point.name)">删除</button>
            </td>
          </tr>
        </tbody>
      </table>
    </div>
    <div class="point-list-pagination">
      <button 
        class="btn btn-sm" 
        :disabled="currentPage === 1"
        @click="changePage(currentPage - 1)"
      >
        上一页
      </button>
      <span>第 {{ currentPage }} 页，共 {{ totalPages }} 页，共 {{ totalItems }} 条数据</span>
      <button 
        class="btn btn-sm" 
        :disabled="currentPage === totalPages"
        @click="changePage(currentPage + 1)"
      >
        下一页
      </button>
    </div>
  </div>
</template>

<script setup>
// 定义组件属性
const props = defineProps({
  points: {
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
});

// 定义事件
const emit = defineEmits(['edit', 'delete', 'page-change']);

// 处理编辑
const handleEdit = (point) => {
  emit('edit', point);
};

// 确认删除
const confirmDelete = (id, name) => {
  if (confirm(`确定要删除点位 "${name}" 吗？`)) {
    emit('delete', id);
  }
};

// 更改页码
const changePage = (page) => {
  emit('page-change', page);
};
</script>

<style scoped>
.point-list {
  padding: 20px;
}

.point-list-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.point-list-content {
  margin-bottom: 20px;
}

.point-list-pagination {
  display: flex;
  justify-content: center;
  align-items: center;
  gap: 10px;
}

.table {
  width: 100%;
  border-collapse: collapse;
}

.table th,
.table td {
  padding: 12px;
  text-align: left;
  border-bottom: 1px solid #ddd;
}

.table th {
  background-color: #f2f2f2;
  font-weight: bold;
}

.btn {
  padding: 6px 12px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
}

.btn-primary {
  background-color: #007bff;
  color: white;
}

.btn-info {
  background-color: #17a2b8;
  color: white;
}

.btn-danger {
  background-color: #dc3545;
  color: white;
}

.btn-sm {
  padding: 4px 8px;
  font-size: 12px;
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
