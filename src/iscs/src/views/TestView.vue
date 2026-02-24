<template>
  <div style="display:flex; gap:16px;">
    <div style="flex:1; border:1px solid #eee; padding:12px;">
      <h3>Station Floor</h3>
      <div style="display:flex; gap:12px; flex-wrap:wrap">
        <DeviceIcon v-for="id in deviceIds" :key="id" :id="id" @control="openControl" />
      </div>
    </div>
    <div style="width:320px; border:1px solid #eee; padding:12px;">
      <h3>Alarm / Controls</h3>
      <ul style="list-style:none; padding:0; margin:0">
        <li v-for="id in deviceIds" :key="id" style="padding:8px 0; border-bottom:1px solid #f0f0f0">
          <div style="display:flex; justify-content:space-between; width:100%">
            <div>{{ id }}</div>
            <div>
              <button @click="openControl(id)">Control</button>
            </div>
          </div>
        </li>
      </ul>
    </div>
    <ControlModal :visible="modalVisible" :pointId="modalPoint" @close="modalVisible=false" @send="handleSend" />
  </div>
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue'
import DeviceIcon from '../components/DeviceIcon.vue'
import ControlModal from '../components/ControlModal.vue'
import axios from 'axios'
import { usePointStore } from '../stores/pointStore'
import { NList, NListItem, NButton, useMessage } from 'naive-ui'

export default defineComponent({
  components: { DeviceIcon, ControlModal, NList, NListItem, NButton },
  setup(){
    const deviceIds = ['fan.1','pump.1','light.1']
    const store = usePointStore()
    const modalVisible = ref(false)
    const modalPoint = ref('')
    const message = useMessage()

    function openControl(id:string){ modalPoint.value = id; modalVisible.value = true }

    async function handleSend(payload:{pointId:string,value:string,callback:(res:{success:boolean,message?:string})=>void}){
      const { pointId, value, callback } = payload
      try{
        // set pending UI
        store.setPending(pointId)
        const resp = await axios.post(`/api/v1/control`, { pointId, value })
        const data = resp && resp.data ? resp.data : { success: false, message: 'no response' }
        callback({ success: !!data.success, message: data.message })
        if (data.success) message.success(data.message || 'Command sent')
        else message.error(data.message || 'Command failed')
      }catch(e:any){
        callback({ success: false, message: e?.message ?? 'error' })
        message.error(e?.message ?? 'Request error')
      } finally {
        modalVisible.value = false
      }
    }

    return { deviceIds, openControl, modalVisible, modalPoint, handleSend }
  }
})
</script>

<style scoped>
</style>
