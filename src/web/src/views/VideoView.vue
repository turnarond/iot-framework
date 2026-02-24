<template>
  <div class="video-view">
    <el-page-header>
      <template #title>视频监控</template>
      <template #subtitle>摄像头管理和视频预览控制</template>
    </el-page-header>

    <el-tabs v-model="activeTab" class="video-tabs">
      <el-tab-pane label="视频预览" name="preview">
        <VideoPreview />
      </el-tab-pane>
      <el-tab-pane label="摄像头管理" name="management">
        <CameraManagement />
      </el-tab-pane>
      <el-tab-pane label="分区管理" name="partition">
        <PartitionManagement />
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import VideoPreview from '@/components/video/VideoPreview.vue'
import CameraManagement from '@/components/video/CameraManagement.vue'
import PartitionManagement from '@/components/video/PartitionManagement.vue'

const activeTab = ref('preview')
</script>

<style scoped>
.video-view {
  padding: 20px;
  background-color: #f5f7fa;
  min-height: 100vh;
  overflow-y: auto;
}

.dark-mode .video-view {
  background-color: #1a1a1a;
}

.video-tabs {
  margin-top: 20px;
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  overflow: visible;
}

.dark-mode .video-tabs {
  background-color: #1f1f1f;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.3);
}
</style>