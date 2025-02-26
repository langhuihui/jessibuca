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
import { ConnectionState, ConnectionEvent } from "jv4-connection/src/types";
import { TimelineDataSeries, TimelineGraphView } from "webrtc-internals";
import { ArchiveOutline as ArchiveIcon } from "@vicons/ionicons5";
import {
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
  AudioDecoderHard,
  VideoDecoderMSE,
} from "jv4-decoder/src";
import { FlvDemuxer, DemuxEvent, PSDemuxer } from "jv4-demuxer/src";
import {
  WebCodecsVideoRenderer,
  YUVCanvasRenderer,
  CanvasRenderer,
} from "jv4-renderer/src";
import { DemuxMode } from "jv4-demuxer/src/base";
import { fileSave, fileOpen } from "browser-fs-access";

type RendererType = "webcodecs" | "yuv" | "canvas";
type DecoderType = "soft" | "simd" | "webcodecs" | "mse";

const message = useMessage();
// const url = ref("ws://localhost:8080/flv/vod/test");
const url = ref("http://giroro.tpddns.cn:8889/001.m3u8");
let messageReactive: MessageReactive | null = null;
const removeMessage = () => {
  if (messageReactive) {
    messageReactive.destroy();
    messageReactive = null;
  }
};
const muxType = ref<"flv" | "ps" | "hls">("flv");
const decoderv = ref<DecoderType>("simd");
const rendererType = ref<RendererType>("canvas");
const dump = ref(false);
const mode = ref(DemuxMode.PULL);
const stopDump = ref(() => {});
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

const vframs = ref(0);
const aframs = ref(0);
let playTimeout: NodeJS.Timeout | null = null;

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
    // 重置帧计数
    vframs.value = 0;
    aframs.value = 0;
    
    // 设置5秒超时
    if (playTimeout) {
      clearTimeout(playTimeout);
    }
    playTimeout = setTimeout(() => {
      if (vframs.value === 0) {
        message.error("播放失败：5秒内未渲染出任何视频帧");
        disconnect();
      }
    }, 5000);

    const cache: Uint8Array[] = [];
    const p = new Promise<Blob>((resolve) => {
      stopDump.value = () => {
        resolve(new Blob(cache));
      };
    });
    const dumpFile = dump.value ? fileSave(p) : null;
    console.log(`connect ${file} url ${url.value}`);

    // Initialize video/canvas element before connection
    if (rendererType.value === "webcodecs" && videoElement.value) {
      const video = videoElement.value;
      video.style.width = "100%";
      video.style.height = "100%";
      video.autoplay = true;
      video.muted = true;
      video.playsInline = true;
      // Ensure video is ready to play
      await video.play().catch(console.error);
    } else if (
      (rendererType.value === "yuv" || rendererType.value === "canvas") &&
      canvasElement.value
    ) {
      const canvas = canvasElement.value;
      canvas.style.width = "100%";
      canvas.style.height = "100%";
      // Set canvas size to match container
      const container = canvas.parentElement;
      if (container) {
        canvas.width = container.clientWidth;
        canvas.height = container.clientHeight;
      }
    }

    if (file) {
      mode.value = DemuxMode.PULL;
      conn = new FileConnection(file);
      console.log(file.name, file.type);
      switch (file.type) {
        case "video/mp4":
          muxType.value = "flv";
          break;
        case "video/x-flv":
          muxType.value = "flv";
          break;
        case "application/vnd.apple.mpegurl":
        case "application/x-mpegURL":
          muxType.value = "hls";
          break;
      }
    } else {
      if (url.value.endsWith(".flv")) {
        muxType.value = "flv";
      } else if (url.value.endsWith(".m3u8")) {
        muxType.value = "hls";
        mode.value = DemuxMode.PULL;
      } else if (url.value.endsWith(".ps")) {
        muxType.value = "ps";
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

    const { videoDecoder, audioDecoder, renderer } = await initializeDecoder();

    if (mode.value === DemuxMode.PULL) {
      await conn.connect();
    }
    const demuxer =
      muxType.value === "flv"
        ? new FlvDemuxer(conn, mode.value)
        : muxType.value === "hls"
        ? null //new HLSDemuxer(conn, mode.value)
        : new PSDemuxer(conn, mode.value);
    if (!demuxer) {
      throw new Error("Demuxer not supported");
    }

    demuxer.on(
      DemuxEvent.VIDEO_ENCODER_CONFIG_CHANGED,
      (vconfig: VideoDecoderConfig) => {
        console.log('[Renderer] Video encoder config changed:', {
          codec: vconfig.codec,
          width: vconfig.codedWidth,
          height: vconfig.codedHeight,
          description: vconfig.description ? 
            Array.from(vconfig.description)
              .map(b => b.toString(16).padStart(2, '0'))
              .join(' ') 
            : undefined
        });
        videoDecoder.configure(vconfig);
      }
    );
    demuxer.on(
      DemuxEvent.AUDIO_ENCODER_CONFIG_CHANGED,
      (aconfig: AudioDecoderConfig) => {
        audioDecoder.configure(aconfig);
      }
    );
    if (decoderv.value == "mse") {
      // @ts-ignore - MediaStreamTrackGenerator is experimental
      const audioTrack = new MediaStreamTrackGenerator({ kind: "audio" });
      const audioWriter = audioTrack.writable.getWriter();
      audioDecoder.on(AudioDecoderEvent.AudioFrame, (audioFrame: AudioData) => {
        display.audioDecodedFrames++;
        audioWriter.write(audioFrame);
      });
      const audioElement = new Audio();
      audioElement.srcObject = new MediaStream([audioTrack]);
      await audioElement.play();
    }

    // audioDecoder.on(AudioDecoderEvent.Error, (error: ErrorInfo) => { });
    const gotAudio = (data: EncodedAudioChunkInit) => {
      display.audioTS = data.timestamp;
      display.audioSize = data.data.byteLength;

      console.log('[Renderer] Got audio data:', {
        timestamp: data.timestamp,
        size: data.data.byteLength,
        type: data.type
      });

      audioDecoder.decode(data);
      aframs.value++;
    };
    const gotVideo = (data: EncodedVideoChunkInit) => {
      display.videoTS = data.timestamp;
      display.videoSize = data.data.byteLength;
      vframs.value++;

      console.log('[Renderer] Got video data:', {
        timestamp: data.timestamp,
        size: data.data.byteLength,
        type: data.type,
        duration: data.duration,
        hasConfig: !!videoDecoder.config
      });

      if (videoDecoder.config) {
        try {
          if (dumpFile) {
            cache.push(data.data as Uint8Array);
          }
          videoDecoder.decode(data);
          console.log('[Renderer] Video data sent to decoder');
        } catch (err) {
          console.error('[Renderer] Error decoding video:', err);
        }
      } else {
        console.warn('[Renderer] Video decoder not configured yet');
      }
    };
    if (mode.value == DemuxMode.PULL) {
      console.log('[Renderer] Setting up PULL mode streams');
      demuxer.audioReadable?.pipeTo(
        new WritableStream({
          write(chunk: EncodedAudioChunkInit) {
            console.log('[Renderer] Audio stream received chunk:', {
              timestamp: chunk.timestamp,
              size: chunk.data.byteLength
            });
            gotAudio(chunk);
            if (file && options)
              options.onProgress({
                percent: (100 * conn.down.total) / file.size,
              });
            return readDelay(0);
          },
        })
      );
      demuxer.videoReadable?.pipeTo(
        new WritableStream({
          write(chunk: EncodedVideoChunkInit) {
            console.log('[Renderer] Video stream received chunk:', {
              timestamp: chunk.timestamp,
              size: chunk.data.byteLength,
              type: chunk.type
            });
            gotVideo(chunk);

            if (file && options)
              options.onProgress({
                percent: (100 * conn.down.total) / file.size,
              });

            return readDelay(40);
          },
        })
      );
    } else {
      console.log('[Renderer] Setting up PUSH mode callbacks');
      demuxer.gotAudio = gotAudio;
      demuxer.gotVideo = gotVideo;
      await conn.connect();
    }
  } catch (e: unknown) {
    if (options) options.onFinish();
    removeMessage();
    console.error(e);
    if (e instanceof Error) {
      message.error(e.message);
    } else {
      message.error("An unknown error occurred");
    }
  }
}
function disconnect() {
  if (playTimeout) {
    clearTimeout(playTimeout);
    playTimeout = null;
  }
  conn?.close();
  vframs.value = 0;
  aframs.value = 0;
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
      (vframs.value * 1000) / (now - lastsec)
    );
    vframs.value = 0;
    display.audioDecodedFrameRate = Math.floor(
      (aframs.value * 1000) / (now - lastsec)
    );
    aframs.value = 0;

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

const videoElement = ref<HTMLVideoElement | null>(null);
const canvasElement = ref<HTMLCanvasElement | null>(null);

interface WASMDecoderInit {
  print: (text: string) => void;
  printErr: (text: string) => void;
  onAbort: () => void;
}

const initializeDecoder = async () => {
  const videoDecoder = (() => {
    const isCanvasRenderer =
      rendererType.value === "yuv" || rendererType.value === "canvas";
    const canvas = canvasElement.value;
    const video = videoElement.value;

    if (!video && !canvas) {
      throw new Error("Either video or canvas element is required");
    }

    switch (decoderv.value) {
      case "soft":
      case "simd":
        if (isCanvasRenderer && !canvas) {
          throw new Error("Canvas element required for YUV/Canvas renderer");
        }
        const decoder =
          decoderv.value === "soft" ? VideoDecoderSoft : VideoDecoderSoftSIMD;
        return new decoder({
          yuvMode: rendererType.value === "yuv",
          canvas: canvas as HTMLCanvasElement,
          workerMode: false,
          wasmPath: `${
            decoderv.value === "soft" ? "videodec" : "videodec_simd"
          }.wasm`,
        });
      case "webcodecs":
        return new VideoDecoderHard();
      case "mse":
        if (!video) {
          throw new Error("Video element required for MSE decoder");
        }
        return new VideoDecoderMSE();
      default:
        if (!video) {
          throw new Error("Video element required for MSE decoder");
        }
        return new VideoDecoderMSE();
    }
  })();

  const audioDecoder = (() => {
    switch (decoderv.value) {
      case "soft":
      case "simd":
        return new AudioDecoderSoft();
      case "webcodecs":
      case "mse":
      default:
        return new AudioDecoderHard();
    }
  })();

  // Initialize decoders
  if (decoderv.value === "mse" && videoElement.value) {
    await videoDecoder.initialize(videoElement.value);
  } else if (decoderv.value === "soft" || decoderv.value === "simd") {
    // @ts-ignore - WASM decoder initialization requires different parameters
    await (videoDecoder as any).initialize({
      print: (text: string) => console.log(text),
      printErr: (text: string) => console.error(text),
      onAbort: () => console.error("WASM aborted"),
    } as WASMDecoderInit);
  } else {
    // @ts-ignore - WebCodecs decoder doesn't need parameters
    await (videoDecoder as any).initialize();
  }
  await audioDecoder.initialize();

  const renderer = (() => {
    const video = videoElement.value;
    const canvas = canvasElement.value;

    if (!video && !canvas) {
      throw new Error("Either video or canvas element is required");
    }

    switch (rendererType.value) {
      case "webcodecs":
        if (!video) {
          throw new Error("Video element required for WebCodecs renderer");
        }
        return new WebCodecsVideoRenderer(video);
      case "yuv":
        if (!canvas) {
          throw new Error("Canvas element required for YUV renderer");
        }
        return new YUVCanvasRenderer(canvas);
      case "canvas":
      default:
        if (!canvas) {
          throw new Error("Canvas element required for Canvas renderer");
        }
        return new CanvasRenderer(canvas);
    }
  })();

  videoDecoder.on(
    VideoDecoderEvent.VideoFrame,
    async (videoFrame: VideoFrame) => {
      display.videoDecodedFrames++;
      vframs.value++;
      if (vframs.value === 5) {  
        message.success("视频播放成功！");
        if (playTimeout) {
          clearTimeout(playTimeout);
          playTimeout = null;
        }
      }
      if (rendererType.value === "yuv" || rendererType.value === "canvas") {
        if (
          renderer instanceof YUVCanvasRenderer ||
          renderer instanceof CanvasRenderer
        ) {
          renderer.writeVideo(videoFrame);
        }
      } else if (renderer instanceof WebCodecsVideoRenderer) {
        await renderer.writeVideo(videoFrame);
      }
    }
  );

  videoDecoder.on(
    VideoDecoderEvent.VideoCodecInfo,
    (codecinfo: VideoCodecInfo) => {
      message.info(`width: ${codecinfo.width} height: ${codecinfo.height}`);
    }
  );

  videoDecoder.on(VideoDecoderEvent.Error, (error: Error) => {
    console.error(error);
    if (decoderv.value === "mse" && videoElement.value) {
      // @ts-ignore - MSE decoder requires HTMLVideoElement
      videoDecoder.initialize(videoElement.value);
    } else if (decoderv.value === "soft" || decoderv.value === "simd") {
      // @ts-ignore - WASM decoder initialization requires different parameters
      (videoDecoder as any).initialize({
        print: (text: string) => console.log(text),
        printErr: (text: string) => console.error(text),
        onAbort: () => console.error("WASM aborted"),
      } as WASMDecoderInit);
    } else {
      // @ts-ignore - WebCodecs decoder doesn't need parameters
      (videoDecoder as any).initialize();
    }
  });

  audioDecoder.on(
    AudioDecoderEvent.AudioCodecInfo,
    (codecinfo: AudioCodecInfo) => {}
  );

  audioDecoder.on(AudioDecoderEvent.AudioFrame, (audioFrame: AudioData) => {
    display.audioDecodedFrames++;
    aframs.value++;
    if (renderer instanceof WebCodecsVideoRenderer) {
      renderer.writeAudio(audioFrame);
    }
  });

  audioDecoder.on(AudioDecoderEvent.Error, (error: Error) => {
    console.error(error);
  });

  return { videoDecoder, audioDecoder, renderer };
};
</script>

<template>
  <n-space>
    默认格式<n-select
      v-model:value="muxType"
      :options="[
        { label: 'flv', value: 'flv' },
        { label: 'ps', value: 'ps' },
        { label: 'hls', value: 'hls' },
      ]"
    ></n-select>
    dump<n-switch v-model:value="dump"></n-switch> 视频解码器<n-select
      v-model:value="decoderv"
      :options="[
        { label: 'soft', value: 'soft' },
        { label: 'simd', value: 'simd' },
        { label: 'webcodecs', value: 'webcodecs' },
        { label: 'mse', value: 'mse' },
      ]"
    ></n-select>
    渲染方式<n-select
      v-model:value="rendererType"
      :options="[
        { label: 'WebCodecs', value: 'webcodecs' },
        { label: 'YUV Canvas', value: 'yuv' },
        { label: 'Canvas', value: 'canvas' },
      ]"
    ></n-select>
  </n-space>
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
    <n-button @click="disconnect">Close</n-button>
  </n-space>
  <video
    v-if="rendererType === 'webcodecs'"
    ref="videoElement"
    id="video"
    style="width: 100%; height: 100%"
    autoplay
    muted
    playsinline
  ></video>
  <canvas
    v-else
    ref="canvasElement"
    id="video"
    style="width: 100%; height: 100%"
  ></canvas>
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
      <n-statistic
        label="视频解码帧率"
        :value="display.videoDecodedFrameRate"
      ></n-statistic>
    </n-col>
    <n-col :span="6">
      <n-statistic
        label="视频解码帧数"
        :value="display.videoDecodedFrames"
      ></n-statistic>
    </n-col>
    <n-col :span="6">
      <n-statistic
        label="音频解码帧率"
        :value="display.audioDecodedFrameRate"
      ></n-statistic>
    </n-col>
    <n-col :span="6">
      <n-statistic
        label="音频解码帧数"
        :value="display.audioDecodedFrames"
      ></n-statistic>
    </n-col>
  </n-row>
  <canvas id="bps"></canvas>
</template>
