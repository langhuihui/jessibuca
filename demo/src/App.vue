<script setup lang="ts">
// This starter template is using Vue 3 <script setup> SFCs
// Check out https://vuejs.org/api/sfc-script-setup.html#script-setup
import Connection from "./components/Connection.vue";
import Demuxer from "./components/Demuxer.vue";
import Decoder from "./components/Decoder.vue";
import Renderer from "./components/Renderer.vue";
import UI from "./components/UI.vue";
// 导入 HLS Web Component（自动注册为 hls-video 自定义元素）
import "jv4-hls";
import { ref } from "vue";
const m3u8 = ref("https://test-streams.mux.dev/x36xhzz/x36xhzz.m3u8");
</script>

<template>
  <n-message-provider>
    <n-tabs type="segment" default-value="hls">
      <n-tab-pane name="connection" tab="Connection">
        <Connection />
      </n-tab-pane>
      <n-tab-pane name="demuxer" tab="Demuxer">
        <Demuxer />
      </n-tab-pane>
      <n-tab-pane name="decoder" tab="Decoder">
        <Decoder />
      </n-tab-pane>
      <n-tab-pane name="renderer" tab="Renderer">
        <Renderer />
      </n-tab-pane>
      <n-tab-pane name="ui" tab="UI">
        <UI />
      </n-tab-pane>
      <n-tab-pane name="hls" tab="HLS">
        <jv4-hls :src="m3u8" controls width="800" debug></jv4-hls>
        <n-input v-model:value="m3u8"></n-input>
      </n-tab-pane>
    </n-tabs>
  </n-message-provider>
</template>
