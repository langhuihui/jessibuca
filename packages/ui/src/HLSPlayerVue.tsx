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

    const updateProgress = () => {
      if (player.value) {
        currentTime.value = player.value.getCurrentTime();
        duration.value = player.value.getDuration();
      }
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

        if (props.src) {
          await player.value.load(props.src);
        }
      }
    });

    // Watch for changes in the src prop
    watch(
      () => props.src,
      async (newSrc, oldSrc) => {
        if (newSrc && newSrc !== oldSrc && player.value) {
          isPlaying.value = false;
          await player.value.load(newSrc);
        }
      }
    );

    onBeforeUnmount(() => {
      if (videoRef.value) {
        videoRef.value.removeEventListener("timeupdate", updateProgress);
        videoRef.value.removeEventListener("durationchange", updateProgress);
      }
      player.value?.destroy();
    });

    const togglePlay = async () => {
      if (!player.value) return;

      if (isPlaying.value) {
        player.value.pause();
        isPlaying.value = false;
      } else {
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
