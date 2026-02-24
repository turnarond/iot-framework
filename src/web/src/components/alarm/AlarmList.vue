<template>
  <div class="alarm-list">
    <!-- 报警规则列表 -->
    <div class="alarm-rules-table">
      <table class="table table-striped">
        <thead>
          <tr>
            <th>ID</th>
            <th>点位名称</th>
            <th>设备名称</th>
            <th>规则名称</th>
            <th>报警方法</th>
            <th>报警阈值</th>
            <th>恢复阈值</th>
            <th>回差值</th>
            <th>启用状态</th>
            <th>操作</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="rule in alarmRules" :key="rule.id">
            <td>{{ rule.id }}</td>
            <td>{{ rule.point_name || '未知点位' }}</td>
            <td>{{ rule.device_name || '未知设备' }}</td>
            <td>{{ rule.name }}</td>
            <td>{{ getMethodName(rule.method) }}</td>
            <td>{{ rule.threshold }}</td>
            <td>{{ rule.restore_threshold || '-' }}</td>
            <td>{{ rule.hysteresis || '-' }}</td>
            <td>
              <span class="status-badge" :class="rule.enable ? 'status-active' : 'status-inactive'">
                {{ rule.enable ? '启用' : '禁用' }}
              </span>
            </td>
            <td>
              <button class="btn btn-sm btn-info" @click="$emit('edit', rule)">编辑</button>
              <button class="btn btn-sm btn-danger" @click="$emit('delete', rule.id)">删除</button>
            </td>
          </tr>
        </tbody>
      </table>
    </div>
    <!-- 分页控件 -->
    <div class="pagination">
      <button class="btn btn-sm" @click="changePage(currentPage - 1)" :disabled="currentPage === 1">上一页</button>
      <span>{{ currentPage }} / {{ totalPages }}</span>
      <button class="btn btn-sm" @click="changePage(currentPage + 1)" :disabled="currentPage === totalPages">下一页</button>
    </div>
  </div>
</template>

<script>
export default {
  name: 'AlarmList',
  props: {
    alarmRules: {
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
    // 切换规则页面
    changePage(page) {
      if (page >= 1 && page <= this.totalPages) {
        this.$emit('page-change', page);
      }
    },

    // 获取报警方法名称
    getMethodName(method) {
      const ALARM_METHOD_MAP = {
        1: '高高限',
        2: '高限',
        3: '低限',
        4: '低低限',
        5: '定值'
      };
      return ALARM_METHOD_MAP[method] || '未知方法';
    }
  }
};
</script>

<style scoped>
.alarm-list {
  width: 100%;
}

.alarm-rules-table {
  margin-bottom: 20px;
  overflow-x: auto;
}

.table {
  width: 100%;
  border-collapse: collapse;
}

.table th,
.table td {
  padding: 10px;
  text-align: left;
  border-bottom: 1px solid #ddd;
}

.table th {
  background-color: #f8f9fa;
  font-weight: bold;
}

.status-badge {
  display: inline-block;
  padding: 4px 8px;
  border-radius: 12px;
  font-size: 12px;
  font-weight: 500;
}

.status-active {
  background-color: #d4edda;
  color: #155724;
}

.status-inactive {
  background-color: #f8d7da;
  color: #721c24;
}

.pagination {
  display: flex;
  justify-content: center;
  align-items: center;
  gap: 10px;
  margin-top: 20px;
}

.btn {
  padding: 8px 16px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
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

.btn:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}
</style>