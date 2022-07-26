<script setup lang="ts">import { ref } from 'vue';
import { Connection } from '../../../packages/conn/src';
import { MessageReactive, useMessage } from 'naive-ui';
import { ConnectionState, ConnectionEvent } from '../../../packages/conn/src/types';
const glog = window['glog'] || console.log;
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
let lastTs = 0;
let bytes = 0;
async function connect() {
  try {
    const readable = await conn.connect(url.value);
    readable.pipeTo(new WritableStream({
      write(chunk) {
        if (lastTs) {
          const ts = Date.now();
          bytes += chunk.byteLength;
          if (ts - lastTs > 1000) {
            glog({ networkInputbps: bytes / (ts - lastTs) });
            bytes = 0;
            lastTs = ts;
          }
        } else {
          lastTs = Date.now();
        }
      }
    }));
  } catch (e) {
    removeMessage();
    console.error(e)
    message.error(e.message);
  }
}

</script>

<template>
  <n-input v-model:value="url" :input-props="{ type: 'url' }" placeholder="URL" />
  <n-button @click="connect">Connect</n-button>
</template>
