<template>
  <div class="log-panel">
    <div class="log-header">
      <div class="log-title">调试日志</div>
      <div class="log-controls">
        <n-switch v-model:value="autoScroll" size="small">
          <template #checked>自动滚动</template>
          <template #unchecked>手动滚动</template>
        </n-switch>
        <n-button size="small" @click="clearLogs">清空日志</n-button>
      </div>
    </div>
    <div class="log-content" ref="logContentRef">
      <div
        v-for="(log, index) in logs"
        :key="index"
        class="log-entry"
        :class="{
          'log-entry-error': log.type === 'error',
          'log-entry-warning': log.type === 'warning',
          'log-entry-info': log.type === 'info',
          'log-entry-debug': log.type === 'debug',
          'log-entry-success': log.type === 'success',
        }"
      >
        <div class="log-timestamp">{{ formatTime(log.timestamp) }}</div>
        <div class="log-type">{{ log.type.toUpperCase() }}</div>
        <div class="log-message">{{ log.message }}</div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, watch, nextTick } from "vue";
import { NButton, NSwitch } from "naive-ui";

// 日志类型定义
type LogType = "info" | "debug" | "error" | "warning" | "success";

// 日志条目接口
interface LogEntry {
  timestamp: number;
  type: LogType;
  message: string;
}

// 属性定义
const props = defineProps<{
  maxLogs?: number;
}>();

// 事件定义
const emit = defineEmits<{
  (event: "clear"): void;
}>();

// 状态
const logs = ref<LogEntry[]>([]);
const autoScroll = ref(true);
const logContentRef = ref<HTMLDivElement | null>(null);

// 提供给父组件的方法
const addLog = (message: string, type: LogType = "info") => {
  const logEntry: LogEntry = {
    timestamp: Date.now(),
    type,
    message,
  };

  logs.value.push(logEntry);

  // 限制日志数量
  if (props.maxLogs && logs.value.length > props.maxLogs) {
    logs.value = logs.value.slice(-props.maxLogs);
  }

  // 自动滚动
  if (autoScroll.value) {
    nextTick(() => {
      scrollToBottom();
    });
  }
};

// 清空日志
const clearLogs = () => {
  logs.value = [];
  emit("clear");
};

// 滚动到底部
const scrollToBottom = () => {
  if (logContentRef.value) {
    logContentRef.value.scrollTop = logContentRef.value.scrollHeight;
  }
};

// 监听日志变化，支持自动滚动
watch(logs, () => {
  if (autoScroll.value) {
    nextTick(() => {
      scrollToBottom();
    });
  }
});

// 格式化时间
const formatTime = (timestamp: number): string => {
  const date = new Date(timestamp);
  return date.toLocaleTimeString("zh-CN", {
    hour12: false,
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
    fractionalSecondDigits: 3,
  });
};

// 暴露公共方法
defineExpose({
  addLog,
  clearLogs,
});
</script>

<style scoped>
.log-panel {
  display: flex;
  flex-direction: column;
  width: 100%;
  height: 300px;
  border: 1px solid #444;
  border-radius: 4px;
  background-color: #1a1a1a;
  color: #eee;
  font-family: "Courier New", monospace;
  overflow: hidden;
}

.log-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 8px 12px;
  background-color: #333;
  border-bottom: 1px solid #444;
}

.log-title {
  font-weight: bold;
  font-size: 14px;
  color: white;
}

.log-controls {
  display: flex;
  gap: 8px;
}

.log-content {
  flex: 1;
  overflow-y: auto;
  padding: 8px;
}

.log-entry {
  display: flex;
  align-items: flex-start;
  padding: 4px 0;
  font-size: 12px;
  line-height: 1.5;
  border-bottom: 1px solid rgba(255, 255, 255, 0.1);
}

.log-timestamp {
  min-width: 110px;
  color: #888;
  font-size: 11px;
}

.log-type {
  min-width: 70px;
  font-weight: bold;
  color: #ccc;
}

.log-message {
  flex: 1;
  word-break: break-word;
}

.log-entry-error .log-type {
  color: #ff4d4f;
}

.log-entry-warning .log-type {
  color: #faad14;
}

.log-entry-info .log-type {
  color: #1677ff;
}

.log-entry-debug .log-type {
  color: #aaa;
}

.log-entry-success .log-type {
  color: #52c41a;
}
</style>
