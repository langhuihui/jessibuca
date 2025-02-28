import {
  defineComponent,
  onMounted,
  onBeforeUnmount,
  ref,
  PropType,
  h,
  watch,
} from "vue";
import { HLSPlayer, HLSPlayerOptions, TimeRange } from "./HLSPlayer";
import BasicTimeline from "./BasicTimeline";
import RangeTimeline from "./RangeTimeline";
import "./HLSPlayer.css";

export default defineComponent({
  name: "HLSPlayerVue",
  props: {
    src: {
      type: String,
      required: false,
      default: "",
    },
    options: {
      type: Object as PropType<HLSPlayerOptions>,
      default: () => ({}),
    },
    timeRanges: {
      type: Array as PropType<TimeRange[]>,
      default: () => [],
    },
  },
  setup(props) {
    const videoRef = ref<HTMLVideoElement>();
    const player = ref<HLSPlayer>();
    const currentTime = ref(0);
    const duration = ref(0);
    const isPlaying = ref(false);
    const currentRate = ref(1);
    const hasMetadata = ref(false);
    const loadingState = ref("idle"); // idle, loading, ready

    // 添加一个定时器，确保时间轴数据定期更新
    let progressUpdateInterval: number | null = null;

    const updateProgress = () => {
      if (player.value) {
        // 获取当前时间并确保是有效数字
        const newTime = player.value.getCurrentTime();
        if (!isNaN(newTime) && isFinite(newTime) && newTime >= 0) {
          currentTime.value = newTime;
          console.log(
            `[HLSPlayerVue] 当前时间更新: ${currentTime.value.toFixed(2)}秒`
          );
        } else {
          console.log(
            `[HLSPlayerVue] 检测到无效的当前时间: ${newTime}, 保持原值: ${currentTime.value.toFixed(
              2
            )}秒`
          );
        }

        // 获取总时长并确保是有效数字
        const newDuration = player.value.getDuration();

        // Only update duration if it's valid and different from current value
        if (
          !isNaN(newDuration) &&
          isFinite(newDuration) &&
          newDuration > 0 &&
          Math.abs(newDuration - duration.value) > 0.5
        ) {
          duration.value = newDuration;
          console.log(
            `[HLSPlayerVue] 总时长更新: ${duration.value.toFixed(2)}秒`
          );
          hasMetadata.value = true;
        }

        // 记录当前时间和总时长，用于调试
        if (currentTime.value > 0 || duration.value > 0) {
          console.log(
            `[HLSPlayerVue] 时间轴数据: currentTime=${currentTime.value.toFixed(
              2
            )}秒, duration=${duration.value.toFixed(2)}秒, 进度=${(
              (currentTime.value / duration.value) *
              100
            ).toFixed(2)}%`
          );
        }
      }
    };

    // Track buffering state
    const setupBufferingListeners = () => {
      if (!videoRef.value) return;

      videoRef.value.addEventListener("waiting", () => {
        loadingState.value = "loading";
      });

      videoRef.value.addEventListener("canplay", () => {
        loadingState.value = "ready";
      });

      // Additional events to track buffering state
      videoRef.value.addEventListener("playing", () => {
        isPlaying.value = true;
        loadingState.value = "ready";
      });

      videoRef.value.addEventListener("pause", () => {
        isPlaying.value = false;
      });

      // Track buffered ranges
      videoRef.value.addEventListener("progress", () => {
        if (videoRef.value && videoRef.value.buffered.length > 0) {
          const bufferedEnd = videoRef.value.buffered.end(
            videoRef.value.buffered.length - 1
          );
          console.log(`Buffered range: 0 - ${bufferedEnd.toFixed(2)}s`);
        }
      });
    };

    onMounted(() => {
      if (videoRef.value && props.src) {
        console.log(`[HLSPlayerVue] 组件挂载，初始化播放器: ${props.src}`);

        // 初始化播放器
        player.value = new HLSPlayer(videoRef.value, {
          ...props.options,
          autoGenerateUI: false,
        });

        // 加载视频
        player.value.load(props.src).then(() => {
          console.log(`[HLSPlayerVue] 视频加载完成，初始化时间数据`);
          // 初始化时间数据
          updateProgress();

          // 设置定时器，定期更新时间数据
          progressUpdateInterval = window.setInterval(() => {
            updateProgress();
          }, 250); // 每 250ms 更新一次
        });

        // 设置播放速率
        if (
          props.options.playbackRates &&
          props.options.playbackRates.length > 0
        ) {
          currentRate.value = props.options.playbackRates[0];
        }

        // 设置缓冲状态监听
        setupBufferingListeners();
      }
    });

    // Watch for changes in the src prop
    watch(
      () => props.src,
      async (newSrc, oldSrc) => {
        if (newSrc && newSrc !== oldSrc && player.value) {
          isPlaying.value = false;
          loadingState.value = "loading";
          hasMetadata.value = false; // Reset metadata flag
          await player.value.load(newSrc);

          // Force several updates after loading
          setTimeout(updateProgress, 500);
          setTimeout(updateProgress, 1000);
          setTimeout(updateProgress, 2000);
        }
      }
    );

    onBeforeUnmount(() => {
      // 清除定时器
      if (progressUpdateInterval !== null) {
        clearInterval(progressUpdateInterval);
        progressUpdateInterval = null;
      }

      // 销毁播放器
      if (player.value) {
        player.value.destroy();
      }
    });

    const togglePlay = async () => {
      if (!player.value) return;

      if (isPlaying.value) {
        player.value.pause();
        isPlaying.value = false;
      } else {
        loadingState.value = "loading";
        await player.value.play();
        isPlaying.value = true;
      }
    };

    const handleSeek = (time: number) => {
      player.value?.seek(time);
    };

    const setPlaybackRate = (rate: number) => {
      if (player.value) {
        player.value.setPlaybackRate(rate);
        currentRate.value = rate;
      }
    };

    return () =>
      h("div", { class: "hls-player" }, [
        h("video", {
          ref: videoRef,
          class: "hls-player-video",
        }),
        h("div", { class: "hls-player-controls" }, [
          h(
            "button",
            {
              onClick: togglePlay,
            },
            isPlaying.value ? "暂停" : "播放"
          ),
          props.timeRanges.length > 0
            ? h(RangeTimeline, {
                timeRanges: props.timeRanges,
                currentTime: currentTime.value,
                height: 20,
                onTimeUpdate: handleSeek,
                onSeek: handleSeek,
              })
            : h(BasicTimeline, {
                currentTime: currentTime.value,
                duration: duration.value,
                height: 20,
                backgroundColor: "rgba(255, 255, 255, 0.1)",
                progressColor: "rgba(24, 160, 88, 0.6)",
                cursorColor: "#18a058",
                onTimeUpdate: handleSeek,
                onSeek: handleSeek,
              }),
          props.options.showPlaybackRate &&
            h(
              "div",
              { class: "hls-player-rate" },
              props.options.playbackRates?.map((rate) =>
                h(
                  "button",
                  {
                    key: rate,
                    onClick: () => setPlaybackRate(rate),
                    class: {
                      "rate-active": currentRate.value === rate,
                    },
                  },
                  `${rate}x`
                )
              )
            ),
        ]),
      ]);
  },
});
