<template>
  <div class="segment-visualizer">
    <div class="segment-timeline">
      <!-- 时间线刻度 -->
      <div class="timeline-ticks">
        <div
          v-for="tick in timelineTicks"
          :key="tick.time"
          class="timeline-tick"
          :style="{ left: `${(tick.time / duration) * 100}%` }"
        >
          <div class="tick-label">{{ tick.label }}</div>
        </div>
      </div>

      <!-- 缓冲区范围显示 -->
      <div
        v-for="(range, index) in bufferRanges"
        :key="`buffer-${index}`"
        class="buffer-range"
        :style="{
          left: `${(range.start / duration) * 100}%`,
          width: `${((range.end - range.start) / duration) * 100}%`,
        }"
        :title="`已缓冲: ${formatTime(range.start)} - ${formatTime(range.end)}`"
      ></div>

      <!-- 播放区域背景 -->
      <div class="playable-area" :style="{
        width: `${(currentTime / duration) * 100}%`
      }"></div>

      <!-- 片段显示 -->
      <div
        v-for="segment in normalizedSegments"
        :key="`segment-${segment.index}`"
        class="segment"
        :class="{
          'is-current': segment.index === currentSegmentIndex,
          'is-loaded': segment.isLoaded,
          'is-loading': segment.isLoading,
          'is-buffered': segment.isBuffered,
        }"
        :style="{
          left: `${(segment.virtualStartTime / duration) * 100}%`,
          width: `${
            ((segment.virtualEndTime - segment.virtualStartTime) / duration) *
            100
          }%`,
        }"
        :title="`片段 #${segment.index}: ${formatTime(segment.virtualStartTime)}-${formatTime(segment.virtualEndTime)} (${formatTime(segment.duration)})`"
        @click="onSegmentClick(segment)"
      >
        <div class="segment-info">
          <div class="segment-index">#{{ segment.index }}</div>
          <div class="segment-duration">{{ formatTime(segment.duration) }}</div>
          <div class="segment-status" :class="`status-${getSegmentStatusClass(segment)}`">
            {{ getSegmentStatusText(segment) }}
          </div>
        </div>
      </div>

      <!-- 当前时间指示器 -->
      <div
        class="current-time-indicator"
        :style="{ left: `${(currentTime / duration) * 100}%` }"
        :title="`当前时间: ${formatTime(currentTime)}`"
      >
        <div class="time-indicator-label">{{ formatTime(currentTime) }}</div>
      </div>
    </div>

    <div class="timeline-info">
      <div class="timeline-legend">
        <div class="legend-item">
          <div class="legend-color buffered"></div>
          <span>已缓冲</span>
        </div>
        <div class="legend-item">
          <div class="legend-color loaded"></div>
          <span>已加载</span>
        </div>
        <div class="legend-item">
          <div class="legend-color loading"></div>
          <span>加载中</span>
        </div>
        <div class="legend-item">
          <div class="legend-color unloaded"></div>
          <span>未加载</span>
        </div>
      </div>
      
      <div class="timeline-summary">
        <span>总时长: {{ formatTime(duration) }}</span>
        <span>已加载: {{ loadedSegmentsCount }} / {{ segments.length }}</span>
      </div>
    </div>

    <div class="segment-controls">
      <n-space>
        <n-button size="small" @click="loadAllSegments">加载所有片段</n-button>
        <n-button size="small" @click="clearAllSegments">清除所有片段</n-button>
      </n-space>
    </div>

    <!-- 当前选中片段信息 -->
    <div class="segment-details" v-if="selectedSegment">
      <n-card title="片段详情" size="small">
        <n-descriptions bordered size="small">
          <n-descriptions-item label="索引">{{
            selectedSegment.index
          }}</n-descriptions-item>
          <n-descriptions-item label="URL">{{
            selectedSegment.url
          }}</n-descriptions-item>
          <n-descriptions-item label="时长">{{
            formatTime(selectedSegment.duration)
          }}</n-descriptions-item>
          <n-descriptions-item label="虚拟开始时间">{{
            formatTime(selectedSegment.virtualStartTime)
          }}</n-descriptions-item>
          <n-descriptions-item label="虚拟结束时间">{{
            formatTime(selectedSegment.virtualEndTime)
          }}</n-descriptions-item>
          <n-descriptions-item label="物理开始时间">{{
            formatTime(selectedSegment.physicalStartTime)
          }}</n-descriptions-item>
          <n-descriptions-item label="物理结束时间">{{
            formatTime(selectedSegment.physicalEndTime)
          }}</n-descriptions-item>
          <n-descriptions-item label="状态">
            <n-tag :type="selectedSegment.isLoaded ? 'success' : 'warning'">
              {{ selectedSegment.isLoaded ? "已加载" : "未加载" }}
            </n-tag>
            <n-tag
              :type="selectedSegment.isBuffered ? 'success' : 'warning'"
              style="margin-left: 8px"
            >
              {{ selectedSegment.isBuffered ? "已缓冲" : "未缓冲" }}
            </n-tag>
          </n-descriptions-item>
        </n-descriptions>
      </n-card>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, defineProps, watch, onMounted } from "vue";
import {
  NButton,
  NSpace,
  NCard,
  NDescriptions,
  NDescriptionsItem,
  NTag,
} from "naive-ui";
import {
  MediaSegment,
  BufferRange,
} from "jv4-demuxer/src/VirtualTimeline";

// 属性定义
const props = defineProps<{
  segments: MediaSegment[];
  currentSegmentIndex: number;
  currentTime: number;
  duration: number;
  bufferRanges: BufferRange[];
}>();

// 事件定义
const emit = defineEmits<{
  (event: "loadSegment", index: number): void;
  (event: "loadAllSegments"): void;
  (event: "clearAllSegments"): void;
  (event: "seek", time: number): void;
}>();

// 状态
const selectedSegment = ref<MediaSegment | null>(null);

// 时间线刻度
const timelineTicks = computed(() => {
  if (!props.duration) return [];

  const ticks = [];
  const interval = props.duration > 60 ? 10 : 5; // 根据总时长调整刻度间隔

  for (let time = 0; time <= props.duration; time += interval) {
    ticks.push({
      time,
      label: formatTime(time),
    });
  }

  return ticks;
});

// 计算加载的片段数量
const loadedSegmentsCount = computed(() => {
  return props.segments.filter(segment => segment.isLoaded || segment.isBuffered).length;
});

// 添加辅助计算属性来确保片段的虚拟时间设置正确
const normalizedSegments = computed(() => {
  if (!props.segments.length) return [];
  
  // 检查片段是否有有效的虚拟时间，如果没有则计算
  let currentVirtualTime = 0;
  
  return props.segments.map(segment => {
    // 如果片段没有有效的虚拟开始/结束时间，计算它们
    const virtualStartTime = segment.virtualStartTime || currentVirtualTime;
    const duration = segment.duration || 0;
    const virtualEndTime = segment.virtualEndTime || (virtualStartTime + duration);
    
    // 更新累计时间
    currentVirtualTime = virtualEndTime;
    
    return {
      ...segment,
      virtualStartTime,
      virtualEndTime
    };
  });
});

// 监视currentTime的变化
watch(() => props.currentTime, (newTime, oldTime) => {
  console.log(`[SegmentVisualizer] currentTime变化: ${oldTime} -> ${newTime}`);
}, { immediate: true });

// 监视是否有segments数据
watch(() => props.segments, (newSegments) => {
  console.log(`[SegmentVisualizer] segments更新，长度: ${newSegments?.length || 0}`);
}, { immediate: true });

onMounted(() => {
  console.log('[SegmentVisualizer] 组件挂载完成，初始currentTime:', props.currentTime);
  console.log('[SegmentVisualizer] 初始segments:', props.segments);
});

// 方法
const onSegmentClick = (segment: MediaSegment) => {
  selectedSegment.value = segment;

  // 如果未加载，触发加载事件
  if (!segment.isLoaded) {
    emit("loadSegment", segment.index);
  }
};

const loadAllSegments = () => {
  emit("loadAllSegments");
};

const clearAllSegments = () => {
  emit("clearAllSegments");
  selectedSegment.value = null;
};

// 获取片段状态文本
const getSegmentStatusText = (segment: MediaSegment): string => {
  if (segment.isBuffered) return '已缓冲';
  if (segment.isLoaded) return '已加载';
  if (segment.isLoading) return '加载中';
  return '未加载';
};

// 获取片段状态的CSS类名
const getSegmentStatusClass = (segment: MediaSegment): string => {
  if (segment.isBuffered) return 'buffered';
  if (segment.isLoaded) return 'loaded';
  if (segment.isLoading) return 'loading';
  return 'unloaded';
};

// 格式化时间显示
const formatTime = (seconds: number): string => {
  const minutes = Math.floor(seconds / 60);
  const secs = Math.floor(seconds % 60);
  return `${minutes}:${secs.toString().padStart(2, "0")}`;
};
</script>

<style scoped>
.segment-visualizer {
  display: flex;
  flex-direction: column;
  gap: 16px;
  width: 100%;
  max-width: 800px;
}

.segment-timeline {
  position: relative;
  height: 60px;
  background-color: #333;
  border-radius: 4px;
  overflow: hidden;
}

.timeline-ticks {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  pointer-events: none;
}

.timeline-tick {
  position: absolute;
  top: 0;
  height: 100%;
  width: 1px;
  background-color: rgba(255, 255, 255, 0.3);
}

.tick-label {
  position: absolute;
  bottom: 2px;
  left: 2px;
  font-size: 10px;
  color: #ccc;
  white-space: nowrap;
}

.buffer-range {
  position: absolute;
  height: 8px;
  bottom: 0;
  background-color: rgba(0, 255, 0, 0.3);
  border-radius: 2px;
}

.segment {
  position: absolute;
  top: 8px;
  height: calc(100% - 16px);
  border-radius: 3px;
  background-color: #555;
  border: 1px solid #777;
  cursor: pointer;
  transition: all 0.2s ease;
  overflow: hidden;
}

.segment:hover {
  transform: translateY(-2px);
  box-shadow: 0 2px 6px rgba(0, 0, 0, 0.3);
}

.segment.is-current {
  border-color: #1a7aff;
  background-color: rgba(26, 122, 255, 0.7);
}

.segment.is-loading {
  background: repeating-linear-gradient(
    45deg,
    #555,
    #555 10px,
    #666 10px,
    #666 20px
  );
  animation: loading 1s linear infinite;
}

.segment.is-loaded {
  background-color: rgba(64, 158, 255, 0.5);
  border: 2px solid #409eff;
  z-index: 2;
}

.segment.is-buffered {
  background-color: rgba(103, 194, 58, 0.5);
  border: 2px solid #67c23a;
  z-index: 3;
}

.segment-info {
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  height: 100%;
  padding: 0 4px;
  color: white;
  font-size: 10px;
  text-shadow: 0 1px 2px rgba(0, 0, 0, 0.8);
}

.segment-index {
  font-weight: bold;
}

.segment-duration {
  font-size: 9px;
  opacity: 0.8;
}

.segment-status {
  font-size: 8px;
  opacity: 0.8;
}

.current-time-indicator {
  position: absolute;
  top: 0;
  width: 2px;
  height: 100%;
  background-color: red;
  z-index: 10;
  pointer-events: none;
}

.segment-controls {
  display: flex;
  justify-content: flex-end;
}

.segment-details {
  margin-top: 8px;
}

@keyframes loading {
  0% {
    background-position: 0 0;
  }
  100% {
    background-position: 50px 50px;
  }
}

.playable-area {
  position: absolute;
  height: 100%;
  top: 0;
  left: 0;
  background-color: rgba(255, 255, 255, 0.05);
  pointer-events: none;
  z-index: 1;
}

.time-indicator-label {
  position: absolute;
  top: -20px;
  transform: translateX(-50%);
  background-color: rgba(255, 0, 0, 0.8);
  color: white;
  padding: 2px 4px;
  border-radius: 2px;
  font-size: 10px;
  white-space: nowrap;
}

.segment-status.status-buffered {
  color: #67c23a;
  font-weight: bold;
}

.segment-status.status-loaded {
  color: #409eff;
  font-weight: bold;
}

.segment-status.status-loading {
  color: #e6a23c;
  animation: blink 1s infinite;
}

.segment-status.status-unloaded {
  color: #f56c6c;
}

.timeline-info {
  display: flex;
  justify-content: space-between;
  align-items: center;
  font-size: 12px;
  margin-top: 4px;
}

.timeline-legend {
  display: flex;
  gap: 10px;
}

.legend-item {
  display: flex;
  align-items: center;
  gap: 4px;
}

.legend-color {
  width: 12px;
  height: 12px;
  border-radius: 2px;
}

.legend-color.buffered {
  background-color: rgba(103, 194, 58, 0.5);
  border: 1px solid #67c23a;
}

.legend-color.loaded {
  background-color: rgba(64, 158, 255, 0.5);
  border: 1px solid #409eff;
}

.legend-color.loading {
  background: repeating-linear-gradient(
    45deg,
    #555,
    #555 5px,
    #666 5px,
    #666 10px
  );
}

.legend-color.unloaded {
  background-color: #555;
  border: 1px solid #777;
}

.timeline-summary {
  display: flex;
  gap: 10px;
}

@keyframes blink {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.5; }
}
</style>
