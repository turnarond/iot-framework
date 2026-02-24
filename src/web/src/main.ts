import { createApp, watch } from 'vue'
import { createPinia } from 'pinia'
import ElementPlus from 'element-plus'
import 'element-plus/dist/index.css'
import { useDark } from '@vueuse/core'
import App from './App.vue'
import router from './router'

const app = createApp(App)
const pinia = createPinia()
const isDarkMode = useDark()

// 监听深色模式变化，更新Element Plus主题
watch(isDarkMode, (newValue) => {
  if (newValue) {
    document.documentElement.classList.add('dark')
  } else {
    document.documentElement.classList.remove('dark')
  }
}, { immediate: true })

app.use(pinia)
app.use(router)
app.use(ElementPlus, {
  // Element Plus 主题配置
  dark: isDarkMode.value
})

app.mount('#app')
