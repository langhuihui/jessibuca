<template>
  <n-space vertical>
    <n-card title="Vue Component Usage">
      <HLSPlayerVue
        :src="playerSrc"
        :options="{
          showPlaybackRate: true,
          playbackRates: [0.5, 0.75, 1, 1.25, 1.5, 2],
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
import { ref } from "vue";
import { HLSPlayerVue } from "jv4-ui";
import { NSpace, NCard, NInput, NInputGroup, NButton } from "naive-ui";

const hlsUrl = ref(
  "http://localhost:8080/hls/vod/fmp4.m3u8?start=1740539654&streamPath=live/1581F6Q8X24BU00G01HK"
);

const playerSrc = ref("");

async function loadUrl() {
  // 实现 URL 加载逻辑
  playerSrc.value = hlsUrl.value;
}
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
</style>
