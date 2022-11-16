<script setup lang="ts">
import { onMounted, onUnmounted, reactive, ref } from "vue";
import {
  FileConnection,
  getURLType,
  HttpConnection,
  WebSocketConnection,
} from "../../../packages/conn/src";
import {
  MessageReactive,
  NButton,
  NCol,
  NIcon,
  NInput,
  NP,
  NRow,
  NSpace,
  NStatistic,
  NText,
  NUpload,
  NUploadDragger,
  UploadCustomRequestOptions,
  UploadFileInfo,
  useMessage,
} from "naive-ui";

import { TimelineDataSeries, TimelineGraphView } from "webrtc-internals";
import { ArchiveOutline as ArchiveIcon } from "@vicons/ionicons5";
import { Connection } from "../../../packages/conn/src/base";
import {
  ConnectionEvent,
  ConnectionState,
} from "../../../packages/conn/src/types";

const message = useMessage();
const url = ref("");
let messageReactive: MessageReactive | null = null;
const removeMessage = () => {
  if (messageReactive) {
    messageReactive.destroy();
    messageReactive = null;
  }
};
let conn: Connection;

async function connect(file?: File) {
  try {
    if (file) conn = new FileConnection(file);
    else
      switch (getURLType(url.value)) {
        case "ws":
          conn = new WebSocketConnection(url.value);
          break;
        case "http":
          conn = new HttpConnection(url.value);
          break;
      }
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
    await conn.connect();
    if (!file) {
      while (conn.oput) {
        if (conn.oput.buffer) await conn.read(conn.oput.buffer!.length);
        else {
          await new Promise((resolve) => {
            setTimeout(resolve, 1000);
          });
        }
      }
    }
  } catch (e) {
    removeMessage();
    console.error(e);
    message.error(e.message);
  }
}
function close() {
  console.log(` close btn clicked`);
  conn?.close();
}
const data = reactive({
  totalDown: 0,
  bpsDown: 0,
});
onMounted(() => {
  const gv = new TimelineGraphView(
    document.getElementById("bps") as HTMLCanvasElement
  );
  const series = new TimelineDataSeries();
  gv.addDataSeries(series);
  let id = setInterval(() => {
    if (conn) {
      data.totalDown = conn.down.total;
      data.bpsDown = conn.down.bps;
      series.addPoint(Date.now(), conn.down.bps);
      gv.updateEndDate();
    }
  }, 1000);
  onUnmounted(() => {
    clearInterval(id);
  });
});
async function onUpload(options: UploadCustomRequestOptions) {
  const file = options.file.file;
  const totalSize = file?.size;
  let read = 0;
  if (file && totalSize) {
    try {
      await connect(file);
      while (conn.oput) {
        read += 100;
        await conn.read(100);
        options.onProgress({ percent: (100 * read) / totalSize });
        await new Promise((resolve) => {
          setTimeout(resolve, 1000);
        });
      }
    } catch (e) {
      options.onFinish();
      removeMessage();
      console.error(e);
      message.error(e.message);
    }
  }
}
function onRemove(options: {
  file: UploadFileInfo;
  fileList: Array<UploadFileInfo>;
}) {
  conn.close();
  return true;
}
</script>

<template>
  <n-upload :custom-request="onUpload" :max="1" @remove="onRemove">
    <n-upload-dragger>
      <div style="margin-bottom: 12px">
        <n-icon size="48" :depth="3">
          <archive-icon />
        </n-icon>
      </div>
      <n-text style="font-size: 16px"> 点击或者拖动文件到该区域来上传 </n-text>
      <n-p depth="3" style="margin: 8px 0 0 0">
        支持上传的文件类型：<n-text>mp4,flv,ts,h264,h265</n-text>
      </n-p>
    </n-upload-dragger>
  </n-upload>
  <n-space justify="end" :wrap-item="false">
    <div style="flex-grow: 1">
      <n-input
        v-model:value="url"
        :input-props="{ type: 'url' }"
        placeholder="URL"
      />
    </div>
    <n-button @click="connect()">Connect</n-button>
    <n-button @click="close">Close</n-button>
  </n-space>
  <n-row>
    <n-col :span="12">
      <n-statistic label="下行总量" :value="data.totalDown"> </n-statistic>
    </n-col>
    <n-col :span="12">
      <n-statistic label="下行bps">
        {{ data.bpsDown }}
      </n-statistic>
    </n-col>
  </n-row>
  <canvas id="bps"></canvas>
</template>
