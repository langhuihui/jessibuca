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
    const loadingState = ref('idle'); // idle, loading, ready

    const updateProgress = () => {
      if (player.value) {
        currentTime.value = player.value.getCurrentTime();
        const newDuration = player.value.getDuration();
        
        // Only update duration if it's valid and different from current value
        if (!isNaN(newDuration) && isFinite(newDuration) && newDuration > 0 && Math.abs(newDuration - duration.value) > 0.5) {
          duration.value = newDuration;
          console.log(`Duration updated: ${duration.value.toFixed(2)}s`);
          hasMetadata.value = true;
        }
      }
    };

    // Track buffering state
    const setupBufferingListeners = () => {
      if (!videoRef.value) return;
      
      videoRef.value.addEventListener('waiting', () => {
        loadingState.value = 'loading';
      });
      
      videoRef.value.addEventListener('canplay', () => {
        loadingState.value = 'ready';
      });
      
      // Additional events to track buffering state
      videoRef.value.addEventListener('playing', () => {
        isPlaying.value = true;
        loadingState.value = 'ready';
      });
      
      videoRef.value.addEventListener('pause', () => {
        isPlaying.value = false;
      });
      
      // Track buffered ranges
      videoRef.value.addEventListener('progress', () => {
        if (videoRef.value && videoRef.value.buffered.length > 0) {
          const bufferedEnd = videoRef.value.buffered.end(videoRef.value.buffered.length - 1);
          console.log(`Buffered range: 0 - ${bufferedEnd.toFixed(2)}s`);
        }
      });
    };

    onMounted(async () => {
      if (videoRef.value) {
        player.value = new HLSPlayer(videoRef.value, {
          ...props.options,
          timeRanges: props.timeRanges,
          timeRangeMode: props.timeRanges.length > 0,
          autoGenerateUI: false,
        });

        videoRef.value.addEventListener("timeupdate", updateProgress);
        videoRef.value.addEventListener("durationchange", updateProgress);
        // Additional event listeners for better metadata detection
        videoRef.value.addEventListener("loadedmetadata", updateProgress);
        videoRef.value.addEventListener("loadeddata", updateProgress);
        
        // Setup buffering state tracking
        setupBufferingListeners();
        
        // Create an interval to check duration regularly, in case events don't fire
        const durationCheckInterval = setInterval(() => {
          if (player.value && !hasMetadata.value) {
            updateProgress();
          } else if (hasMetadata.value) {
            clearInterval(durationCheckInterval);
          }
        }, 1000);

        if (props.src) {
          loadingState.value = 'loading';
          await player.value.load(props.src);
          
          // Force several updates after loading to ensure UI is up-to-date
          setTimeout(updateProgress, 500);
          setTimeout(updateProgress, 1000);
          setTimeout(updateProgress, 2000);
        }
      }
    });

    // Watch for changes in the src prop
    watch(
      () => props.src,
      async (newSrc, oldSrc) => {
        if (newSrc && newSrc !== oldSrc && player.value) {
          isPlaying.value = false;
          loadingState.value = 'loading';
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
      if (videoRef.value) {
        videoRef.value.removeEventListener("timeupdate", updateProgress);
        videoRef.value.removeEventListener("durationchange", updateProgress);
        videoRef.value.removeEventListener("loadedmetadata", updateProgress);
        videoRef.value.removeEventListener("loadeddata", updateProgress);
        videoRef.value.removeEventListener("waiting", () => {});
        videoRef.value.removeEventListener("canplay", () => {});
        videoRef.value.removeEventListener("playing", () => {});
        videoRef.value.removeEventListener("pause", () => {});
        videoRef.value.removeEventListener("progress", () => {});
      }
      player.value?.destroy();
    });

    const togglePlay = async () => {
      if (!player.value) return;

      if (isPlaying.value) {
        player.value.pause();
        isPlaying.value = false;
      } else {
        loadingState.value = 'loading';
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
