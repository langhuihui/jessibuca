<script setup lang="ts">
import { onMounted, onUnmounted, reactive, ref, watchEffect } from "vue";
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
  ConnectionState,
  ConnectionEvent,
} from "jv4-connection/src/types";
import { TimelineDataSeries, TimelineGraphView } from "webrtc-internals";
import { ArchiveOutline as ArchiveIcon } from "@vicons/ionicons5";
import {
  AudioDecoderConfig,
  VideoDecoderEvent,
  AudioDecoderEvent,
  ErrorInfo,
  VideoCodecInfo,
  AudioCodecInfo,
  AudioFrame,
} from "jv4-decoder/src/types";
import {
  FileConnection,
  getURLType,
  WebSocketConnection,
  HttpConnection,
} from "jv4-connection/src";
import { Connection } from "jv4-connection/src/base";
import {
  VideoDecoderHard,
  VideoDecoderSoft,
  VideoDecoderSoftSIMD,
  AudioDecoderSoft,
} from "jv4-decoder/src";
import { FlvDemuxer, DemuxEvent, PSDemuxer } from "jv4-demuxer/src";
import { WebCodecsVideoRenderer } from "jv4-renderer/src";
import { DemuxMode } from "jv4-demuxer/src/base";
import { fileSave, fileOpen } from 'browser-fs-access';
const message = useMessage();
const url = ref("ws://localhost:8080/ps/live/test");
let messageReactive: MessageReactive | null = null;
const removeMessage = () => {
  if (messageReactive) {
    messageReactive.destroy();
    messageReactive = null;
  }
};
const muxType = ref('ps');
const dump = ref(false);
const stopDump = ref(() => { });
let conn: Connection;
const display = reactive({
  audioTS: 0,
  audioSize: 0,
  videoTS: 0,
  videoSize: 0,
  videoDecodedFrameRate: 0,
  videoDecodedFrames: 0,
  audioDecodedFrameRate: 0,
  audioDecodedFrames: 0,
});

let vframs = 0;
let aframs = 0;
watchEffect(() => {
  if (!dump.value) {
    stopDump.value();
    return;
  }
});
function readDelay(t: number): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, t));
}
async function connect(file?: File, options?: UploadCustomRequestOptions) {
  try {
    const cache: Uint8Array[] = [];
    const p = new Promise<Blob>((resolve) => {
      stopDump.value = () => {
        resolve(new Blob(cache));
      };
    });
    const dumpFile = dump.value ? fileSave(p) : null;
    console.log(`connect ${file} url ${url.value}`);
    if (file) {
      conn = new FileConnection(file);
      console.log(file.name, file.type);
      switch (file.type) {
        case "video/mp4":
          muxType.value = 'mp4';
          break;
        case "video/x-flv":
          muxType.value = 'flv';
          break;
      }
    } else {
      if (url.value.endsWith(".flv")) {
        muxType.value = "flv";
      }
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

    // const videoDecoder = new VideoDecoderHard();
    const videoDecoder = new VideoDecoderSoftSIMD();
    await videoDecoder.initialize();

    // const audioDecoder = new AudioDecoderSoft();
    // await audioDecoder.initialize();

    const demuxer = muxType.value == 'flv' ? new FlvDemuxer(conn, DemuxMode.PUSH) : new PSDemuxer(conn, DemuxMode.PUSH);
    const renderer = new WebCodecsVideoRenderer(
      document.getElementById("video") as HTMLVideoElement
    );
    demuxer.on(DemuxEvent.AUDIO_ENCODER_CONFIG_CHANGED, (data: Uint8Array) => {
      message.info(DemuxEvent.AUDIO_ENCODER_CONFIG_CHANGED);
      // const aconfig: AudioDecoderConfig = {
      //   codec: demuxer.audioEncoderConfig!.codec,
      //   extraData: data,
      //   outSampleType: "f32-planar",
      // };
      // audioDecoder.configure(aconfig);
    });
    demuxer.on(DemuxEvent.VIDEO_ENCODER_CONFIG_CHANGED, (extraData: Uint8Array) => {
      message.info(DemuxEvent.VIDEO_ENCODER_CONFIG_CHANGED);
      switch (demuxer.videoEncoderConfig!.codec) {
        case "avc":
          videoDecoder.configure({
            codec: "avc1.420028",
            extraData,
            videoType: "avc",
            avc: { format: extraData ? "avcc" : "annexb" },
          });
          break;
        case "hevc":
          videoDecoder.configure({
            codec: "hvc1.1.6.L0.12.34.56.78.9A.BC",
            extraData,
            videoType: "hevc",
            hevc: { format: extraData ? "hvcc" : "annexb" },
          });
      }
    });
    demuxer.gotAudio = (data: EncodedAudioChunkInit) => {
      // display.audioTS = data.timestamp;
      // display.audioSize = data.data.byteLength;
      // audioDecoder.decode(data);
      aframs++;
    };
    demuxer.gotVideo = (data: EncodedVideoChunkInit) => {
      display.videoTS = data.timestamp;
      display.videoSize = data.data.byteLength;
      if (videoDecoder.config) {
        try {
          if (dumpFile) {
            cache.push(data.data as Uint8Array);
          }
          videoDecoder.decode(data);
        } catch (err) {
          console.error(err);
        }
      }

    };
    // demuxer.audioReadable.pipeTo(
    //   new WritableStream({
    //     write(chunk: EncodedAudioChunkInit) {
    //       display.audioTS = chunk.timestamp;
    //       display.audioSize = chunk.data.byteLength;

    //       audioDecoder.decode(chunk);
    //       if (file && options)
    //         options.onProgress({
    //           percent: (100 * conn.down.total) / file.size,
    //         });
    //       return readDelay(0);
    //     },
    //   })
    // );
    // demuxer.videoReadable.pipeTo(
    //   new WritableStream({
    //     write(chunk: EncodedVideoChunkInit) {
    //       display.videoTS = chunk.timestamp;
    //       display.videoSize = chunk.data.byteLength;

    //       // console.log(`JS Chunk ${chunk.data.byteLength}  ${chunk.data[5]} ${chunk.data[6]} ${chunk.data[7]} ${chunk.data[8]} ${chunk.data[9]}`)

    //       videoDecoder.decode(chunk);

    //       if (file && options)
    //         options.onProgress({
    //           percent: (100 * conn.down.total) / file.size,
    //         });

    //       return readDelay(0);
    //     },
    //   })
    // );

    videoDecoder.on(
      VideoDecoderEvent.VideoCodecInfo,
      (codecinfo: VideoCodecInfo) => {
        message.info(`width: ${codecinfo.width} height: ${codecinfo.height}`);
      }
    );
    videoDecoder.on(VideoDecoderEvent.VideoFrame, (videoFrame: VideoFrame) => {
      display.videoDecodedFrames++;
      vframs++;
      renderer.writeVideo(videoFrame);
    });
    // videoDecoder.on(VideoDecoderEvent.VideoFrame, (videoFrame: JVideoFrame) => {
    //   display.videoDecodedFrames++;
    //   vframs++;
    //   renderer.writeVideo(videoFrame.data, {
    //     codedWidth: videoFrame.width,
    //     codedHeight: videoFrame.height,
    //     format: videoFrame.pixelType,
    //     timestamp: videoFrame.pts,
    //   });
    // });

    videoDecoder.on(VideoDecoderEvent.Error, (error: Error) => {
      console.error(error);
    });

    // audioDecoder.on(
    //   AudioDecoderEvent.AudioCodecInfo,
    //   (codecinfo: AudioCodecInfo) => { }
    // );

    // audioDecoder.on(AudioDecoderEvent.AudioFrame, (audioFrame: AudioFrame) => {
    //   display.audioDecodedFrames++;
    //   // renderer.writeAudio(new AudioData({
    //   //   data: audioFrame.data[0],
    //   // }));
    // });

    // audioDecoder.on(AudioDecoderEvent.Error, (error: ErrorInfo) => { });
    await conn.connect();
  } catch (e) {
    if (options) options.onFinish();
    removeMessage();
    console.error(e);
    message.error(e.message);
  }
}
function disconnect() {
  message.info(`disconnetion`);
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
  let lastsec = new Date().getTime();
  let id = setInterval(() => {
    if (!conn) return;
    data.totalDown = conn.down.total;
    data.bpsDown = conn.down.bps;

    let now = new Date().getTime();

    display.videoDecodedFrameRate = Math.floor(
      (vframs * 1000) / (now - lastsec)
    );
    vframs = 0;
    display.audioDecodedFrameRate = Math.floor(
      (aframs * 1000) / (now - lastsec)
    );
    aframs = 0;

    lastsec = now;

    series.addPoint(Date.now(), display.videoDecodedFrameRate);
    gv.updateEndDate();
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
  <n-select v-model:value="muxType" :options='[{ label: "flv", value: "flv" }, { label: "ps", value: "ps" }]'></n-select>
  <n-switch v-model:value="dump">dump</n-switch>
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
    <n-button @click="disconnect">Close</n-button>
  </n-space>
  <video id="video"></video>
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
  <n-row>
    <n-col :span="6">
      <n-statistic label="视频解码帧率" :value="display.videoDecodedFrameRate"></n-statistic>
    </n-col>
    <n-col :span="6">
      <n-statistic label="视频解码帧数" :value="display.videoDecodedFrames"></n-statistic>
    </n-col>
    <n-col :span="6">
      <n-statistic label="音频解码帧率" :value="display.audioDecodedFrameRate"></n-statistic>
    </n-col>
    <n-col :span="6">
      <n-statistic label="音频解码帧数" :value="display.audioDecodedFrames"></n-statistic>
    </n-col>
  </n-row>
  <canvas id="bps"></canvas>
</template>
