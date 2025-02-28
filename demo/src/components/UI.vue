<template>
  <n-space vertical>
    <n-card title="Vue Component Usage">
      <HLSPlayerVue
        :src="playerSrc"
        :options="{
          showPlaybackRate: true,
          showProgress: true,
          autoGenerateUI: true,
          playbackRates: [0.5, 0.75, 1, 1.25, 1.5, 2],
          debug: {
            enabled: true,
            showTimeRanges: true,
            showMediaTimeline: true,
          },
        }"
      />
    </n-card>

    <n-card title="Controls">
      <n-input-group>
        <n-input
          v-model:value="hlsUrl"
          placeholder="Enter HLS URL"
          class="url-input"
        />
        <n-button type="primary" @click="loadUrl">Load URL</n-button>
      </n-input-group>
    </n-card>
  </n-space>
</template>

<script lang="ts" setup>
import { ref, onMounted } from "vue";
import { HLSPlayerVue } from "jv4-ui";
import { NSpace, NCard, NInput, NInputGroup, NButton } from "naive-ui";

const hlsUrl = ref(
  "http://localhost:8080/hls/vod/fmp4.m3u8?start=1740967350&streamPath=live/video"
);

const playerSrc = ref("");

async function loadUrl() {
  playerSrc.value = hlsUrl.value;
}

// 组件挂载时自动加载 URL
onMounted(() => {
  loadUrl();
});
</script>

<style scoped>
.n-card {
  max-width: 800px;
  margin: 0 auto;
}

:deep(.n-input-group) {
  width: 100%;
}

:deep(.url-input) {
  flex: 1;
}

:deep(.hls-player) {
  width: 100%;
  aspect-ratio: 16/9;
  background: #000;
}

:deep(.hls-player-video) {
  width: 100%;
  height: 100%;
  object-fit: contain;
}

:deep(.hls-player-controls) {
  position: absolute;
  bottom: 0;
  left: 0;
  right: 0;
  padding: 10px;
  background: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: center;
  gap: 10px;
}

:deep(.hls-player-progress) {
  flex: 1;
  display: flex;
  align-items: center;
  gap: 10px;
}

:deep(.progress-wrapper) {
  flex: 1;
  position: relative;
  height: 20px;
}

:deep(.progress-canvas) {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  z-index: 1;
}

:deep(.progress-input) {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  margin: 0;
  opacity: 0;
  z-index: 2;
  cursor: pointer;
}
</style>
