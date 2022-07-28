<script setup lang="ts">
import { onMounted, onUnmounted, reactive, ref, watchEffect } from 'vue';
import { Connection } from '../../../packages/conn/src';
import { MessageReactive, useMessage } from 'naive-ui';
import { ConnectionState, ConnectionEvent } from '../../../packages/conn/src/types';
import { TimelineDataSeries, TimelineGraphView } from 'webrtc-internals';
const message = useMessage();
const url = ref("");
let messageReactive: MessageReactive | null = null;
const removeMessage = () => {
  if (messageReactive) {
    messageReactive.destroy();
    messageReactive = null;
  }
};
const conn = new Connection();
conn.on(ConnectionEvent.Connecting, () => {
  messageReactive = message.loading(ConnectionEvent.Connecting);
});
conn.on(ConnectionEvent.Reconnecting, () => {
  messageReactive = message.loading(ConnectionEvent.Reconnecting);
});
conn.on(ConnectionState.CONNECTED, () => {
  removeMessage();
  message.success(ConnectionState.CONNECTED);
});

conn.on(ConnectionState.DISCONNECTED, () => {
  removeMessage();
  message.error(ConnectionState.DISCONNECTED);
});

conn.on(ConnectionState.RECONNECTED, () => {
  removeMessage();
  message.success(ConnectionState.RECONNECTED);
});
async function connect() {
  try {
    await conn.connect(url.value);
    while (conn.oput) {
      if (conn.oput.buffer)
        await conn.read(conn.oput.buffer!.length);
      else {
        await new Promise((resolve) => {
          setTimeout(resolve, 1000);
        });
      }
    }
  } catch (e) {
    removeMessage();
    console.error(e);
    message.error(e.message);
  }
}
const data = reactive({
  totalDown: 0,
  bpsDown: 0
});
onMounted(() => {
  const gv = new TimelineGraphView(document.getElementById('bps') as HTMLCanvasElement);
  const series = new TimelineDataSeries();
  gv.addDataSeries(series);
  let id = setInterval(() => {
    data.totalDown = conn.down.total;
    data.bpsDown = conn.down.bps
    series.addPoint(Date.now(), conn.down.bps);
    gv.updateEndDate();
  }, 1000);
  onUnmounted(() => {
    clearInterval(id);
  });
});
</script>

<template>
  <n-space justify="end" :wrap-item="false">
    <div style="flex-grow: 1">
      <n-input v-model:value="url" :input-props="{ type: 'url' }" placeholder="URL" />
    </div>
    <n-button @click="connect">Connect</n-button>
  </n-space>
  <n-row>
    <n-col :span="12">
      <n-statistic label="下行总量" :value="data.totalDown">
      </n-statistic>
    </n-col>
    <n-col :span="12">
      <n-statistic label="下行bps">
        {{ data.bpsDown}}
      </n-statistic>
    </n-col>
  </n-row>
  <canvas id="bps"></canvas>
</template>
