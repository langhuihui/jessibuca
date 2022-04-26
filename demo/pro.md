## PRO 版本支持的特性

- 支持录制MP4格式(MPEG-4)的视频录制
- 支持webrtc标准流播放
- 支持语音通讯
- 支持音视频流的倍数播放
- 支持Webtransport标准流播放
- 支持打包成单一文件能力

## 支持录制MP4格式(MPEG-4)的视频录制

### 目前存在的弊端
目前支持视频录制(WebM、MP4格式)，

- MP4格式支持在IOS VLC播放器显示时长播放
- Android VLC播放器无法显示时长播放
- PC VLC播放器可以播放


### 解决了啥问题

支持全PC IOS Android 默认浏览器打开。



## 支持webrtc标准流播放

### 目前存在的弊端
目前不管是http-flv 或者 websocket-flv协议 以及websocket-raw私有协议 都存在了部分延迟。

### 解决了啥问题

webrtc 超低延迟

## 支持语音通讯

- 支持采集PCM/G711A/G711U格式的数据、
- 支持采样率16000Hz或8000Hz，
- 采样精度32bits或者16bits，支持单通道或双通道


## 支持音视频流的倍数播放

- 支持TF卡流的倍数播放
- 支持自定义帧率(fps)
- 支持音频变速不变调

### 解决了啥问题

Audio Context的部分音频节点提供了playbackRate属性以实现倍速播放的功能，但是音频在倍速播放的同时，音频的音调（pitch）也同步地升高/降低。


## 支持Webtransport标准流播放

延时更低、可以秒开、弥补WebRTC不足


## 支持打包成单一文件能力

支持 js 和 wasm胶水文件打包成单一文件，方便npm安装。
