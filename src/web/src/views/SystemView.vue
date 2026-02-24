<template>
  <div class="system-view">
    <el-page-header>
      <template #title>系统配置</template>
      <template #subtitle>管理系统的网络、NTP和其他配置</template>
    </el-page-header>

    <el-tabs type="border-card">
      <el-tab-pane label="网络配置">
        <el-card class="system-card">
          <template #header>
            <div class="card-header">
              <span>网络接口配置</span>
              <el-button type="primary" size="small" @click="handleSaveNetworkConfig">
                <el-icon><Check /></el-icon>
                保存配置
              </el-button>
            </div>
          </template>

          <!-- 使用NetworkConfig组件 -->
          <NetworkConfig :config="networkConfig" @update:config="networkConfig = $event" />
        </el-card>
      </el-tab-pane>

      <el-tab-pane label="NTP配置">
        <el-card class="system-card">
          <template #header>
            <div class="card-header">
              <span>NTP服务器配置</span>
              <el-button type="primary" size="small" @click="handleSaveNtpConfig">
                <el-icon><Check /></el-icon>
                保存配置
              </el-button>
            </div>
          </template>

          <!-- 使用NtpConfig组件 -->
          <NtpConfig :config="ntpConfig" @update:config="ntpConfig = $event" />
        </el-card>
      </el-tab-pane>

      <el-tab-pane label="用户管理">
        <el-card class="system-card">
          <template #header>
            <div class="card-header">
              <span>用户列表</span>
              <el-button type="primary" size="small" @click="handleAddUser">
                <el-icon><Plus /></el-icon>
                添加用户
              </el-button>
            </div>
          </template>

          <!-- 使用UserList组件 -->
          <UserList :users="users" @edit="handleEditUser" @delete="handleDeleteUser" />
        </el-card>
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { Check, Plus } from '@element-plus/icons-vue'
import NetworkConfig from '@/components/system/NetworkConfig.vue'
import NtpConfig from '@/components/system/NtpConfig.vue'
import UserList from '@/components/system/UserList.vue'
import type { NetworkConfig as NetworkConfigType, NtpConfig as NtpConfigType } from '@/services/systemApi'
import { systemApi } from '@/services/systemApi'
import type { User } from '@/services/userApi'
import { userApi } from '@/services/userApi'

const networkConfig = ref<NetworkConfigType>({
  id: '1',
  interface: 'eth0',
  ipAddress: '192.168.1.100',
  subnetMask: '255.255.255.0',
  gateway: '192.168.1.1',
  dns1: '8.8.8.8',
  dns2: '8.8.4.4'
})

const ntpConfig = ref<NtpConfigType>({
  id: '1',
  server: 'pool.ntp.org',
  timezone: 'Asia/Shanghai',
  enabled: true
})

const users = ref<User[]>([])

// 初始化数据
onMounted(() => {
  fetchNetworkConfig()
  fetchNtpConfig()
  fetchUsers()
})

// 获取网络配置
const fetchNetworkConfig = async () => {
  try {
    const response = await systemApi.getNetworkConfig()
    networkConfig.value = response
  } catch (error) {
    console.error('获取网络配置失败:', error)
  }
}

// 获取NTP配置
const fetchNtpConfig = async () => {
  try {
    const response = await systemApi.getNtpConfig()
    ntpConfig.value = response
  } catch (error) {
    console.error('获取NTP配置失败:', error)
  }
}

// 获取用户列表
const fetchUsers = async () => {
  try {
    const response = await userApi.getUsers()
    users.value = response.data
  } catch (error) {
    console.error('获取用户列表失败:', error)
  }
}

// 保存网络配置
const handleSaveNetworkConfig = async () => {
  try {
    await systemApi.updateNetworkConfig(networkConfig.value)
    console.log('网络配置保存成功')
  } catch (error) {
    console.error('保存网络配置失败:', error)
  }
}

// 保存NTP配置
const handleSaveNtpConfig = async () => {
  try {
    await systemApi.updateNtpConfig(ntpConfig.value)
    console.log('NTP配置保存成功')
  } catch (error) {
    console.error('保存NTP配置失败:', error)
  }
}

// 用户操作
const handleAddUser = () => {
  console.log('添加用户')
  // 这里可以添加打开用户添加对话框的逻辑
}

const handleEditUser = (user: User) => {
  console.log('编辑用户:', user.id)
  // 这里可以添加打开用户编辑对话框的逻辑
}

const handleDeleteUser = async (id: string) => {
  try {
    await userApi.deleteUser(id)
    fetchUsers()
  } catch (error) {
    console.error('删除用户失败:', error)
  }
}
</script>

<style scoped>
.system-view {
  padding: 20px;
  background-color: #f5f7fa;
  min-height: 100vh;
}

.system-card {
  margin-top: 20px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.el-tabs {
  margin-top: 20px;
}
</style>