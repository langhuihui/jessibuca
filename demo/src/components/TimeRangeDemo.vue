<template>
  <n-space vertical>
    <n-card title="Time Range Mode Demo">
      <n-space vertical class="controls-panel">
        <!-- Time Range Configuration -->
        <n-form-item label="Time Ranges">
          <n-space vertical>
            <n-space
              v-for="(range, index) in timeRanges"
              :key="index"
              align="center"
            >
              <n-date-picker
                v-model:value="range.startDate"
                type="datetime"
                placeholder="Start Time"
                :is-date-disabled="(timestamp: number) =>
                  isDateDisabled(timestamp, range, index, 'start')"
                @update:value="updateTimeRange(index, 'start')"
              />
              <span>to</span>
              <n-date-picker
                v-model:value="range.endDate"
                type="datetime"
                placeholder="End Time"
                :is-date-disabled="(timestamp: number) =>
                  isDateDisabled(timestamp, range, index, 'end')"
                @update:value="updateTimeRange(index, 'end')"
              />
              <n-button @click="removeTimeRange(index)" type="error" circle>
                <template #icon>
                  <n-icon><trash-icon /></n-icon>
                </template>
              </n-button>
            </n-space>
            <n-button @click="addTimeRange" type="primary"
              >Add Time Range</n-button
            >
          </n-space>
        </n-form-item>

        <!-- Timeline Component -->
        <RangeTimeline
          :time-ranges="timeRanges"
          :current-time="currentTime"
          :height="40"
          :show-time-label="true"
          @time-update="(time) => onTimeUpdate(time, 'range')"
          @seek="(time) => onSeek(time, 'range')"
        />

        <!-- Debug Information -->
        <n-card title="Debug Info">
          <n-descriptions bordered>
            <n-descriptions-item label="Segment Count">
              {{ timeRanges.length }}
            </n-descriptions-item>
            <n-descriptions-item label="Current Time">
              {{ formatTime(currentTime) }}
            </n-descriptions-item>
            <n-descriptions-item label="Current Media Time">
              {{ formatMediaTime(currentMediaTime) }}
            </n-descriptions-item>
            <n-descriptions-item label="Gaps">
              {{ getGapsInfo() }}
            </n-descriptions-item>
          </n-descriptions>
        </n-card>

        <!-- Basic Timeline Comparison -->
        <n-card title="基本时间轴模式对比" style="margin-top: 20px">
          <BasicTimeline
            :current-time="getBasicCurrentTime()"
            :height="30"
            :duration="getBasicTimelineDuration()"
            @time-update="(time) => onTimeUpdate(time, 'basic')"
            @seek="(time) => onSeek(time, 'basic')"
          />
        </n-card>
      </n-space>
    </n-card>
  </n-space>
</template>

<script lang="ts" setup>
import { ref } from "vue";
import { RangeTimeline, BasicTimeline, type TimeRange } from "jv4-ui";
import {
  NSpace,
  NCard,
  NFormItem,
  NDatePicker,
  NButton,
  NIcon,
  NDescriptions,
  NDescriptionsItem,
} from "naive-ui";
import { TrashBin as TrashIcon } from "@vicons/ionicons5";

interface ExtendedTimeRange extends TimeRange {
  start: number;
  end: number;
  url: string;
  mediaStart: number;
  mediaDuration: number;
  startDate: number | null;
  endDate: number | null;
}

const currentTime = ref(0);
const currentMediaTime = ref(0);

// 初始化时间片段
const timeRanges = ref<ExtendedTimeRange[]>([
  {
    start: Date.now() / 1000,
    end: Date.now() / 1000 + 300,
    url: "test1.m3u8",
    mediaStart: 0,
    mediaDuration: 300,
    startDate: Date.now(),
    endDate: Date.now() + 300 * 1000,
  },
  {
    start: Date.now() / 1000 + 600,
    end: Date.now() / 1000 + 900,
    url: "test2.m3u8",
    mediaStart: 300,
    mediaDuration: 300,
    startDate: Date.now() + 600 * 1000,
    endDate: Date.now() + 900 * 1000,
  },
  {
    start: Date.now() / 1000 + 1200,
    end: Date.now() / 1000 + 1800,
    url: "test3.m3u8",
    mediaStart: 600,
    mediaDuration: 600,
    startDate: Date.now() + 1200 * 1000,
    endDate: Date.now() + 1800 * 1000,
  },
]);

// 添加测试数据
const basicTimeRanges = ref<TimeRange[]>([
  {
    start: Date.now() / 1000,
    end: Date.now() / 1000 + 180,
    url: "basic1.m3u8",
    mediaStart: 0,
    mediaDuration: 180,
  },
  {
    start: Date.now() / 1000 + 240,
    end: Date.now() / 1000 + 420,
    url: "basic2.m3u8",
    mediaStart: 180,
    mediaDuration: 180,
  },
]);

// 检查日期是否禁用
function isDateDisabled(
  timestamp: number,
  range: TimeRange & ExtendedTimeRange,
  index: number,
  type: "start" | "end"
): boolean {
  // 检查是否与其他时间段重叠
  for (let i = 0; i < timeRanges.value.length; i++) {
    if (i === index) continue;
    const otherRange = timeRanges.value[i];
    if (type === "start") {
      // 开始时间不能在其他时间段内
      if (
        timestamp >= (otherRange.startDate || 0) &&
        timestamp <= (otherRange.endDate || 0)
      ) {
        return true;
      }
      // 开始时间不能大于当前时间段的结束时间
      if (range.endDate && timestamp >= range.endDate) {
        return true;
      }
    } else {
      // 结束时间不能在其他时间段内
      if (
        timestamp >= (otherRange.startDate || 0) &&
        timestamp <= (otherRange.endDate || 0)
      ) {
        return true;
      }
      // 结束时间不能小于当前时间段的开始时间
      if (range.startDate && timestamp <= range.startDate) {
        return true;
      }
    }
  }
  return false;
}

// 更新时间范围
function updateTimeRange(index: number, type: "start" | "end") {
  const range = timeRanges.value[index];
  if (type === "start" && range.startDate) {
    range.start = range.startDate / 1000;
  } else if (type === "end" && range.endDate) {
    range.end = range.endDate / 1000;
  }
  range.mediaDuration = range.end - range.start;
  updateTimeRanges();
}

// 获取时间间隙信息
function getGapsInfo(): string {
  const gaps: { start: number; end: number }[] = [];
  const sortedRanges = [...timeRanges.value].sort((a, b) => a.start - b.start);

  for (let i = 0; i < sortedRanges.length - 1; i++) {
    const gap = sortedRanges[i + 1].start - sortedRanges[i].end;
    if (gap > 0) {
      gaps.push({
        start: sortedRanges[i].end,
        end: sortedRanges[i + 1].start,
      });
    }
  }
  return gaps
    .map((gap) => `${formatTime(gap.start)}-${formatTime(gap.end)}`)
    .join(", ");
}

// 格式化媒体时间显示（显示为持续时间）
function formatMediaTime(seconds: number): string {
  const hours = Math.floor(seconds / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  const secs = Math.floor(seconds % 60);
  return `${hours.toString().padStart(2, "0")}:${minutes
    .toString()
    .padStart(2, "0")}:${secs.toString().padStart(2, "0")}`;
}

// 格式化时间显示
function formatTime(seconds: number): string {
  const date = new Date(seconds * 1000);
  return date.toLocaleString("zh-CN", {
    month: "2-digit",
    day: "2-digit",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
    hour12: false,
  });
}

// 添加新的时间范围
function addTimeRange() {
  const lastRange = timeRanges.value[timeRanges.value.length - 1];
  const newStartDate = lastRange ? lastRange.endDate! + 1000 : Date.now();
  const newEndDate = newStartDate + 300 * 1000;

  timeRanges.value.push({
    start: newStartDate / 1000,
    end: newEndDate / 1000,
    url: `test${timeRanges.value.length + 1}.m3u8`,
    mediaStart: lastRange ? lastRange.mediaStart + lastRange.mediaDuration : 0,
    mediaDuration: 300,
    startDate: newStartDate,
    endDate: newEndDate,
  } as TimeRange & ExtendedTimeRange);
  updateTimeRanges();
}

// 移除时间范围
function removeTimeRange(index: number) {
  timeRanges.value.splice(index, 1);
  updateTimeRanges();
}

// 更新时间范围
function updateTimeRanges() {
  let currentMediaStart = 0;
  const sortedRanges = [...timeRanges.value].sort((a, b) => a.start - b.start);

  sortedRanges.forEach((range) => {
    range.mediaStart = currentMediaStart;
    range.mediaDuration = range.end - range.start;
    currentMediaStart += range.mediaDuration;
  });

  timeRanges.value = sortedRanges;

  // 同步更新 basicTimeRanges
  basicTimeRanges.value = [
    {
      start: sortedRanges[0].start,
      end: sortedRanges[sortedRanges.length - 1].end,
      url: "basic.m3u8",
      mediaStart: 0,
      mediaDuration:
        sortedRanges[sortedRanges.length - 1].end - sortedRanges[0].start,
    },
  ];
}

// 计算基础时间轴的总时长
const getBasicTimelineDuration = () => {
  if (!timeRanges.value || timeRanges.value.length === 0) return 0;
  // 计算所有有效时间段的总和
  return timeRanges.value.reduce((total, range) => {
    return total + (range.end - range.start);
  }, 0);
};

// 计算基础时间轴的当前时间（相对时间）
const getBasicCurrentTime = () => {
  if (!timeRanges.value || timeRanges.value.length === 0) return 0;
  const sortedRanges = [...timeRanges.value].sort((a, b) => a.start - b.start);

  // 找到当前时间所在的范围
  const currentRange = sortedRanges.find(
    (range) =>
      currentTime.value >= range.start && currentTime.value <= range.end
  );

  if (!currentRange) return 0;

  // 计算当前范围之前的所有有效时间
  let previousDuration = 0;
  for (const range of sortedRanges) {
    if (range === currentRange) break;
    previousDuration += range.end - range.start;
  }

  // 加上当前范围内的偏移量
  return previousDuration + (currentTime.value - currentRange.start);
};

// 处理时间更新
function onTimeUpdate(time: number, source: "basic" | "range" = "range") {
  if (!timeRanges.value || timeRanges.value.length === 0) return;

  const sortedRanges = [...timeRanges.value].sort((a, b) => a.start - b.start);

  if (source === "basic") {
    // 基本时间轴传入的是相对时间，需要转换为绝对时间
    let remainingTime = time;
    for (const range of sortedRanges) {
      const rangeDuration = range.end - range.start;
      if (remainingTime <= rangeDuration) {
        // 找到目标范围
        currentTime.value = range.start + remainingTime;
        break;
      }
      remainingTime -= rangeDuration;
    }
  } else {
    // 范围时间轴传入的是绝对时间，直接使用
    currentTime.value = time;
  }

  // 计算媒体时间
  const range = timeRanges.value.find(
    (r) => currentTime.value >= r.start && currentTime.value <= r.end
  );
  if (range) {
    currentMediaTime.value =
      range.mediaStart + (currentTime.value - range.start);
  }
}

// 处理跳转
function onSeek(time: number, source: "basic" | "range" = "range") {
  if (!timeRanges.value || timeRanges.value.length === 0) return;

  const sortedRanges = [...timeRanges.value].sort((a, b) => a.start - b.start);

  if (source === "basic") {
    // 基本时间轴传入的是相对时间，需要转换为绝对时间
    let remainingTime = time;
    for (const range of sortedRanges) {
      const rangeDuration = range.end - range.start;
      if (remainingTime <= rangeDuration) {
        // 找到目标范围
        currentTime.value = range.start + remainingTime;
        break;
      }
      remainingTime -= rangeDuration;
    }
  } else {
    // 范围时间轴传入的是绝对时间，直接使用
    currentTime.value = time;
  }

  // 计算媒体时间
  const range = timeRanges.value.find(
    (r) => currentTime.value >= r.start && currentTime.value <= r.end
  );
  if (range) {
    currentMediaTime.value =
      range.mediaStart + (currentTime.value - range.start);
  }
}
</script>

<style scoped>
.controls-panel {
  max-width: 800px;
  margin: 0 auto;
}

:deep(.n-form-item) {
  margin-bottom: 24px;
}

:deep(.n-date-picker) {
  width: 220px;
}

:deep(.n-card-header) {
  padding: 8px 12px;
}

:deep(.n-card) {
  margin-bottom: 12px;
}
</style>
