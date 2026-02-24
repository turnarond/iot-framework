<template>
  <div class="ntp-config">
    <div class="header">
      <h3>NTP配置</h3>
    </div>
    
    <div class="config-card">
      <div class="config-item">
        <label for="ntp-server">NTP服务器地址</label>
        <input type="text" id="ntp-server" v-model="ntpConfig.server" placeholder="例如：pool.ntp.org">
      </div>
      
      <div class="config-item">
        <label for="ntp-enabled">启用自动时间同步</label>
        <input type="checkbox" id="ntp-enabled" v-model="ntpConfig.enabled">
      </div>
      
      <div class="status-item">
        <label>当前系统时间</label>
        <span>{{ ntpConfig.current_time }}</span>
      </div>
      
      <div class="status-item">
        <label>同步状态</label>
        <span>{{ ntpConfig.sync_status }}</span>
      </div>
      
      <div class="action-buttons">
        <button class="btn btn-primary" @click="saveNtpConfig">保存配置</button>
      </div>
    </div>
  </div>
</template>

<script>
import { systemApi } from '../../services/systemApi';

export default {
  name: 'NtpConfig',
  data() {
    return {
      ntpConfig: {
        server: '',
        enabled: false,
        current_time: '',
        sync_status: ''
      }
    };
  },
  mounted() {
    this.fetchNtpConfig();
  },
  methods: {
    async fetchNtpConfig() {
      try {
        const data = await systemApi.getNtpConfig();
        this.ntpConfig = data;
      } catch (error) {
        console.error('获取NTP配置失败:', error);
      }
    },
    async saveNtpConfig() {
      try {
        const updatedConfig = await systemApi.updateNtpConfig(this.ntpConfig);
        this.ntpConfig = updatedConfig;
        alert('NTP配置保存成功');
      } catch (error) {
        console.error('保存NTP配置失败:', error);
        alert('保存NTP配置失败，请重试');
      }
    }
  }
};
</script>

<style scoped>
.ntp-config {
  padding: 20px;
}

.header {
  margin-bottom: 20px;
}

.config-card {
  background-color: #f9f9f9;
  padding: 20px;
  border-radius: 8px;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
}

.config-item {
  margin-bottom: 20px;
  display: flex;
  align-items: center;
}

.config-item label {
  width: 150px;
  font-weight: bold;
}

.config-item input[type="text"] {
  flex: 1;
  padding: 8px;
  border: 1px solid #ddd;
  border-radius: 4px;
}

.config-item input[type="checkbox"] {
  width: 20px;
  height: 20px;
}

.status-item {
  margin-bottom: 15px;
  display: flex;
  align-items: center;
}

.status-item label {
  width: 150px;
  font-weight: bold;
}

.status-item span {
  flex: 1;
  padding: 8px;
  background-color: #e9ecef;
  border-radius: 4px;
}

.action-buttons {
  margin-top: 30px;
  display: flex;
  justify-content: flex-start;
}

.btn {
  padding: 10px 20px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
}

.btn-primary {
  background-color: #007bff;
  color: white;
}

.btn-primary:hover {
  background-color: #0069d9;
}
</style>
