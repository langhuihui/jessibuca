# Jessibuca Player - Lit WebComponent

基于 Lit 框架的 Jessibuca 播放器 Web Component。这是一个独立的、可复用的视频播放器组件，支持多种流媒体协议（HLS, ws-flv, webrtc, http-flv），可以在任何 HTML 页面中使用，无需依赖 Vue 或其他框架。

## 特性

- ✅ **框架无关**: 基于 Web Components 标准，可在任何现代浏览器和框架中使用
- 🎨 **自定义样式**: 使用 Shadow DOM，完全隔离样式
- 🎬 **完整控制**: 播放/暂停、进度条、播放速率控制
- 📊 **可视化进度**: Canvas 绘制的缓冲区和播放进度可视化
- 🐛 **调试模式**: 内置调试信息显示
- 🎯 **TypeScript 支持**: 完整的类型定义
- 📱 **响应式设计**: 自适应 16:9 视频比例
- 🌐 **多协议支持**: HLS (m3u8), ws-flv, webrtc, http-flv

## 安装

```bash
pnpm add jv4-ui
# 或
npm install jv4-ui
# 或
yarn add jv4-ui
```

## 使用方法

### 1. 在 HTML 中使用

```html
<!DOCTYPE html>
<html>
<head>
  <title>Jessibuca Player Demo</title>
</head>
<body>
  <!-- HLS播放 -->
  <jessibuca
    src="https://example.com/video.m3u8"
    show-playback-rate
    show-progress
    auto-generate-ui
  ></jessibuca>

  <!-- WebSocket FLV播放 -->
  <jessibuca
    src="ws://example.com/live/stream.flv"
    auto-generate-ui
  ></jessibuca>

  <!-- WebRTC播放 -->
  <jessibuca
    src="webrtc://example.com/live/stream"
    auto-generate-ui
  ></jessibuca>

  <!-- 导入组件 -->
  <script type="module">
    import 'jv4-ui';
  </script>
</body>
</html>
```

### 2. 在 Vue 3 中使用

```vue
<template>
  <jessibuca
    :src="videoUrl"
    show-playback-rate
    show-progress
    auto-generate-ui
    debug
    @play="handlePlay"
    @pause="handlePause"
    @timeupdate="handleTimeUpdate"
  />
</template>

<script setup>
import { ref } from 'vue';
import 'jv4-ui';

const videoUrl = ref('https://example.com/video.m3u8');

function handlePlay() {
  console.log('播放开始');
}

function handlePause() {
  console.log('播放暂停');
}

function handleTimeUpdate(e) {
  console.log('当前时间:', e.detail.currentTime);
}
</script>
```

### 3. 在 React 中使用

```jsx
import { useRef, useEffect } from 'react';
import 'jv4-ui';

function VideoPlayer() {
  const playerRef = useRef(null);

  useEffect(() => {
    const player = playerRef.current;
    
    const handlePlay = () => console.log('播放开始');
    const handlePause = () => console.log('播放暂停');
    
    player.addEventListener('play', handlePlay);
    player.addEventListener('pause', handlePause);
    
    return () => {
      player.removeEventListener('play', handlePlay);
      player.removeEventListener('pause', handlePause);
    };
  }, []);

  return (
    <jessibuca
      ref={playerRef}
      src="https://example.com/video.m3u8"
      show-playback-rate
      show-progress
      auto-generate-ui
    />
  );
}
```

### 4. 使用 JavaScript API

```javascript
// 获取播放器元素
const player = document.querySelector('jessibuca');

// 设置视频源
player.src = 'https://example.com/video.m3u8';

// 播放控制
await player.play();
player.pause();
player.seek(30); // 跳转到 30 秒

// 获取状态
const currentTime = player.getCurrentTime();
const duration = player.getDuration();

// 监听事件
player.addEventListener('play', () => {
  console.log('播放开始');
});

player.addEventListener('pause', () => {
  console.log('播放暂停');
});

player.addEventListener('timeupdate', (e) => {
  console.log('时间更新:', e.detail);
});

player.addEventListener('error', (e) => {
  console.error('播放器错误:', e.detail.error);
});
```

## 属性 (Attributes)

| 属性 | 类型 | 默认值 | 描述 |
|-----|-----|-------|------|
| `src` | String | `''` | 视频源 URL (支持 m3u8, ws://, webrtc://, http-flv) |
| `protocol` | String | `'auto'` | 协议类型: `'hls'`, `'ws-flv'`, `'webrtc'`, `'http-flv'`, 或 `'auto'` (自动检测) |
| `show-playback-rate` | Boolean | `true` | 是否显示播放速率控制 |
| `show-progress` | Boolean | `true` | 是否显示进度条 |
| `auto-generate-ui` | Boolean | `true` | 是否自动生成控制 UI |
| `time-range-mode` | Boolean | `false` | 是否启用时间片段模式 (仅HLS) |
| `time-ranges` | Array | `[]` | 时间片段数组 (仅HLS) |
| `playback-rates` | Array | `[0.5, 0.75, 1, 1.25, 1.5, 2]` | 可选的播放速率 |
| `debug` | Boolean | `false` | 是否启用调试模式 |
| `show-time-ranges` | Boolean | `false` | 是否显示时间范围 (调试, 仅HLS) |
| `show-media-timeline` | Boolean | `false` | 是否显示媒体时间轴 (调试, 仅HLS) |
| `autoplay` | Boolean | `false` | 是否自动播放 |
| `lang` | String | `'zh-CN'` | UI语言 (`'zh-CN'` 或 `'en-US'`) |

## 支持的协议

| 协议 | URL格式 | 示例 | 说明 |
|-----|---------|------|------|
| HLS | `http(s)://.../*.m3u8` | `https://example.com/video.m3u8` | HTTP Live Streaming |
| WebSocket FLV | `ws(s)://...` | `ws://example.com/live/stream.flv` | FLV over WebSocket |
| WebRTC | `webrtc://...` | `webrtc://example.com/live/stream` | WebRTC 实时流 |
| HTTP FLV | `http(s)://.../*.flv` | `https://example.com/live/stream.flv` | FLV over HTTP |

组件会根据 URL 自动检测协议类型，也可以通过 `protocol` 属性手动指定。

## 方法 (Methods)

| 方法 | 参数 | 返回值 | 描述 |
|-----|-----|--------|------|
| `play()` | - | `Promise<void>` | 开始播放 |
| `pause()` | - | `void` | 暂停播放 |
| `seek(time)` | `time: number` | `void` | 跳转到指定时间（秒） |
| `getCurrentTime()` | - | `number` | 获取当前播放时间（秒） |
| `getDuration()` | - | `number` | 获取视频总时长（秒） |

## 事件 (Events)

| 事件 | 详情 (detail) | 描述 |
|-----|--------------|------|
| `play` | - | 播放开始时触发 |
| `pause` | - | 播放暂停时触发 |
| `timeupdate` | `{ currentTime: number, duration: number }` | 播放时间更新时触发 |
| `error` | `{ error: Error }` | 发生错误时触发 |

## 时间片段模式

WebComponent 支持时间片段模式，可以播放不连续的视频片段：

```javascript
const player = document.querySelector('jessibuca');

// 设置时间片段
player.timeRanges = [
  {
    start: 0,        // 片段开始时间（秒）
    end: 30,         // 片段结束时间（秒）
    url: 'video1.m3u8',
    mediaStart: 0,   // 媒体时间起点
    mediaDuration: 30 // 媒体时长
  },
  {
    start: 60,
    end: 90,
    url: 'video2.m3u8',
    mediaStart: 30,
    mediaDuration: 30
  }
];

player.timeRangeMode = true;
```

## 样式自定义

虽然 WebComponent 使用 Shadow DOM，但你可以通过 CSS 自定义变量来调整某些样式：

```css
jessibuca {
  width: 100%;
  max-width: 800px;
  --player-background: #000;
  --controls-background: rgba(0, 0, 0, 0.7);
  --primary-color: #18a058;
}
```

## 浏览器兼容性

- ✅ Chrome/Edge 90+
- ✅ Firefox 88+
- ✅ Safari 14+
- ✅ 支持 ES Modules 的现代浏览器

## 开发

```bash
# 安装依赖
pnpm install

# 开发模式
pnpm run dev

# 构建
pnpm run build

# 测试
pnpm run test
```

## 示例

查看 [webcomponent-demo.html](../../demo/webcomponent-demo.html) 获取完整的使用示例。

## 技术栈

- [Lit](https://lit.dev/) - 轻量级 Web Components 库
- [TypeScript](https://www.typescriptlang.org/) - 类型安全
- [Vite](https://vitejs.dev/) - 构建工具

## 许可证

MIT License

## 相关链接

- [Lit 官方文档](https://lit.dev/)
- [Web Components 标准](https://developer.mozilla.org/en-US/docs/Web/Web_Components)
- [Jessibuca 项目](https://github.com/langhuihui/jessibuca)
