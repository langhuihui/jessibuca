# Jessibuca Open Source API (for agents)

This file is the machine-readable API spec for the **open-source** Jessibuca H5 live player.
Use it to write working player code. Do not invent Pro-only APIs (`JessibucaPro`, `decoder-pro.js`, VR, AI, talk).

- HTML docs: `/api.html`
- TypeScript types (may lag this file): `/jessibuca.d.ts`
- Demo: `/player.html`

## Identity

- Global constructor: `window.Jessibuca`
- Instance methods mutate one player bound to one DOM container
- Supported open-source protocols: `ws-raw`, `ws-flv`, `http(s)-flv`
- Codecs: H.264 / H.265 via wasm; MSE / WebCodecs hard decode is H.264 only
- `decoder.js` and `decoder.wasm` MUST live in the same directory

## Hard rules when generating code

1. Wait until `typeof window.Jessibuca === 'function'` before `new Jessibuca(...)`.
2. `container` is required. String values use `document.querySelector`. Do not pass a `<canvas>` or `<video>` node. Do not CSS-transform / rotate / scale the container.
3. Call `play(url)` after construct. `play()` returns `Promise<void>` — always `.catch()`.
4. Release with `await player.destroy()`. Do not use deprecated `close()`.
5. Browser autoplay policy: default is muted. Resume audio only from a user gesture via `cancelMute()` or `audioResume()`.
6. Compare errors with `Jessibuca.ERROR.*` and timeouts with `Jessibuca.TIMEOUT.*`, not guessed strings.
7. Relative `decoder` paths resolve against the **page URL**, not the JS file URL.

## Minimal working example

```html
<div id="player-wrap">
  <div id="container" style="width:640px;height:360px;background:#000;"></div>
</div>
<button id="play">play</button>
<button id="stop">stop</button>
<script src="/jessibuca.js"></script>
<script>
  const player = new Jessibuca({
    container: '#container',
    decoder: '/decoder.js',
    videoBuffer: 0.2,
    isResize: true,
    debug: false,
    useMSE: false,
    useWCS: false,
    autoWasm: true,
    operateBtns: {
      fullscreen: true,
      screenshot: true,
      play: true,
      audio: true,
      record: true
    }
  })

  document.getElementById('play').onclick = () => {
    player.play('https://example.com/live/test.flv').catch((err) => {
      console.error(err)
    })
  }

  document.getElementById('stop').onclick = async () => {
    await player.destroy()
  }

  player.on('error', (error) => {
    if (error === Jessibuca.ERROR.fetchError) console.error('http fetch failed')
    if (error === Jessibuca.ERROR.websocketError) console.error('websocket failed')
  })

  player.on('timeout', (type) => {
    console.warn('timeout', type)
  })
</script>
```

## Constructor

```ts
new Jessibuca(options: Config): Jessibuca
```

`options.container` is required. All other fields are optional.

### Config

| Name | Type | Default | Meaning |
| --- | --- | --- | --- |
| `container` | `HTMLElement \| string` | required | Player mount node or CSS selector |
| `decoder` | `string` | `'decoder.js'` | Worker script URL; `decoder.wasm` must be beside it |
| `videoBuffer` | `number` | `1` | Buffer seconds |
| `videoBufferDelay` | `number` | `1` | Extra delay seconds before dropping frames |
| `hiddenAutoPause` | `boolean` | `false` | Pause when `document.visibilityState === 'hidden'` |
| `hasAudio` | `boolean` | `true` | Decode audio. `false` skips audio and is faster |
| `rotate` | `number` | `0` | Constructor rotation: `0` \| `180` \| `270` |
| `isResize` | `boolean` | `true` | Contain (letterbox). Same as `setScaleMode(1)` |
| `isFullResize` | `boolean` | `false` | Cover (crop). Same as `setScaleMode(2)` |
| `isFlv` | `boolean` | `false` | Force FLV parse for websocket even without `.flv` |
| `debug` | `boolean` | `false` | Console logs |
| `timeout` | `number` | `10` | Seconds for both loading and heartbeat |
| `heartTimeout` | `number` | `10` | Seconds without frames while playing |
| `heartTimeoutReplay` | `boolean` | `true` | Replay after heartbeat timeout if URL is reachable |
| `heartTimeoutReplayTimes` | `number` | `3` | Replay attempts. `-1` = infinite |
| `loadingTimeout` | `number` | `10` | Seconds with no data after `play()` |
| `loadingTimeoutReplay` | `boolean` | `true` | Replay after loading timeout if URL is reachable |
| `loadingTimeoutReplayTimes` | `number` | `3` | Replay attempts. `-1` = infinite |
| `loadingDecoderWorkerTimeout` | `number` | `10` | Seconds to load decoder worker |
| `supportDblclickFullscreen` | `boolean` | `false` | Double-click video (not control bar) toggles fullscreen |
| `showBandwidth` | `boolean` | `false` | Show bitrate on UI |
| `operateBtns` | `object` | all `false` | `{ fullscreen, screenshot, play, audio, record }` |
| `keepScreenOn` | `boolean` | `false` | Mobile screen wake lock |
| `isNotMute` | `boolean` | `false` | Start unmuted. Still needs a user gesture to actually hear audio |
| `loadingText` | `string` | `''` | Loading copy |
| `background` | `string` | `''` | Poster / background image URL |
| `useMSE` | `boolean` | `false` | MSE hard decode (H.264, not iOS Safari). Beats `useWCS` |
| `useWCS` | `boolean` | `false` | WebCodecs hard decode (H.264, Chrome 94+, https/localhost) |
| `wcsUseVideoRender` | `boolean` | `true` | Render WebCodecs frames with `<video>` |
| `autoWasm` | `boolean` | `true` | Fall back to wasm when hard decode cannot play H.265 |
| `hotKey` | `boolean` | `false` | `Esc` exit fullscreen, `ArrowUp`/`ArrowDown` volume |
| `wasmDecodeErrorReplay` | `boolean` | — | Replay after wasm decode error |
| `controlAutoHide` | `boolean` | `false` | Auto-hide control bar |
| `recordType` | `'webm' \| 'mp4'` | `'webm'` | Recording container |
| `useWebFullScreen` | `boolean` | `false` | CSS rotate 90° fullscreen, mobile / iOS |
| `autoUseSystemFullScreen` | `boolean` | — | Prefer system fullscreen when available |
| `forceNoOffscreen` | `boolean` | `true` | Deprecated. Offscreen canvas is disabled internally |
| `openWebglAlignment` | `boolean` | — | Fix green screen when UV width is not divisible by 4 |

MSE / WebCodecs + `<video>` rendering does **not** support `isResize` / `isFullResize` / `setScaleMode` / `clearView`.

Replay options do not retry HTTP 4xx/5xx or websocket connect failure; those go to `error`.

## Methods

| Signature | Returns | Notes |
| --- | --- | --- |
| `play(url?: string, options?: { headers?: Record<string,string> })` | `Promise<void>` | Resolves when play starts. `headers` only for HTTP |
| `pause()` | `Promise<void>` | Later `play()` without URL resumes the same stream |
| `destroy()` | `Promise<void>` | Tear down and free wasm/worker/canvas |
| `close()` | `Promise<void>` | **Deprecated.** Same as `destroy()` |
| `mute()` | `void` | Stops audio path (cheaper than volume 0) |
| `cancelMute()` | `void` | Unmute. Call from a click/tap |
| `audioResume()` | `void` | Unlock audio from a user gesture |
| `setVolume(volume: number)` | `void` | `0..1`. Does not skip audio decode |
| `setDebug(flag: boolean)` | `void` | |
| `setTimeout(time: number)` | `void` | Seconds |
| `setBufferTime(time: number)` | `void` | Same as `videoBuffer` |
| `setScaleMode(mode: 0 \| 1 \| 2)` | `void` | `0` fill+stretch, `1` contain, `2` cover |
| `setRotate(deg: number)` | `void` | `0` \| `90` \| `180` \| `270` |
| `setFullscreen(flag: boolean)` | `void` | iOS: prefer `useWebFullScreen` |
| `setKeepScreenOn()` | `boolean` | |
| `resize()` | `void` | Call after container size / CSS fullscreen changes |
| `clearView()` | `void` | Paint canvas black |
| `screenshot(filename?: string, format?: 'png'\|'jpeg'\|'webp', quality?: number, type?: 'download'\|'base64'\|'blob')` | `void \| string \| Blob` | Default type `download`. After `start`, wait ~1s before first screenshot |
| `startRecord(fileName?: string, fileType?: 'webm'\|'mp4')` | `void` | |
| `stopRecordAndSave()` | `void` | Stops and downloads |
| `isPlaying()` | `boolean` | |
| `isMute()` | `boolean` | |
| `isRecording()` | `boolean` | |
| `hasLoaded()` | `boolean` | Legacy 2.x. Not required in 3.x |
| `toggleControlBar(isShow?: boolean)` | `void` | Omit argument to toggle |
| `getControlBarShow()` | `boolean` | |
| `kbps2Speed(kbps: number)` | `number` | kbps → KB/s |
| `on(event: string, callback: Function)` | `void` | See events |

## Events

Register with `player.on(name, cb)`.

| Event | Callback payload | When |
| --- | --- | --- |
| `load` | `()` | Decoder ready. Optional in 3.x; you may `play()` immediately |
| `play` | `()` | Playing |
| `pause` | `()` | Paused |
| `start` | `()` | First frame render |
| `mute` | `(muted: boolean)` | Mute state |
| `volume` | `(volume: number)` | Volume `0..1` |
| `fullscreen` | `(flag: boolean)` | System fullscreen |
| `webFullscreen` | `(flag: boolean)` | CSS web fullscreen |
| `timeUpdate` | `(ts: number)` | Frame PTS, milliseconds |
| `videoInfo` | `({ width, height, encTypeCode, encType })` | `encTypeCode`: `10` H.264, `12` H.265 |
| `audioInfo` | `({ numOfChannels, sampleRate, encTypeCode, encType })` | `10` AAC, `7` G711A, `8` G711U |
| `kBps` | `(kbps: number)` | Stream bitrate, once per second. Use `kbps2Speed()` for KB/s |
| `stats` | `({ buf, fps, abps, vbps, ts })` | Once per second. `buf`/`ts` ms; `abps`/`vbps` bytes |
| `performance` | `(level: 0 \| 1 \| 2)` | `0` stutter, `1` ok, `2` smooth |
| `log` | `(msg: any)` | Debug / error text |
| `error` | `(error: string)` | Compare with `Jessibuca.ERROR.*` |
| `timeout` | `(type: string)` | Compare with `Jessibuca.TIMEOUT.*` |
| `loadingTimeout` | `()` | No data after `play()` |
| `delayTimeout` | `()` | No frames during playback |
| `recordStart` | `()` | |
| `recordEnd` | `()` | |
| `recordingTimestamp` | `(timestamp: number)` | Recording duration, once per second |
| `playToRenderTimes` | `(times: PlayToRenderTimes)` | Timestamps for init → network → demux → decode → render |

### `PlayToRenderTimes`

```ts
{
  playInitStart: number
  playStart: number
  streamStart: number
  streamResponse: number
  demuxStart: number
  decodeStart: number
  videoStart: number
  playTimestamp: number
  streamTimestamp: number
  streamResponseTimestamp: number
  demuxTimestamp: number
  decodeTimestamp: number
  videoTimestamp: number
  allTimestamp: number
}
```

## Constants

Use the class statics. Do not hard-code error strings.

```js
Jessibuca.ERROR
Jessibuca.TIMEOUT
```

| Constant | Typical meaning |
| --- | --- |
| `Jessibuca.ERROR.playError` | `play()` with empty URL / invalid state. Value is `playIsNotPauseOrUrlIsNull` |
| `Jessibuca.ERROR.fetchError` | HTTP(s) request failed |
| `Jessibuca.ERROR.websocketError` | Websocket failed |
| `Jessibuca.ERROR.webcodecsH265NotSupport` | WebCodecs cannot decode H.265 |
| `Jessibuca.ERROR.mediaSourceH265NotSupport` | MSE cannot decode H.265 |
| `Jessibuca.ERROR.wasmDecodeError` | wasm decode failed |
| `Jessibuca.TIMEOUT.loadingTimeout` | No data after play |
| `Jessibuca.TIMEOUT.delayTimeout` | Playback stalled |

Other `error` values that may appear: `webcodecsConfigureError`, `webcodecsDecodeError`, `webcodecsWidthOrHeightChange`, `mediaSourceFull`, `mseSourceBufferError`, `mediaSourceAppendBufferError`, `webglAlignmentError`, `webglContextLostError`, `webglInitError`.

## Recipes

### Vue / React: create on mount, destroy on unmount

```js
let player
onMounted(() => {
  player = new window.Jessibuca({ container: el, decoder: '/decoder.js' })
})
onBeforeUnmount(async () => {
  if (player) {
    await player.destroy()
    player = null
  }
})
```

### HTTP-FLV with auth header

```js
await player.play(url, { headers: { Authorization: 'Bearer xxx' } })
```

### Prefer hard decode, fall back to wasm

```js
new Jessibuca({
  container: '#container',
  decoder: '/decoder.js',
  useMSE: true,
  useWCS: true,
  autoWasm: true
})
```

### Screenshot / record

```js
player.on('start', () => {
  setTimeout(() => {
    const dataUrl = player.screenshot('shot', 'png', 0.92, 'base64')
  }, 1000)
})
player.startRecord('clip', 'mp4')
player.stopRecordAndSave()
```

### Nested container (safe CSS)

```html
<div id="app-shell"><!-- app styles / transform live here -->
  <div id="container"></div>
</div>
```

```js
new Jessibuca({ container: '#container' })
```

## Out of scope (open-source)

Do not generate: `JessibucaPro`, webrtc / HLS / fmp4 / mpeg-ts / mpeg-ps / WebTransport play URLs, SIMD / multi-thread wasm, PTZ, AI, talk, VR, SM4 crypto, vod seek. Those belong to Pro. See `/player-pro.html`.
