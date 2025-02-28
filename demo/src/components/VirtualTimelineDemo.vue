<template>
  <n-space vertical>
    <n-card title="虚拟时间线演示">
      <n-space vertical size="large">
        <!-- 虚拟视频播放器 -->
        <VirtualVideo
          :current-time="currentVirtualTime"
          :duration="totalDuration"
          :buffer-ranges="bufferRanges"
          :is-playing="isPlaying"
          :is-buffering="isBuffering"
          :current-segment="currentSegmentIndex"
          @play="play"
          @pause="pause"
          @seek="seek"
          @rate-change="setPlaybackRate"
        />

        <!-- 片段可视化 -->
        <SegmentVisualizer
          :segments="segments"
          :current-segment-index="currentSegmentIndex"
          :current-time="currentVirtualTime"
          :duration="totalDuration"
          :buffer-ranges="bufferRanges"
          @load-segment="loadSegment"
          @load-all-segments="loadAllSegments"
          @clear-all-segments="clearAllSegments"
          @seek="seek"
        />

        <!-- M3U8加载控制 -->
        <n-card title="加载M3U8" size="small">
          <n-space vertical>
            <n-input
              v-model:value="m3u8Url"
              placeholder="输入M3U8 URL或使用示例"
            />
            <n-space>
              <n-button @click="loadExampleM3U8">加载示例M3U8</n-button>
              <n-button type="primary" @click="loadM3U8" :disabled="!m3u8Url">
                加载M3U8
              </n-button>
            </n-space>
          </n-space>
        </n-card>

        <!-- 时间线控制 -->
        <n-card title="时间轴映射配置" size="small">
          <n-space vertical>
            <n-form-item label="映射模式">
              <n-radio-group v-model:value="timelineMode">
                <n-radio-button value="1:1">1:1 映射</n-radio-button>
                <n-radio-button value="custom">自定义映射</n-radio-button>
              </n-radio-group>
            </n-form-item>

            <n-form-item label="虚拟/物理时间测试" v-if="virtualTimeline">
              <n-space vertical>
                <n-space>
                  <n-input-number
                    v-model:value="testVirtualTime"
                    placeholder="虚拟时间"
                    :min="0"
                    :max="totalDuration"
                  />
                  <n-button @click="convertToPhysical">转换为物理时间</n-button>
                  <n-text
                    >对应物理时间: {{ testPhysicalTime.toFixed(2) }}s</n-text
                  >
                </n-space>

                <n-space>
                  <n-input-number
                    v-model:value="testPhysicalTime"
                    placeholder="物理时间"
                    :min="0"
                  />
                  <n-button @click="convertToVirtual">转换为虚拟时间</n-button>
                  <n-text
                    >对应虚拟时间: {{ testVirtualTime.toFixed(2) }}s</n-text
                  >
                </n-space>
              </n-space>
            </n-form-item>
          </n-space>
        </n-card>

        <!-- 调试日志 -->
        <LogPanel ref="logPanelRef" :max-logs="500" />
      </n-space>
    </n-card>
  </n-space>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from "vue";
import {
  NSpace,
  NCard,
  NButton,
  NInput,
  NInputNumber,
  NFormItem,
  NRadioGroup,
  NRadioButton,
  NText,
} from "naive-ui";
import {
  VirtualTimeline,
  MediaSegment,
  BufferRange,
  PlaylistInfo,
} from "jv4-demuxer/src/VirtualTimeline";
import VirtualVideo from "./VirtualVideo.vue";
import SegmentVisualizer from "./SegmentVisualizer.vue";
import LogPanel from "./LogPanel.vue";

// 状态
const m3u8Url = ref("");
const timelineMode = ref<"1:1" | "custom">("1:1");
const isPlaying = ref(false);
const isBuffering = ref(false);
const currentVirtualTime = ref(0);
const currentSegmentIndex = ref(-1);
const totalDuration = ref(0);
const segments = ref<MediaSegment[]>([]);
const bufferRanges = ref<BufferRange[]>([]);
const virtualTimeline = ref<VirtualTimeline | null>(null);
const logPanelRef = ref<InstanceType<typeof LogPanel> | null>(null);

// 时间转换测试
const testVirtualTime = ref(0);
const testPhysicalTime = ref(0);

// 示例M3U8内容
const exampleM3U8 = `#EXTM3U
#EXT-X-VERSION:3
#EXT-X-TARGETDURATION:10
#EXT-X-MEDIA-SEQUENCE:0

#EXTINF:9.9,
segment1.ts
#EXTINF:9.8,
segment2.ts
#EXTINF:10.2,
segment3.ts
#EXTINF:9.7,
segment4.ts
#EXTINF:10.3,
segment5.ts
#EXTINF:9.9,
segment6.ts
#EXTINF:10.1,
segment7.ts
#EXTINF:9.8,
segment8.ts
#EXT-X-ENDLIST`;

// 加载示例M3U8
const loadExampleM3U8 = () => {
  if (logPanelRef.value) {
    logPanelRef.value.addLog("加载示例M3U8", "info");
  }

  m3u8Url.value = "http://example.com/playlist.m3u8";

  if (virtualTimeline.value) {
    // 销毁旧实例
    virtualTimeline.value.destroy();
  }

  // 创建新实例
  virtualTimeline.value = new VirtualTimeline();
  setupVirtualTimelineEvents();

  // 解析M3U8内容
  const parsedPlaylist = virtualTimeline.value.createPlaylistFromM3U8(
    exampleM3U8,
    "http://example.com/"
  );

  // 应用自定义映射（如果需要）
  if (timelineMode.value === "custom") {
    customizeTimeMapping(parsedPlaylist);
  }

  // 加载播放列表
  virtualTimeline.value.loadPlaylist(parsedPlaylist);

  // 更新状态
  totalDuration.value = parsedPlaylist.totalDuration;
  segments.value = parsedPlaylist.segments;

  if (logPanelRef.value) {
    logPanelRef.value.addLog(
      `成功加载播放列表：${
        segments.value.length
      }个片段，总时长${totalDuration.value.toFixed(2)}秒`,
      "success"
    );
  }
};

// 加载自定义M3U8
const loadM3U8 = async () => {
  if (!m3u8Url.value) return;

  if (logPanelRef.value) {
    logPanelRef.value.addLog(`开始加载M3U8: ${m3u8Url.value}`, "info");
  }

  isBuffering.value = true;

  try {
    // 模拟加载M3U8（在实际环境中，这里应该是一个fetch请求）
    await new Promise((resolve) => setTimeout(resolve, 1000));

    // 使用与示例相同的内容
    loadExampleM3U8();

    isBuffering.value = false;
  } catch (error) {
    isBuffering.value = false;

    if (logPanelRef.value) {
      logPanelRef.value.addLog(`加载M3U8失败: ${error}`, "error");
    }
  }
};

// 自定义时间映射
const customizeTimeMapping = (playlist: PlaylistInfo) => {
  // 示例：将虚拟时间戳改为一个更复杂的映射
  let virtualTime = 0;

  for (const segment of playlist.segments) {
    // 创建不连续的虚拟时间轴
    // 例如：第一个片段从0开始，第二个片段从20开始，第三个片段从40开始，依此类推
    segment.virtualStartTime = virtualTime;
    segment.virtualEndTime = virtualTime + segment.duration;

    // 每个片段之间添加10秒的间隔
    virtualTime = segment.virtualEndTime + 10;
  }

  // 更新总时长
  playlist.totalDuration = virtualTime - 10; // 减去最后一个间隔
};

// 设置虚拟时间线事件
const setupVirtualTimelineEvents = () => {
  if (!virtualTimeline.value) return;

  virtualTimeline.value.on("segmentLoadStart", (segment) => {
    if (logPanelRef.value) {
      logPanelRef.value.addLog(
        `开始加载片段 #${segment.index}: ${segment.url}`,
        "info"
      );
    }

    isBuffering.value = true;
  });

  virtualTimeline.value.on("segmentLoaded", (segment) => {
    if (logPanelRef.value) {
      logPanelRef.value.addLog(`片段 #${segment.index} 加载完成`, "success");
    }

    isBuffering.value = false;
  });

  virtualTimeline.value.on("segmentLoadError", (segment, error) => {
    if (logPanelRef.value) {
      logPanelRef.value.addLog(
        `片段 #${segment.index} 加载失败: ${error.message}`,
        "error"
      );
    }

    isBuffering.value = false;
  });

  virtualTimeline.value.on("timeUpdate", (virtualTime, physicalTime) => {
    currentVirtualTime.value = virtualTime;

    const segmentResult =
      virtualTimeline.value?.findSegmentForVirtualTime(virtualTime);
    if (segmentResult?.segment) {
      currentSegmentIndex.value = segmentResult.segment.index;
    }
  });

  virtualTimeline.value.on("seeking", (virtualTime, physicalTime) => {
    if (logPanelRef.value) {
      logPanelRef.value.addLog(
        `开始跳转到 ${virtualTime.toFixed(
          2
        )}s (物理时间: ${physicalTime.toFixed(2)}s)`,
        "info"
      );
    }

    isBuffering.value = true;
  });

  virtualTimeline.value.on("seeked", (virtualTime, physicalTime) => {
    if (logPanelRef.value) {
      logPanelRef.value.addLog(
        `完成跳转到 ${virtualTime.toFixed(
          2
        )}s (物理时间: ${physicalTime.toFixed(2)}s)`,
        "success"
      );
    }

    isBuffering.value = false;
  });

  virtualTimeline.value.on("bufferUpdate", (ranges) => {
    bufferRanges.value = ranges;
  });

  virtualTimeline.value.on("playbackStateChange", (playing) => {
    isPlaying.value = playing;

    if (logPanelRef.value) {
      logPanelRef.value.addLog(
        `播放状态更改: ${playing ? "播放" : "暂停"}`,
        "info"
      );
    }
  });

  virtualTimeline.value.on("debug", (message) => {
    if (logPanelRef.value) {
      logPanelRef.value.addLog(message, "debug");
    }
  });
};

// 播放控制
const play = () => {
  virtualTimeline.value?.play();
};

const pause = () => {
  virtualTimeline.value?.pause();
};

const seek = (time: number) => {
  virtualTimeline.value?.seek(time);
};

const setPlaybackRate = (rate: number) => {
  if (!virtualTimeline.value) return;

  virtualTimeline.value.setPlaybackRate(rate);
  if (logPanelRef.value) {
    logPanelRef.value.addLog(`设置播放速率: ${rate}x`, "info");
  }
};

// 片段控制
const loadSegment = (index: number) => {
  // 通过seek操作来触发片段加载
  const segment = segments.value.find((s) => s.index === index);
  if (segment) {
    seek(segment.virtualStartTime);
  }
};

const loadAllSegments = async () => {
  if (!virtualTimeline.value || segments.value.length === 0) return;

  if (logPanelRef.value) {
    logPanelRef.value.addLog("开始加载所有片段", "info");
  }

  isBuffering.value = true;

  // 对所有片段进行seek操作
  for (let i = 0; i < segments.value.length; i++) {
    // 检查是否已加载
    if (segments.value[i].isLoaded) continue;

    seek(segments.value[i].virtualStartTime);
    await new Promise((resolve) => setTimeout(resolve, 600)); // 等待加载完成
  }

  isBuffering.value = false;

  if (logPanelRef.value) {
    logPanelRef.value.addLog("所有片段加载完成", "success");
  }
};

const clearAllSegments = () => {
  if (!virtualTimeline.value) return;

  // 重新创建实例来清除所有片段
  const playlist = {
    segments: segments.value.map((segment) => ({
      ...segment,
      isLoaded: false,
      isLoading: false,
      isBuffered: false,
    })),
    totalDuration: totalDuration.value,
  };

  virtualTimeline.value.destroy();
  virtualTimeline.value = new VirtualTimeline();
  setupVirtualTimelineEvents();
  virtualTimeline.value.loadPlaylist(playlist);

  // 更新状态
  currentVirtualTime.value = 0;
  currentSegmentIndex.value = -1;
  bufferRanges.value = [];

  if (logPanelRef.value) {
    logPanelRef.value.addLog("清除所有片段", "info");
  }
};

// 时间转换测试
const convertToPhysical = () => {
  if (!virtualTimeline.value) return;

  testPhysicalTime.value = virtualTimeline.value.virtualToPhysicalTime(
    testVirtualTime.value
  );

  if (logPanelRef.value) {
    logPanelRef.value.addLog(
      `虚拟时间 ${testVirtualTime.value.toFixed(
        2
      )}s 对应物理时间 ${testPhysicalTime.value.toFixed(2)}s`,
      "info"
    );
  }
};

const convertToVirtual = () => {
  if (!virtualTimeline.value) return;

  testVirtualTime.value = virtualTimeline.value.physicalToVirtualTime(
    testPhysicalTime.value
  );

  if (logPanelRef.value) {
    logPanelRef.value.addLog(
      `物理时间 ${testPhysicalTime.value.toFixed(
        2
      )}s 对应虚拟时间 ${testVirtualTime.value.toFixed(2)}s`,
      "info"
    );
  }
};

// 生命周期钩子
onMounted(() => {
  // 初始化
  if (logPanelRef.value) {
    logPanelRef.value.addLog("虚拟时间线演示页面已加载", "info");
  }
});

onUnmounted(() => {
  // 清理
  if (virtualTimeline.value) {
    virtualTimeline.value.destroy();
  }
});
</script>

<style scoped>
/* 可添加特定样式 */
</style>
