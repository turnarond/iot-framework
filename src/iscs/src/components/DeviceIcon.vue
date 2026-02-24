<template>
  <div class="device" @click="openDetail">
    <div class="svg-wrap" :class="{ running: isRunning, waiting: isWaiting }">
      <svg v-if="type==='fan'" viewBox="0 0 64 64" width="48" height="48">
        <circle cx="32" cy="32" r="30" fill="#f5f5f5" stroke="#ddd" />
        <g :fill="isRunning ? '#1890ff' : '#999'">
          <path d="M32 32 l12 -6 a2 2 0 0 0 0 -4 l-8 -4 a2 2 0 0 0 -2 0 l-6 8" />
          <path d="M32 32 l-6 12 a2 2 0 0 0 4 0 l6 -8 a2 2 0 0 0 0 -4" />
        </g>
      </svg>
      <svg v-else-if="type==='pump'" viewBox="0 0 64 64" width="48" height="48">
        <rect x="8" y="20" width="48" height="24" rx="4" fill="#f5f5f5" stroke="#ddd" />
        <circle cx="32" cy="32" r="8" :fill="isRunning ? '#1890ff' : '#999'" />
      </svg>
      <svg v-else viewBox="0 0 64 64" width="48" height="48">
        <rect x="26" y="10" width="12" height="28" :fill="isRunning ? '#ffec3d' : '#999'" />
        <circle cx="32" cy="46" r="8" fill="#f5f5f5" stroke="#ddd" />
      </svg>
      <div class="overlay" v-if="isWaiting">Waiting...</div>
    </div>
    <div class="meta">
      <div class="name">{{ shortName }}</div>
      <div class="value">{{ valueDisplay }}</div>
    </div>
  </div>
</template>

<script lang="ts">
import { defineComponent, computed } from 'vue'
import { usePointStore } from '../stores/pointStore'

export default defineComponent({
  props: { id: { type: String, required: true } },
  emits: ['control'],
  setup(props, ctx){
    const store = usePointStore()
    const rec = computed(()=> store.getPoint(props.id) )
    const valueDisplay = computed(()=> rec.value?.value ?? '--')
    const shortName = computed(()=> props.id.split('.').pop())
    const type = computed(()=> {
      if (props.id.startsWith('fan')) return 'fan'
      if (props.id.startsWith('pump')) return 'pump'
      return 'light'
    })
    const isRunning = computed(()=> {
      const v = rec.value?.value ?? ''
      return v === '1' || (typeof v === 'string' && (v.toLowerCase() === 'on' || v.toLowerCase() === 'running'))
    })
    const isWaiting = computed(()=> store.isPending(props.id))

    function openDetail(){
      // emit control event to parent (parent will open modal)
      ctx.emit('control', props.id)
    }

    return { valueDisplay, shortName, type, isRunning, isWaiting, openDetail }
  }
})
</script>

<style scoped>
.device{ width:140px; height:110px; border-radius:6px; display:flex; flex-direction:column; align-items:center; justify-content:center; cursor:pointer; border:1px solid #eee }
.svg-wrap{ position:relative; width:48px; height:48px; display:flex; align-items:center; justify-content:center }
.svg-wrap.running{ box-shadow:0 0 6px rgba(24,144,255,0.2) }
.svg-wrap.waiting{ opacity:0.6 }
.overlay{ position:absolute; inset:0; display:flex; align-items:center; justify-content:center; background:rgba(255,255,255,0.6); font-size:12px }
.meta{ margin-top:8px; text-align:center }
.name{ font-weight:600 }
.value{ font-size:14px; color:#666 }
</style>
