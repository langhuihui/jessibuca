---
home: true
heroImage: /logo.png
heroText: Jessibuca
tagline: 纯H5直播流播放器
actionText: 打开播放器
actionLink: /player
sidebarDepth: 2
sidebar: auto
features:
- title: 无插件
  details: 基于MediaSource/WebAssembly(wasm)实现的纯JavaScript直播播放器。
- title: 低延时
  details: 能够在 PC\Android\iOS 浏览器Webview 内实现1秒以内低延迟直播播放。
- title: 支持多种播放格式
  details: 支持ws-raw、http(ws)-flv、hls、webTransport、webrtc、http(ws)-fmp4、http(ws)-h264、http(ws)-h265多种播放格式。
- title: 自带底部UI
  details: 支持原子化配置是否显示(加载中、播放/暂停、音量调节、截屏、录制/暂停录制、全屏/取消全屏、流量显示)。
- title: 支持创建多个播放实例
  details: 支持创建多个播放实例，WebWorker多核解码，提升多画面播放性能。
- title: 支持多种视频缩放模式
  details: 支持填充，等比，等比缩放 3中视频缩放模式。
- title: 支持OffscreenCanvas
  details: 提升Webgl渲染性能。
- title: 支持H264/H265
  details: 软解码H.264/H.265+AAC/G711A/G711U流，WebGL视频渲染，WebAudio音频播放。
- title: 支持MediaSource硬解码（H.264/H.265）
  details: 支持MediaSource硬解码H.264H.265视频。(Safari on iOS不支持)。
- title: 支持Webcodecs硬解码（H.264/H.265）
  details: 支持Webcodecs硬解码H.264/H.265视频。(需在chrome 94版本以上，需要https或者localhost环境),支持在WebWorker中硬解码。
- title: 支持视频录制
  details: 支持录制MP4和WebM格式(video/webm;codecs=h264)的视频录制。(基于MediaRecorder),(MP4格式支持在IOS VLC播放器显示时长播放，Android VLC播放器无法显示时长播放，PC VLC播放器可以播放)。
- title: 支持视频录制
  details: 支持录制MP4格式(MPEG-4)的视频录制（仅视频数据）。
- title: 支持webrtc
  details: 支持 webrtc标准流播放
- title: 支持语音通讯
  details: 支持语音通讯(支持采集PCM/G711A/G711U格式的数据、支持采样率16000Hz或8000Hz，采样精度32bits或者16bits，支持单通道或双通道)
- title: 支持录像流播放
  details: 支持音视频流的倍数播放
- title: 支持HLS直播协议
  details: 支持HLS直播协议
- title: 支持fmp4格式直播协议
  details: 支持fmp4格式直播协议
- title: 支持crypto解密播放
  details: 支持crypto解密播放
- title: 支持WebTransport协议播放
  details: 支持WebTransport协议播放
- title: 支持裸流播放（H.264/H.265）
  details: 支持播放（H.264/H.265）
- title: 支持音频播放器
  details: 支持音频播放器（支持手机端息屏和后台播放）
- title: 支持不规则多屏显示
  details: 支持1*1，2*2，3*3外加不规则多屏显示
- title: 支持语音通讯
  details: 支持web端语音通讯
footer: GPL Licensed | Copyright © 2020-present dexter
---
