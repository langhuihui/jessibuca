<script setup lang="ts">
import { onMounted, onUnmounted, reactive, ref, watchEffect } from "vue";
import {
  FileConnection,
  getURLType,
  HttpConnection,
  WebSocketConnection,
} from "jv4-connection/src";
import { DemuxEvent, FlvDemuxer } from "jv4-demuxer/src";
import { TimelineDataSeries, TimelineGraphView } from "webrtc-internals";
import { ArchiveOutline as ArchiveIcon } from "@vicons/ionicons5";
import { Connection } from "jv4-connection/src/base";
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
import {
  ConnectionEvent,
  ConnectionState,
} from "jv4-connection/src/types";

const message = useMessage();
const url = ref("");
let messageReactive: MessageReactive | null = null;
const removeMessage = () => {
  if (messageReactive) {
    messageReactive.destroy();
    messageReactive = null;
  }
};

const display = reactive({
  audioTS: 0,
  audioSize: 0,
  videoTS: 0,
  videoSize: 0,
});
function readDelay(t: number): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, t));
}
let conn: Connection;
async function connect(file?: File, options?: UploadCustomRequestOptions) {
  try {
    console.log(`connect ${file} url ${url.value}`);
    if (file) {
      conn = new FileConnection(file);
    } else {
      switch (getURLType(url.value)) {
        case "ws":
          conn = new WebSocketConnection(url.value);
          break;
        case "http":
          conn = new HttpConnection(url.value);
          break;
      }
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
    const demuxer = new FlvDemuxer(conn);
    demuxer.on(DemuxEvent.AUDIO_ENCODER_CONFIG_CHANGED, (data: Uint8Array) => {
      message.info(DemuxEvent.AUDIO_ENCODER_CONFIG_CHANGED);
    });
    demuxer.on(DemuxEvent.VIDEO_ENCODER_CONFIG_CHANGED, (data: Uint8Array) => {
      message.info(DemuxEvent.VIDEO_ENCODER_CONFIG_CHANGED);
    });
    demuxer.audioReadable.pipeTo(
      new WritableStream({
        write(chunk: EncodedAudioChunkInit) {
          display.audioTS = chunk.timestamp;
          display.audioSize = chunk.data.byteLength;
          if (file && options)
            options.onProgress({
              percent: (100 * conn.down.total) / file.size,
            });
          return readDelay(20);
        },
      })
    );
    demuxer.videoReadable.pipeTo(
      new WritableStream({
        write(chunk: EncodedVideoChunkInit) {
          display.videoTS = chunk.timestamp;
          display.videoSize = chunk.data.byteLength;
          if (file && options)
            options.onProgress({
              percent: (100 * conn.down.total) / file.size,
            });
          return readDelay(40);
        },
      })
    );
  } catch (e) {
    if (options) options.onFinish();
    removeMessage();
    console.error(e);
    message.error(e.message);
  }
}
function close() {
  conn.close();
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
    if (!conn) return;
    data.totalDown = conn.down.total;
    data.bpsDown = conn.down.bps;
    series.addPoint(Date.now(), conn.down.bps);
    gv.updateEndDate();
  }, 1000);
  onUnmounted(() => {
    clearInterval(id);
  });
});
async function onUpload(options: UploadCustomRequestOptions) {
  const file = options.file.file;
  const totalSize = file?.size;
  if (file && totalSize) {
    connect(file, options);
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
      <n-input v-model:value="url" :input-props="{ type: 'url' }" placeholder="URL" />
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
  <n-row>
    <n-col :span="6">
      <n-statistic label="音频时间戳" :value="display.audioTS"></n-statistic>
    </n-col>
    <n-col :span="6">
      <n-statistic label="音频tag大小" :value="display.audioSize"></n-statistic>
    </n-col>
    <n-col :span="6">
      <n-statistic label="视频时间戳" :value="display.videoTS"></n-statistic>
    </n-col>
    <n-col :span="6">
      <n-statistic label="视频tag大小" :value="display.videoSize"></n-statistic>
    </n-col>
  </n-row>
  <canvas id="bps"></canvas>
</template>
