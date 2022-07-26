<script setup lang="ts">
// This starter template is using Vue 3 <script setup> SFCs
// Check out https://vuejs.org/api/sfc-script-setup.html#script-setup
import Connection from './components/Connection.vue';
</script>

<template>
  <n-message-provider>
    <n-tabs type="segment">
      <n-tab-pane name="connection" tab="Connection">
        <Connection />
      </n-tab-pane>
    </n-tabs>
  </n-message-provider>
</template>