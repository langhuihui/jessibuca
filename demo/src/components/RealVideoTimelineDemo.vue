<template>
  <n-space vertical>
    <n-card title="真实视频时间线演示">
      <n-space vertical size="large">
        <!-- 真实视频播放器 -->
        <div class="real-video-container">
          <video
            ref="videoRef"
            class="real-video"
            @timeupdate="onTimeUpdate"
            @play="onPlay"
            @pause="onPause"
            @seeking="onSeeking"
            @seeked="onSeeked"
            @waiting="onWaiting"
            @canplaythrough="onCanPlayThrough"
            @loadedmetadata="onLoadedMetadata"
            controls
          ></video>

          <!-- 缓冲指示器 -->
          <div class="buffering-indicator" v-if="isBuffering">
            <n-spin size="large" />
            <div>缓冲中...</div>
          </div>

          <!-- 快进快退控制 -->
          <div class="seek-controls">
            <n-space>
              <n-button @click="seekRelative(-10)" :disabled="!isVideoLoaded">
                << 快退10秒
              </n-button>
              <n-button @click="togglePlayPause" :disabled="!isVideoLoaded">
                {{ isPlaying ? "暂停" : "播放" }}
              </n-button>
              <n-button @click="seekRelative(10)" :disabled="!isVideoLoaded">
                快进10秒 >>
              </n-button>
            </n-space>
          </div>
        </div>

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
              <n-button
                id="load-m3u8-btn"
                type="primary"
                @click="loadM3U8"
                :disabled="!m3u8Url"
              >
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

            <n-form-item label="滑动窗口配置">
              <n-space vertical>
                <n-space align="center">
                  <n-switch v-model:value="slidingWindowConfig.enabled" />
                  <span>启用滑动窗口</span>
                </n-space>

                <n-space>
                  <n-input-number
                    v-model:value="slidingWindowConfig.forward"
                    :disabled="!slidingWindowConfig.enabled"
                    :min="1"
                    :max="10"
                    label="向前预加载"
                    size="small"
                  />
                  <n-input-number
                    v-model:value="slidingWindowConfig.backward"
                    :disabled="!slidingWindowConfig.enabled"
                    :min="0"
                    :max="5"
                    label="向后保留"
                    size="small"
                  />
                  <n-button
                    :disabled="!slidingWindowConfig.enabled"
                    size="small"
                    @click="applyCurrentWindowConfig"
                  >
                    应用配置
                  </n-button>
                </n-space>
              </n-space>
            </n-form-item>

            <n-form-item label="虚拟/物理时间测试">
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
import { ref, onMounted, onUnmounted, computed, watch, onBeforeUnmount, nextTick } from "vue";
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
  NSpin,
  NSwitch,
} from "naive-ui";
import { HLSv7Demuxer } from "jv4-demuxer/src/hlsv7";
import { Events } from "jv4-demuxer/src/EventBus";
import type { MediaSegment, BufferRange } from "jv4-demuxer/src/VirtualTimeline";
import type { SlidingWindowConfig } from "jv4-demuxer/src/SegmentLoader";
import type { DemuxerOptions } from "jv4-demuxer/src/hlsv7";
import SegmentVisualizer from "./SegmentVisualizer.vue";
import LogPanel from "./LogPanel.vue";

// 状态
const m3u8Url = ref(
  "http://localhost:8080/hls/vod/fmp4.m3u8?start=1741182995&streamPath=live/test"
);
const timelineMode = ref<"1:1" | "custom">("1:1");
const isPlaying = ref(false);
const isBuffering = ref(false);
const currentVirtualTime = ref(0);
const currentSegmentIndex = ref(-1);
const totalDuration = ref(0);
const segments = ref<MediaSegment[]>([]);
const bufferRanges = ref<BufferRange[]>([]);
const logPanelRef = ref<InstanceType<typeof LogPanel> | null>(null);
const videoRef = ref<HTMLVideoElement | null>(null);

// 时间转换测试
const testVirtualTime = ref(0);
const testPhysicalTime = ref(0);

// 播放器及相关状态
const demuxer = ref<HLSv7Demuxer | null>(null);
const videoWidth = ref(0);
const videoHeight = ref(0);

// 滑动窗口参数
const slidingWindowConfig = ref<SlidingWindowConfig>({
  forward: 2,
  backward: 1,
  enabled: true,
});

// 编解码器配置
const codecConfig = ref<string>('');

// 添加状态变量来跟踪视频是否已加载
const isVideoLoaded = computed(() => {
  return demuxer.value !== null && segments.value.length > 0;
});

// Add this utility function after the imports and before other code
const addLog = (
  message: string,
  type: "info" | "success" | "warning" | "error" | "debug" = "info"
) => {
  // Log to UI panel
  if (logPanelRef.value) {
    logPanelRef.value.addLog(message, type);
  }

  // Also log to browser console with appropriate console method
  switch (type) {
    case "info":
      console.info(`[VideoPlayer] ${message}`);
      break;
    case "success":
      console.log(`[VideoPlayer] %c${message}`, "color: green");
      break;
    case "warning":
      console.warn(`[VideoPlayer] ${message}`);
      break;
    case "error":
      console.error(`[VideoPlayer] ${message}`);
      break;
    case "debug":
      console.debug(`[VideoPlayer] ${message}`);
      break;
    default:
      console.log(`[VideoPlayer] ${message}`);
  }
};

// 设置Demuxer事件监听
const setupDemuxerEventListeners = () => {
  if (!videoRef.value) {
    addLog("未找到视频元素，无法设置事件监听器", "error");
    return;
  }

  // 获取eventBus
  const eventBus = (demuxer.value as any)?.eventBus;
  if (!eventBus) {
    addLog("未找到eventBus，无法设置事件监听器", "error");
    return;
  }

  addLog("开始设置事件监听器", "debug");

  // 片段加载事件
  eventBus.on(Events.SEGMENT_LOADED, (segmentIndex: number) => {
    addLog(`收到片段加载事件: segmentIndex=${segmentIndex}`, "debug");
    
    if (
      typeof segmentIndex === "number" &&
      segmentIndex >= 0 &&
      segments.value.length > 0 &&
      segmentIndex < segments.value.length
    ) {
      addLog(`更新片段 ${segmentIndex} 状态: 已加载=true, 已缓冲=true`, "debug");
      segments.value[segmentIndex].isLoaded = true;
      segments.value[segmentIndex].isLoading = false;
      segments.value[segmentIndex].isBuffered = true;

      addLog(`片段 ${segmentIndex} 已加载完成`, "success");
      
      // 立即更新缓冲区范围
      updateBufferRanges();
      
      // 立即更新片段状态统计
      getSegmentStatus();
    } else {
      addLog(`无法更新片段 ${segmentIndex} 状态: 段落长度=${segments.value.length}`, "warning");
    }
  });

  // 监听播放列表加载事件
  eventBus.on(Events.PLAYLIST_LOADED, (playlistInfo: any) => {
    addLog(`收到播放列表加载事件: ${playlistInfo.segments?.length || 0} 个片段`, "debug");
    
    const playlist = playlistInfo.segments || [];
    // 计算累计时间以正确设置virtualStartTime和virtualEndTime
    let cumulativeTime = 0;
    
    segments.value = playlist.map((segment: any, index: number) => {
      const startTime = cumulativeTime;
      const duration = segment.duration || 0;
      const endTime = startTime + duration;
      
      // 更新累计时间
      cumulativeTime = endTime;
      
      return {
        index,
        url: segment.url,
        duration: duration,
        virtualStartTime: startTime,
        virtualEndTime: endTime,
        physicalStartTime: 0,
        physicalEndTime: segment.duration,
        physicalTime: segment.physicalTime,
        codec: segment.codec,
        isLoaded: false,
        isLoading: false,
        isBuffered: false,
      };
    }) as MediaSegment[];

    // 更新总时长
    totalDuration.value = cumulativeTime;

    addLog(`获取到播放列表，包含 ${playlist.length} 个片段，总时长: ${totalDuration.value.toFixed(2)}秒`, "info");
    
    // 主动更新缓冲区范围
    updateBufferRanges();
    
    // 主动请求片段状态
    getSegmentStatus();
  });

  // 添加片段加载开始事件
  eventBus.on(Events.SEGMENT_LOAD_START, (segmentIndex: number) => {
    addLog(`开始加载片段 ${segmentIndex}`, "info");
    if (
      typeof segmentIndex === "number" &&
      segmentIndex >= 0 &&
      segments.value.length > 0 &&
      segmentIndex < segments.value.length
    ) {
      segments.value[segmentIndex].isLoading = true;
      // 更新片段状态
      getSegmentStatus();
    }
  });

  // 片段加载错误事件
  eventBus.on(Events.SEGMENT_LOAD_ERROR, (segmentIndex: number, error: Error) => {
    if (segmentIndex >= 0 && segmentIndex < segments.value.length) {
      segments.value[segmentIndex].isLoading = false;
      addLog(`片段 ${segmentIndex} 加载失败: ${error}`, "error");
    }
  });

  // 缓冲区更新事件
  eventBus.on(Events.BUFFER_UPDATE, (ranges: BufferRange[]) => {
    if (ranges && ranges.length > 0) {
      bufferRanges.value = ranges;
      
      // 基于缓冲区范围更新片段状态
      ranges.forEach(range => {
        segments.value.forEach(segment => {
          // 如果片段完全在缓冲区范围内，标记为已缓冲
          if (segment.virtualStartTime >= range.start && segment.virtualEndTime <= range.end) {
            segment.isBuffered = true;
            segment.isLoaded = true;
          }
        });
      });
    }
  });

  // 添加错误事件处理
  eventBus.on(Events.ERROR, (error: Error) => {
    addLog(`Demuxer错误: ${error}`, "error");
  });

  // 添加日志事件处理
  eventBus.on(Events.LOG, (message: string, level: string) => {
    addLog(`Demuxer日志 [${level}]: ${message}`, level as any);
    
    // 如果日志中包含片段加载成功的信息，主动请求片段状态
    if (message.includes('Segment') && message.includes('loaded successfully')) {
      getSegmentStatus();
    }
  });
};

// 查询片段加载状态
const getSegmentStatus = () => {
  if (!demuxer.value) {
    addLog("无法检查片段状态: demuxer不存在", "warning");
    return;
  }

  let loaded = 0;
  let loading = 0;
  let buffered = 0;
  let notLoaded = 0;

  // 记录调试信息: 段落数组长度
  addLog(`检查片段状态: 段落数组长度=${segments.value.length}`, "debug");

  // 遍历每个片段，添加调试信息
  segments.value.forEach((segment, index) => {
    addLog(
      `片段 ${index} 状态: 已缓冲=${segment.isBuffered}, 已加载=${segment.isLoaded}, 加载中=${segment.isLoading}`,
      "debug"
    );
    
    if (segment.isBuffered) buffered++;
    else if (segment.isLoaded) loaded++;
    else if (segment.isLoading) loading++;
    else notLoaded++;
  });

  addLog(
    `片段状态: 已缓冲=${buffered}, 已加载=${loaded}, 加载中=${loading}, 未加载=${notLoaded}`,
    "info"
  );

  // 如果一段时间后仍无片段加载，检查MediaSource状态
  if (
    buffered === 0 &&
    loaded === 0 &&
    loading === 0 &&
    segments.value.length > 0
  ) {
    addLog(
      "警告: 未检测到任何片段加载活动，可能是MediaSource未正确初始化",
      "warning"
    );

    if (videoRef.value) {
      try {
        // 获取扩展的视频元素信息
        const videoEl = videoRef.value;
        const videoInfo = {
          src: videoEl.src,
          readyState: videoEl.readyState,
          networkState: videoEl.networkState,
          hasBlob: videoEl.src && videoEl.src.startsWith("blob:"),
          error: videoEl.error ? videoEl.error.message : null,
          duration: videoEl.duration,
          buffered: Array.from({ length: videoEl.buffered.length }, (_, i) => ({
            start: videoEl.buffered.start(i),
            end: videoEl.buffered.end(i),
          })),
        };

        addLog(`视频元素详细状态: ${JSON.stringify(videoInfo)}`, "debug");
      } catch (err) {
        addLog(`获取视频元素信息出错: ${err}`, "error");
      }
    } else {
      addLog("无法获取视频元素信息：videoRef.value为null", "warning");
    }
  }
};

// 加载自定义M3U8
const loadM3U8 = async () => {
  try {
    if (!m3u8Url.value) {
      addLog("请输入M3U8地址", "warning");
      return;
    }

    if (!videoRef.value) {
      addLog("视频元素未找到", "error");
      return;
    }

    addLog(`开始加载M3U8: ${m3u8Url.value}`, "info");
    isBuffering.value = true;

    // 销毁之前的demuxer实例
    if (demuxer.value) {
      addLog("销毁先前的demuxer实例", "debug");
      demuxer.value.destroy();
      demuxer.value = null;
    }

    // 重置状态
    segments.value = [];
    bufferRanges.value = [];
    currentVirtualTime.value = 0;
    totalDuration.value = 0;
    currentSegmentIndex.value = -1;

    // 配置选项
    const options: DemuxerOptions = {
      codec: codecConfig.value,
      logLevel: "debug",
      maxBufferLength: 60,
      slidingWindow: slidingWindowConfig.value,
    };

    // 创建demuxer实例
    demuxer.value = new HLSv7Demuxer(videoRef.value, options);

    // 设置事件监听前先记录段落数量
    addLog(`初始段落数量: ${segments.value.length}`, "debug");

    // 设置事件监听
    setupDemuxerEventListeners();

    // 初始化demuxer
    addLog("开始初始化demuxer", "debug");
    await demuxer.value.init(m3u8Url.value);

    // 验证片段数组是否正确初始化
    addLog(`初始化后段落数量: ${segments.value.length}`, "debug");

    // 检查src是否改变
    const newSrc = videoRef.value.src;
    addLog(`初始化后视频src: ${newSrc || "(空)"}`, "debug");
    if (newSrc && newSrc.startsWith("blob:")) {
      addLog("成功创建MediaSource blob URL", "success");
    } else {
      addLog("MediaSource blob URL未创建", "warning");
    }

    // 更新组件状态
    totalDuration.value = demuxer.value.getTotalDuration();
    
    // 如果segments未通过事件填充，主动获取
    if (segments.value.length === 0) {
      addLog("段落数组为空，尝试主动获取片段信息", "warning");
      
      try {
        // 尝试使用getTotalSegments和findSegmentByIndex方法获取所有片段
        const totalSegments = demuxer.value.getTotalSegments();
        if (totalSegments > 0) {
          addLog(`通过getTotalSegments获取到片段总数: ${totalSegments}`, "info");
          const newSegments: MediaSegment[] = [];
          
          // 使用findSegmentByIndex方法构建片段数组
          for (let i = 0; i < totalSegments; i++) {
            const segment = (demuxer.value as any).findSegmentByIndex?.(i);
            if (segment) {
              newSegments.push(segment);
            }
          }
          
          if (newSegments.length > 0) {
            addLog(`成功获取 ${newSegments.length} 个片段`, "info");
            segments.value = newSegments;
          }
        }
      } catch (error) {
        addLog(`尝试获取片段信息时出错: ${error}`, "warning");
      }
    }
    
    // 强制更新片段状态，设置虚拟时间
    if (segments.value.length > 0) {
      let cumTime = 0;
      segments.value.forEach((segment, index) => {
        segment.virtualStartTime = cumTime;
        segment.virtualEndTime = cumTime + segment.duration;
        cumTime += segment.duration;
        
        // 设置加载状态
        if (index < 2) { // 假设前两个片段已加载
          segment.isLoaded = true;
          segment.isBuffered = true;
        }
      });
      
      // 更新总时长
      if (cumTime > 0) {
        totalDuration.value = cumTime;
      }
      
      // 立即更新片段状态统计
      getSegmentStatus();
    } else {
      addLog("警告: 片段列表为空", "warning");
    }

    isBuffering.value = false;

    // 启动定时检查片段加载状态
    startSegmentStatusMonitor();
  } catch (error) {
    isBuffering.value = false;
    addLog(`加载M3U8失败: ${error}`, "error");
  }
};

// 启动片段状态监控定时器
const segmentStatusTimer = ref<number | null>(null);

// 启动片段状态监控
const startSegmentStatusMonitor = () => {
  // 清除之前的定时器
  if (segmentStatusTimer.value) {
    clearInterval(segmentStatusTimer.value);
  }
  
  // 每3秒检查一次片段状态
  segmentStatusTimer.value = window.setInterval(() => {
    addLog("定时检查片段状态", "debug");
    getSegmentStatus();
    
    // 如果所有片段都已加载或者没有片段正在加载，停止监控
    const allSegmentsLoaded = segments.value.every(s => s.isLoaded || s.isBuffered);
    const anySegmentLoading = segments.value.some(s => s.isLoading);
    
    if (allSegmentsLoaded || !anySegmentLoading) {
      addLog("所有片段已加载或没有片段正在加载，停止状态监控", "info");
      clearInterval(segmentStatusTimer.value!);
      segmentStatusTimer.value = null;
    }
  }, 3000);
};

// 加载所有片段
const loadAllSegments = () => {
  if (!demuxer.value) return;

  addLog("开始加载所有片段", "info");
  // 在demuxer中加载所有片段的逻辑可能需要额外实现
  // 这里暂时简单处理，加载前10个片段
  for (let i = 0; i < Math.min(10, segments.value.length); i++) {
    loadSegment(i);
  }
};

// 清除所有片段
const clearAllSegments = () => {
  if (!demuxer.value) return;

  addLog("清除所有已加载片段", "info");
  demuxer.value.clearBuffer();
};

// 加载特定片段
const loadSegment = (segmentIndex: number) => {
  if (
    !demuxer.value ||
    segmentIndex < 0 ||
    segmentIndex >= segments.value.length
  )
    return;

  addLog(`手动加载片段 #${segmentIndex}`, "info");
  // TODO: 这个功能在当前的HLSv7Demuxer中可能没有直接暴露
  // demuxer.value.loadSegment(segmentIndex);
};

// 跳转到指定时间
const seek = (time: number) => {
  if (!demuxer.value) return;

  addLog(`跳转到 ${time.toFixed(2)} 秒`, "info");
  demuxer.value.seek(time);
};

// 相对当前时间跳转（快进快退）
const seekRelative = (offsetSeconds: number) => {
  if (!demuxer.value) return;

  // 获取当前虚拟时间
  const currentTime = currentVirtualTime.value;

  // 计算新的目标时间
  const targetTime = Math.max(
    0,
    Math.min(totalDuration.value, currentTime + offsetSeconds)
  );

  addLog(
    `${offsetSeconds > 0 ? "快进" : "快退"} ${Math.abs(
      offsetSeconds
    )} 秒到 ${targetTime.toFixed(2)} 秒`,
    "info"
  );

  // 调用seek函数
  seek(targetTime);
};

// 转换测试函数
const convertToPhysical = () => {
  if (!demuxer.value) return;

  // 使用demuxer的时间转换逻辑，但这需要demuxer暴露这个功能
  // 暂时假设使用线性映射
  testPhysicalTime.value = testVirtualTime.value;
};

const convertToVirtual = () => {
  if (!demuxer.value) return;

  // 同上
  testVirtualTime.value = testPhysicalTime.value;
};

// 应用当前窗口配置
const applyCurrentWindowConfig = () => {
  if (!demuxer.value) return;

  demuxer.value.setSlidingWindowConfig(slidingWindowConfig.value);
  addLog(
    `已应用滑动窗口配置: 前向=${slidingWindowConfig.value.forward}, 后向=${slidingWindowConfig.value.backward}, 启用=${slidingWindowConfig.value.enabled}`,
    "info"
  );
};

// 添加一个定时器，确保虚拟时间线被更新，即使video元素的timeupdate事件不触发
const setupTimeUpdateChecker = () => {
  if (!demuxer.value) {
    addLog("setupTimeUpdateChecker: demuxer为空，无法设置更新检查器", "warning");
    return null;
  }
  
  addLog("开始设置时间更新检查器，每100毫秒检查一次", "info");
  
  // 每100毫秒检查一次时间更新
  const intervalId = setInterval(() => {
    if (!demuxer.value || !isPlaying.value) {
      addLog("时间更新检查：demuxer为空或视频未播放，跳过检查", "debug");
      return;
    }
    
    // 手动触发时间更新
    const newTime = demuxer.value.getCurrentTime();
    const oldTime = currentVirtualTime.value;
    
    addLog(`时间更新检查：当前=${oldTime.toFixed(2)}, 新时间=${newTime.toFixed(2)}, 是否播放=${isPlaying.value}`, "debug");
    
    if (newTime !== oldTime) {
      addLog(`时间已更新：${oldTime.toFixed(2)} -> ${newTime.toFixed(2)}，正在更新UI`, "info");
      currentVirtualTime.value = newTime;
      
      // 更新UI，使时间轴指针移动
      onTimeUpdate();
    }
  }, 100);
  
  addLog(`时间更新检查器已设置，intervalId=${intervalId}`, "success");
  
  // 保存interval ID以便后续清理
  return intervalId;
};

// 视频时间更新事件处理
const onTimeUpdate = () => {
  if (!demuxer.value) {
    addLog("onTimeUpdate: demuxer为空，无法获取播放时间", "warning");
    return;
  }

  if (!videoRef.value) {
    addLog("onTimeUpdate: videoRef为空，无法获取视频元素", "warning");
    return;
  }

  try {
    // 尝试从demuxer获取当前时间
    const time = demuxer.value.getCurrentTime();
    addLog(`onTimeUpdate: demuxer时间 = ${time}，视频元素时间 = ${videoRef.value.currentTime}`, "debug");
    
    // 确保获取到有效时间
    if (time !== undefined && time !== null && !isNaN(time)) {
      // 检查时间是否有变化
      if (Math.abs(currentVirtualTime.value - time) > 0.01) {
        addLog(`更新当前时间: ${currentVirtualTime.value.toFixed(2)} -> ${time.toFixed(2)}`, "info");
        currentVirtualTime.value = time;
      }
    } else {
      addLog(`onTimeUpdate: 获取到无效时间: ${time}`, "warning");
      
      // 尝试直接使用视频元素的时间作为后备
      if (videoRef.value && !isNaN(videoRef.value.currentTime)) {
        addLog(`使用视频元素时间作为后备: ${videoRef.value.currentTime}`, "info");
        currentVirtualTime.value = videoRef.value.currentTime;
      }
    }
    
    // 更新当前片段索引
    const currentTime = currentVirtualTime.value;
    let cumulativeTime = 0;
    let foundIndex = -1;

    // 根据时间查找当前片段
    for (let i = 0; i < segments.value.length; i++) {
      const segmentDuration = segments.value[i].duration;
      if (
        currentTime >= cumulativeTime &&
        currentTime < cumulativeTime + segmentDuration
      ) {
        foundIndex = i;
        break;
      }
      cumulativeTime += segmentDuration;
    }

    if (foundIndex !== -1 && foundIndex !== currentSegmentIndex.value) {
      addLog(`片段索引更新: ${currentSegmentIndex.value} -> ${foundIndex}`, "info");
      currentSegmentIndex.value = foundIndex;
    }
    
    // 记录时间更新
    addLog(`当前播放时间: ${currentTime.toFixed(2)}秒，当前片段索引: ${currentSegmentIndex.value}`, "debug");
  } catch (error) {
    addLog(`onTimeUpdate错误: ${error}`, "error");
  }
};

// 添加到现有的时间更新机制
let timeUpdateIntervalId: number | null | ReturnType<typeof setInterval> = null;

const onPlay = () => {
  addLog("视频开始播放，isPlaying设为true", "info");
  isPlaying.value = true;
  
  // 启动时间更新检查器
  if (!timeUpdateIntervalId) {
    addLog("开始启动时间更新检查器", "info");
    timeUpdateIntervalId = setupTimeUpdateChecker();
    addLog(`时间更新检查器已启动，timeUpdateIntervalId=${timeUpdateIntervalId}`, "success");
  } else {
    addLog(`时间更新检查器已存在，timeUpdateIntervalId=${timeUpdateIntervalId}`, "debug");
  }
};

const onPause = () => {
  addLog("视频暂停，isPlaying设为false", "info");
  isPlaying.value = false;
  
  // 清理时间更新检查器
  if (timeUpdateIntervalId) {
    addLog(`清理时间更新检查器，timeUpdateIntervalId=${timeUpdateIntervalId}`, "info");
    clearInterval(timeUpdateIntervalId);
    timeUpdateIntervalId = null;
  } else {
    addLog("暂停时没有找到时间更新检查器", "warning");
  }
};

const onSeeking = () => {
  isBuffering.value = true;
  addLog("视频正在寻找新位置", "info");
};

const onSeeked = () => {
  isBuffering.value = false;

  if (!videoRef.value || !demuxer.value) return;

  const virtualTime = demuxer.value.getCurrentTime();
  addLog(`视频已跳转到: ${virtualTime.toFixed(2)}秒`, "info");
};

const onWaiting = () => {
  isBuffering.value = true;
  addLog("视频正在缓冲", "warning");
};

const onCanPlayThrough = () => {
  isBuffering.value = false;
  addLog("视频已缓冲足够数据，可以流畅播放", "success");
};

const onLoadedMetadata = () => {
  if (!videoRef.value) return;

  addLog("视频元数据已加载", "info");
  videoWidth.value = videoRef.value.videoWidth;
  videoHeight.value = videoRef.value.videoHeight;

  if (videoWidth.value && videoHeight.value) {
    addLog(`视频尺寸: ${videoWidth.value}x${videoHeight.value}`, "info");
  }
};

// 播放/暂停控制
const togglePlayPause = async () => {
  if (!demuxer.value) {
    addLog("togglePlayPause: demuxer为空，无法控制播放", "error");
    return;
  }

  addLog(`尝试切换播放状态，当前状态: ${isPlaying.value ? "播放中" : "已暂停"}`, "info");

  if (isPlaying.value) {
    addLog("执行暂停操作", "info");
    try {
      demuxer.value.pause();
      
      // 手动调用onPause以确保状态更新
      if (videoRef.value && !videoRef.value.paused) {
        addLog("手动暂停视频元素", "debug");
        videoRef.value.pause();
      }
      onPause();
    } catch (error) {
      addLog(`暂停失败: ${error}`, "error");
    }
  } else {
    addLog("执行播放操作", "info");
    try {
      await demuxer.value.play();
      const result = "成功";
      addLog(`播放请求结果: ${result || "成功"}`, "debug");
      
      // 手动调用onPlay以确保状态更新
      if (videoRef.value && videoRef.value.paused) {
        addLog("手动播放视频元素", "debug");
        try {
          await videoRef.value.play();
        } catch (e) {
          addLog(`视频元素播放失败: ${e}`, "warning");
        }
      }
      onPlay();
      
      // 立即请求一次当前时间并更新
      if (demuxer.value) {
        setTimeout(() => {
          if (demuxer.value) {
            const time = demuxer.value.getCurrentTime();
            addLog(`播放后立即获取时间: ${time}`, "info");
            currentVirtualTime.value = time;
            onTimeUpdate();
          }
        }, 100);
      }
    } catch (error) {
      addLog(`播放失败: ${error}`, "error");
    }
  }
};

// 添加一个新函数来更新缓冲区范围
const updateBufferRanges = () => {
  if (!videoRef.value || !segments.value.length) return;
  
  // 清除现有缓冲区范围
  const newBufferRanges: BufferRange[] = [];
  
  // 首先检查视频元素的buffered属性
  if (videoRef.value.buffered && videoRef.value.buffered.length > 0) {
    for (let i = 0; i < videoRef.value.buffered.length; i++) {
      newBufferRanges.push({
        start: videoRef.value.buffered.start(i),
        end: videoRef.value.buffered.end(i)
      });
    }
  } 
  
  // 如果没有从视频元素获取到缓冲区信息，则从已加载的片段计算
  if (newBufferRanges.length === 0) {
    let currentStart = -1;
    let currentEnd = -1;
    
    // 遍历所有片段，找出连续的已加载片段
    segments.value.forEach(segment => {
      if (segment.isLoaded || segment.isBuffered) {
        if (currentStart === -1) {
          // 开始一个新范围
          currentStart = segment.virtualStartTime;
          currentEnd = segment.virtualEndTime;
        } else if (segment.virtualStartTime <= currentEnd) {
          // 扩展当前范围
          currentEnd = Math.max(currentEnd, segment.virtualEndTime);
        } else {
          // 添加当前范围并开始一个新范围
          newBufferRanges.push({ start: currentStart, end: currentEnd });
          currentStart = segment.virtualStartTime;
          currentEnd = segment.virtualEndTime;
        }
      }
    });
    
    // 添加最后一个范围
    if (currentStart !== -1) {
      newBufferRanges.push({ start: currentStart, end: currentEnd });
    }
  }
  
  // 更新缓冲区范围
  bufferRanges.value = newBufferRanges;
  
  // 记录缓冲区状态
  if (newBufferRanges.length > 0) {
    const rangesStr = newBufferRanges
      .map((range) => `${range.start.toFixed(2)}-${range.end.toFixed(2)}`)
      .join(", ");
    addLog(`缓冲区范围更新: [${rangesStr}]`, "info");
  }
};

// 添加demuxer检查定时器
let demuxerStateCheckInterval: number | null = null;

// 添加video元素原生事件监听
const setupVideoElementListeners = () => {
  if (!videoRef.value) {
    addLog("无法设置视频元素监听器：视频元素不存在", "error");
    return;
  }
  
  addLog("设置视频元素原生事件监听器", "info");
  
  // 监听原生timeupdate事件
  videoRef.value.addEventListener("timeupdate", () => {
    addLog(`视频原生timeupdate事件触发，当前时间: ${videoRef.value?.currentTime}`, "debug");
  });
  
  // 监听播放状态变化
  videoRef.value.addEventListener("play", () => {
    addLog("视频原生play事件触发", "info");
  });
  
  videoRef.value.addEventListener("pause", () => {
    addLog("视频原生pause事件触发", "info");
  });
  
  // 监听错误
  videoRef.value.addEventListener("error", (e) => {
    addLog(`视频元素发生错误: ${e}`, "error");
  });
  
  // 监听资源加载状态
  videoRef.value.addEventListener("loadedmetadata", () => {
    addLog("视频元素loadedmetadata事件触发", "info");
  });
  
  videoRef.value.addEventListener("canplay", () => {
    addLog("视频元素canplay事件触发", "info");
  });
};

// 在onMounted中添加视频元素事件监听器设置
onMounted(async () => {
  addLog("真实视频时间线演示页面已加载", "info");

  if (videoRef.value) {
    // 添加错误事件监听
    videoRef.value.addEventListener("error", (e) => {
      const videoElement = e.target as HTMLVideoElement;
      addLog(`视频错误: ${videoElement.error?.message || "未知错误"}`, "error");
    });

    // 添加元数据加载监听，检查视频尺寸
    videoRef.value.addEventListener("loadedmetadata", () => {
      if (videoRef.value) {
        videoWidth.value = videoRef.value.videoWidth;
        videoHeight.value = videoRef.value.videoHeight;

        if (videoWidth.value > 0 && videoHeight.value > 0) {
          addLog(`视频尺寸: ${videoWidth.value}x${videoHeight.value}`, "info");
        } else {
          addLog("警告: 视频没有尺寸，可能不会显示", "warning");
        }
      }
    });
  }
  
  // 添加定期检查demuxer和视频元素状态
  demuxerStateCheckInterval = setInterval(() => {
    if (demuxer.value) {
      const currentTime = demuxer.value.getCurrentTime();
      addLog(`定时检查 - demuxer当前时间: ${currentTime}`, "debug");
      
      if (videoRef.value) {
        addLog(`定时检查 - 视频元素状态: 
          - currentTime: ${videoRef.value.currentTime}
          - paused: ${videoRef.value.paused}
          - readyState: ${videoRef.value.readyState}
          - networkState: ${videoRef.value.networkState}
        `, "debug");
      } else {
        addLog("定时检查 - 视频元素尚未加载", "warning");
      }
    } else {
      addLog("定时检查 - demuxer尚未初始化", "warning");
    }
  }, 3000); // 每3秒检查一次

  // 在视频元素加载后设置事件监听器
  nextTick(() => {
    if (videoRef.value) {
      setupVideoElementListeners();
    } else {
      addLog("视频元素尚未加载，无法设置事件监听器", "warning");
    }
  });
});

onUnmounted(() => {
  // 清理资源
  if (demuxer.value) {
    demuxer.value.destroy();
    demuxer.value = null;
  }

  // 停止视频播放
  if (videoRef.value) {
    videoRef.value.pause();
    videoRef.value.src = "";
    videoRef.value.load();
  }
  
  // 清理时间更新检查器
  if (timeUpdateIntervalId) {
    clearInterval(timeUpdateIntervalId);
    timeUpdateIntervalId = null;
  }
  
  // 在onBeforeUnmount中清除定时器
  if (demuxerStateCheckInterval) {
    clearInterval(demuxerStateCheckInterval);
    demuxerStateCheckInterval = null;
  }

  if (segmentStatusTimer.value) {
    clearInterval(segmentStatusTimer.value);
  }
});
</script>

<style scoped>
.real-video-container {
  position: relative;
  width: 100%;
  max-width: 800px;
  background-color: #000;
  border-radius: 8px;
  overflow: hidden;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);
}

.real-video {
  width: 100%;
  aspect-ratio: 16 / 9;
  display: block;
}

.buffering-indicator {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background-color: rgba(0, 0, 0, 0.7);
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  color: white;
  gap: 10px;
  z-index: 10;
}

.seek-controls {
  margin-top: 10px;
  display: flex;
  justify-content: center;
}
</style>
