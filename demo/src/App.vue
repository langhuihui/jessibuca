<script setup lang="ts">
// This starter template is using Vue 3 <script setup> SFCs
// Check out https://vuejs.org/api/sfc-script-setup.html#script-setup
import Connection from './components/Connection.vue';
import Demuxer from './components/Demuxer.vue';
import Decoder from './components/Decoder.vue'
import Renderer from './components/Renderer.vue'
import UI from './components/UI.vue'
import TimeRangeDemo from './components/TimeRangeDemo.vue'
</script>

<template>
  <n-message-provider>
    <n-tabs type="segment" default-value="time-range-demo">
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
        <Renderer/>
      </n-tab-pane>
      <n-tab-pane name="ui" tab="UI">
        <UI/>
      </n-tab-pane>
      <n-tab-pane name="time-range-demo" tab="Time Range Demo">
        <TimeRangeDemo />
      </n-tab-pane>
    </n-tabs>
  </n-message-provider>
</template>