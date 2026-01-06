<template>
  <n-space vertical>
    <n-card title="Jessibuca Web Component (Lit Framework)">
      <template #header-extra>
        <n-tag type="success">Lit WebComponent</n-tag>
      </template>

      <n-space vertical>
        <n-alert type="info" title="使用说明">
          这是一个基于 Lit 框架创建的 Web Component，提供了完整的 HLS 播放功能。
          可以在任何 HTML 页面中使用，无需依赖 Vue 或其他框架。
        </n-alert>

        <!-- Jessibuca Player Web Component -->
        <jessibuca-player
          :src="playerSrc"
          show-playback-rate
          show-progress
          auto-generate-ui
          :debug="debugMode"
          :playback-rates="playbackRates"
          @play="handlePlay"
          @pause="handlePause"
          @timeupdate="handleTimeUpdate"
          @error="handleError"
        ></jessibuca-player>

        <n-card title="控制面板" size="small">
          <n-space vertical>
            <n-input-group>
              <n-input
                v-model:value="hlsUrl"
                placeholder="输入 HLS URL"
                class="url-input"
              />
              <n-button type="primary" @click="loadUrl">加载 URL</n-button>
            </n-input-group>

            <n-space>
              <n-switch v-model:value="debugMode">
                <template #checked>调试模式: 开</template>
                <template #unchecked>调试模式: 关</template>
              </n-switch>
            </n-space>

            <n-divider />

            <n-descriptions :column="2" size="small" bordered>
              <n-descriptions-item label="播放状态">
                {{ playbackState }}
              </n-descriptions-item>
              <n-descriptions-item label="当前时间">
                {{ formatTime(currentTime) }}
              </n-descriptions-item>
              <n-descriptions-item label="总时长">
                {{ formatTime(duration) }}
              </n-descriptions-item>
              <n-descriptions-item label="播放进度">
                {{ progress }}%
              </n-descriptions-item>
            </n-descriptions>
          </n-space>
        </n-card>

        <n-card title="代码示例" size="small">
          <n-code
            language="html"
            :code="codeExample"
            word-wrap
          />
        </n-card>

        <n-card title="事件日志" size="small">
          <n-log
            :log="eventLog"
            :rows="10"
            language="log"
          />
        </n-card>
      </n-space>
    </n-card>
  </n-space>
</template>

<script lang="ts" setup>
import { ref, computed, onMounted } from "vue";
import {
  NSpace,
  NCard,
  NInput,
  NInputGroup,
  NButton,
  NSwitch,
  NDescriptions,
  NDescriptionsItem,
  NCode,
  NLog,
  NAlert,
  NTag,
  NDivider,
} from "naive-ui";
import "jv4-ui"; // Import to register the web component

const hlsUrl = ref(
  "http://localhost:8080/hls/vod/fmp4.m3u8?start=1740967350&streamPath=live/video"
);

const playerSrc = ref("");
const debugMode = ref(true);
const playbackRates = ref([0.5, 0.75, 1, 1.25, 1.5, 2]);
const playbackState = ref("未播放");
const currentTime = ref(0);
const duration = ref(0);
const eventLog = ref("事件日志:\n");

const progress = computed(() => {
  if (duration.value === 0) return 0;
  return ((currentTime.value / duration.value) * 100).toFixed(2);
});

const codeExample = computed(() => {
  return `<!-- 基本使用 -->
<jessibuca-player
  src="${hlsUrl.value}"
  show-playback-rate
  show-progress
  auto-generate-ui
></jessibuca-player>

<!-- 带调试模式 -->
<jessibuca-player
  src="${hlsUrl.value}"
  show-playback-rate
  show-progress
  auto-generate-ui
  debug
  show-time-ranges
  show-media-timeline
></jessibuca-player>

<!-- 自定义播放速率 -->
<jessibuca-player
  src="${hlsUrl.value}"
  show-playback-rate
  :playback-rates="[0.5, 1, 1.5, 2, 3]"
></jessibuca-player>

<!-- 在原生 JavaScript 中使用 -->
<script type="module">
  import 'jv4-ui';
  
  const player = document.querySelector('jessibuca-player');
  player.src = '${hlsUrl.value}';
  
  // 监听事件
  player.addEventListener('play', () => console.log('播放开始'));
  player.addEventListener('pause', () => console.log('播放暂停'));
  player.addEventListener('timeupdate', (e) => {
    console.log('时间更新:', e.detail);
  });
  
  // 调用方法
  player.play();
  player.pause();
  player.seek(30);
</script>`;
});

function loadUrl() {
  playerSrc.value = hlsUrl.value;
  addLog(`加载 URL: ${hlsUrl.value}`);
}

function formatTime(seconds: number): string {
  if (isNaN(seconds) || !isFinite(seconds) || seconds < 0) {
    return "00:00";
  }

  const hours = Math.floor(seconds / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  const secs = Math.floor(seconds % 60);

  if (hours > 0) {
    return `${hours.toString().padStart(2, "0")}:${minutes
      .toString()
      .padStart(2, "0")}:${secs.toString().padStart(2, "0")}`;
  }
  return `${minutes.toString().padStart(2, "0")}:${secs
    .toString()
    .padStart(2, "0")}`;
}

function handlePlay() {
  playbackState.value = "播放中";
  addLog("事件: play - 播放开始");
}

function handlePause() {
  playbackState.value = "已暂停";
  addLog("事件: pause - 播放暂停");
}

function handleTimeUpdate(e: CustomEvent) {
  const detail = e.detail;
  currentTime.value = detail.currentTime || 0;
  duration.value = detail.duration || 0;
}

function handleError(e: CustomEvent) {
  const error = e.detail.error;
  addLog(`事件: error - ${error?.message || error}`);
}

function addLog(message: string) {
  const timestamp = new Date().toLocaleTimeString();
  eventLog.value += `[${timestamp}] ${message}\n`;
}

// 组件挂载时自动加载 URL
onMounted(() => {
  loadUrl();
  addLog("组件已挂载");
});
</script>

<style scoped>
:deep(.n-input-group) {
  width: 100%;
}

:deep(.url-input) {
  flex: 1;
}

jessibuca-player {
  width: 100%;
  max-width: 800px;
  margin: 0 auto;
  display: block;
}
</style>
