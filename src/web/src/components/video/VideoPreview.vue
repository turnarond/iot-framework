<template>
  <div class="video-preview">
    <div class="preview-header">
      <div class="header-left">
        <h2 class="page-title">视频预览</h2>
        <p class="page-description">实时监控摄像头画面</p>
      </div>
      <div class="header-right">
        <el-select v-model="selectedPartition" placeholder="选择分区" class="partition-select">
          <el-option v-for="partition in partitions" :key="partition.id" :label="partition.name" :value="partition.id" />
        </el-select>
        <el-button type="primary" @click="refreshCameras">
          <el-icon><Refresh /></el-icon>
          刷新摄像头
        </el-button>
      </div>
    </div>

    <div class="camera-grid">
      <div v-for="camera in cameras" :key="camera.id" class="camera-item">
        <div class="camera-header">
          <h3>{{ camera.name }}</h3>
          <div class="camera-info">
            <span class="camera-status" :class="{ 'online': camera.status === 'online', 'offline': camera.status === 'offline' }">
              {{ camera.status === 'online' ? '在线' : '离线' }}
            </span>
            <span class="camera-protocol">{{ camera.stream_protocol }}</span>
          </div>
        </div>
        <div class="video-container">
          <div class="video-player" v-if="camera.status === 'online'">
            <img 
              :src="`https://trae-api-cn.mchost.guru/api/ide/v1/text_to_image?prompt=security%20camera%20live%20feed%20screenshot&image_size=landscape_16_9`" 
              :alt="camera.name"
              class="video-placeholder"
            />
            <div class="video-overlay">
              <el-button 
                circle 
                size="small" 
                @click="controlCamera(camera.id, 'ptz', 'up')"
                class="control-btn"
              >
                <el-icon><ArrowUp /></el-icon>
              </el-button>
              <el-button 
                circle 
                size="small" 
                @click="controlCamera(camera.id, 'ptz', 'down')"
                class="control-btn"
              >
                <el-icon><ArrowDown /></el-icon>
              </el-button>
              <el-button 
                circle 
                size="small" 
                @click="controlCamera(camera.id, 'ptz', 'left')"
                class="control-btn"
              >
                <el-icon><ArrowLeft /></el-icon>
              </el-button>
              <el-button 
                circle 
                size="small" 
                @click="controlCamera(camera.id, 'ptz', 'right')"
                class="control-btn"
              >
                <el-icon><ArrowRight /></el-icon>
              </el-button>
            </div>
          </div>
          <div class="video-offline" v-else>
            <el-icon class="offline-icon"><VideoCamera /></el-icon>
            <p>设备离线</p>
          </div>
        </div>
        <div class="camera-controls">
          <el-button size="small" @click="fullscreenCamera(camera.id)">
            <el-icon><FullScreen /></el-icon>
            全屏
          </el-button>
          <el-button size="small" @click="snapshotCamera(camera.id)">
            <el-icon><CameraIcon /></el-icon>
            截图
          </el-button>
          <el-button size="small" @click="recordCamera(camera.id, !camera.isRecording)">
            <el-icon>{{ camera.isRecording ? 'VideoPause' : 'VideoPlay' }}</el-icon>
            {{ camera.isRecording ? '停止录制' : '开始录制' }}
          </el-button>
        </div>
      </div>
      <div v-if="cameras.length === 0" class="no-cameras">
        <el-icon class="no-cameras-icon"><VideoCamera /></el-icon>
        <p>当前分区暂无摄像头</p>
        <el-button type="primary" @click="$emit('switchToCameraManagement')">
          添加摄像头
        </el-button>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, watch } from 'vue'
import { Refresh, ArrowUp, ArrowDown, ArrowLeft, ArrowRight, FullScreen, Camera as CameraIcon, VideoPlay, VideoPause, VideoCamera } from '@element-plus/icons-vue'
import { cameraApi, partitionApi } from '@/services/videoApi'
import type { Camera, Partition } from '@/types'

interface CameraWithRecording extends Camera {
  isRecording: boolean
}

const emit = defineEmits(['switchToCameraManagement', 'cameraAdded'])

const selectedPartition = ref('')
const partitions = ref<Partition[]>([])
const cameras = ref<CameraWithRecording[]>([])
const loading = ref(false)
const error = ref('')

const loadPartitions = async () => {
  try {
    loading.value = true
    error.value = ''
    const response = await partitionApi.getPartitions()
    partitions.value = response.items
    if (partitions.value.length > 0 && !selectedPartition.value) {
      selectedPartition.value = partitions.value[0].id
      await loadCameras()
    }
  } catch (err) {
    error.value = '加载分区失败'
    console.error('Failed to load partitions:', err)
  } finally {
    loading.value = false
  }
}

const loadCameras = async () => {
  try {
    loading.value = true
    error.value = ''
    const response = await cameraApi.getCameras(1, 100, selectedPartition.value)
    cameras.value = response.items.map(camera => ({
      ...camera,
      isRecording: false
    }))
  } catch (err) {
    error.value = '加载摄像头失败'
    console.error('Failed to load cameras:', err)
  } finally {
    loading.value = false
  }
}

const refreshCameras = async () => {
  await loadCameras()
}

// 接收添加摄像头的事件，自动刷新摄像头列表
const handleCameraAdded = () => {
  loadCameras()
}

// 暴露给父组件的方法，用于添加摄像头后刷新
const refreshAfterAdd = () => {
  loadCameras()
}

defineExpose({
  refreshAfterAdd
})

const controlCamera = async (cameraId: string, action: string, direction?: string) => {
  try {
    await cameraApi.controlCamera(cameraId, action, { direction })
    console.log(`控制摄像头 ${cameraId}: ${action} ${direction}`)
  } catch (err) {
    console.error('Failed to control camera:', err)
  }
}

const fullscreenCamera = (cameraId: string) => {
  // 模拟全屏功能
  console.log(`摄像头 ${cameraId} 全屏`)
}

const snapshotCamera = (cameraId: string) => {
  // 模拟截图功能
  console.log(`摄像头 ${cameraId} 截图`)
}

const recordCamera = (cameraId: string, start: boolean) => {
  // 模拟录制功能
  const camera = cameras.value.find(c => c.id === cameraId)
  if (camera) {
    camera.isRecording = start
  }
  console.log(`${start ? '开始' : '停止'}录制摄像头 ${cameraId}`)
}

// 监听分区变化，重新加载摄像头
watch(selectedPartition, (newPartition) => {
  if (newPartition) {
    loadCameras()
  }
})

onMounted(() => {
  loadPartitions()
})
</script>

<style scoped>
.video-preview {
  padding: 30px;
  min-height: 100%;
  max-height: calc(100vh - 40px);
  overflow-y: auto;
}

.video-preview::-webkit-scrollbar {
  width: 8px;
}

.video-preview::-webkit-scrollbar-track {
  background: #f1f1f1;
  border-radius: 4px;
}

.video-preview::-webkit-scrollbar-thumb {
  background: #c1c1c1;
  border-radius: 4px;
}

.video-preview::-webkit-scrollbar-thumb:hover {
  background: #a8a8a8;
}

.dark-mode .video-preview::-webkit-scrollbar-track {
  background: #333;
}

.dark-mode .video-preview::-webkit-scrollbar-thumb {
  background: #555;
}

.dark-mode .video-preview::-webkit-scrollbar-thumb:hover {
  background: #666;
}

.preview-header {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  margin-bottom: 30px;
  padding-bottom: 20px;
  border-bottom: 1px solid #f0f0f0;
}

.dark-mode .preview-header {
  border-bottom: 1px solid #333;
}

.header-left {
  flex: 1;
}

.page-title {
  font-size: 24px;
  font-weight: 700;
  margin: 0 0 8px 0;
  color: #303133;
}

.dark-mode .page-title {
  color: #ffffff;
}

.page-description {
  font-size: 14px;
  color: #606266;
  margin: 0;
}

.dark-mode .page-description {
  color: #909399;
}

.header-right {
  display: flex;
  align-items: center;
  gap: 16px;
}

.partition-select {
  width: 220px;
}

.camera-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(380px, 1fr));
  gap: 24px;
}

.camera-item {
  background-color: #ffffff;
  border-radius: 12px;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.08);
  overflow: hidden;
  transition: all 0.3s ease;
}

.dark-mode .camera-item {
  background-color: #1f1f1f;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.3);
}

.camera-item:hover {
  transform: translateY(-4px);
  box-shadow: 0 6px 16px 0 rgba(0, 0, 0, 0.12);
}

.dark-mode .camera-item:hover {
  box-shadow: 0 6px 16px 0 rgba(0, 0, 0, 0.4);
}

.camera-header {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  padding: 20px;
  border-bottom: 1px solid #f0f0f0;
}

.dark-mode .camera-header {
  border-bottom: 1px solid #333;
}

.camera-header h3 {
  margin: 0 0 8px 0;
  font-size: 18px;
  font-weight: 600;
  color: #303133;
}

.dark-mode .camera-header h3 {
  color: #ffffff;
}

.camera-info {
  display: flex;
  flex-direction: column;
  align-items: flex-end;
  gap: 8px;
}

.camera-status {
  padding: 4px 12px;
  border-radius: 12px;
  font-size: 12px;
  font-weight: 500;
}

.camera-status.online {
  background-color: #f0f9eb;
  color: #67c23a;
}

.dark-mode .camera-status.online {
  background-color: #1b3a23;
  color: #85ce61;
}

.camera-status.offline {
  background-color: #fef0f0;
  color: #f56c6c;
}

.dark-mode .camera-status.offline {
  background-color: #3a1b1b;
  color: #f78989;
}

.camera-protocol {
  font-size: 12px;
  color: #909399;
  background-color: #f5f7fa;
  padding: 2px 8px;
  border-radius: 4px;
}

.dark-mode .camera-protocol {
  color: #c0c4cc;
  background-color: #2c3e50;
}

.video-container {
  position: relative;
  height: 220px;
  background-color: #f5f5f5;
}

.dark-mode .video-container {
  background-color: #2c2c2c;
}

.video-player {
  width: 100%;
  height: 100%;
  position: relative;
}

.video-placeholder {
  width: 100%;
  height: 100%;
  object-fit: cover;
  border-radius: 0;
}

.video-overlay {
  position: absolute;
  bottom: 16px;
  left: 50%;
  transform: translateX(-50%);
  display: flex;
  gap: 12px;
  background-color: rgba(0, 0, 0, 0.6);
  padding: 12px;
  border-radius: 8px;
}

.control-btn {
  background-color: rgba(255, 255, 255, 0.9);
  border: none;
  width: 36px;
  height: 36px;
  transition: all 0.2s ease;
}

.control-btn:hover {
  background-color: #409EFF;
  color: white;
}

.video-offline {
  width: 100%;
  height: 100%;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  color: #999;
}

.offline-icon {
  font-size: 56px;
  margin-bottom: 12px;
  color: #c0c4cc;
}

.camera-controls {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
  padding: 18px;
  border-top: 1px solid #f0f0f0;
  background-color: #fafafa;
}

.dark-mode .camera-controls {
  border-top: 1px solid #333;
  background-color: #1a1a1a;
}

.no-cameras {
  grid-column: 1 / -1;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  padding: 100px 20px;
  background-color: #fafafa;
  border-radius: 12px;
  border: 2px dashed #e4e7ed;
}

.dark-mode .no-cameras {
  background-color: #1a1a1a;
  border: 2px dashed #333;
}

.no-cameras-icon {
  font-size: 72px;
  color: #c0c4cc;
  margin-bottom: 16px;
}

.no-cameras p {
  font-size: 16px;
  color: #909399;
  margin: 0 0 24px 0;
}

.dark-mode .no-cameras p {
  color: #909399;
}

@media (max-width: 768px) {
  .video-preview {
    padding: 20px;
  }

  .preview-header {
    flex-direction: column;
    align-items: flex-start;
    gap: 16px;
  }

  .header-right {
    width: 100%;
    justify-content: space-between;
  }

  .camera-grid {
    grid-template-columns: 1fr;
  }
}
</style>