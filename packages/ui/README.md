# Jessibuca UI Components

A Vue 3 based UI component library for the Jessibuca video player.

## Installation

```bash
pnpm add jv4-ui
```

## Components

### HLSPlayerVue

A Vue component wrapper for the HLS player with built-in UI controls.

#### Basic Usage

```vue
<template>
  <HLSPlayerVue
    src="http://example.com/video.m3u8"
    :options="{
      showPlaybackRate: true,
      showProgress: true,
      playbackRates: [0.5, 1, 1.5, 2]
    }"
  />
</template>

<script setup lang="ts">
import { HLSPlayerVue } from 'jv4-ui';
</script>
```

#### Props

| Prop | Type | Required | Default | Description |
|------|------|----------|---------|-------------|
| src | string | Yes | - | HLS stream URL |
| options | HLSPlayerOptions | No | {} | Player configuration options |

### HLSPlayer Class

A TypeScript class for direct player integration.

#### Basic Usage

```typescript
import { HLSPlayer } from 'jv4-ui';

const videoElement = document.querySelector('video');
const player = new HLSPlayer(videoElement, {
  showPlaybackRate: true,
  showProgress: true,
  playbackRates: [0.5, 1, 1.5, 2],
  autoGenerateUI: true
});

await player.load('http://example.com/video.m3u8');
await player.play();
```

#### Interface Definitions

```typescript
interface HLSPlayerOptions {
  showPlaybackRate?: boolean;      // Show playback rate controls
  showProgress?: boolean;          // Show progress bar
  playbackRates?: number[];        // Available playback rates
  autoGenerateUI?: boolean;        // Auto generate player UI
  timeRangeMode?: boolean;         // Enable time range mode
  timeRanges?: TimeRange[];        // Time ranges for segmented playback
}

interface TimeRange {
  start: number;                   // Start time in seconds
  end: number;                     // End time in seconds
  url: string;                     // Video URL for this range
}
```

#### Methods

| Method | Parameters | Return Type | Description |
|--------|------------|-------------|-------------|
| load | url: string | Promise<void> | Load an HLS stream |
| play | - | Promise<void> | Start playback |
| pause | - | void | Pause playback |
| seek | time: number | void | Seek to specific time |
| setPlaybackRate | rate: number | void | Set playback speed |
| destroy | - | void | Clean up player resources |
| getCurrentTime | - | number | Get current playback time |
| getDuration | - | number | Get video duration |
| isPlaying | - | boolean | Check if video is playing |
| getPlaybackRate | - | number | Get current playback rate |
| setTimeRanges | ranges: TimeRange[] | void | Set time ranges for segmented playback |

## Time Range Mode

The player supports a special time range mode for segmented playback:

```typescript
const player = new HLSPlayer(videoElement);

// Set time ranges
player.setTimeRanges([
  { start: 0, end: 300, url: 'video1.mp4' },
  { start: 600, end: 900, url: 'video2.mp4' },
  { start: 1200, end: 1500, url: 'video3.mp4' }
]);

// Load first segment
await player.load('video1.mp4');
```

In time range mode:
- Progress bar shows available segments
- Seeking automatically switches between segments
- Progress display shows relative time within segments

## Styling

The player comes with default styles that can be customized using CSS:

```css
.hls-player {
  /* Container styles */
}

.hls-player-video {
  /* Video element styles */
}

.hls-player-controls {
  /* Controls container styles */
}

.hls-player-progress {
  /* Progress bar styles */
}

.hls-player-rate {
  /* Playback rate controls styles */
}
```

## Browser Support

Requires browsers with support for:
- HTML5 Video
- Media Source Extensions (MSE)
- ES2020+ JavaScript features
- Vue 3.3+

## License

MIT 