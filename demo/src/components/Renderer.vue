<script setup lang="ts">
import { onMounted, onUnmounted, reactive, ref, watchEffect } from 'vue';
import { Connection } from '../../../packages/conn/src';
import { Demuxer } from '../../../packages/demuxer/src';
import { Renderer } from '../../../packages/renderer/src';
import { MessageReactive, UploadCustomRequestOptions, UploadFileInfo, useMessage } from 'naive-ui';
import { ConnectionState, ConnectionEvent } from '../../../packages/conn/src/types';
import { TimelineDataSeries, TimelineGraphView } from 'webrtc-internals';
import { ArchiveOutline as ArchiveIcon } from '@vicons/ionicons5';
import { VideoDecoder, AudioDecoder } from '../../../packages/decoder/src';
import { VideoDecoderConfig, AudioDecoderConfig, VideoDecoderEvent, AudioDecoderEvent, ErrorInfo, VideoCodecInfo, AudioCodecInfo, AudioFrame, VideoPacket, AudioPacket, JVideoFrame } from '../../../packages/decoder/src/types';

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


function readDelay(t: number): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, t));
}
async function connect(file?: File, options?: UploadCustomRequestOptions) {
  try {

    if (url.value === '') {

      await conn.connect(file);

    } else {

      await conn.connect(url.value);
    }


    const videoDecoder = new VideoDecoder('soft-simd');
    await videoDecoder.initialize();

    const audioDecoder = new AudioDecoder('soft');
    await audioDecoder.initialize();

    const demuxer = new Demuxer(conn);
    const renderer = new Renderer(document.getElementById('video') as HTMLVideoElement);
    demuxer.on(Demuxer.AUDIO_ENCODER_CONFIG_CHANGED, (data: Uint8Array) => {
      message.info(Demuxer.AUDIO_ENCODER_CONFIG_CHANGED);

      let aconfig: AudioDecoderConfig = {
        audioType: demuxer.audioEncoderConfig.codec,
        extraData: data,
        outSampleType: 'f32-planar',
      };

      audioDecoder.configure(aconfig);

    });
    demuxer.on(Demuxer.VIDEO_ENCODER_CONFIG_CHANGED, (data: Uint8Array) => {
      message.info(Demuxer.VIDEO_ENCODER_CONFIG_CHANGED);


      let vconfig: VideoDecoderConfig = {
        videoType: demuxer.videoEncoderConfig.codec === 'h264' ? 'avc' : 'hevc',
        extraData: data,
        outPixelType: 'I420',
      };

      if (demuxer.videoEncoderConfig.codec === 'h264') {

        vconfig.avc = { format: 'avcc' };
      } else {
        vconfig.hevc = { format: 'hvcc' };
      }

      videoDecoder.configure(vconfig);
    });
    demuxer.audioReadable.pipeTo(new WritableStream({
      write(chunk: EncodedAudioChunkInit) {
        display.audioTS = chunk.timestamp;
        display.audioSize = chunk.data.byteLength;

        let packet: AudioPacket = {

          data: chunk.data,
          pts: chunk.timestamp
        };

        audioDecoder.decode(packet);
        if (file && options)
          options.onProgress({ percent: 100 * conn.down.total / file.size });
        return readDelay(0);
      }
    }));
    demuxer.videoReadable.pipeTo(new WritableStream({
      write(chunk: EncodedVideoChunkInit) {
        display.videoTS = chunk.timestamp;
        display.videoSize = chunk.data.byteLength;

        // console.log(`JS Chunk ${chunk.data.byteLength}  ${chunk.data[5]} ${chunk.data[6]} ${chunk.data[7]} ${chunk.data[8]} ${chunk.data[9]}`)

        let packet: VideoPacket = {

          data: chunk.data,
          keyFrame: chunk.type === 'key' ? true : false,
          pts: chunk.timestamp
        };

        videoDecoder.decode(packet);

        if (file && options)
          options.onProgress({ percent: 100 * conn.down.total / file.size });

        return readDelay(0);
      }
    }));



    videoDecoder.on(VideoDecoderEvent.VideoCodecInfo, (codecinfo: VideoCodecInfo) => {

      message.info(`width: ${codecinfo.width} height: ${codecinfo.height}`);

    });

    videoDecoder.on(VideoDecoderEvent.VideoFrame, (videoFrame: JVideoFrame) => {

      display.videoDecodedFrames++;
      vframs++;
      renderer.writeVideo(videoFrame.data, { codedWidth: videoFrame.width, codedHeight: videoFrame.height, format: videoFrame.pixelType, timestamp: videoFrame.pts });
    });

    videoDecoder.on(VideoDecoderEvent.Error, (error: ErrorInfo) => {


    });

    audioDecoder.on(AudioDecoderEvent.AudioCodecInfo, (codecinfo: AudioCodecInfo) => {

    });

    audioDecoder.on(AudioDecoderEvent.AudioFrame, (AudioFrame: AudioFrame) => {

      display.audioDecodedFrames++;
      aframs++;
    });

    audioDecoder.on(AudioDecoderEvent.Error, (error: ErrorInfo) => {


    });



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
  bpsDown: 0
});
onMounted(() => {
  const gv = new TimelineGraphView(document.getElementById('bps') as HTMLCanvasElement);
  const series = new TimelineDataSeries();
  gv.addDataSeries(series);
  let lastsec = new Date().getTime();
  let id = setInterval(() => {
    data.totalDown = conn.down.total;
    data.bpsDown = conn.down.bps;

    let now = new Date().getTime();

    display.videoDecodedFrameRate = Math.floor(vframs * 1000 / (now - lastsec));
    vframs = 0;
    display.audioDecodedFrameRate = Math.floor(aframs * 1000 / (now - lastsec));
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
function onRemove(options: { file: UploadFileInfo, fileList: Array<UploadFileInfo>; }) {
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
      <n-text style="font-size: 16px">
        点击或者拖动文件到该区域来上传
      </n-text>
      <n-p depth="3" style="margin: 8px 0 0 0">
        支持上传的文件类型：<n-text>mp4,flv,ts,h264,h265</n-text>
      </n-p>
    </n-upload-dragger>
  </n-upload>
  <n-space justify="end" :wrap-item="false">
    <div style="flex-grow: 1">
      <n-input v-model:value="url" :input-props="{ type: 'url' }" placeholder="URL" />
    </div>
    <n-button @click="connect">Connect</n-button>
    <n-button @click="disconnect">Close</n-button>
  </n-space>
  <video id="video"></video>
  <n-row>
    <n-col :span="12">
      <n-statistic label="下行总量" :value="data.totalDown">
      </n-statistic>
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
  