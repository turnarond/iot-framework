<template>
  <div class="action-list">
    <el-table :data="actions" style="width: 100%" stripe>
      <el-table-column prop="id" label="ID" width="80" align="center" />
      <el-table-column prop="name" label="名称" min-width="180">
        <template #default="scope">
          <span class="ellipsis">{{ scope.row.name }}</span>
        </template>
      </el-table-column>
      <el-table-column label="类型" width="120">
        <template #default="scope">
          <span>{{ actionTypeMap[scope.row.action_type_id] ? (actionTypeMap[scope.row.action_type_id].cname || actionTypeMap[scope.row.action_type_id].name) : scope.row.action_type_id }}</span>
        </template>
      </el-table-column>
      <el-table-column prop="param1" label="参数1" min-width="120" show-overflow-tooltip />
      <el-table-column prop="param2" label="参数2" min-width="120" show-overflow-tooltip />
      <el-table-column label="确认" width="80" align="center">
        <template #default="scope">
          <span>{{ scope.row.need_confirm ? '是' : '否' }}</span>
        </template>
      </el-table-column>
      <el-table-column label="操作" width="150" align="center">
        <template #default="scope">
          <el-button type="primary" size="small" @click="edit(scope.row)" style="margin-right: 8px">编辑</el-button>
          <el-button type="danger" size="small" @click="remove(scope.row.id)">删除</el-button>
        </template>
      </el-table-column>
    </el-table>

    <ActionForm v-if="showForm" :visible="showForm" :action-data="current" @close="closeForm" @saved="onSaved" />
  </div>
</template>

<script>
import { ref, onMounted } from 'vue';
import { useLinkageApi } from '../../hooks/useApi.js';
import ActionForm from './ActionForm.vue';

export default {
  name: 'ActionList',
  components: { ActionForm },
  props: {
    actions: {
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
  setup(props, { emit }) {
    const { getActionTypes } = useLinkageApi();
    const actionTypes = ref([]);
    const actionTypeMap = ref({});
    const showForm = ref(false);
    const current = ref({});

    const fetchActionTypes = async () => {
      try {
        const at = await getActionTypes.call(1, 200);
        actionTypes.value = at.items || [];
        actionTypeMap.value = {};
        (actionTypes.value || []).forEach(x => actionTypeMap.value[x.id] = x);
      } catch (e) { console.error(e); }
    };

    onMounted(async () => { await fetchActionTypes(); });

    const edit = (a) => {
      emit('edit', a);
    };

    const remove = async (id) => {
      emit('delete', id);
    };

    return { 
      actionTypeMap,
      edit, 
      remove 
    };
  }
};
</script>

<style scoped>
.ellipsis {
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}
</style>
