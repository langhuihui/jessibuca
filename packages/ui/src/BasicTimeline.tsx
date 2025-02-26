import {
  defineComponent,
  ref,
  onMounted,
  onBeforeUnmount,
  h,
  computed,
  CSSProperties,
} from "vue";
import { TimelineBaseProps } from "./TimelineBase";
import "./Timeline.css";

export interface BasicTimelineProps extends TimelineBaseProps {
  duration: number;
  progressColor?: string;
}

export default defineComponent({
  name: "BasicTimeline",
  props: {
    currentTime: {
      type: Number,
      required: true,
    },
    duration: {
      type: Number,
      required: true,
    },
    height: {
      type: Number,
      default: 20,
    },
    backgroundColor: {
      type: String,
      default: "rgba(0, 0, 0, 0.1)",
    },
    progressColor: {
      type: String,
      default: "rgba(24, 160, 88, 0.3)",
    },
    cursorColor: {
      type: String,
      default: "#18a058",
    },
  },
  setup(props, { emit }) {
    const containerRef = ref<HTMLDivElement>();
    const isDragging = ref(false);
    const timeTooltip = ref<HTMLDivElement>();

    const formatTime = (seconds: number): string => {
      const hours = Math.floor(seconds / 3600);
      const minutes = Math.floor((seconds % 3600) / 60);
      const secs = Math.floor(seconds % 60);
      return `${hours.toString().padStart(2, "0")}:${minutes
        .toString()
        .padStart(2, "0")}:${secs.toString().padStart(2, "0")}`;
    };

    const handleMouseMove = (e: MouseEvent) => {
      if (!containerRef.value || !timeTooltip.value) return;
      const rect = containerRef.value.getBoundingClientRect();
      const x = e.clientX - rect.left;
      const time = (x / rect.width) * props.duration;

      // Update tooltip position
      timeTooltip.value.style.left = `${e.clientX}px`;
      timeTooltip.value.style.top = `${e.clientY - 25}px`;
      timeTooltip.value.textContent = formatTime(time);
      timeTooltip.value.style.display = "block";

      if (isDragging.value) {
        emit("timeUpdate", time);
      }
    };

    const handleMouseDown = (e: MouseEvent) => {
      isDragging.value = true;
      document.body.style.cursor = "grabbing";
      const rect = containerRef.value!.getBoundingClientRect();
      const x = e.clientX - rect.left;
      const time = (x / rect.width) * props.duration;
      emit("seek", time);
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
      const currentPosition = (props.currentTime / props.duration) * 100;

      return {
        container: {
          position: "relative",
          width: "100%",
          height: `${props.height + 20}px`,
          userSelect: "none",
        } as CSSProperties,
        labels: {
          position: "absolute",
          top: 0,
          left: 0,
          right: 0,
          height: "16px",
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
          top: "20px",
          left: 0,
          right: 0,
          height: `${props.height}px`,
          backgroundColor: props.backgroundColor,
          overflow: "hidden",
        } as CSSProperties,
        progress: {
          position: "absolute",
          left: 0,
          top: 0,
          width: `${currentPosition}%`,
          height: "100%",
          backgroundColor: props.progressColor,
          borderRight: `1px solid ${props.progressColor.replace(
            "0.3)",
            "0.6)"
          )}`,
        } as CSSProperties,
        cursor: {
          position: "absolute",
          left: `${currentPosition}%`,
          top: "20px",
          width: "2px",
          height: `${props.height}px`,
          backgroundColor: props.cursorColor,
          transform: "translateX(-50%)",
          boxShadow: "0 0 4px rgba(24, 160, 88, 0.5)",
        } as CSSProperties,
        cursorTriangle: {
          position: "absolute",
          left: `${currentPosition}%`,
          top: `${20 + props.height - 4}px`,
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
          h("div", { style: timelineStyles.value.labels }, [
            h("div", { style: timelineStyles.value.startLabel }, formatTime(0)),
            h(
              "div",
              { style: timelineStyles.value.endLabel },
              formatTime(props.duration)
            ),
          ]),

          // Timeline background with progress
          h("div", { style: timelineStyles.value.timeline }, [
            h("div", { style: timelineStyles.value.progress }),
          ]),

          // Cursor
          h("div", { style: timelineStyles.value.cursor }),
          h("div", { style: timelineStyles.value.cursorTriangle }),
        ]
      );
    };
  },
});
