import {
  defineComponent,
  ref,
  onMounted,
  onBeforeUnmount,
  h,
  PropType,
  computed,
  CSSProperties,
} from "vue";
import { TimeRange } from "./TimelineBase";
import "./Timeline.css";

export interface RangeTimelineProps {
  timeRanges: TimeRange[];
  currentTime: number;
  height?: number;
  backgroundColor?: string;
  segmentColor?: string;
  gapColor?: string;
  cursorColor?: string;
  onTimeUpdate?: (time: number) => void;
  onSeek?: (time: number) => void;
  showTimeLabel?: boolean;
}

export default defineComponent({
  name: "RangeTimeline",
  props: {
    timeRanges: {
      type: Array as PropType<TimeRange[]>,
      required: true,
    },
    currentTime: {
      type: Number,
      required: true,
    },
    height: {
      type: Number,
      default: 20,
    },
    backgroundColor: {
      type: String,
      default: "rgba(255, 255, 255, 0.1)",
    },
    segmentColor: {
      type: String,
      default: "rgba(24, 160, 88, 0.6)",
    },
    gapColor: {
      type: String,
      default: "rgba(0, 0, 0, 0.2)",
    },
    cursorColor: {
      type: String,
      default: "#18a058",
    },
    showTimeLabel: {
      type: Boolean,
      default: true,
    },
  },
  setup(props, { emit }) {
    const containerRef = ref<HTMLDivElement>();
    const isDragging = ref(false);
    const timeTooltip = ref<HTMLDivElement>();

    const formatTime = (seconds: number): string => {
      const date = new Date(seconds * 1000);
      return date.toLocaleString("zh-CN", {
        month: "2-digit",
        day: "2-digit",
        hour: "2-digit",
        minute: "2-digit",
        second: "2-digit",
        hour12: false,
      });
    };

    const getTimeRangeExtent = () => {
      if (!props.timeRanges.length) return { start: 0, end: 0 };
      return {
        start: Math.min(...props.timeRanges.map((r) => r.start)),
        end: Math.max(...props.timeRanges.map((r) => r.end)),
      };
    };

    const findTimeRangeForTime = (time: number) => {
      return props.timeRanges.find(
        (range) => time >= range.start && time <= range.end
      );
    };

    const handleMouseMove = (e: MouseEvent) => {
      if (!containerRef.value || !timeTooltip.value) return;
      const rect = containerRef.value.getBoundingClientRect();
      const x = e.clientX - rect.left;
      const { start, end } = getTimeRangeExtent();
      const time = start + (x / rect.width) * (end - start);

      // Update tooltip position
      timeTooltip.value.style.left = `${e.clientX}px`;
      timeTooltip.value.style.top = `${e.clientY - 25}px`;
      timeTooltip.value.textContent = formatTime(time);
      timeTooltip.value.style.display = "block";

      if (isDragging.value) {
        const range = findTimeRangeForTime(time);
        if (range) {
          emit("timeUpdate", time);
        }
      }
    };

    const handleMouseDown = (e: MouseEvent) => {
      isDragging.value = true;
      document.body.style.cursor = "grabbing";
      const rect = containerRef.value!.getBoundingClientRect();
      const x = e.clientX - rect.left;
      const { start, end } = getTimeRangeExtent();
      const time = start + (x / rect.width) * (end - start);
      const range = findTimeRangeForTime(time);
      if (range) {
        emit("seek", time);
      }
    };

    const handleMouseUp = () => {
      isDragging.value = false;
      document.body.style.cursor = "default";
      if (timeTooltip.value) {
        timeTooltip.value.style.display = "none";
      }
    };

    const handleMouseLeave = () => {
      if (timeTooltip.value) {
        timeTooltip.value.style.display = "none";
      }
      if (!isDragging.value) {
        document.body.style.cursor = "default";
      }
    };

    const handleMouseEnter = () => {
      document.body.style.cursor = "grab";
    };

    onMounted(() => {
      // Create tooltip element
      const tooltip = document.createElement("div");
      tooltip.className = "timeline-tooltip";
      document.body.appendChild(tooltip);
      timeTooltip.value = tooltip;
    });

    onBeforeUnmount(() => {
      // Remove tooltip
      if (timeTooltip.value) {
        document.body.removeChild(timeTooltip.value);
      }
    });

    const timelineStyles = computed(() => {
      const { start, end } = getTimeRangeExtent();
      const duration = end - start;
      const currentPosition = ((props.currentTime - start) / duration) * 100;

      return {
        container: {
          position: "relative",
          width: "100%",
          height: `${props.height + (props.showTimeLabel ? 20 : 0)}px`,
          userSelect: "none",
        } as CSSProperties,
        labels: {
          position: "absolute",
          top: 0,
          left: 0,
          right: 0,
          height: "16px",
          display: props.showTimeLabel ? "block" : "none",
        } as CSSProperties,
        startLabel: {
          position: "absolute",
          left: "4px",
          top: 0,
          fontSize: "10px",
          fontFamily:
            "-apple-system, BlinkMacSystemFont, Segoe UI, Roboto, sans-serif",
          color: "#666666",
        } as CSSProperties,
        endLabel: {
          position: "absolute",
          right: "4px",
          top: 0,
          fontSize: "10px",
          fontFamily:
            "-apple-system, BlinkMacSystemFont, Segoe UI, Roboto, sans-serif",
          color: "#666666",
        } as CSSProperties,
        timeline: {
          position: "absolute",
          top: props.showTimeLabel ? "20px" : 0,
          left: 0,
          right: 0,
          height: `${props.height}px`,
          backgroundColor: props.backgroundColor,
        } as CSSProperties,
        cursor: {
          position: "absolute",
          left: `${currentPosition}%`,
          top: props.showTimeLabel ? "20px" : 0,
          width: "2px",
          height: `${props.height}px`,
          backgroundColor: props.cursorColor,
          transform: "translateX(-50%)",
          boxShadow: "0 0 4px rgba(24, 160, 88, 0.5)",
        } as CSSProperties,
        cursorTriangle: {
          position: "absolute",
          left: `${currentPosition}%`,
          top: `${
            props.showTimeLabel ? 20 + props.height - 4 : props.height - 4
          }px`,
          width: 0,
          height: 0,
          borderLeft: "4px solid transparent",
          borderRight: "4px solid transparent",
          borderTop: `4px solid ${props.cursorColor}`,
          transform: "translateX(-50%)",
        } as CSSProperties,
      };
    });

    return () => {
      const { start, end } = getTimeRangeExtent();
      const duration = end - start;
      if (duration <= 0) return null;

      return h(
        "div",
        {
          ref: containerRef,
          class: "timeline-container",
          style: timelineStyles.value.container,
          onMousedown: handleMouseDown,
          onMouseup: handleMouseUp,
          onMousemove: handleMouseMove,
          onMouseleave: handleMouseLeave,
          onMouseenter: handleMouseEnter,
        },
        [
          // Time labels
          props.showTimeLabel &&
            h("div", { style: timelineStyles.value.labels }, [
              h(
                "div",
                { style: timelineStyles.value.startLabel },
                formatTime(start)
              ),
              h(
                "div",
                { style: timelineStyles.value.endLabel },
                formatTime(end)
              ),
            ]),

          // Timeline background
          h("div", { style: timelineStyles.value.timeline }, [
            // Segments
            ...props.timeRanges.map((range, i) => {
              const startPercent = ((range.start - start) / duration) * 100;
              const widthPercent = ((range.end - range.start) / duration) * 100;

              return h("div", {
                key: `segment-${i}`,
                style: {
                  position: "absolute",
                  left: `${startPercent}%`,
                  top: 0,
                  width: `${widthPercent}%`,
                  height: "100%",
                  backgroundColor: props.segmentColor,
                  border: `1px solid ${props.segmentColor.replace(
                    "0.6)",
                    "0.8)"
                  )}`,
                },
              });
            }),

            // Gaps
            ...props.timeRanges.slice(0, -1).map((_, i) => {
              const gap = {
                start: props.timeRanges[i].end,
                end: props.timeRanges[i + 1].start,
              };
              const startPercent = ((gap.start - start) / duration) * 100;
              const widthPercent = ((gap.end - gap.start) / duration) * 100;

              return h("div", {
                key: `gap-${i}`,
                style: {
                  position: "absolute",
                  left: `${startPercent}%`,
                  top: 0,
                  width: `${widthPercent}%`,
                  height: "100%",
                  background: `repeating-linear-gradient(
                  45deg,
                  ${props.gapColor},
                  ${props.gapColor} 4px,
                  transparent 4px,
                  transparent 8px
                )`,
                },
              });
            }),
          ]),

          // Cursor
          h("div", { style: timelineStyles.value.cursor }),
          h("div", { style: timelineStyles.value.cursorTriangle }),
        ]
      );
    };
  },
});
