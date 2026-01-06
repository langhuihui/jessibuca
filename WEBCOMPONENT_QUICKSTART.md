# WebComponent Implementation - Quick Start

This document provides a quick start guide for using the new Jessibuca Lit WebComponent.

## What's New

A new framework-agnostic WebComponent has been added that wraps the Jessibuca Player. This component supports multiple protocols (HLS, ws-flv, webrtc, http-flv) and can be used in any modern web application, regardless of the framework.

## Files Added

1. **`packages/ui/src/HLSPlayerLit.ts`** - Main WebComponent implementation
2. **`packages/ui/src/GeneralPlayer.ts`** - General player for ws-flv/webrtc/http-flv
3. **`demo/src/components/WebComponent.vue`** - Vue 3 demo component
4. **`demo/webcomponent-demo.html`** - Standalone HTML demo
5. **`packages/ui/WEBCOMPONENT_README.md`** - Comprehensive documentation
6. **`WEBCOMPONENT_IMPLEMENTATION_SUMMARY.md`** - Technical implementation details

## Quick Usage

### In HTML

```html
<!DOCTYPE html>
<html>
<head>
  <script type="module">
    import 'jv4-ui';
  </script>
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
</body>
</html>
```

### In Vue 3

```vue
<template>
  <jessibuca
    :src="videoUrl"
    show-playback-rate
    show-progress
    auto-generate-ui
    @play="handlePlay"
    @pause="handlePause"
  />
</template>

<script setup>
import { ref } from 'vue';
import 'jv4-ui';

const videoUrl = ref('ws://example.com/live/stream.flv');

function handlePlay() {
  console.log('Playing');
}

function handlePause() {
  console.log('Paused');
}
</script>
```

### With JavaScript API

```javascript
const player = document.querySelector('jessibuca');

// Set source
player.src = 'https://example.com/video.m3u8';

// Control playback
await player.play();
player.pause();
player.seek(30);

// Get state
const currentTime = player.getCurrentTime();
const duration = player.getDuration();

// Listen to events
player.addEventListener('play', () => console.log('Playing'));
player.addEventListener('pause', () => console.log('Paused'));
player.addEventListener('timeupdate', (e) => {
  console.log('Time:', e.detail.currentTime);
});
```

## Features

- ✅ Framework-agnostic (works with Vue, React, Angular, or vanilla JS)
- ✅ Shadow DOM for style isolation
- ✅ Multi-protocol support (HLS, ws-flv, webrtc, http-flv)
- ✅ Custom controls (play/pause, seek, playback rate)
- ✅ Visual progress bar with buffer visualization
- ✅ Debug mode
- ✅ Time range mode (HLS only)
- ✅ Internationalization (i18n) support (Chinese, English)
- ✅ TypeScript support
- ✅ Event-driven API
- ✅ Public methods for programmatic control

## Supported Protocols

| Protocol | URL Format | Example | Description |
|----------|------------|---------|-------------|
| HLS | `http(s)://.../*.m3u8` | `https://example.com/video.m3u8` | HTTP Live Streaming |
| WebSocket FLV | `ws(s)://...` | `ws://example.com/live/stream.flv` | FLV over WebSocket |
| WebRTC | `webrtc://...` | `webrtc://example.com/live/stream` | WebRTC live stream |
| HTTP FLV | `http(s)://.../*.flv` | `https://example.com/live/stream.flv` | FLV over HTTP |

The component automatically detects the protocol from the URL, or you can manually specify it:

```html
<!-- Auto-detect protocol (default) -->
<jessibuca src="ws://example.com/live/stream.flv" auto-generate-ui></jessibuca>

<!-- Manually specify protocol -->
<jessibuca src="..." protocol="ws-flv" auto-generate-ui></jessibuca>
```

## Internationalization

The component supports multiple languages:

```html
<!-- Chinese (default) -->
<jessibuca src="video.m3u8" lang="zh-CN"></jessibuca>

<!-- English -->
<jessibuca src="video.m3u8" lang="en-US"></jessibuca>
```

You can also register custom locales:

```javascript
import { JessibucaPlayer } from 'jv4-ui';

JessibucaPlayer.registerLocale('fr-FR', {
  play: 'Lecture',
  pause: 'Pause',
  loading: 'Chargement...',
  progressLabel: 'Barre de progression vidéo',
});
```

## Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `src` | String | `''` | Video source URL (HLS m3u8) |
| `show-playback-rate` | Boolean | `true` | Show playback rate controls |
| `show-progress` | Boolean | `true` | Show progress bar |
| `auto-generate-ui` | Boolean | `true` | Auto-generate control UI |
| `debug` | Boolean | `false` | Enable debug mode |
| `lang` | String | `'zh-CN'` | UI language |
| `autoplay` | Boolean | `false` | Auto-start playback |

See `packages/ui/WEBCOMPONENT_README.md` for complete documentation.

## Building

```bash
# Build UI package
cd packages/ui
pnpm run build

# Run demo
cd demo
pnpm run dev
```

## Browser Support

- Chrome/Edge 90+
- Firefox 88+
- Safari 14+
- All modern browsers with ES Modules support

## Documentation

- **User Guide**: `packages/ui/WEBCOMPONENT_README.md`
- **Implementation Details**: `WEBCOMPONENT_IMPLEMENTATION_SUMMARY.md`
- **Vue Demo**: `demo/src/components/WebComponent.vue`
- **HTML Demo**: `demo/webcomponent-demo.html`

## Examples

Check out the demo files for complete working examples:
- Vue 3: Navigate to "WebComponent (Lit)" tab in the demo app
- HTML: Open `demo/webcomponent-demo.html` in a browser after building

## License

MIT
