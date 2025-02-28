<template>
  <div class="virtual-video-container">
    <!-- 虚拟视频显示区域 -->
    <div
      class="virtual-video"
      ref="videoRef"
      :class="{ 'is-playing': isPlaying }"
      @click="togglePlay"
    >
      <!-- 模拟视频帧 -->
      <div class="video-frame" :style="videoFrameStyle">
        <div class="segment-indicator">片段 #{{ currentSegment }}</div>
        <div class="time-indicator">{{ formatTime(currentTime) }}</div>
        <div class="play-indicator" v-if="!isPlaying">
          <n-icon size="48" class="play-icon"><play-icon /></n-icon>
        </div>
      </div>

      <!-- 缓冲指示器 -->
      <div class="buffering-indicator" v-if="isBuffering">
        <n-spin size="large" />
        <div>缓冲中...</div>
      </div>
    </div>

    <!-- 控制条 -->
    <div class="video-controls">
      <!-- 播放/暂停按钮 -->
      <n-button quaternary circle @click="togglePlay">
        <template #icon>
          <n-icon size="24">
            <component :is="isPlaying ? PauseIcon : PlayIcon" />
          </n-icon>
        </template>
      </n-button>

      <!-- 时间显示 -->
      <div class="time-display">
        {{ formatTime(currentTime) }} / {{ formatTime(duration) }}
      </div>

      <!-- 进度条和缓冲区 -->
      <div class="progress-container" @click="onProgressClick">
        <!-- 缓冲区指示 -->
        <div
          v-for="(range, index) in bufferRanges"
          :key="index"
          class="buffer-range"
          :style="{
            left: `${(range.start / duration) * 100}%`,
            width: `${((range.end - range.start) / duration) * 100}%`,
          }"
        ></div>

        <!-- 播放进度 -->
        <div class="progress-bar" :style="{ width: `${progress}%` }"></div>

        <!-- 进度条拖动手柄 -->
        <div class="progress-handle" :style="{ left: `${progress}%` }"></div>
      </div>

      <!-- 播放速率控制 -->
      <n-dropdown
        trigger="click"
        :options="playbackRateOptions"
        @select="onPlaybackRateSelect"
      >
        <n-button quaternary> {{ playbackRate }}x </n-button>
      </n-dropdown>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted } from "vue";
import { NButton, NIcon, NSpin, NDropdown } from "naive-ui";
import { Play as PlayIcon, Pause as PauseIcon } from "@vicons/ionicons5";
import { BufferRange } from "../../../packages/demuxer/src/VirtualTimeline";

// 属性定义
const props = defineProps<{
  currentTime: number;
  duration: number;
  bufferRanges: BufferRange[];
  isPlaying: boolean;
  isBuffering: boolean;
  currentSegment: number;
}>();

// 事件定义
const emit = defineEmits<{
  (event: "play"): void;
  (event: "pause"): void;
  (event: "seek", time: number): void;
  (event: "rateChange", rate: number): void;
}>();

// 状态
const videoRef = ref<HTMLDivElement | null>(null);
const playbackRate = ref(1.0);

// 计算属性
const progress = computed(() => {
  if (!props.duration) return 0;
  return (props.currentTime / props.duration) * 100;
});

// 视频帧样式
const videoFrameStyle = computed(() => {
  // 根据当前片段索引生成不同的背景色
  const hue = (props.currentSegment * 35) % 360;
  return {
    backgroundColor: `hsl(${hue}, 70%, 80%)`,
  };
});

// 播放速率选项
const playbackRateOptions = [
  {
    label: "0.5x",
    key: 0.5,
  },
  {
    label: "1.0x",
    key: 1,
  },
  {
    label: "1.5x",
    key: 1.5,
  },
  {
    label: "2.0x",
    key: 2,
  },
];

// 方法
const togglePlay = () => {
  if (props.isPlaying) {
    emit("pause");
  } else {
    emit("play");
  }
};

const onProgressClick = (event: MouseEvent) => {
  if (!videoRef.value) return;

  const progressContainer = event.currentTarget as HTMLElement;
  const rect = progressContainer.getBoundingClientRect();
  const position = (event.clientX - rect.left) / rect.width;
  const targetTime = position * props.duration;

  emit("seek", targetTime);
};

const onPlaybackRateSelect = (key: number) => {
  playbackRate.value = key;
  emit("rateChange", key);
};

// 格式化时间显示
const formatTime = (seconds: number): string => {
  const hours = Math.floor(seconds / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  const secs = Math.floor(seconds % 60);

  if (hours > 0) {
    return `${hours}:${minutes.toString().padStart(2, "0")}:${secs
      .toString()
      .padStart(2, "0")}`;
  } else {
    return `${minutes}:${secs.toString().padStart(2, "0")}`;
  }
};
</script>

<style scoped>
.virtual-video-container {
  display: flex;
  flex-direction: column;
  width: 100%;
  max-width: 800px;
  background-color: #000;
  border-radius: 8px;
  overflow: hidden;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);
}

.virtual-video {
  position: relative;
  width: 100%;
  aspect-ratio: 16 / 9;
  background-color: #111;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  overflow: hidden;
}

.video-frame {
  width: 100%;
  height: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  position: relative;
  transition: background-color 0.5s ease;
}

.segment-indicator {
  position: absolute;
  top: 10px;
  left: 10px;
  background-color: rgba(0, 0, 0, 0.7);
  color: white;
  padding: 4px 8px;
  border-radius: 4px;
  font-size: 14px;
}

.time-indicator {
  position: absolute;
  top: 10px;
  right: 10px;
  background-color: rgba(0, 0, 0, 0.7);
  color: white;
  padding: 4px 8px;
  border-radius: 4px;
  font-size: 14px;
}

.play-indicator {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  color: white;
  background-color: rgba(0, 0, 0, 0.5);
  border-radius: 50%;
  width: 60px;
  height: 60px;
  display: flex;
  align-items: center;
  justify-content: center;
}

.buffering-indicator {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background-color: rgba(0, 0, 0, 0.7);
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  color: white;
  gap: 10px;
}

.video-controls {
  display: flex;
  align-items: center;
  padding: 8px 16px;
  background-color: #222;
  color: white;
  gap: 12px;
}

.time-display {
  font-size: 14px;
  color: #eee;
  min-width: 120px;
}

.progress-container {
  flex: 1;
  height: 8px;
  background-color: #444;
  border-radius: 4px;
  position: relative;
  cursor: pointer;
  overflow: hidden;
}

.buffer-range {
  position: absolute;
  height: 100%;
  background-color: #666;
  border-radius: 4px;
}

.progress-bar {
  position: absolute;
  height: 100%;
  background-color: #1a7aff;
  border-radius: 4px;
  transition: width 0.1s linear;
}

.progress-handle {
  position: absolute;
  width: 12px;
  height: 12px;
  background-color: white;
  border-radius: 50%;
  top: 50%;
  transform: translate(-50%, -50%);
  pointer-events: none;
}

.play-icon {
  filter: drop-shadow(0 0 4px rgba(0, 0, 0, 0.5));
}
</style>
