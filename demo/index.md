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
  details: 能够在 PC\Android\iOS 浏览器Webview 内实现2秒以内低延迟直播播放。
- title: 支持多种播放格式
  details: 支持ws-raw、ws-flv、http-flv多种播放格式。
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
- title: 支持MediaSource硬解码
  details: 支持MediaSource硬解码H.264视频。(Safari on iOS不支持)。
- title: 支持Webcodecs硬解码
  details: 支持Webcodecs硬解码H.264视频。(需在chrome 94版本以上，需要https或者localhost环境),支持在WebWorker中硬解码。
- title: 支持视频录制
  details: 支持录制MP4和WebM格式(video/webm;codecs=h264)的视频录制。(基于MediaRecorder)。
- title: 支持视频录制 - 开发中
  details: 支持录制MP4格式(MPEG-4)的视频录制。
- title: 支持webrtc - 开发中
  details: 支持 webrtc标准流播放
- title: 支持语音通讯 - 开发中
  details: 支持语音通讯(支持采集PCM/G711A/G711U格式的数据、支持采样率16000Hz或8000Hz，采样精度32bits或者16bits，支持单通道或双通道)
- title: 支持倍数播放 - 开发中
  details: 支持音视频流的倍数播放
- title: 支持HLS直播协议 - 开发中
  details: 支持HLS直播协议
footer: GPL Licensed | Copyright © 2020-present dexter
---
