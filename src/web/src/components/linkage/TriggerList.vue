<template>
  <div class="trigger-list">
    <el-table :data="triggers" style="width: 100%" stripe>
      <el-table-column prop="id" label="ID" width="80" align="center" />
      <el-table-column prop="name" label="名称" min-width="180">
        <template #default="scope">
          <span class="ellipsis">{{ scope.row.name }}</span>
        </template>
      </el-table-column>
      <el-table-column label="类型" width="120">
        <template #default="scope">
          <span>{{ eventTypeMap[scope.row.event_type_id] ? (eventTypeMap[scope.row.event_type_id].cname || eventTypeMap[scope.row.event_type_id].name) : scope.row.event_type_id }}</span>
        </template>
      </el-table-column>
      <el-table-column label="关联报警规则 / 表达式" min-width="200">
        <template #default="scope">
          <div v-if="scope.row.event_type_id===1 || scope.row.event_type_id===2">{{ alarmMap[scope.row.alarm_rule_id] ? alarmMap[scope.row.alarm_rule_id].name : scope.row.alarm_rule_id }}</div>
          <div v-else-if="scope.row.event_type_id===3">{{ expressionMap[scope.row.expr_id] ? expressionMap[scope.row.expr_id].expression : (scope.row.expr_name || '-') }}</div>
          <div v-else-if="scope.row.event_type_id===4">{{ scope.row.custom_event_key || '-' }}</div>
          <div v-else>-</div>
        </template>
      </el-table-column>
      <el-table-column prop="description" label="描述" min-width="150" show-overflow-tooltip />
      <el-table-column label="操作" width="150" align="center">
        <template #default="scope">
          <el-button type="primary" size="small" @click="edit(scope.row)" style="margin-right: 8px">编辑</el-button>
          <el-button type="danger" size="small" @click="remove(scope.row.id)">删除</el-button>
        </template>
      </el-table-column>
    </el-table>

    <TriggerForm v-if="showForm" :visible="showForm" :trigger-data="current" @close="closeForm" @saved="onSaved" />
  </div>
</template>

<script>
import { ref, onMounted } from 'vue';
import { useLinkageApi, useAlarmApi } from '../../hooks/useApi.js';
import TriggerForm from './TriggerForm.vue';

export default {
  name: 'TriggerList',
  components: { TriggerForm },
  props: {
    triggers: {
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
    const { getEventTypes, getExpressions } = useLinkageApi();
    const { getAlarmRules } = useAlarmApi();
    const eventTypes = ref([]);
    const eventTypeMap = ref({});
    const alarms = ref([]);
    const alarmMap = ref({});
    const expressions = ref([]);
    const expressionMap = ref({});
    const showForm = ref(false);
    const current = ref({});

    const fetchTypesAndAlarms = async () => {
      try {
        const et = await getEventTypes.call(1, 200);
        eventTypes.value = et.items || [];
        eventTypeMap.value = {};
        (eventTypes.value || []).forEach(x => eventTypeMap.value[x.id] = x);
      } catch (e) { console.error(e); }
      try {
        const ar = await getAlarmRules.call(1, 200);
        alarms.value = ar.items || [];
        alarmMap.value = {};
        (alarms.value || []).forEach(a => alarmMap.value[a.id] = a);
      } catch (e) { console.error(e); }
      try {
        const ex = await getExpressions.call(1, 200);
        expressions.value = ex.items || [];
        expressionMap.value = {};
        (expressions.value || []).forEach(x => expressionMap.value[x.id] = x);
      } catch (e) { console.error(e); }
    };

    onMounted(async () => { await fetchTypesAndAlarms(); });

    const edit = (t) => {
      emit('edit', t);
    };

    const remove = async (id) => {
      emit('delete', id);
    };

    return { 
      eventTypeMap, 
      alarmMap,
      expressionMap,
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
