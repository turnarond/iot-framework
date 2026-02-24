<template>
  <div class="app-container" :class="{ 'dark-mode': isDarkMode }">
    <el-container style="height: 100vh; overflow: hidden;">
      <el-header height="60px" class="app-header">
        <div class="header-content">
          <div class="logo">
            <h1>综合监控系统</h1>
          </div>
          <div class="header-right">
            <el-tooltip content="切换主题">
              <el-button 
                link 
                icon="el-icon-moon" 
                @click="toggleTheme"
                style="margin-right: 20px;"
              />
            </el-tooltip>
            <el-dropdown>
              <span class="user-dropdown">
                <el-avatar size="small">
                  <img src="https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=user%20avatar%20icon&image_size=square" alt="用户头像" />
                </el-avatar>
                <span class="user-name">管理员</span>
              </span>
              <template #dropdown>
                <el-dropdown-menu>
                  <el-dropdown-item>个人中心</el-dropdown-item>
                  <el-dropdown-item>退出登录</el-dropdown-item>
                </el-dropdown-menu>
              </template>
            </el-dropdown>
          </div>
        </div>
      </el-header>
      <el-container>
        <el-aside width="200px" class="app-sidebar" :class="{ 'collapsed': isSidebarCollapsed }">
          <el-button 
            link 
            class="collapse-btn" 
            @click="toggleSidebar"
            :icon="isSidebarCollapsed ? 'el-icon-s-unfold' : 'el-icon-s-fold'"
          />
          <el-menu
            default-active="/servermgr"
            class="sidebar-menu"
            router
            :collapse="isSidebarCollapsed"
          >
            <el-menu-item index="/servermgr">
              <el-icon><InfoFilled /></el-icon>
              <span>基础信息</span>
            </el-menu-item>
            <el-menu-item index="/monitor">
              <el-icon><Monitor /></el-icon>
              <span>状态监控</span>
            </el-menu-item>
            <el-menu-item index="/driver">
              <el-icon><Setting /></el-icon>
              <span>驱动管理</span>
            </el-menu-item>
            <el-menu-item index="/device">
              <el-icon><Menu /></el-icon>
              <span>设备管理</span>
            </el-menu-item>
            <el-menu-item index="/point">
              <el-icon><DataAnalysis /></el-icon>
              <span>点位管理</span>
            </el-menu-item>
            <el-menu-item index="/alarm">
              <el-icon><Warning /></el-icon>
              <span>报警管理</span>
            </el-menu-item>
            <el-sub-menu index="/linkage">
              <template #title>
                <el-icon><Connection /></el-icon>
                <span>联动管理</span>
              </template>
              <el-menu-item index="/linkage/trigger">触发器管理</el-menu-item>
              <el-menu-item index="/linkage/action">执行器管理</el-menu-item>
              <el-menu-item index="/linkage/rule">联动规则</el-menu-item>
            </el-sub-menu>
            <el-menu-item index="/video">
              <el-icon><VideoCamera /></el-icon>
              <span>视频监控</span>
            </el-menu-item>
            <el-menu-item index="/system">
              <el-icon><Tools /></el-icon>
              <span>系统配置</span>
            </el-menu-item>
          </el-menu>
        </el-aside>
        <el-main class="app-main">
          <router-view v-slot="{ Component }">
            <transition name="fade" mode="out-in">
              <component :is="Component" />
            </transition>
          </router-view>
        </el-main>
      </el-container>
    </el-container>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, watch } from 'vue'
import { Monitor, Setting, Menu, DataAnalysis, Warning, Connection, Tools, VideoCamera, InfoFilled } from '@element-plus/icons-vue'
import { useDark, useToggle } from '@vueuse/core'

// 深色模式
const isDarkMode = useDark()
const toggleTheme = useToggle(isDarkMode)

// 侧边栏折叠状态
const isSidebarCollapsed = ref(false)

// 切换侧边栏
const toggleSidebar = () => {
  isSidebarCollapsed.value = !isSidebarCollapsed.value
}

// 监听主题变化
watch(isDarkMode, (newValue) => {
  if (newValue) {
    document.documentElement.classList.add('dark')
  } else {
    document.documentElement.classList.remove('dark')
  }
})

onMounted(() => {
  // 初始化主题
  if (isDarkMode.value) {
    document.documentElement.classList.add('dark')
  }
  
  // 初始化 WebSocket 连接和 monitor store
  import('@/stores/monitor').then(({ useMonitorStore }) => {
    const monitorStore = useMonitorStore()
    monitorStore.initWebSocket()
  })
})
</script>

<style>
/* 全局样式重置 */
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: 'Helvetica Neue', Helvetica, 'PingFang SC', 'Hiragino Sans GB', 'Microsoft YaHei', Arial, sans-serif;
  font-size: 14px;
  line-height: 1.5;
  color: #303133;
  background-color: #f5f7fa;
  transition: all 0.3s ease;
}

/* 深色模式 */
.dark-mode body {
  color: #e4e7ed;
  background-color: #1a1a1a;
}

/* 应用容器 */
.app-container {
  width: 100%;
  height: 100vh;
  transition: all 0.3s ease;
}

/* 头部样式 */
.app-header {
  background-color: #ffffff;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  z-index: 100;
  transition: all 0.3s ease;
}

.dark-mode .app-header {
  background-color: #1f1f1f;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.3);
}

.header-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  height: 100%;
  padding: 0 20px;
}

.logo h1 {
  font-size: 20px;
  font-weight: bold;
  color: #409EFF;
  transition: all 0.3s ease;
}

.dark-mode .logo h1 {
  color: #66b1ff;
}

.header-right {
  display: flex;
  align-items: center;
}

.user-dropdown {
  display: flex;
  align-items: center;
  cursor: pointer;
}

.user-name {
  margin-left: 10px;
  font-size: 14px;
  color: #606266;
  transition: all 0.3s ease;
}

.dark-mode .user-name {
  color: #c0c4cc;
}

/* 侧边栏样式 */
.app-sidebar {
  background-color: #ffffff;
  box-shadow: 2px 0 4px rgba(0, 0, 0, 0.1);
  transition: all 0.3s ease;
  position: relative;
}

.dark-mode .app-sidebar {
  background-color: #1f1f1f;
  box-shadow: 2px 0 4px rgba(0, 0, 0, 0.3);
}

.collapse-btn {
  position: absolute;
  top: 20px;
  right: -10px;
  width: 20px;
  height: 20px;
  border-radius: 50%;
  background-color: #409EFF;
  color: white;
  z-index: 10;
  display: flex;
  align-items: center;
  justify-content: center;
}

.sidebar-menu {
  height: 100%;
  border-right: none;
  transition: all 0.3s ease;
}

.dark-mode .sidebar-menu {
  background-color: #1f1f1f;
}

.sidebar-menu .el-menu-item {
  height: 60px;
  line-height: 60px;
  margin: 0;
  transition: all 0.3s ease;
}

.dark-mode .sidebar-menu .el-menu-item {
  color: #c0c4cc;
}

.sidebar-menu .el-menu-item.is-active {
  background-color: #ecf5ff;
  color: #409EFF;
}

.dark-mode .sidebar-menu .el-menu-item.is-active {
  background-color: #1a2332;
  color: #66b1ff;
}

/* 主内容区域 */
.app-main {
  padding: 20px;
  overflow-y: auto;
  background-color: #f5f7fa;
  transition: all 0.3s ease;
}

.dark-mode .app-main {
  background-color: #1a1a1a;
}

/* 路由过渡动画 */
.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.3s ease;
}

.fade-enter-from,
.fade-leave-to {
  opacity: 0;
}

/* 响应式设计 */
@media screen and (max-width: 768px) {
  .app-sidebar {
    width: 60px !important;
  }

  .app-sidebar:not(.collapsed) .sidebar-menu .el-menu-item span {
    display: none;
  }

  .logo h1 {
    font-size: 16px;
  }

  .header-right .user-name {
    display: none;
  }
}

@media screen and (max-width: 480px) {
  .app-main {
    padding: 10px;
  }

  .logo h1 {
    font-size: 14px;
  }
}
</style>
