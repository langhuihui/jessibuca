import {
  defineComponent,
  PropType,
  ref,
  onMounted,
  onBeforeUnmount,
  watch,
  h,
} from "vue";
import { TimelineBase, TimeRange, TimelineOptions } from "./TimelineBase";
import "./Timeline.css";

export interface TimelineProps extends TimelineOptions {
  showOriginalTimeline?: boolean;
  showMediaTimeline?: boolean;
  height?: number;
  backgroundColor?: string;
  segmentColor?: string;
  gapColor?: string;
  cursorColor?: string;
}

export default defineComponent({
  name: "Timeline",
  props: {
    timeRanges: {
      type: Array as PropType<TimeRange[]>,
      default: () => [],
    },
    showOriginalTimeline: {
      type: Boolean,
      default: true,
    },
    showMediaTimeline: {
      type: Boolean,
      default: false,
    },
    height: {
      type: Number,
      default: 20,
    },
    backgroundColor: {
      type: String,
      default: "#f5f5f5",
    },
    segmentColor: {
      type: String,
      default: "rgba(0, 160, 255, 0.3)",
    },
    gapColor: {
      type: String,
      default: "rgba(0, 0, 0, 0.1)",
    },
    cursorColor: {
      type: String,
      default: "#18a058",
    },
    onTimeUpdate: Function as PropType<(time: number) => void>,
    onSeek: Function as PropType<(time: number) => void>,
  },
  setup(props) {
    const timeline = ref<TimelineBase>();
    const originalCanvas = ref<HTMLCanvasElement>();
    const mediaCanvas = ref<HTMLCanvasElement>();
    const isDragging = ref(false);
    const timeTooltip = ref<HTMLDivElement>();
    let animationFrame: number;

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

    const formatMediaTime = (seconds: number): string => {
      const hours = Math.floor(seconds / 3600);
      const minutes = Math.floor((seconds % 3600) / 60);
      const secs = Math.floor(seconds % 60);
      return `${hours.toString().padStart(2, "0")}:${minutes
        .toString()
        .padStart(2, "0")}:${secs.toString().padStart(2, "0")}`;
    };

    const formatMediaTimeLabel = (seconds: number): string => {
      const minutes = Math.floor(seconds / 60);
      const secs = Math.floor(seconds % 60);
      return `${minutes}:${secs.toString().padStart(2, "0")}`;
    };

    const createTimeline = () => {
      timeline.value = new TimelineBase({
        timeRanges: props.timeRanges,
        onTimeUpdate: props.onTimeUpdate,
        onSeek: props.onSeek,
      });
    };

    const drawTimeLabels = (
      ctx: CanvasRenderingContext2D,
      start: number,
      end: number,
      width: number,
      height: number,
      isMediaTimeline: boolean = false
    ) => {
      ctx.save();
      ctx.fillStyle = "#666";
      ctx.font = "10px Arial";

      // Draw start time
      const startText = isMediaTimeline
        ? formatMediaTimeLabel(start)
        : formatTime(start);
      ctx.textAlign = "left";
      ctx.fillText(startText, 8, height - 3);

      // Draw end time
      const endText = isMediaTimeline
        ? formatMediaTimeLabel(end)
        : formatTime(end);
      ctx.textAlign = "right";
      ctx.fillText(endText, width - 8, height - 3);

      ctx.restore();
    };

    const findNextSegment = (time: number): TimeRange | undefined => {
      if (!timeline.value) return undefined;
      const ranges = timeline.value.getTimeRanges();

      // 找到下一个片段
      const nextSegment = ranges.find((range) => range.start > time);
      if (nextSegment) return nextSegment;

      // 如果没有下一个片段，找到最后一个片段
      return ranges[ranges.length - 1];
    };

    const handleSeekInGap = (time: number) => {
      if (!timeline.value) return;
      const nextSegment = findNextSegment(time);
      if (nextSegment) {
        timeline.value.seek(nextSegment.start);
      }
    };

    const updateOriginalCanvas = () => {
      if (!originalCanvas.value || !timeline.value) return;
      const canvas = originalCanvas.value;
      const ctx = canvas.getContext("2d");
      if (!ctx) return;

      // Set canvas size with higher resolution
      canvas.width = canvas.offsetWidth * window.devicePixelRatio;
      canvas.height = props.height * window.devicePixelRatio;
      ctx.scale(window.devicePixelRatio, window.devicePixelRatio);

      // Clear canvas
      ctx.clearRect(0, 0, canvas.width, canvas.height);

      const { start, end } = timeline.value.getTimeRangeExtent();
      const duration = end - start;
      if (duration <= 0) return;

      const canvasWidth = canvas.offsetWidth;
      const canvasHeight = canvas.offsetHeight;

      // Draw background
      ctx.fillStyle = props.backgroundColor;
      ctx.fillRect(0, 0, canvasWidth, canvasHeight);

      // Draw gaps with pattern
      ctx.fillStyle = props.gapColor;
      timeline.value.getGaps().forEach((gap) => {
        const startX = ((gap.start - start) / duration) * canvasWidth;
        const width = ((gap.end - gap.start) / duration) * canvasWidth;

        // Draw striped pattern for gaps
        const stripeWidth = 8;
        const stripeCount = Math.ceil(width / stripeWidth);
        for (let i = 0; i < stripeCount; i++) {
          const x = startX + i * stripeWidth;
          ctx.fillRect(x, 0, stripeWidth / 2, canvasHeight);
        }
      });

      // Draw segments with gradient
      timeline.value.getTimeRanges().forEach((range) => {
        const startX = ((range.start - start) / duration) * canvasWidth;
        const width = ((range.end - range.start) / duration) * canvasWidth;

        // Use solid color instead of gradient
        ctx.fillStyle = props.segmentColor;
        ctx.fillRect(startX, 0, width, canvasHeight);

        // Add border
        ctx.strokeStyle = props.segmentColor.replace("0.3)", "0.6)");
        ctx.lineWidth = 1;
        ctx.strokeRect(startX, 0, width, canvasHeight);
      });

      // Draw time labels
      drawTimeLabels(ctx, start, end, canvasWidth, canvasHeight);

      // Draw current position with glow effect
      const currentX =
        ((timeline.value.getCurrentTime() - start) / duration) * canvasWidth;

      // Draw glow
      ctx.shadowColor = props.cursorColor;
      ctx.shadowBlur = 4;
      ctx.fillStyle = props.cursorColor;
      ctx.fillRect(currentX - 1, 0, 2, canvasHeight);

      // Draw playhead triangle
      ctx.beginPath();
      ctx.moveTo(currentX - 2, canvasHeight - 4);
      ctx.lineTo(currentX + 2, canvasHeight - 4);
      ctx.lineTo(currentX, canvasHeight);
      ctx.closePath();
      ctx.fill();

      // Reset shadow
      ctx.shadowBlur = 0;
    };

    const updateMediaCanvas = () => {
      if (!mediaCanvas.value || !timeline.value) return;
      const canvas = mediaCanvas.value;
      const ctx = canvas.getContext("2d");
      if (!ctx) return;

      // Set canvas size with higher resolution
      canvas.width = canvas.offsetWidth * window.devicePixelRatio;
      canvas.height = props.height * window.devicePixelRatio;
      ctx.scale(window.devicePixelRatio, window.devicePixelRatio);

      // Clear canvas
      ctx.clearRect(0, 0, canvas.width, canvas.height);

      const totalDuration = timeline.value.getTotalDuration();
      if (totalDuration <= 0) return;

      const canvasWidth = canvas.offsetWidth;
      const canvasHeight = canvas.offsetHeight;

      // Draw background
      ctx.fillStyle = props.backgroundColor;
      ctx.fillRect(0, 0, canvasWidth, canvasHeight);

      // Draw segments with gradient
      timeline.value.getTimeRanges().forEach((range) => {
        const startX = (range.mediaStart / totalDuration) * canvasWidth;
        const width = (range.mediaDuration / totalDuration) * canvasWidth;

        // Use solid color instead of gradient
        ctx.fillStyle = props.segmentColor;
        ctx.fillRect(startX, 0, width, canvasHeight);

        // Add border
        ctx.strokeStyle = props.segmentColor.replace("0.3)", "0.6)");
        ctx.lineWidth = 1;
        ctx.strokeRect(startX, 0, width, canvasHeight);
      });

      // Draw time labels
      drawTimeLabels(ctx, 0, totalDuration, canvasWidth, canvasHeight, true);

      // Draw current position with glow effect
      const currentX =
        (timeline.value.getCurrentMediaTime() / totalDuration) * canvasWidth;

      // Draw glow
      ctx.shadowColor = props.cursorColor;
      ctx.shadowBlur = 4;
      ctx.fillStyle = props.cursorColor;
      ctx.fillRect(currentX - 1, 0, 2, canvasHeight);

      // Draw playhead triangle
      ctx.beginPath();
      ctx.moveTo(currentX - 2, canvasHeight - 4);
      ctx.lineTo(currentX + 2, canvasHeight - 4);
      ctx.lineTo(currentX, canvasHeight);
      ctx.closePath();
      ctx.fill();

      // Reset shadow
      ctx.shadowBlur = 0;
    };

    const updateCanvases = () => {
      if (props.showOriginalTimeline) {
        updateOriginalCanvas();
      }
      if (props.showMediaTimeline) {
        updateMediaCanvas();
      }
      animationFrame = requestAnimationFrame(updateCanvases);
    };

    const handleOriginalSeek = (e: MouseEvent) => {
      if (!originalCanvas.value || !timeline.value) return;
      const canvas = originalCanvas.value;
      const rect = canvas.getBoundingClientRect();
      const x = e.clientX - rect.left;
      const { start, end } = timeline.value.getTimeRangeExtent();
      const time = start + (x / rect.width) * (end - start);
      timeline.value.seek(time);
    };

    const handleMediaSeek = (e: MouseEvent) => {
      if (!mediaCanvas.value || !timeline.value) return;
      const canvas = mediaCanvas.value;
      const rect = canvas.getBoundingClientRect();
      const x = e.clientX - rect.left;
      const time = (x / rect.width) * timeline.value.getTotalDuration();
      timeline.value.seek(time, true);
    };

    const handleMouseMove = (e: MouseEvent, canvas: HTMLCanvasElement) => {
      if (!timeline.value || !timeTooltip.value) return;
      const rect = canvas.getBoundingClientRect();
      const x = e.clientX - rect.left;

      // Update tooltip position
      timeTooltip.value.style.left = `${e.clientX}px`;
      timeTooltip.value.style.top = `${e.clientY - 25}px`;

      // Calculate and update time
      const { start, end } = timeline.value.getTimeRangeExtent();
      const time = start + (x / rect.width) * (end - start);
      timeTooltip.value.textContent = formatTime(time);
      timeTooltip.value.style.display = "block";

      if (isDragging.value) {
        const range = timeline.value.findTimeRangeForTime(time);
        if (range) {
          timeline.value.seek(time);
        } else {
          handleSeekInGap(time);
        }
      }
    };

    const handleMouseDown = (e: MouseEvent) => {
      isDragging.value = true;
      document.body.style.cursor = "grabbing";
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

    const handleMediaMouseMove = (e: MouseEvent, canvas: HTMLCanvasElement) => {
      if (!timeline.value || !timeTooltip.value) return;
      const rect = canvas.getBoundingClientRect();
      const x = e.clientX - rect.left;

      // Update tooltip position
      timeTooltip.value.style.left = `${e.clientX}px`;
      timeTooltip.value.style.top = `${e.clientY - 25}px`;

      // Calculate and update time
      const time = (x / rect.width) * timeline.value.getTotalDuration();
      timeTooltip.value.textContent = formatMediaTime(time);
      timeTooltip.value.style.display = "block";

      if (isDragging.value) {
        timeline.value.seek(time, true);
      }
    };

    watch(
      () => props.timeRanges,
      (newRanges) => {
        if (timeline.value) {
          timeline.value.setTimeRanges(newRanges);
        }
      },
      { deep: true }
    );

    onMounted(() => {
      createTimeline();
      updateCanvases();
      window.addEventListener("resize", updateCanvases);

      // Create tooltip element
      const tooltip = document.createElement("div");
      tooltip.className = "timeline-tooltip";
      document.body.appendChild(tooltip);
      timeTooltip.value = tooltip;
    });

    onBeforeUnmount(() => {
      if (animationFrame) {
        cancelAnimationFrame(animationFrame);
      }
      window.removeEventListener("resize", updateCanvases);

      // Remove tooltip
      if (timeTooltip.value) {
        document.body.removeChild(timeTooltip.value);
      }
    });

    const drawPlayhead = (
      ctx: CanvasRenderingContext2D,
      x: number,
      height: number
    ) => {
      ctx.fillStyle = "#18a058";
      ctx.beginPath();
      ctx.moveTo(x, height - 4);
      ctx.lineTo(x - 2, height - 8);
      ctx.lineTo(x + 2, height - 8);
      ctx.closePath();
      ctx.fill();
    };

    const drawTimeRanges = (
      ctx: CanvasRenderingContext2D,
      ranges: TimeRange[],
      width: number,
      height: number,
      startTime: number,
      endTime: number
    ) => {
      const duration = endTime - startTime;

      // Draw background
      ctx.fillStyle = "#f5f5f5";
      ctx.fillRect(0, 0, width, height);

      // Draw ranges
      ctx.fillStyle = "rgba(24, 160, 88, 0.2)";
      ranges.forEach((range) => {
        const x = ((range.start - startTime) / duration) * width;
        const w = ((range.end - range.start) / duration) * width;
        ctx.fillRect(x, 0, w, height);
      });
    };

    const drawMediaRanges = (
      ctx: CanvasRenderingContext2D,
      ranges: TimeRange[],
      width: number,
      height: number
    ) => {
      const totalDuration = ranges.reduce(
        (acc, range) => acc + range.mediaDuration,
        0
      );

      // Draw background
      ctx.fillStyle = "#f5f5f5";
      ctx.fillRect(0, 0, width, height);

      // Draw ranges
      ctx.fillStyle = "rgba(24, 160, 88, 0.2)";
      let currentX = 0;
      ranges.forEach((range) => {
        const w = (range.mediaDuration / totalDuration) * width;
        ctx.fillRect(currentX, 0, w, height);
        currentX += w;
      });
    };

    return () =>
      h("div", { class: "timeline-container" }, [
        props.showOriginalTimeline &&
          h("div", { class: "timeline-wrapper" }, [
            h("div", { class: "timeline-label" }, "Original Timeline"),
            h(
              "div",
              {
                class: "canvas-wrapper",
                style: {
                  height: `${props.height}px`,
                  backgroundColor: props.backgroundColor,
                },
              },
              [
                h("canvas", {
                  ref: originalCanvas,
                  class: "timeline-canvas",
                  onMousedown: handleMouseDown,
                  onMouseup: handleMouseUp,
                  onMousemove: (e: MouseEvent) =>
                    handleMouseMove(e, originalCanvas.value!),
                  onMouseleave: handleMouseLeave,
                  onMouseenter: handleMouseEnter,
                }),
              ]
            ),
          ]),
        props.showMediaTimeline &&
          h("div", { class: "timeline-wrapper" }, [
            h("div", { class: "timeline-label" }, "Media Timeline"),
            h(
              "div",
              {
                class: "canvas-wrapper",
                style: {
                  height: `${props.height}px`,
                  backgroundColor: props.backgroundColor,
                },
              },
              [
                h("canvas", {
                  ref: mediaCanvas,
                  class: "timeline-canvas",
                  onMousedown: handleMouseDown,
                  onMouseup: handleMouseUp,
                  onMousemove: (e: MouseEvent) =>
                    handleMediaMouseMove(e, mediaCanvas.value!),
                  onMouseleave: handleMouseLeave,
                  onMouseenter: handleMouseEnter,
                }),
              ]
            ),
          ]),
      ]);
  },
});
