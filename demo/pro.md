# 简介
jessibuca pro 是在开源版本的基础上额外支持的深入业务解决方案的。解决了一些痛点，比如H265的硬解码。
-
-
-
-
-
-
-
-
-
-
-
## PRO 版本支持的特性
- wasm解码模式下默认work线程中发起Http-Flv、WS请求，减少主线程往worker线程传递数据，提升性能。
- Windows系统下,360浏览器可播放使用MSE加速解码H265。
- Windows系统下,win10商店购买hevc解码器后最新edge可硬件加速解码播放H265。
- 支持语音通讯：支持采集PCM/G711A/G711U格式的数据、支持采样率16000Hz或8000Hz，采样精度32bits或者16bits，支持单通道或双通道。
- 支持底部UI语音通讯交互。
- 支持音视频流（TF卡流）的倍数播放。
- 支持UI控件音视频流（TF卡流）的底部24小时进度条，并支持精度控制，提供事件回调，方便全屏模式下操作。
- 支持UI控件PTZ操作盘，提供事件回调，方便全屏模式下操作。
- 支持UI控件流分辨率配置和展示，提供事件回调，方便在全屏模型下操作。
- 支持HLS H264格式协议。
- 支持crypto解密播放。
- 支持webrtc标准流播放。
- 支持WebTransport协议播放。
- 支持MSE硬解码之后通过canvas渲染播放。
- 支持播放器自定义水印,支持播放器播放过程中，显示水印，例如公司名称，公司logo等。
- 支持截图加自定义水印,支持调用截图接口的时候，添加自定义水印，例如公司名称，公司logo等。
- 支持定制化服务开发。

## wasm解码模式下默认work线程中发起Http-Flv、WS请求，减少主线程往worker线程传递数据，提升性能
wasm解码模式下默认work线程中发起Http-Flv、WS请求，减少主线程往worker线程传递数据，提升性能


## Windows系统下,360浏览器可播放使用MSE加速解码H265。
Windows系统下,360浏览器可播放使用MSE加速解码H265。
## Windows系统下,win10商店购买hevc解码器后最新edge可硬件加速解码播放H265。
Windows系统下,win10商店购买hevc解码器后最新edge可硬件加速解码播放H265。

### Chromium 内核的 Edge 在 Windows 系统下，额外支持了硬解 HEVC 视频，但必须满足如下条件：

- 操作系统版本必须为 Windows 10 1709（16299.0）及以后版本。
- 安装付费的 HEVC 视频扩展或免费的来自设备制造商的 HEVC 视频扩展且版本号必须大于等于 1.0.50361.0（由于一个存在了一年半以上的 Bug，老版本存在抖动的 Bug，Issue：https://techcommunity.microsoft.com/t5/discussions/hevc-video-decoding-broken-with-b-frames/td-p/2077247/page/4）。
- 版本号必须大于等于 Edge 99 。

在安装插件后，进入 edge://gpu 页面，可以查看 Edge 对于 HEVC 硬解支持的 Profile：

### 指标：

- 分辨率最高支持 8192px * 8192px。
- 支持 HEVC Main / Main10 / Main Still Picture Profile。
### 优势：

- 在显卡支持的情况，性能是最好的。
- HTMLVideoElement、MSE 等原生 API 的直接支持。
### 劣势：

- 不支持 Windows 8 和老版本 Windows 10。
- 需要手动装插件。
- HDR 支持不够好。

## macOS系统下，Safari浏览器可播放使用MSE加速解码H265。

由于 Apple 是 HEVC 标准的主要推动者，因此早在 17 年的 Safari 11 即完成了 HEVC 视频硬解的支持，无需安装任何插件开箱即用。

### 指标：

- 分辨率最高支持 8192px * 8192px。
- 支持 HEVC Main / Main10 Profile，M1+ 机型支持部分 HEVC Rext Profile。
### 优势：

- 在显卡支持的情况，性能是最好的。
- HTMLVideoElement、MSE 等原生 API 的直接支持。
- 开箱即用，无需装插件。
- HDR 支持最好（比如：杜比视界 Profile5，杜比全景声）。
### 劣势：

- 生态不足，缺乏大量 Chromium 内核下“可用、好用的”插件。
- Safari 俗称“下一个 IE”，其浏览器 API 兼容性与实现，相比 Chromium 仍有差距。
- 部分 HEVC 视频莫名其妙无法播放，哪怕视频本身没问题。

## 支持webrtc标准流播放

### 目前存在的弊端
目前不管是http-flv 或者 websocket-flv协议 以及websocket-raw私有协议 都存在了部分延迟。

### 解决了啥问题

webrtc 超低延迟

## 支持语音通讯

- 支持采集PCM/G711A/G711U格式的数据、
- 支持采样率16000Hz或8000Hz，
- 采样精度32bits或者16bits，支持单通道或双通道

## 支持音视频流（TF卡流）的倍数播放

- 支持TF卡流的倍数播放
- 支持自定义帧率(fps)
- 支持音频变速不变调

## 支持音视频流（TF卡流）的底部24小时进度条，并支持精度控制

- 支持全屏模式下，可以查看进度条
- 支持全屏模式下，进度时间的点击交互

### 解决了啥问题

Audio Context的部分音频节点提供了playbackRate属性以实现倍速播放的功能，但是音频在倍速播放的同时，音频的音调（pitch）也同步地升高/降低。

## 支持Webtransport标准流播放

延时更低、可以秒开、弥补WebRTC不足。

## 支持播放器自定义水印

支持播放器播放过程中，显示水印，例如公司名称，公司logo等。

## 支持截图加自定义水印

支持调用截图接口的时候，添加自定义水印，例如公司名称，公司logo。

## 支持HLS H264直播流

支持播放HLS H264直播流内容


## 支持UI控件流分辨率配置和展示，提供事件回调，方便在全屏模型下操作



## 支持直播流crypto解密播放

支持直播流crypto解密播放

## 支持定制化服务开发
支持一系类和业务耦合的开发需求。

## 支持作者

<img src="/public/wx.jpg"><img src="/public/alipay.jpg">
