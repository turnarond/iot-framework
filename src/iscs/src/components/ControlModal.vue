<template>
  <n-modal :show="visible" :title="`Control ${pointId}`" @update:show="onUpdateShow">
    <div style="margin:8px 0">
      <n-input v-model:value="value" placeholder="Enter control value" />
    </div>
    <template #footer>
      <n-space style="float:right">
        <n-button tertiary @click="onCancel">Cancel</n-button>
        <n-button type="primary" @click="onSubmit">Send</n-button>
      </n-space>
    </template>
    <div v-if="message" style="margin-top:8px">{{ message }}</div>
  </n-modal>
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue'
import { NModal, NInput, NButton, NSpace } from 'naive-ui'

export default defineComponent({
  props: { visible: { type: Boolean, required: true }, pointId: { type: String, required: true } },
  emits: ['close','send'],
  components: { NModal, NInput, NButton, NSpace },
  setup(props, ctx){
    const value = ref('')
    const message = ref('')

    function onCancel(){ ctx.emit('close') }
    function onSubmit(){
      message.value = ''
      ctx.emit('send', { pointId: props.pointId, value: value.value, callback: (res:{success:boolean,message?:string})=>{ message.value = res.message ?? (res.success? 'ok':'failed') } })
    }
    function onUpdateShow(v:boolean){ if (!v) ctx.emit('close') }

    return { value, message, onCancel, onSubmit, onUpdateShow }
  }
})
</script>

<style scoped>
/* keep minimal styling; Naive UI provides styles */
</style>
