
# 简介
jessibuca pro 是在开源版本的基础上额外支持的深入业务解决方案的。解决了一些痛点，比如H265的硬解码，SIMD软解码加速。
-
-
-
-
-
-
-
-
-
<Rice/>
## 开源版与PRO 差异性

|                                                               | 开源版   | Pro |
|---------------------------------------------------------------|-------| --- |
| H.264 WASM视频(720P)                                            | 支持    | 支持 |
| H.265 WASM视频(720P)                                            | 支持    | 支持 |
| H.264 WASM多线程视频(1080P及以上)                                     | 不支持   | 支持 |
| H.265 WASM多线程视频(1080P及以上)                                     | 不支持   | 支持 |
| H.264 WASM SIMD视频(1080P及以上)                                   | 不支持 | 支持 |
| H.265 WASM SIMD视频(1080P及以上)                                   | 不支持   | 支持 |
| H.264 WASM SIMD多线程视频(1080P及以上)                                | 不支持   | 支持 |
| H.265 WASM SIMD多线程视频(1080P及以上)                                | 不支持   | 支持 |
| Mpeg4 视频格式软解码                                                 | 不支持   | 支持 |
| WASM(SIMD)支持webgl canvas渲染                                    | 支持   | 支持 |
| WASM(SIMD)支持 video 渲染                                         | 不支持   | 支持 |
| WASM智能不花屏丢帧，长时间播放绝不累积延迟                                       | 支持   | 支持 |
| WASM 离屏渲染                                                     | 支持    | 支持 |
| 支持浏览器打开窗口立即播放视频                                               | 支持    | 支持 |
| H.264 MSE解码视频                                                 | 支持    | 支持 |
| H.265 MSE解码视频                                                 | 不支持   | 支持 |
| MSE支持Worker线程解码H.264/H.265音视频                                 | 不支持   | 支持 |
| MSE智能不花屏丢帧，长时间播放绝不累积延迟                                        | 不支持   | 支持 |
| MSE解码失败自动切换到WASM解码                                            | 支持    | 支持 |
| H.264 WCS解码视频                                                 | 支持    | 支持 |
| H.265 WCS解码视频                                                 | 不支持   | 支持 |
| wasm支持webgl canvas渲染                                          | 支持   | 支持 |
| wasm支持webgpu canvas渲染                                         | 不支持   | 支持 |
| wasm支持video渲染                                                 | 不支持   | 支持 |
| MSE支持canvas渲染                                                 | 不支持   | 支持 |
| MSE支持video渲染                                                  | 支持   | 支持 |
| WCS支持canvas渲染                                                 | 支持   | 支持 |
| WCS支持canvas webgl2渲染                                          | 不支持   | 支持 |
| WCS支持video渲染                                                  | 不支持   | 支持 |
| WCS智能不花屏丢帧，长时间播放绝不累积延迟                                        | 不支持   | 支持 |
| WCS解码失败自动切换到WASM解码                                            | 支持    | 支持 |
| WCS 离屏渲染                                                      | 支持    | 支持 |
| WebRTC支持canvas渲染                                              | 不支持   | 支持 |
| AAC音频                                                         | 支持    | 支持 |
| PCMA(g711a)、PCMU(g711u)音频                                     | 支持    | 支持 |
| MP3音频                                                         | 不支持   | 支持 |
| 音频worlet引擎（https）                                             | 不支持   | 支持 |
| 音频script引擎 (默认)                                               | 支持    | 支持 |
| 音频active引擎（兼容性更强）                                             | 不支持   | 支持 |
| 支持单视频播放                                                       | 支持    | 支持 |
| 支持单音频播放                                                       | 不支持   | 支持 |
| 支持纯音频播放器                                                      | 不支持   | 支持 |
| 支持移动端浏览器最小化后台播放音频                                             | 不支持   | 支持 |
| 支持移动端设备息屏播放音频                                                 | 不支持   | 支持 |
| 设置播放缓冲区时长                                                     | 支持    | 支持 |
| 设置播放最大延迟时长                                                    | 不支持    | 支持 |
| 创建多个播放实例                                                      | 支持    | 支持 |
| http-flv协议流(H264/H265)                                        | 支持    | 支持 |
| websocket-flv协议流(H264/H265)                                   | 支持    | 支持 |
| websocket-raw(M7S)协议流(H264/H265)                              | 支持    | 支持 |
| Hls协议流(H264/H265+aac) m3u8+ts                                 | 不支持   | 支持 |
| Hls协议流(H264/H265+aac) m3u8+mp4                                | 不支持   | 支持 |
| WebTransport(M7S)协议流(H264/H265)                               | 不支持   | 支持 |
| WebRTC协议流(H264)                                               | 不支持   | 支持 |
| WebRTC协议流(H265) m7s 服务器                                       | 不支持   | 支持 |
| WebRTC协议流(H264) zlmediakit 服务器                                | 不支持   | 支持 |
| WebRTC协议流(H264) SRS 服务器                                       | 不支持   | 支持 |
| WebRTC协议流(H264) 其他服务器                                         | 不支持   | 支持 |
| 阿里云Web Rtc                                                    | 不支持   | 支持 |
| 加密流（M7S）(H264/H265)                                           | 不支持   | 支持 |
| 国标SM4加密流                                                      | 不支持   | 支持 |
| xor加密流                                                        | 不支持    | 支持 |
| HLS(aes-128-cbc)加密流                                                      | 不支持    | 支持 |
| 支持fmp4协议流(H264/H265)                                          | 不支持   | 支持 |
| 支持裸流格式(H264/H265)                                             | 不支持   | 支持 |
| 支持MPEG-TS格式(H264/H265)                                        | 不支持   | 支持 |
| 动态分辨率                                                         | 不支持   | 支持 |
| 微信等H5页面加载自动播放视频                                               | 支持    | 支持 |
| 显示模式（填充，等比，等比缩放）                                              | 支持    | 支持 |
| 画面镜像（水平，垂直）                                                   | 不支持   | 支持 |
| 画面旋转（0，90，180，270）                                            | 支持    | 支持 |
| 底部UI                                                          | 支持    | 支持 |
| 底部UI（H5）                                                      | 支持    | 支持 |
| 底部UI支持自动隐藏                                                    | 支持    | 支持 |
| 底部UI按钮支持自定义方法                                                 | 不支持   | 支持 |
| 底部UI按钮支持添加自定义按钮                                               | 不支持   | 支持 |
| 底部UI自定义HTML内容                                                 | 不支持   | 支持 |
| 键盘快捷键                                                         | 支持    | 支持 |
| 支持隐藏默认loading效果                                               | 不支持   | 支持 |
| 支持配置loading的icon                                              | 不支持   | 支持 |
| 配置右键菜单以及事件                                                    | 不支持   | 支持 |
| 性能面板<br/>可以查看实时数据（延迟，时间戳，基本信息，解码器信息等）                         | 不支持   | 支持 |
| WebWorker                                                     | 支持    | 支持 |
| 视频录制（webm格式的webm、mp4文件（音频+视频））                                | 支持    | 支持 |
| 视频录制（MPEG-4格式的mp4文件，（视频））                                     | 不支持   | 支持 |
| 视频录制（Flv格式的flv文件（音频+视频））                                      | 不支持   | 支持 |
| wasm worker视频录制（MPEG-4格式的mp4文件，（音频+视频））                       | 不支持   | 支持 |
| 支持网络延迟检测，设置超过延迟重新拉流播放                                         | 不支持   | 支持 |
| 支持设置播放加载超时，设置间隔时间，自动重试次数                                      | 支持    | 支持 |
| 支持设置播放过程流断检测，设置间隔时间，自动重试次数                                    | 支持    | 支持 |
| 异常事件                                                          | 支持   | 支持 |
| work线程中发起Http-Flv、WS等请求                                       | 不支持   | 支持 |
| work线程中解封装数据，解码数据                                             | 不支持   | 支持 |
| 语音通讯（采集麦克风数据）                                                 | 不支持   | 支持 |
| 语音通讯（设置编码格式PCM/G711A/G711U/Opus）                              | 不支持   | 支持 |
| 语音通讯（设置采样率16000Hz或8000Hz）                                     | 不支持   | 支持 |
| 语音通讯（设置采样位数3位或16位或8位）                                         | 不支持   | 支持 |
| 语音通讯（设置声道单声道或双声道）                                             | 不支持   | 支持 |
| 语音通讯（设置rtp包封装）                                                | 不支持   | 支持 |
| TF卡（录像流）流播放                                                   | 不支持   | 支持 |
| TF卡（录像流）流播放，（2/4/8/16/32/64）倍率播放                              | 不支持   | 支持 |
| TF卡（录像流）流播放，关键帧解码播放                                           | 不支持   | 支持 |
| TF卡（录像流）流播放，配套UI 24小时进度条                                      | 不支持   | 支持 |
| TF卡（录像流）流播放，配套UI 固定时长进度条                                      | 不支持   | 支持 |
| TF卡（录像流）流播放，配套UI 倍率（2/4/8/16/32/64）选择                         | 不支持   | 支持 |
| TF卡（录像流）流播放，暂停播放但不断开连接                                        | 不支持   | 支持 |
| TF卡（录像流）流播放，直接解码前缓存数据                                         | 不支持   | 支持 |
| UI控件PTZ（云台）操作盘<br/>(支持配置点击事件和鼠标按下和松开事件两种交互)                   | 不支持   | 支持 |
| UI控件PTZ（云台）操作盘支持两种布局（横向，纵向）                                   | 不支持   | 支持 |
| UI控件PTZ（云台）操作盘支持拖拽                                            | 不支持   | 支持 |
| UI控件PTZ（云台 镜头(+/-)，聚焦(+/-)，光圈(+/-) ，巡航(开/关)，透雾(开/关)，雨刷(开/关)按钮 | 不支持   | 支持 |
| UI控件流分辨率配置(自定义)和展示                                            | 不支持   | 支持 |
| 电子放大                                                          | 不支持   | 支持 |
| 播放器自定义水印                                                      | 不支持   | 支持 |
| 截图加自定义水印                                                      | 不支持   | 支持 |
| 全屏水印                                                          | 不支持   | 支持 |
| 动态水印                                                          | 不支持   | 支持 |
| 幽灵水印                                                          | 不支持   | 支持 |
| AI人脸识别                                                        | 不支持   | 支持 |
| AI物品识别（人、车辆等）                                                 | 不支持   | 支持 |
| AI黑屏、绿屏、花屏、马赛克检查                                              | 不支持   | 支持 |
| AI遮挡物检查                                                       | 不支持   | 支持 |
| 支持通过ws接口获取服务器端画面坐标系（画框子、文字、不规则形状）<br/>实时渲染在播放器上，              | 不支持   | 支持 |
| 播放异常(崩溃)日志收集（方便发送给服务器端）                                       | 不支持   | 支持 |
| 播放过程中超时，重新加载的时候，显示最后一帧画面                                      | 不支持   | 支持 |
| pause() 到 play()的时候，显示暂停的时候的最后一帧画面                            | 不支持   | 支持 |
| 支持分析视频流里面SEI数据，并提取出来，通过事件回调给业务层使用                             | 不支持   | 支持 |
| 支持监听播放器是否在可视区域，获取焦点，失去焦点事件                                    | 不支持   | 支持 |
| NxN路UI多屏播放(支持1x1,2x2,3x3,4x4)                                 | 不支持   | 支持 |
| NxN路UI多屏播放支持不规则多屏播放(3-1，4-1)                                  | 不支持   | 支持 |
| NxN路UI支持双击单个视频窗口，局部全屏（相对于container）                           | 不支持   | 支持 |
| NxN路UI支持拖拽排序                                                  | 不支持   | 支持 |
| 提供业务解决方案文档                                                    | 不支持   | 支持 |
| 支持定制化服务开发                                                     | 不支持   | 支持 |



## Pro 体验版本和 Pro正式版本区别

1. 体验版的解码时间限制是在一个小时，超过一个小时wasm会暂停解码。
2. 体验版会强制在右上角有 jessibuca-pro 水印。（有坏心思人会通过修改代码移除这块逻辑）
3. 体验版会强制全屏水印（JessibucaPro 体验）。（有坏心思人会通过修改代码移除这块逻辑）
4. 体验版会强制动态水印（JessibucaPro 体验）。
5. 体验版会强制幽灵水印（JessibucaPro 体验）。


> 如需要购买PRO版本可以联系添加作者微信：bosswancheng


包含的文件有：

### 音视频播放器
- jessibuca-pro-demo.js(script标签引入)
- jessibuca-pro-multi-demo.js(script标签引入)
- decoder-pro.js (初始化参数decoder参数配置)
- decoder-pro.wasm(胶水文件)
- decoder-pro-simd.js (初始化参数decoder参数配置)
- decoder-pro-simd.wasm(胶水文件)
- decoder-pro-audio.js(不需要配置，播放器内部会引用)
- decoder-pro-audio.wasm(胶水文件)
- decoder-pro-hard.js(硬解码解封装数据,播放器内部会引用)
- decoder-pro-hard-not-wasm.js(硬解码解封装数据,播放器内部会引用)
- decoder-pro-mt.js(不需要配置，播放器内部会引用)
- decoder-pro-mt-worker.js(不需要配置，播放器内部会引用)
- decoder-pro-mt-worker.wasm(胶水文件)
- decoder-pro-mt-worker.worker.js(不需要配置，播放器内部会引用)
- decoder-pro-simd-mt.js(不需要配置，播放器内部会引用)
- decoder-pro-simd-mt-worker.js(不需要配置，播放器内部会引用)
- decoder-pro-simd-mt-worker.wasm(胶水文件)
- decoder-pro-simd-mt-worker.worker.js(不需要配置，播放器内部会引用)

> `jessibuca-pro-demo.js`与`jessibuca-pro-multi-demo.js`的区别是，前者是单路播放，后者是支持多路播放。 两者只需要引入一个就行了，不需要同时引入。

### 音频播放器
- jessibuca-pro-audio-payer-demo.js(script标签引入)
- decoder-pro-audio-player.js(初始化参数decoder参数配置)
- decoder-pro-audio.wasm(胶水文件)

### 语音通讯（单向采集数据发送给服务器端）

- jessibuca-pro-talk-demo.js(script标签引入)


## 直播协议支持

支持18种直播格式

| 协议                         | 支持情况 |
|:---------------------------| :--- |
| ws-raw(M7S私有协议)            | 支持 |
| ws-flv                     | 支持 |
| http-flv                   | 支持 |
| HLS                        | 支持 |
| http-fmp4                  | 支持 |
| ws-fmp4                    | 支持 |
| http-h264                  | 支持 |
| ws-h264                    | 支持 |
| http-h265                  | 支持 |
| ws-h265                    | 支持 |
| mpeg4                      | 支持 |
| mpeg-ts                    | 支持 |
| WebTransport(M7S服务器)       | 支持 |
| Webrtc H264(M7S服务器)        | 支持 |
| Webrtc H265(M7S服务器)        | 支持 |
| Webrtc H264(zlmediakit服务器) | 支持 |
| Webrtc H264(SRS服务器)        | 支持 |
| Webrtc H264(其他服务器)         | 支持 |
| 阿里云Web Rtc                 | 支持 |
| 加密私有协议 (M7S服务器)            | 支持 |
| 国标SM4加密流                   | 支持 |

> 大部分协议都支持H264和H265编码格式。

> 当电脑支持硬解码的时候优先使用硬解码，不支持硬解码的时候降级使用软解码（保证H264、H265都可以播放）。

> webrtc 遵循的协议是：https://datatracker.ietf.org/doc/html/draft-ietf-wish-whip-03


## 视频编码格式支持

| 编码格式       | 支持情况 |
|:-----------|:-----|
| H264（软解码）  | 支持   |
| H265（软解码）  | 支持   |
| H264（硬解码）  | 支持   |
| H265（硬解码）  | 支持   |
| MPEG4（软解码） | 支持   |

> H265硬解码需要对于浏览器、电脑硬件有要求。

检查浏览器是否支持H265硬解码，

1.浏览器输入：`chrome://gpu/` 如果edge浏览器就`edge://gpu/`
2.全局搜索下`hevc`关键词

查看是否有：

<img src="/public/img/h265.png">


H265硬解码，对于电脑硬件支持情况：

[支持情况](https://github.com/StaZhu/enable-chromium-hevc-hardware-decoding/blob/main/README.zh_CN.md)

## 音频编码格式支持

| 编码格式  | 支持情况 |
|:------| :--- |
| G711A | 支持 |
| G711U | 支持 |
| AAC   | 支持 |
| MP3   | 支持 |


## 加密流支持

| 加密格式          | 支持情况 |
|:--------------| :--- |
| m7s-crypto加密流 | 支持 |
| 国标SM4加密流      | 支持 |
| XOR加密流        | 支持 |
| HLS(aes-128-cbc)           | 支持 |

> 播放 `m7s-crypto`加密流需要配合M7S Pro加密插件（plugin-crypto）

> 播放 `国标SM4`加密流 需要流媒体服务器端自己开发支持（集成规则请咨询作者bosswancheng）

> 播放 `XOR`加密流 需要流媒体服务器端自己开发支持（集成规则请咨询作者bosswancheng）

> 播放 `HLS(aes-128-cbc)`加密流 需要流媒体服务器端自己开发支持（集成规则请咨询作者bosswancheng）


## 分辨率支持

支持任意分辨率（720P、1080P、2k、4k）

> 分辨力越高，对于解码要求越高，对于浏览器有要求，对于设备有要求。

## 浏览器支持

> 基础功能支持主流浏览器。

> 会存在部分功能只支持部分浏览器。

> MSE和WebCodecs硬解码H265需要Chrome 107及以上。

> Chrome/Edge 91, Firefox89及之后正式提供的SIMD指令集加速解码(在1080P以上分辨率带来100%-300%的性能提升)。

> Safari 16.4版本之后正式提供的WCS硬解码H264。

> Safari暂不支持SIMD指令集加速解码。

> Chrome 113 支持 WebGPU

> Chrome 97 支持 WebTransport


| 浏览器     | 支持情况 |
|:--------| :--- |
| Chrome  | 支持 |
| Firefox | 支持 |
| Edge    | 支持 |
| Safari  | 支持 |
| 统信浏览器   | 支持 |
| 奇安信浏览器  | 支持 |


> 统信浏览器 和 奇安信浏览器 是国产操作系统的浏览器。

> 默认指的都是较新版本的浏览器，如果是较老版本的浏览器，可能会有部分功能不支持。

### 对于hevc(265)支持情况

<img src="/public/hevc-support.png">

[caniuse详情](https://caniuse.com/?search=hevc)

> 可以在浏览器地址栏上面输入：`chrome://gpu/` 就可以查看显卡支持情况了


<img src="/public/img/gpu-support.jpg">



### 对于Webassembly支持情况

<img src="/public/webassembly-support.png">

[caniuse详情](https://caniuse.com/?search=webassembly)


### PC端

根据推荐配置，优先使用 `mse硬解码`，如果不支持 `mse`，会降级到 `wasm simd解码`，如果不支持`wasm simd`，会降级到 `wasm解码`。


| 浏览器 | wasm | wasm simd | mse 解码H264| mse 解码H265 | webcodecs 解码H264 |webcodecs 解码H265 |
| :--- | :--- | :--- |:-----------| :--- |:-----------------| :--- |
| Chrome | 支持 | 支持 | 支持         | 支持 | 支持               | 支持 |
| Firefox | 支持 | 支持 | 支持         | 不支持 | 不支持              |  不支持 |
| Edge | 支持 | 支持 | 支持         | 支持 | 支持               | 支持 |
| Safari | 支持 | 不支持 | 支持        | 支持 | 支持               | 不支持 |
| 统信浏览器 | 支持 | 支持 | 支持      | 不支持 | 不支持               | 不支持 |
| 奇安信浏览器 | 支持 | 支持 | 支持      | 不支持 | 不支持               | 不支持 |

> mse 解码H265 需要window系统在10及以上，chrome/edge的版本得108及以上才能够支持

> edge支持 硬解码H265 需要额外安装hevc扩展 具体见：[edge/chrome开启硬解码](https://blog.csdn.net/water1209/article/details/126959579)

> webcodecs 解码H265 需要https环境，同样需要window系统在10及以上，chrome的版本得107及以上，safari的16.4及以上才能够支持。


#### Chromium 内核的 Edge 在 Windows 系统下，额外支持了硬解 HEVC 视频，但必须满足如下条件：

- 操作系统版本必须为 Windows 10 1709（16299.0）及以后版本。
- 安装付费的 HEVC 视频扩展或免费的来自设备制造商的 HEVC 视频扩展且版本号必须大于等于 1.0.50361.0（由于一个存在了一年半以上的 Bug，老版本存在抖动的 Bug，Issue：https://techcommunity.microsoft.com/t5/discussions/hevc-video-decoding-broken-with-b-frames/td-p/2077247/page/4）。
- 版本号必须大于等于 Edge 99 。

在安装插件后，进入 edge://gpu 页面，可以查看 Edge 对于 HEVC 硬解支持的 Profile：

##### 指标：

- 分辨率最高支持 8192px * 8192px。
- 支持 HEVC Main / Main10 / Main Still Picture Profile。
##### 优势：

- 在显卡支持的情况，性能是最好的。
- HTMLVideoElement、MSE 等原生 API 的直接支持。
##### 劣势：

- 不支持 Windows 8 和老版本 Windows 10。
- 需要手动装插件。
- HDR 支持不够好。


### 移动端

#### 安卓

> 安卓的浏览器基本都支持mse，优先使用mse硬解码。

根据推荐配置，优先使用`mse硬解码(h264+h265)`，如果不支持`mse`，会降级到`wasm simd解码`，如果不支持`wasm simd`，会降级到`wasm解码`。

| 浏览器      | wasm | wasm simd | mse 解码H264| mse 解码H265 | webcodecs 解码H264 | webcodecs 解码H265|
|:---------|:-----|:----------|:-----------| :----------- |:-------------------| :--------------- |
| Chrome   | 支持   | 支持        | 支持         | 支持 | 支持         | 支持 |
| 微信内置     | 支持   | 支持        | 支持       | 支持 | 支持         | 支持 |



#### iphone

> iphone自带的浏览器不支持mse，所以只能使用wasm解码。

> iphone 16.4.1版本之后，支持webcodecs硬解码h264，但是不支持h265。

> iphone 17.1.x版本之后，支持MSE硬解码h264，但是不支持h265。


| 浏览器    | wasm | wasm simd | mse 解码H264 | mse 解码H265 | webcodecs 解码H264 | webcodecs 解码H265|
|:-------| :--- | :--- |:-----------| :--- |:-----------------| :--- |
| Safari | 支持 | 不支持 | 支持(17.1.x以上)         | 不支持 | 支持(16.4.1以上)     | 不支持 |
| Chrome | 支持 | 不支持 | 支持(17.1.x以上)         | 不支持 | 支持(16.4.1以上)               | 不支持 |
| 微信内置   | 支持 | 不支持 | 支持(17.1.x以上)         | 不支持 | 支持(16.4.1以上)               | 不支持 |


## 操作系统

> 基础功能支持主流操作系统。

> 会存在部分功能只支持部分操作系统。

| 操作系统    | 支持情况 |
|:--------|:-----|
| Windows | 支持   |
| Linux   | 部分支持 |
| 国产化     | 部分支持 |
| Mac     | 支持   |
| Android | 支持   |
| iOS     | 支持   |


### 国产化操作系统

> 国产化操作系统是指：麒麟、龙芯、飞腾、兆芯等。

对于国产化操作系统，支持的性能有限。

### 支持加密流

目前播放器对接了`M7S的私有格式加密流`,`国标SM4加密流`，`XOR加密流`播放

> 支持额外定制私有的加密解密方式。

## 电脑型号（硬件配置）以及码率

### windows 默认
> windows 7 与 windows 10也是存在差异性的。windows 7的性能会比windows 10/11差一些。

<img src="/public/configuration.png">


> 2k,4k的视频，需要更高的配置才能够流畅播放。一般默认情况下，只能支持一路2k或者4k的视频。

> “每路所需内存”，大部分是wasm所需要的内存。

### windows 开启多线程模式下

> 多线程依赖https，并且需要额外开启支持才行 见：[开启多线程解码](/pro-doc.html#localhost)


<img src="/public/configuration-2.png">


### 国产操作系统

> 国产操作系统是指：麒麟、龙芯、飞腾、兆芯等。

<img src="/public/configuration-3.png">

### 小结

> 例如：6路 1080p的视频，所需的内存为：6 * 300 = 1800M，1800/1024 = 1.75G

> 例如：6路 720p的视频，所需的内存为：6 * 200 = 1200M，1200/1024 = 1.17G

> 对于硬解码，如果只是集成显卡的情况，对于1080P的H265视频，4路基本不卡（推荐，gpu的占比基本30%以下），5路，6路都是轻微（gpu的占比基本50%左右），7路卡顿增多（gpu的占比基本60%左右），8路，9路（gpu的占比基本60%-80%）就是整个画面一直在卡了。

> 所以如果想要上4路以上的多屏，就得上独立显卡了。


!!! 如果电脑的解码性能跟不上，可能会导致解码数据堆积，内存会一直上涨，导致网页崩溃。!!!

### N卡官方对视频并发解码路数有限制

使用N卡进行视频编解码有着并发数的限制，https://developer.nvidia.com/video-encode-decode-gpu-support-matrix#Encoder，可以看到，GeForce 系列各种显卡大部分限制为 3 路并发。这个限制貌似并不是硬件能力的限制，而是Nvidia为了让更多的人购买高端系列的显卡而做的手脚。
有同志已经制作了补丁，可以打破这一限制，https://github.com/keylase/nvidia-patch 。使用这个补丁后，现在同时进行19路编解码，连续两周，还很稳定。


### 码率

网络带宽方面
#### 240p分辨率：

这是最低的视频分辨率，适用于低带宽网络环境。在这种情况下，视频的清晰度较低，但传输所需的带宽也较少，通常需要大约`150 kbps- 200 kbps`的带宽。

#### 360p分辨率：

这是一种标准的视频分辨率，适用于中等带宽网络环境。视频的清晰度相对较好，但传输所需的带宽也相对较高，通常需要大约`400 kbps - 800 kbps`的带宽。

#### 480p分辨率：

这是一种较高的视频分辨率，适用于较好的带宽网络环境。视频的清晰度较高，但传输所需的带宽也较高，通常需要大约`800 kbps - 1500 kbps`的带宽。

#### 720p分辨率：

这是一种高清视频分辨率，适用于较好的带宽网络环境。视频的清晰度非常好，但传输所需的带宽也相对较高，通常需要大约`1.5 Mbps - 4 Mbps`的带宽。

#### 1080p分辨率：

这是一种全高清视频分辨率，适用于高速带宽网络环境。视频的清晰度非常高，但传输所需的带宽也非常高，通常需要大约`4 Mbps - 8 Mbps`的带宽。

#### 2.5k分辨率

这是一种超高清视频分辨率，适用于高速带宽网络环境。视频的清晰度最佳，但传输所需的带宽也非常高，通常需要大约`2.5 Mbps - 8 Mbps`的带宽。


#### 4k分辨率

这是一种超高清视频分辨率，适用于高速带宽网络环境。视频的清晰度最佳，但传输所需的带宽也非常高，通常需要大约`6 Mbps - 22 Mbps`的带宽。




### 对于H265硬解码显卡的支持情况：

<img src="/public/hevc-1.png">

如果出现chrome（版本大于104）但是暂未开启硬解码的情况，可以参考下面的链接开启硬解码。
[chrome 开启HEVC硬件解码](https://blog.csdn.net/water1209/article/details/126959579)


[硬件支持情况](https://github.com/StaZhu/enable-chromium-hevc-hardware-decoding/blob/main/README.zh_CN.md)

#### 独显
NVIDIA GTX950 及以上

AMD RX460 及以上

#### 集显
Intel HD4400, HD515 及以上

AMD Radeon R7, Vega M 及以上

Apple M1, M1 Pro, M1 Max, M1 Ultra 及以上


#### 详细支持列表
[Intel](https://bluesky-soft.com/en/dxvac/deviceInfo/decoder/intel.html)

[AMD](https://bluesky-soft.com/en/dxvac/deviceInfo/decoder/amd.html)

[NVIDIA](https://bluesky-soft.com/en/dxvac/deviceInfo/decoder/nvidia.html)



## 首屏时间、延迟

### 首屏时间
| 运行环境   | 时间   |
|:-------|:-----|
| PC端    | 1s以内 |
| 移动端安卓  | 1s以内 |
| 移动端IOS | 1s以内 |

> 首屏时间指的是从播放器初始化到第一帧画面出现的时间。

> 标准配置延迟能够基本控制在1s以内。


### 延迟

| 运行环境   | 时间      |
|:-------|:--------|
| PC端    | 1s以内    |
| 移动端安卓  | 1s以内    |
| 移动端IOS | 1s以内 |

> 延迟指的是从采集到的数据到播放器播放出来的时间。

> 可以支持超低延迟300ms以内。

## 关于延迟

## 测试报告：

[单路](https://jessibuca.com/pro-doc/single.pdf)

[多路-4路](https://jessibuca.com/pro-doc/multi.pdf)

[多路-16路-内存消耗](https://jessibuca.com/pro-doc/16multi-1080p.pdf)

[多路-12路-低延迟](https://jessibuca.com/pro-doc/12multi-delay.pdf)

[多路-24路-720p-1080p](https://jessibuca.com/pro-doc/24multi-720-1080.pdf)


## PRO AI

### 人脸识别

主要是利用openvc的人脸识别功能，识别出人脸后，打上框子。

>
> 需要单独购买人脸识别的授权，授权后才能使用。

体验demo [demo](https://jessibuca.com/pro/face-detector-demo.html)

### 物品识别

主要是给识别到的物品打上tag，支持人、车辆、物品等。

>
> 需要单独购买人脸识别的授权，授权后才能使用。

体验demo [demo](https://jessibuca.com/pro/object-detector-demo.html)

### 黑屏、绿屏、花屏、马赛克检查

利用opencv实现的黑屏、绿屏、花屏、马赛克检查

> 需要单独购买，授权后才能使用。

体验demo [demo](https://jessibuca.com/pro/demo-check-frame.html)


### 遮挡物检查
利用opencv实现的遮挡物检查

> 需要单独购买，授权后才能使用。

体验demo [demo](https://jessibuca.com/pro/demo-check-occlusion.html)


## 支持定制化服务开发
支持一系类和业务耦合的开发需求。

## 支持作者

### 第一作者
<img src="/public/wx.jpg"><img src="/public/alipay.jpg">

### V3版本作者
<img src="/public/wx-pay-wc.jpg" style="width:333px"><img src="/public/alipay-wc.jpg" style="width:333px">


# 群
<img src="/public/qrcode.jpeg">

# 群
<img src="/public/qrcode-qw.jpeg">

## qq频道
<img src="/public/qq-qrcode.jpg">

