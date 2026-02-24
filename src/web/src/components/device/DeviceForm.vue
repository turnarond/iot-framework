<template>
  <div class="modal" v-if="visible" @click="handleModalClick">
    <div class="modal-content" @click.stop>
      <div class="modal-header">
        <h3>{{ editMode ? '编辑设备' : '添加设备' }}</h3>
        <button class="modal-close" @click="$emit('close')">&times;</button>
      </div>
      <form @submit.prevent="handleSubmit">
        <div class="form-group">
          <label for="deviceName">设备名称 *</label>
          <input
            type="text"
            id="deviceName"
            v-model="deviceForm.name"
            required
          />
        </div>
        <div class="form-group">
          <label for="deviceDriver">驱动类型 *</label>
          <select
            id="deviceDriver"
            v-model="deviceForm.driver_id"
            @change="handleDriverChange"
            required
          >
            <option value="">请选择驱动</option>
            <option v-for="driver in drivers" :key="driver.id" :value="driver.id">
              {{ driver.name }}
            </option>
          </select>
        </div>
        <div class="form-group">
          <label for="deviceConnType">连接类型 *</label>
          <select
            id="deviceConnType"
            v-model="deviceForm.conn_type"
            @change="handleConnTypeChange"
            required
          >
            <option value="">请选择连接类型</option>
            <option v-for="type in connTypes" :key="type.id" :value="type.id">
              {{ type.cname }}
            </option>
          </select>
        </div>
        <div class="form-group" v-if="deviceForm.conn_type">
          <label>连接参数 *</label>
          <div v-if="deviceForm.conn_type === 1">
            <!-- TCP客户端参数 -->
            <div class="sub-form-group">
              <label for="tcpHost">IP地址 *</label>
              <input
                type="text"
                id="tcpHost"
                v-model="connParams.tcpHost"
                placeholder="请输入IP地址"
                required
              />
            </div>
            <div class="sub-form-group">
              <label for="tcpPort">端口 *</label>
              <input
                type="number"
                id="tcpPort"
                v-model="connParams.tcpPort"
                placeholder="请输入端口号"
                required
              />
            </div>
          </div>
          <div v-else-if="deviceForm.conn_type === 2">
            <!-- TCP服务端参数 -->
            <div class="sub-form-group">
              <label for="tcpListenAddress">监听IP *</label>
              <input
                type="text"
                id="tcpListenAddress"
                v-model="connParams.tcpListenAddress"
                placeholder="请输入监听IP地址"
                required
              />
            </div>
            <div class="sub-form-group">
              <label for="tcpListenPort">监听端口 *</label>
              <input
                type="number"
                id="tcpListenPort"
                v-model="connParams.tcpListenPort"
                placeholder="请输入监听端口号"
                required
              />
            </div>
          </div>
          <div v-else-if="deviceForm.conn_type === 3">
            <!-- UDP参数 -->
            <div class="sub-form-group">
              <label for="udpLocalPort">本地端口 *</label>
              <input
                type="number"
                id="udpLocalPort"
                v-model="connParams.udpLocalPort"
                placeholder="请输入本地端口号"
                required
              />
            </div>
            <div class="sub-form-group">
              <label for="udpRemoteAddress">远程IP</label>
              <input
                type="text"
                id="udpRemoteAddress"
                v-model="connParams.udpRemoteAddress"
                placeholder="请输入远程IP地址"
              />
            </div>
            <div class="sub-form-group">
              <label for="udpRemotePort">远程端口</label>
              <input
                type="number"
                id="udpRemotePort"
                v-model="connParams.udpRemotePort"
                placeholder="请输入远程端口号"
              />
            </div>
          </div>
          <div v-else-if="deviceForm.conn_type === 4">
            <!-- 串口参数 -->
            <div class="sub-form-group">
              <label for="serialPort">串口 *</label>
              <input
                type="text"
                id="serialPort"
                v-model="connParams.serialPort"
                placeholder="请输入串口路径"
                required
              />
            </div>
            <div class="sub-form-group">
              <label for="serialBaudrate">波特率 *</label>
              <input
                type="number"
                id="serialBaudrate"
                v-model="connParams.serialBaudrate"
                placeholder="请输入波特率"
                required
              />
            </div>
            <div class="sub-form-group">
              <label for="serialParity">奇偶校验 *</label>
              <select
                id="serialParity"
                v-model="connParams.serialParity"
                required
              >
                <option value="N">无</option>
                <option value="O">奇校验</option>
                <option value="E">偶校验</option>
              </select>
            </div>
            <div class="sub-form-group">
              <label for="serialDatabits">数据位 *</label>
              <select
                id="serialDatabits"
                v-model="connParams.serialDatabits"
                required
              >
                <option value="5">5</option>
                <option value="6">6</option>
                <option value="7">7</option>
                <option value="8">8</option>
              </select>
            </div>
            <div class="sub-form-group">
              <label for="serialStopbits">停止位 *</label>
              <select
                id="serialStopbits"
                v-model="connParams.serialStopbits"
                required
              >
                <option value="1">1</option>
                <option value="2">2</option>
              </select>
            </div>
          </div>
        </div>
        <div class="form-group">
          <label for="deviceDescription">设备描述</label>
          <textarea
            id="deviceDescription"
            v-model="deviceForm.description"
            placeholder="请简要描述设备功能或用途"
          ></textarea>
        </div>
        <div class="form-group" v-if="deviceParams.length > 0">
          <label>参数配置</label>
          <table>
            <thead>
              <tr>
                <th>序号</th>
                <th>参数名称</th>
                <th>参数值</th>
                <th>描述</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="(param, index) in deviceParams" :key="index">
                <td>{{ index + 1 }}</td>
                <td>{{ param.name }}</td>
                <td>
                  <div class="input-with-placeholder">
                    <input
                      type="text"
                      v-model="param.value"
                      @focus="param.focused = true"
                      @blur="param.focused = false"
                    />
                    <span v-if="!param.focused && !param.value" class="input-placeholder">{{ param.desc }}</span>
                  </div>
                </td>
                <td>{{ param.desc }}</td>
              </tr>
            </tbody>
          </table>
        </div>
        <div class="form-actions">
          <button type="button" class="btn" @click="$emit('close')">
            取消
          </button>
          <button type="submit" class="btn btn-primary">
            确定
          </button>
        </div>
      </form>
    </div>
  </div>
</template>

<script>
export default {
  name: 'DeviceForm',
  props: {
    visible: {
      type: Boolean,
      default: false
    },
    editMode: {
      type: Boolean,
      default: false
    },
    deviceData: {
      type: Object,
      default: () => ({
        id: '',
        name: '',
        driver_id: '',
        conn_type: '',
        connparam: '',
        description: '',
        param1: '',
        param2: '',
        param3: '',
        param4: ''
      })
    },
    drivers: {
      type: Array,
      default: () => []
    },
    connTypes: {
      type: Array,
      default: () => []
    }
  },
  data() {
    return {
      deviceForm: {
        id: '',
        name: '',
        driver_id: '',
        conn_type: '',
        connparam: '',
        description: '',
        param1: '',
        param2: '',
        param3: '',
        param4: ''
      },
      deviceParams: [],
      connParams: {
        // TCP客户端参数
        tcpHost: '',
        tcpPort: '',
        // TCP服务端参数
        tcpListenAddress: '',
        tcpListenPort: '',
        // UDP参数
        udpLocalPort: '',
        udpRemoteAddress: '',
        udpRemotePort: '',
        // 串口参数
        serialPort: '',
        serialBaudrate: 9600,
        serialParity: 'N',
        serialDatabits: 8,
        serialStopbits: 1
      }
    }
  },
  watch: {
    deviceData: {
      handler(newData) {
        this.deviceForm = { ...newData };
        // 初始化参数列表
        this.initDeviceParams();
        // 初始化连接参数
        this.initConnParams();
      },
      deep: true,
      immediate: true
    }
  },
  emits: ['close', 'submit'],
  methods: {
    // 处理驱动选择变化
    handleDriverChange() {
      this.initDeviceParams();
    },
    // 处理连接类型变化
    handleConnTypeChange() {
      // 重置连接参数
      this.resetConnParams();
    },
    // 重置连接参数
    resetConnParams() {
      this.connParams = {
        // TCP客户端参数
        tcpHost: '',
        tcpPort: '',
        // TCP服务端参数
        tcpListenAddress: '',
        tcpListenPort: '',
        // UDP参数
        udpLocalPort: '',
        udpRemoteAddress: '',
        udpRemotePort: '',
        // 串口参数
        serialPort: '',
        serialBaudrate: 9600,
        serialParity: 'N',
        serialDatabits: 8,
        serialStopbits: 1
      };
    },
    // 初始化设备参数列表
    initDeviceParams() {
      this.deviceParams = [];
      if (this.deviceForm.driver_id) {
        const selectedDriver = this.drivers.find(d => d.id === this.deviceForm.driver_id);
        if (selectedDriver) {
          // 根据驱动的参数配置生成设备参数列表
          if (selectedDriver.param1_name) {
            this.deviceParams.push({
              name: selectedDriver.param1_name,
              value: this.deviceForm.param1 || '',
              desc: selectedDriver.param1_desc || '',
              focused: false
            });
          }
          if (selectedDriver.param2_name) {
            this.deviceParams.push({
              name: selectedDriver.param2_name,
              value: this.deviceForm.param2 || '',
              desc: selectedDriver.param2_desc || '',
              focused: false
            });
          }
          if (selectedDriver.param3_name) {
            this.deviceParams.push({
              name: selectedDriver.param3_name,
              value: this.deviceForm.param3 || '',
              desc: selectedDriver.param3_desc || '',
              focused: false
            });
          }
          if (selectedDriver.param4_name) {
            this.deviceParams.push({
              name: selectedDriver.param4_name,
              value: this.deviceForm.param4 || '',
              desc: selectedDriver.param4_desc || '',
              focused: false
            });
          }
        }
      }
    },
    // 初始化连接参数
    initConnParams() {
      // 根据connparam解析连接参数
      if (this.deviceForm.connparam) {
        const connparam = this.deviceForm.connparam;
        // 这里可以根据连接类型和connparam字符串解析参数
        // 暂时留空，实际使用时需要实现
      }
    },
    // 格式化连接参数
    formatConnParams() {
      let connparam = '';
      
      switch (this.deviceForm.conn_type) {
        case 1: // TCP客户端
          connparam = `ip=${this.connParams.tcpHost};port=${this.connParams.tcpPort}`;
          break;
        case 2: // TCP服务端
          connparam = `listenip=${this.connParams.tcpListenAddress};listenport=${this.connParams.tcpListenPort}`;
          break;
        case 3: // UDP
          connparam = `localport=${this.connParams.udpLocalPort}`;
          if (this.connParams.udpRemoteAddress) {
            connparam += `;ip=${this.connParams.udpRemoteAddress};port=${this.connParams.udpRemotePort}`;
          }
          break;
        case 4: // 串口
          connparam = `port=${this.connParams.serialPort};baudrate=${this.connParams.serialBaudrate}`;
          connparam += `;parity=${this.connParams.serialParity}`;
          connparam += `;databits=${this.connParams.serialDatabits}`;
          connparam += `;stopbits=${this.connParams.serialStopbits}`;
          break;
      }
      
      return connparam;
    },
    // 处理表单提交
    handleSubmit() {
      // 将参数值更新到deviceForm中
      this.deviceParams.forEach((param, index) => {
        this.deviceForm[`param${index + 1}`] = param.value;
      });
      
      // 格式化连接参数
      this.deviceForm.connparam = this.formatConnParams();
      
      this.$emit('submit', { ...this.deviceForm });
    },
    // 处理点击模态框外部关闭
    handleModalClick() {
      this.$emit('close');
    }
  }
}
</script>

<style scoped>
.modal {
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background: rgba(0,0,0,0.5);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 1000;
}

.modal-content {
  background: white;
  border-radius: 8px;
  width: 90%;
  max-width: 800px;
  max-height: 90vh;
  overflow-y: auto;
}

.modal-header {
  padding: 16px;
  border-bottom: 1px solid #eee;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.modal-header h3 {
  margin: 0;
}

.modal-close {
  font-size: 24px;
  background: none;
  border: none;
  cursor: pointer;
}

form {
  padding: 20px;
}

.form-group {
  margin-bottom: 15px;
}

.sub-form-group {
  margin-bottom: 10px;
}

.form-group label,
.sub-form-group label {
  display: block;
  margin-bottom: 5px;
  font-weight: bold;
}

.form-group input,
.form-group select,
.form-group textarea,
.sub-form-group input,
.sub-form-group select {
  width: 100%;
  padding: 8px;
  border: 1px solid #ddd;
  border-radius: 4px;
}

.form-group textarea {
  resize: vertical;
  min-height: 80px;
}

.form-group table {
  width: 100%;
  border-collapse: collapse;
  margin-top: 10px;
}

.form-group table th,
.form-group table td {
  padding: 8px;
  border: 1px solid #ddd;
  text-align: left;
}

.form-group table th {
  background-color: #f5f5f5;
}

.input-with-placeholder {
  position: relative;
}

.input-with-placeholder input {
  width: 100%;
  padding: 8px;
  border: 1px solid #ddd;
  border-radius: 4px;
}

.input-placeholder {
  position: absolute;
  left: 8px;
  top: 8px;
  color: #999;
  pointer-events: none;
}

.form-actions {
  margin-top: 20px;
  display: flex;
  gap: 10px;
  justify-content: flex-end;
}

.btn {
  padding: 8px 16px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
}

.btn-primary {
  background-color: #409eff;
  color: white;
}

.btn-primary:hover {
  background-color: #66b1ff;
}

.btn:hover {
  opacity: 0.9;
}
</style>
