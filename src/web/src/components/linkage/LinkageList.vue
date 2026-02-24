<template>
  <div class="linkage-list">
    <el-table :data="linkageRules" style="width: 100%" stripe>
      <el-table-column prop="name" label="规则名称" min-width="200">
        <template #default="scope">
          <span class="ellipsis">{{ scope.row.name }}</span>
        </template>
      </el-table-column>
      <el-table-column label="触发条件摘要" min-width="250">
        <template #default="scope">
          <span class="ellipsis">{{ summarizeTriggers(scope.row) }}</span>
        </template>
      </el-table-column>
      <el-table-column label="动作列表" min-width="200">
        <template #default="scope">
          <span class="ellipsis">{{ summarizeActions(scope.row) }}</span>
        </template>
      </el-table-column>
      <el-table-column label="状态" width="100">
        <template #default="scope">
          <el-tag :type="scope.row.enable ? 'success' : 'info'">
            {{ scope.row.enable ? '启用' : '禁用' }}
          </el-tag>
        </template>
      </el-table-column>
      <el-table-column label="操作" width="150" align="center">
        <template #default="scope">
          <el-button type="primary" size="small" @click="$emit('edit', scope.row)" style="margin-right: 8px">编辑</el-button>
          <el-button type="danger" size="small" @click="$emit('delete', scope.row.id)">删除</el-button>
        </template>
      </el-table-column>
    </el-table>
  </div>
</template>

<script>
export default {
  name: 'LinkageList',
  props: {
    linkageRules: {
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
    // 总结触发条件
    summarizeTriggers(rule) {
      if (rule.triggers && rule.triggers.length > 0) {
        const names = rule.triggers.slice(0, 2).map(t => t.name || '触发器');
        const more = rule.triggers.length > 2 ? ` +${rule.triggers.length - 2}` : '';
        return names.join(', ') + more;
      } else if (rule.action_names) {
        return rule.action_names.split(';').slice(0, 2).join(', ') + (rule.action_names.split(';').length > 2 ? '...' : '');
      }
      return '-';
    },
    
    // 总结动作列表
    summarizeActions(rule) {
      if (rule.actions && rule.actions.length > 0) {
        const names = rule.actions.slice(0, 2).map(a => a.name || '动作');
        const more = rule.actions.length > 2 ? ` +${rule.actions.length - 2}` : '';
        return names.join(', ') + more;
      } else if (rule.action_names) {
        const names = rule.action_names.split(';').slice(0, 2);
        const more = rule.action_names.split(';').length > 2 ? '...' : '';
        return names.join(', ') + more;
      }
      return '-';
    }
  }
};
</script>

<style scoped>
.linkage-list {
  width: 100%;
}

.ellipsis {
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}
</style>
