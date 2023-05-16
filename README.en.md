<h1 align="center"> Jessibuca </h1>

<p align="center"> 
<b> English </b> | <a href="https://github.com/langhuihui/jessibuca/blob/v3/README.md"> 简体中文 </a>
</p>

Jessibuca is an open-source pure H5 live streaming player. 

It compiles the audio and video decoding library into Js (wasm) through Emscripten to run in the browser. It is compatible with almost all browsers and can run on PC, mobile, and WeChat without the need for additional plug-ins.

## Features

- Supports H.264 video decoding (Baseline, Main, and High Profile, supporting B frame video decoding)
- Supports H.265 video decoding (flv id == 12)
- Supports AAC audio decoding (LC, HE, and HEv2 Profile)
- Supports PCMA and PCMU audio formats decoding
- Can set the playback buffer duration, and can set 0 buffer limit for extremely low latency (network jitter may cause stuttering)
- Supports WASM smart frame discarding, ensuring no accumulated delay during long-time playback.
- Can create multiple playback instances
- Optimized programming, accelerated by CDN, GZIP compressed (actual download size is 500k), faster loading speed
- Supports both http-flv and websocket-flv protocols and websocket-raw private protocols (raw data, smaller transmission volume, need to be used with Monibuca server)
Note: when making http-flv requests, there may be cross-domain request problems, need to set access-control-allow-origin, websocket-flv does not have this problem by default
- Supports HTTPS/WSS encryption for video transmission, ensuring video content transmission security
- Video playback in mobile browsers will not become full-screen playback
- Long-time playback on mobile browsers will not turn off the screen
- Supports decoding of 8kHz PCM_ALAW, PCM_MULAW G.711 audio
- Supports 3 video scaling modes: fill, aspect ratio, and aspect ratio scaling
- Supports 0, 90, 180, 270 degree screen rotation
- Comes with a bottom UI, supporting atomized configuration of whether to display (play/pause, volume adjustment, screenshot, recording/pausing recording, full screen/cancel full screen, traffic display)
- The bottom UI is adapted to H5 mobile end and supports web full-screen (the operation bar adapts to landscape mode)
- The bottom UI supports setting it to automatically hide, only displaying when the mouse focuses on the inside of the player, and disappearing when removed
- Supports keyboard shortcuts
- Supports OffscreenCanvas to improve Webgl rendering performance
- Supports WebWorker multi-core decoding to improve multi-screen playback performance
- Supports WebCodecs hardware decoding API
- Supports WebCodecs hardware decoding configuration to display the picture through the video tag rendering or canvas tag rendering
- Supports MediaSourceExtensions hardware decoding
- Automatically switches to wasm software decoding when WebCodecs and MediaSourceExtensions hardware decoding fails
- Supports video not fluttering when the resolution changes for the same playback address (software decoding only)
- Supports automatic audio switching when the same playback address audio sampling and encoding changes
- Supports video recording (WebM, MP4 format [video/webm;codecs=h264]), (MP4 format supports playing duration on IOS VLC player, but Android VLC player cannot display it, PC VLC player can play it)

## PRO Version

- Supports almost all methods and events of the open source version, and supports seamless upgrade to the PRO version.
- In wasm decoding mode, Http-Flv and WS requests are initiated in the work thread by default to reduce data transfer from the main thread to the worker thread and improve performance.
- 360 browser can play H265 accelerated decoding using MSE.
- Latest edge can use MSE accelerated decoding for H265.
- Latest chrome version 107 and above support hardware-accelerated decoding and playback of H265 using Webcodecs.
- Supports MSE hardware decoding with intelligent frame skipping to eliminate screen flicker and frame dropping, with no accumulated delays during long-term playback.
- Supports Webcodecs hardware decoding with intelligent frame skipping to eliminate screen flicker and frame dropping, with no accumulated delays during long-term playback.
- Supports detecting network latency and can trigger playback logic again after a certain delay.
- Supports playing single video or audio data from live streaming or TF card streaming.
- Supports audio communication: supports setting PCM / G711A / G711U format data, supports setting a sampling rate of 16000Hz or 8000Hz, supports setting a sampling precision of 32bits or 16bits or 8bits, and supports setting single-channel or dual-channel.
- Supports UI control voice communication interaction buttons, providing event callbacks for easy operation in full-screen mode.
- Supports multiple playback speeds for audio and video streams (TF card streams), with control for 2/4/8/16/32/64 times, and option to only decode and play I-frames after a certain multiple.
- Supports a bottom 24-hour progress bar for TF card streams (audio and video streams), with precision control and event callbacks for easy operation in full-screen mode.
- Supports pausing and resuming TF card streams (without disconnecting, stopping rendering, facilitating business notifications to the server not to push streams, and avoiding rendering timeouts).
- Supports UI control PTZ operation disc, providing event callbacks for easy operation in full-screen mode.
- Supports UI control to directly close the player, with notification event, for easy closure of a specific player page in multi-screen mode.
- Supports UI control flow resolution configuration and display, providing event callbacks for easy operation in full-screen mode.
- Supports UI control electronic zoom, providing event callbacks for easy operation in full-screen mode.
- Supports UI control to pause the non-sending stream in TF card stream configuration, and provides event monitoring for the upper layer to cooperate with the server to stop sending the stream and implement the pause function of the non-sending stream.
- Supports switching display modes (stretch, zoom, normal) for UI controls.
- Supports HLS H264 protocol.
- Supports mirror rotation (horizontal + vertical).
- Supports crypto decryption playback.
- Supports webrtc standard stream playback.
- Supports WebTransport protocol playback.
- Supports H264 raw stream playback.
- Supports H265 raw stream playback.
- Supports rendering and playback through canvas after MSE hardware decoding.
- Supports rendering and playback through video after WASM decoding.
- Supports video recording in MP4 (MPEG-4) format, supports H264 and H265 formats, and recorded videos support seeking, fast forwarding, displaying the total duration, etc. (video recording only, audio is not currently supported).
- Supports custom watermarks for the player, displaying watermarks such as company name or logo during player playback.
- Supports adding custom watermarks during screenshot capture, such as company name or logo.
- Supports SIMD software decoding acceleration (using Chrome/Edge 91, Firefox 89, and later officially provided SIMD instruction set acceleration, bringing 100%-300% performance improvement for resolutions above 1080P).
- Supports dynamically switching resolutions during playback.
- Supports canvas rendering with webgpu (requires chrome canary version and enabling chrome://flags/#enable-unsafe-webgpu).
- Supports customized service development.

### Comparison between PRO version and open source version

[http://jessibuca.monibuca.com/pro.html#%E5%BC%80%E6%BA%90%E7%89%88%E4%B8%8Epro-%E5%B7%AE%E5%BC%82%E6%80%A7](http://jessibuca.monibuca.com/pro.html#%E5%BC%80%E6%BA%90%E7%89%88%E4%B8%8Epro-%E5%B7%AE%E5%BC%82%E6%80%A7)

## PRO AI Capability

- Face Recognition

## Experience Links

### Jessibuca Open Source Version

- https Link: [https://jessibuca.com/player.html](https://jessibuca.com/player.html)
- http Link: [http://jessibuca.monibuca.com/player.html](http://jessibuca.monibuca.com/player.html)

### Jessibuca Pro

- https Link: [https://jessibuca.com/player-pro.html](https://jessibuca.com/player-pro.html)
- http Link: [http://jessibuca.monibuca.com/player-pro.html](http://jessibuca.monibuca.com/player-pro.html)

## Performance Testing

### Test Results by User "百鸣"

[https://blog.csdn.net/huapeng_guo/article/details/124385345](https://blog.csdn.net/huapeng_guo/article/details/124385345)

## Local Testing

- Run `yarn` or `npm i`
- Run `yarn dev` or `npm run dev`

## API

[API](/demo/api.md)
## HTTP Address

[http://jessibuca.monibuca.com/](http://jessibuca.monibuca.com/)

## HTTPS Address

[https://jessibuca.com](https://jessibuca.com)

## Demo

[Demo](/demo/demo.md)

## Directory Structure of Source Code

- `wasm/obj` contains the bytecode library of the emscripten-compiled FFmpeg decoding library.
- `dist` contains the compiled output js and wasm files.
- `src` contains the source code of js.

## Packaging JS

Run `yarn build` or `npm run build`.

## Reference Relationship

- `jessibuca.js` is the business logic js code.
- `decoder.js` is the js code that runs in the worker process responsible for audio and video decoding.
- `decoder.wasm` is the glue code for `decoder.js`.

## Compiling C++ Code

Run `yarn build:wasm` or `npm run build:wasm`.

## Basic Principle

<img src="/demo/public/tech.png">

## star

[![Star History Chart](https://api.star-history.com/svg?repos=langhuihui/jessibuca&type=Date)](https://star-history.com/#langhuihui/jessibuca&Date)

## Support

<img src="/demo/public/wx.jpg">
<img src="/demo/public/alipay.jpg">

## Group

<img src="/demo/public/qrcode.jpeg">
