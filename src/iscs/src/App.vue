<template>
  <n-config-provider>
    <n-message-provider>
      <n-layout style="height:100vh;">
          <n-layout-header style="padding:12px; font-weight:600">ISCS Monitor Prototype</n-layout-header>
          <n-layout-content style="padding:16px;">
            <TestView />
          </n-layout-content>
        </n-layout>
    </n-message-provider>
  </n-config-provider>
</template>

<script lang="ts">
import { defineComponent } from 'vue'
import TestView from './views/TestView.vue'
import { NConfigProvider, NLayout, NLayoutHeader, NLayoutContent, NMessageProvider } from 'naive-ui'

export default defineComponent({
  components: { TestView, NConfigProvider, NLayout, NLayoutHeader, NLayoutContent, NMessageProvider }
})
</script>
