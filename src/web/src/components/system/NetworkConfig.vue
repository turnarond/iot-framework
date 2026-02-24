<template>
  <div class="network-config">
    <div class="header">
      <h3>网络配置</h3>
    </div>
    
    <div class="config-card">
      <div class="config-item">
        <label for="network-mode">网络模式</label>
        <select id="network-mode" v-model="networkConfig.mode" @change="handleNetworkModeChange">
          <option value="dhcp">DHCP</option>
          <option value="static">静态IP</option>
        </select>
      </div>
      
      <div class="config-item" v-if="networkConfig.mode === 'static'">
        <label for="ip-address">IP地址</label>
        <input type="text" id="ip-address" v-model="networkConfig.ip_address" placeholder="例如：192.168.1.100">
      </div>
      
      <div class="config-item" v-if="networkConfig.mode === 'static'">
        <label for="subnet-mask">子网掩码</label>
        <input type="text" id="subnet-mask" v-model="networkConfig.subnet_mask" placeholder="例如：255.255.255.0">
      </div>
      
      <div class="config-item" v-if="networkConfig.mode === 'static'">
        <label for="gateway">网关</label>
        <input type="text" id="gateway" v-model="networkConfig.gateway" placeholder="例如：192.168.1.1">
      </div>
      
      <div class="config-item">
        <label for="dns1">首选DNS</label>
        <input type="text" id="dns1" v-model="networkConfig.dns1" placeholder="例如：8.8.8.8">
      </div>
      
      <div class="config-item">
        <label for="dns2">备用DNS</label>
        <input type="text" id="dns2" v-model="networkConfig.dns2" placeholder="例如：8.8.4.4">
      </div>
      
      <div class="action-buttons">
        <button class="btn btn-primary" @click="saveNetworkConfig">保存配置</button>
        <button class="btn btn-secondary" @click="restartNetworkService">重启网络服务</button>
        <button class="btn btn-danger" @click="confirmRestartSystem">重启系统</button>
      </div>
    </div>
  </div>
</template>

<script>
import { systemApi } from '../../services/systemApi';

export default {
  name: 'NetworkConfig',
  data() {
    return {
      networkConfig: {
        mode: 'dhcp',
        ip_address: '',
        subnet_mask: '',
        gateway: '',
        dns1: '',
        dns2: ''
      }
    };
  },
  mounted() {
    this.fetchNetworkConfig();
  },
  methods: {
    async fetchNetworkConfig() {
      try {
        const data = await systemApi.getNetworkConfig();
        this.networkConfig = data;
      } catch (error) {
        console.error('获取网络配置失败:', error);
      }
    },
    handleNetworkModeChange() {
      // 当切换到DHCP模式时，清空静态IP相关字段
      if (this.networkConfig.mode === 'dhcp') {
        this.networkConfig.ip_address = '';
        this.networkConfig.subnet_mask = '';
        this.networkConfig.gateway = '';
      }
    },
    async saveNetworkConfig() {
      try {
        const updated = await systemApi.updateNetworkConfig(this.networkConfig);
        this.networkConfig = updated;
        alert('网络配置保存成功');
      } catch (error) {
        console.error('保存网络配置失败:', error);
        alert('保存网络配置失败，请重试');
      }
    },
    async restartNetworkService() {
      if (confirm('确定要重启网络服务吗？这可能会导致网络连接暂时中断。')) {
        try {
          await systemApi.restartNetworkService();
          alert('网络服务重启请求已发送，请稍候...');
        } catch (error) {
          console.error('重启网络服务失败:', error);
          alert('重启网络服务失败，请重试');
        }
      }
    },
    confirmRestartSystem() {
      if (confirm('确定要重启系统吗？这将会重启整个设备，请确保所有工作已保存。')) {
        this.restartSystem();
      }
    },
    async restartSystem() {
      try {
        await systemApi.restartSystem();
        alert('系统重启请求已发送，设备将会在几秒钟后重启...');
      } catch (error) {
        console.error('重启系统失败:', error);
        alert('重启系统失败，请重试');
      }
    }
  }
};
</script>

<style scoped>
.network-config {
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
}

.config-item label {
  display: block;
  margin-bottom: 5px;
  font-weight: bold;
}

.config-item input,
.config-item select {
  width: 100%;
  padding: 8px;
  border: 1px solid #ddd;
  border-radius: 4px;
}

.action-buttons {
  margin-top: 30px;
  display: flex;
  gap: 10px;
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

.btn-secondary {
  background-color: #6c757d;
  color: white;
}

.btn-secondary:hover {
  background-color: #5a6268;
}

.btn-danger {
  background-color: #dc3545;
  color: white;
}

.btn-danger:hover {
  background-color: #c82333;
}
</style>
