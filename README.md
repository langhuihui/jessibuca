<h1 align="center"> Jessibuca </h1>

<p align="center">
<a href="https://github.com/langhuihui/jessibuca/blob/v3/README.en.md"> English </a> | <b> 简体中文 </b>
</p>

Jessibuca 是一款开源的纯H5直播流播放器，通过Emscripten将音视频解码库编译成Js（wasm)运行于浏览器之中。兼容几乎所有浏览器，可以运行在PC、手机、微信中，无需额外安装插件。

## 功能

- 支持解码H.264视频(Baseline, Main, High Profile全支持，支持解码B帧视频)
- 支持解码H.265视频（flv id == 12）
- 支持[enhanced-rtmp](https://github.com/veovera/enhanced-rtmp/blob/main/enhanced-rtmp.pdf)H265格式。
- 支持解码AAC音频(LC,HE,HEv2 Profile全支持)
- 支持解码PCMA音频以及PCMU音频格式
- 可设置播放缓冲区时长，可设置0缓冲极限低延迟（网络抖动会造成卡顿现象）
- 支持WASM智能不花屏丢帧，长时间播放绝不累积延迟。
- 可创建多个播放实例
- 程序精简，经CDN加速，GZIP压缩（实际下载500k），加载速度更快
- 同时支持http-flv和websocket-flv协议以及websocket-raw私有协议（裸数据，传输量更小，需要搭配Monibuca服务器）
注：以http-flv请求时，存在跨域请求的问题，需要设置access-control-allow-origin, websocket-flv默认不存在此问题
- 支持HTTPS/WSS加密视频传输，保证视频内容传输安全
- 手机浏览器内打开视频不会变成全屏播放
- 手机浏览器内打开长时间不会息屏
- 支持解码8kHz PCM_ALAW, PCM_MULAW的G.711音频
- 支持填充，等比，等比缩放 3中视频缩放模式
- 支持0，90，180，270度画面旋转
- 自带底部UI,支持原子化配置是否显示(播放/暂停、音量调节、截屏、录制/暂停录制、全屏/取消全屏、流量显示)
- 自带底部UI适配H5移动端，并支持web端全屏(操作栏适配横屏)
- 自带底部UI支持设置成自动隐藏，只有鼠标聚焦到播放器内部才会显示，移除之后，会消失。
- 支持键盘快捷键
- 支持OffscreenCanvas，提升Webgl渲染性能
- 支持WebWorker多核解码，提升多画面播放性能
- 支持WebCodecs硬件解码API
- 支持WebCodecs硬件解码配置通过video标签渲染或者canvas标签渲染画面
- 支持MediaSourceExtensions 硬件解码
- 支持WebCodecs和MediaSourceExtensions硬解码失败的情况下自动切换到wasm软解码
- 支持同一个播放地址视频分辨率发生变化的时候视频不花屏(仅软解码)
- 支持同一个播放地址音频采样和编码发生改变的时候音频自动切换
- 支持视频录制(WebM、MP4格式[video/webm;codecs=h264])，(MP4格式支持在IOS VLC播放器显示时长播放，Android VLC播放器无法显示时长播放,PC VLC播放器可以播放)

## PRO版本

- 支持开源版几乎所有的方法和事件，支持无缝升级到PRO版本。
- wasm解码模式下默认work线程中发起Http-Flv、WS请求，减少主线程往worker线程传递数据，提升性能。
- 360浏览器可播放使用MSE加速解码H265。
- 最新edge可使用MSE加速解码H265。
- 最新chrome 107版本及以上支持使用Webcodecs硬件加速解码播放H265。
- 支持MSE硬解码智能不花屏丢帧，长时间播放绝不累积延迟。
- 支持Webcodecs硬解码智能不花屏丢帧，长时间播放绝不累积延迟。
- 支持检测网络延迟，并可以设置延迟达到一定时间段重新触发播放逻辑。
- 支持只播放直播流或者TF卡流中的单一视频或者单一音频数据。
- 支持语音通讯：支持设置采集PCM/G711A/G711U格式的数据、支持设置采样率16000Hz或8000Hz，支持设置采样精度32bits或者16bits或者8bits，支持设置单通道或双通道。
- 支持UI控件语音通讯交互按钮，提供事件回调，方便全屏模式下操作。
- 支持音视频流（TF卡流）的倍数播放，支持2/4/8/16/32/64倍数控制，支持设置多少倍之后只解码I帧播放。
- 支持UI控件音视频流（TF卡流）的底部24小时进度条，并支持精度控制，提供事件回调，方便全屏模式下操作。
- 支持TF卡流暂停(不断开连接，停止渲染，方便业务通过接口方式通知服务器不推流，不会触发页面渲染超时)和恢复播放。
- 支持UI控件PTZ操作盘，提供事件回调，方便全屏模式下操作。
- 支持UI控件直接关闭播放器，并有事件通知，方便在多屏模式下，对直接对某个播放页面进行关闭操作。
- 支持UI控件流分辨率配置和展示，提供事件回调，方便在全屏模型下操作。
- 支持UI控件电子放大，提供事件回调，方便在全屏模型下操作。
- 支持UI控件在TF卡流配置不断流暂停，并且提供事件监听，方便上层去配合服务器端停止发送流，实现不断流的暂停功能。
- 支持UI控件切换显示模式(拉伸、缩放、正常)
- 支持HLS H264格式协议。
- 支持HLS H265格式协议。
- 支持镜像旋转(水平+垂直)。
- 支持加密私有协议流 (M7S服务器)播放。
- 支持webrtc标准流播放。
- 支持webrtc(zlm服务器播放地址)流播放。
- 支持WebTransport协议播放。
- 支持H264裸流播放。
- 支持H265裸流播放。
- 支持Fmp4 H264格式播放。
- 支持Fmp4 H265格式播放。
- 支持MSE硬解码之后通过canvas渲染播放。
- 支持WASM解码之后通过video渲染播放。
- 支持WCS硬解码之后通过canvas webgl2渲染播放。
- 支持视频录制MP4(MPEG-4)格式,支持H264、H265格式，录制的视频支持seek，快进，显示总时长等（仅支持视频录制，暂不支持音频）。
- 支持视频录制Flv格式，支持H264、H265格式，录制的视频支持seek，快进，显示总时长等（vlc播放器）。
- 支持播放器自定义水印,支持播放器播放过程中，显示水印，例如公司名称，公司logo等。
- 支持截图加自定义水印,支持调用截图接口的时候，添加自定义水印，例如公司名称，公司logo等。
- 支持全屏水印，支持在播放窗口下，显示水印，例如公司名称，公司logo等。
- 支持SIMD软解码加速(使用Chrome/Edge 91, Firefox89及之后正式提供的SIMD指令集加速解码, 在1080P以上分辨率带来100%-300%的性能提升)。
- 支持播放过程中动态切换分辨率。
- 支持webgpu渲染canvas ~~（需要chrome canary 版本，然后设置chrome://flags/#enable-unsafe-webgpu 打开）~~ 最新版本chrome（>=113）默认支持。
- 支持抛出SEI数据，方便上层业务进行二次开发。
- 支持NxN路UI多屏播放(支持1x1,2x2,3x3,4x4),支持不规则多屏播放(3-1，4-1)。
- 支持国标SM4加密流播放。
- 支持XOR加密流播放。
- 支持定制化服务开发。

### PRO和开源版对比

[http://jessibuca.monibuca.com/pro.html#%E5%BC%80%E6%BA%90%E7%89%88%E4%B8%8Epro-%E5%B7%AE%E5%BC%82%E6%80%A7](http://jessibuca.monibuca.com/pro.html#%E5%BC%80%E6%BA%90%E7%89%88%E4%B8%8Epro-%E5%B7%AE%E5%BC%82%E6%80%A7)

## PRO AI 能力

- 人脸识别 [链接](https://jessibuca.com/pro-ai.html)
- 物品识别 [链接](https://jessibuca.com/pro-ai.html)
- 黑屏、绿屏、花屏、马赛克检查 [链接](https://jessibuca.com/pro-ai.html)
- 遮挡物检查 [链接](https://jessibuca.com/pro-ai.html)

## PRO 扩展模块

- Mp4 录制（MPEG-4）(支持音视频) [链接](https://jessibuca.com/pro-module.html)
- 直播流、回放(录像)流下载（录制）器 [链接](https://jessibuca.com/pro-module.html)


## LiveVideoStack 分享

- [掘金](https://juejin.cn/post/7306736066361753634)
- [CSDN](https://blog.csdn.net/wancheng815926/article/details/134705526?spm=1001.2014.3001.5502)
- [知乎](https://zhuanlan.zhihu.com/p/669605282)

## 体验地址

### Jessibuca 开源版

- https Link: [https://jessibuca.com/player.html](https://jessibuca.com/player.html)
- http Link: [http://jessibuca.monibuca.com/player.html](http://jessibuca.monibuca.com/player.html)

### Jessibuca Pro

- https Link: [https://jessibuca.com/player-pro.html](https://jessibuca.com/player-pro.html)
- http Link: [http://jessibuca.monibuca.com/player-pro.html](http://jessibuca.monibuca.com/player-pro.html)

## 性能测试情况

### 网友`百鸣`的测评

[https://blog.csdn.net/huapeng_guo/article/details/124385345](https://blog.csdn.net/huapeng_guo/article/details/124385345)


## 性能测试情况（Pro）

[单路](https://jessibuca.com/pro-doc/single.pdf)

[多路](https://jessibuca.com/pro-doc/multi.pdf)

[多路-16路-内存消耗](https://jessibuca.com/pro-doc/16multi-1080p.pdf)

[多路-12路-低延迟](https://jessibuca.com/pro-doc/12multi-delay.pdf)

[多路-24路-720p-1080p](https://jessibuca.com/pro-doc/24multi-720p-1080p.pdf)

## 本地测试

- 执行 `yarn` 或者 `npm i`
- 执行 `yarn dev` 或者 `npm run dev`

## API

[API](/demo/api.md)

## HTTP 地址

[http://jessibuca.monibuca.com/](http://jessibuca.monibuca.com/)

## HTTPS 地址

[https://jessibuca.com](https://jessibuca.com)

## DEMO

[Demo](/demo/demo.md)

## 源码目录结构

- wasm/obj 存放 emscripten 编译好的 ffmpeg 解码库的字节码库
- dist 存放编译输出的 js 和 wasm 文件
- src 存放 js 源码

## 打包 js

执行 `yarn build` 或者 `npm run build`

## 引用关系

- jessibuca.js 是业务 js 代码
- decoder.js 是 worker 进程跑的负责音视频解码的 js 代码
- decoder.wasm 是 decoder.js 的胶水代码

## 编译 C++ 代码

执行yarn build:wasm 或者 npm run build:wasm

## 基本原理

<img src="/demo/public/tech.png">

## star

[![Star History Chart](https://api.star-history.com/svg?repos=langhuihui/jessibuca&type=Date)](https://star-history.com/#langhuihui/jessibuca&Date)

## 支持

<img src="/demo/public/wx.jpg">

<img src="/demo/public/alipay.jpg">

## 群

<img src="/demo/public/qrcode.jpeg">

## 企微群
<img src="/demo/public/qrcode-qw.jpeg">

## qq频道
<img src="/public/qq-qrcode.jpg">

