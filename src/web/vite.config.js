import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import path from 'path'

// https://vite.dev/config/
export default defineConfig({
  plugins: [vue()],
  resolve: {
    alias: {
      '@': path.resolve(__dirname, './src')
    }
  },
  server: {
    port: 3000,
    proxy: {
      '/api': {
        // 在开发时可通过环境变量 BACKEND_URL 指定后端地址
        target: process.env.BACKEND_URL || 'http://localhost:8080',
        changeOrigin: true
      }
    }
  },
  build: {
    rollupOptions: {
      output: {
        manualChunks: {
          // 将第三方库打包到单独的chunk中
          vendor: ['vue', 'vue-router', 'pinia', 'element-plus'],
          // 将图表库单独打包
          echarts: ['echarts'],
          // 将工具库单独打包
          utils: ['axios', '@vueuse/core']
        }
      }
    },
    // 调整chunk大小警告阈值
    chunkSizeWarningLimit: 600
  }
})