# 简介

jessibuca pro 是在开源版本的基础上额外支持的深入业务解决方案的。解决了一些痛点，比如H265的硬解码，SIMD软解码加速。
<Rice/>
## 开源版与PRO 差异性

<ProDiffTable />


## Pro 体验版本和 Pro正式版本区别

1. 体验版的解码时间限制是在一个小时，超过一个小时wasm会暂停解码。（一小时断开, 刷新页面可继续测试）
2. 体验版的解码时间会在某天到期，到期后wasm会暂停解码。（到期断开, 可以更新官网最新版本可继续测试）
3. 体验版会强制在右上角有 jessibuca-pro 水印。（有坏心思人会通过修改代码移除这块逻辑）
4. 体验版会强制全屏水印（JessibucaPro 体验）。（有坏心思人会通过修改代码移除这块逻辑）
5. 体验版会强制动态水印（JessibucaPro 体验）。
6. 体验版会强制幽灵水印（JessibucaPro 体验）。

> 如需要购买PRO版本可以联系添加作者微信：bosswancheng

## 开源版升级到Pro(体验)版本

[升级文档](https://jessibuca.com/pro-upgrade.html)

## 直播协议支持(封装格式)
支持20种直播格式

| 协议                         | 支持情况 |
|:---------------------------|:-----|
| ws(s)-raw(M7S私有协议)         | 支持   |
| ws(s)-flv                  | 支持   |
| http(s)-flv                | 支持   |
| http(s)-HLS（直播流）           | 支持   |
| http(s)-fmp4               | 支持   |
| ws(s)-fmp4                 | 支持   |
| http(s)-h264(裸流)           | 支持   |
| ws(s)-h264 (裸流)            | 支持   |
| http(s)-h265 (裸流)          | 支持   |
| ws(s)-h265(裸流)             | 支持   |
| http(s)-mpeg4              | 支持   |
| ws(s)-mpeg4                | 支持   |
| http(s)-mpeg-ts            | 支持   |
| http(s)-mpeg(ps流)          | 支持   |
| WebTransport(M7S服务器)       | 支持   |
| Webrtc H264(M7S服务器)        | 支持   |
| Webrtc H265(M7S服务器)        | 支持   |
| Webrtc H264/H265(zlmediakit服务器) | 支持   |
| Webrtc H264/H265(SRS服务器)        | 支持   |
| Webrtc H264/H265(其他服务器)    | 支持   |
| 阿里云Web Rtc                 | 支持   |
| 加密私有协议 (M7S服务器)            | 支持   |
| 国标SM4加密流                   | 支持   |

> 大部分协议都支持H264和H265编码格式。

> 当电脑支持硬解码的时候优先使用硬解码，不支持硬解码的时候降级使用软解码（保证H264、H265都可以播放）。

> Webrtc H265(M7S服务器) 是通过 DataChannel 技术实现的，不是标准的webrtc协议。

> Webrtc H264/H265(zlmediakit服务器/SRS服务器/其他服务器) 是原生浏览器支持的 webrtc协议。需要服务器与浏览器环境同时支持才行。

> webrtc 遵循的协议是：https://datatracker.ietf.org/doc/html/draft-ietf-wish-whip-03


### Webrtc H265(zlmediakit服务器/SRS服务器/其他服务器) 依赖环境

1. 硬件支持 HEVC 编解码
2. 操作系统提供 HEVC 编解码能力
3. 浏览器版本支持 WebRTC H.265 协商

对于硬件支持 HEVC 编解码 的情况，可以参考以下链接：

[支持情况](/pro.html#对于h265硬解码显卡的支持情况：)

操作系统提供 HEVC 编解码能力

1. Windows 平台 （`Windows 11` & `安装 Microsoft HEVC Video Extensions`）
2. macOS 平台 （`macOS 10.13 High Sierra` 及以上）
3. Linux 平台（不支持）

浏览器版本支持 WebRTC H.265 协商

1. chrome 138+
2. Safari 16.4+
3. Edge 138+
4. Firefox（不支持）


## 点播文件协议支持(封装格式)

支持三种点播格式
| 协议 | 支持情况 |
|:---------------------------| :--- |
| http(s)-mp4 | 支持 |
| http(s)-hls(mp4)            | 支持 |
| http(s)-hls(ts)            | 支持 |

## 视频编码格式支持

| 编码格式       | 支持情况 |
|:-----------|:-----|
| H264（软解码）  | 支持   |
| H265（软解码）  | 支持   |
| H264（硬解码）  | 支持   |
| H265（硬解码）  | 支持   |
| MPEG4（软解码） | 支持   |
| AV1(硬解码)| 支持   |


> H265硬解码需要对于浏览器、电脑硬件有要求。

检查浏览器是否支持H265硬解码，

1.浏览器输入：`chrome://gpu/` 如果edge浏览器就`edge://gpu/`
2.全局搜索下`hevc`关键词

查看是否有：

<img src="/img/h265.png">


H265硬解码，对于电脑硬件支持情况：

[支持情况](https://github.com/StaZhu/enable-chromium-hevc-hardware-decoding/blob/main/README.zh_CN.md)

## 音频编码格式支持

| 编码格式  | 支持情况 |
|:------|:-----|
| G711A | 支持   |
| G711U | 支持   |
| AAC   | 支持   |
| MP3   | 支持   |

> G711A、G711U音频编码格式 不支持MSE解码。支持WCS/WASM/WASM(SIMD)解码。

## 直播协议支持(封装格式) + 编码格式(音频、视频)

| 协议                           | 视频H264 | 视频H265 | 音频aac | 音频mp3 | 音频g711a | 音频g711u |
|:-----------------------------|:-------|:-------|:------|:------|:--------|:--------|
| ws(s)-raw                    | 支持     | 支持     | 支持    | 支持    | 支持      | 支持      |
| ws(s)/http(s)-flv            | 支持     | 支持     | 支持    | 支持    | 支持      | 支持      |
| ws(s)/http(s)-fmp4           | 支持     | 支持     | 支持    | 支持   | 支持（需定制）      | 支持（需定制）      |
| ws(s)/http(s)-h264           | 支持     | 不支持    | 不支持   | 不支持   | 不支持     | 不支持     |
| ws(s)/http(s)-h265           | 不支持    | 支持     | 不支持   | 不支持   | 不支持     | 不支持     |
| http(s)-HLS-mp4              | 支持     | 支持     | 支持    | 不支持   | 支持（需定制） | 支持（需定制）     |
| http(s)-HLS-ts               | 支持     | 支持     | 支持    | 支持    | 支持（需定制）      | 支持（需定制）      |
| http(s)-mpeg-ts              | 支持     | 支持     | 支持    | 支持    | 支持（需定制）      | 支持（需定制）      |
| http(s)-mpeg-ps              | 支持     | 支持     | 支持    | 支持    | 支持（需定制）      | 支持（需定制）      |
| WebTransport(M7S)            | 支持     | 支持     | 支持    | 支持    | 支持      | 支持      |
| Webrtc H264(M7S)             | 支持     | 不支持    | 不支持   | 不支持   | 支持      | 支持      |
| Webrtc H265(M7S)             | 支持     | 支持     | 支持    | 支持    | 支持      | 支持      |
| Webrtc H264/H265(zlmediakit) | 支持     | 不支持    | 不支持   | 不支持   | 支持      | 支持      |
| Webrtc H264/H265(SRS)             | 支持     | 不支持    | 不支持   | 不支持   | 支持      | 支持      |
| Webrtc H264/H265(其他)              | 支持     | 不支持    | 不支持   | 不支持   | 支持      | 支持      |
| 阿里云Web Rtc                   | 支持     | 不支持    | 不支持   | 不支持   | 不支持     | 不支持     |
| 加密流（M7S）                     | 支持     | 支持     | 支持    | 支持    | 支持      | 支持      |
| 国标SM4加密流                     | 支持     | 支持     | 支持    | 支持    | 支持      | 支持      |

> G711A、G711U音频编码格式 不支持MSE解码。支持WCS/WASM/WASM(SIMD)解码。

> G711A、G711U音频编码格式，如定制请咨询作者bosswancheng。

> `ws(s)/http(s)-h264`,`ws(s)/http(s)-h265` 协议只支持视频流，不支持音频流。如需要支持视频+音频，则需要改成自定义封装格式。

> `webrtc` 在chrome高版本浏览器上面，如果显卡支持265硬解码，则支持播出H265编码格式的流。

## 回放流（GB28181 TF卡（录像流）、GA/T 1078 TF卡（录像流）、特殊回放流）

目前播放器支持三种业务的回放流逻辑。

> 传输协议上面与直播流基本类似，只不过播放器端的业务逻辑会有一些区别，主要是为了适配回放流的控制逻辑（暂停-播放-暂停，倍速播放，seek等）。

### GB28181 TF卡（录像流）

GB28181 TF卡（录像流）流播放 [介绍](https://jessibuca.com/pro-playback-gb28181.html)

### GA/T 1078 TF卡（录像流）
GA/T 1078 TF卡（录像流）流播放 [介绍](https://jessibuca.com/pro-playback-gat1078.html)

### 特殊回放流

特殊回放流播放[介绍](https://jessibuca.com/pro-playback-special.html)

## 点播文件支持(封装格式) + 编码格式(音频、视频)

| 协议               | 视频H264 | 视频H265 | 音频aac | 音频mp3 | 音频g711a | 音频g711u |
|:-----------------|:-------|:-------|:------|:------|:--------|:--------|
| http(s)-mp4      | 支持     | 支持     | 支持    | 不支持   | 支持（需定制）      | 支持（需定制）      |
| http(s)-hls(mp4) | 支持     | 支持     | 支持    | 不支持   | 支持（需定制）     | 支持（需定制）      |
| http(s)-hls(ts)  | 支持     | 支持     | 支持    | 支持    | 支持（需定制）      | 支持（需定制）      |

> 支持 mp4点播文件的 moov 在文件头或者文件尾。

> G711A、G711U音频编码格式 不支持MSE解码。支持WCS/WASM/WASM(SIMD)解码。

> G711A、G711U音频编码格式 ，如定制请咨询作者bosswancheng。

## 倍率播放（回放流，点播文件）H265、25FPS支持的最大倍数

> MSE(MediaSource)、WebCodecs 是浏览器原生支持的解码器，走的是硬解码。

> MSE(MediaSource)、WebCodecs 支持H265硬解码需要Chrome 107及以上，以及底层显卡支持。

> Wasm 是通过Wasm技术实现的解码器，走的是软解码。

### 4k 视频倍率支持


| 解码器           | 分辨率 | 最大倍率              |
|:--------------|:----|:------------------|
| Wasm          | 4k  | 不支持               |
| Wasm 多线程      | 4k  | 不支持               |
| Wasm SIMD     | 4k  | 不支持               |
| Wasm SIMD 多线程 | 4k  | 1倍                |
| MediaSource   | 4k  | 看chrome://gpu/ 查看 |
| Webcodec      | 4k  |看chrome://gpu/ 查看|

> SIMD 软解码4k 只支持 3840X2160格式25FPS，4096X2160暂不支持

> 4k 软解码非常吃CPU性能，对于CPU有要求。测试效果不是很明显，不建议使用。

### 2k 视频倍率支持

| 解码器              | 分辨率   | 最大倍率 |
|:-----------------|:------|:-----|
| Wasm             | 2k | 不支持  |
| Wasm 多线程         | 2k | 1倍   |
| Wasm SIMD        | 2k | 1倍   |
| Wasm SIMD 多线程    | 2k | 2倍   |
| MediaSource   | 2k  | 看chrome://gpu/ 查看 |
| Webcodec      | 2k  |看chrome://gpu/ 查看|


### 1080p 视频倍率支持

| 解码器              | 分辨率   | 最大倍率 |
|:-----------------|:------|:-----|
| Wasm             | 1080P | 1倍   |
| Wasm 多线程         | 1080P | 4倍   |
| Wasm SIMD        | 1080P | 3倍   |
| Wasm SIMD 多线程    | 1080P | 4倍   |
| MediaSource   | 1080P  | 看chrome://gpu/ 查看 |
| Webcodec      | 1080P  |看chrome://gpu/ 查看|

### 720p 视频倍率支持

| 解码器              | 分辨率   | 最大倍率 |
|:-----------------|:------|:-----|
| Wasm             | 720p | 4倍   |
| Wasm 多线程         | 720p | 16倍  |
| Wasm SIMD        | 720p | 4倍   |
| Wasm SIMD 多线程    | 720p | 16倍    |
| MediaSource   | 720p  | 看chrome://gpu/ 查看 |
| Webcodec      | 720p  |看chrome://gpu/ 查看|

## 视频录制支持

| 封装格式                                                               | 支持情况 |
|:-------------------------------------------------------------------|:-----|
| Flv格式的flv文件（音频+视频）                                                 | 支持   |
| MPEG-4格式的mp4文件（视频）                                                 | 支持   |
| webm格式的webm、mp4文件（音频+视频）                                           | 支持   |
| MPEG-4格式的mp4文件（音频+视频）[扩展模块](https://jessibuca.com/pro-module.html) | 支持   |

> webrtc播放地址，录制的文件是webm格式的。

## 加密流支持

| 加密格式             | 支持情况 |
|:-----------------|:-----|
| m7s-crypto加密流    | 支持   |
| 国标SM4-OFB加密流     | 支持   |
| 国标SM4-ECB加密流     | 支持   |
| XOR加密流           | 支持   |
| HLS(aes-128-cbc) | 支持   |

> 播放 `m7s-crypto`加密流需要配合M7S Pro加密插件（plugin-crypto）

> 播放 `国标SM4`加密流 需要流媒体服务器端自己开发支持（集成规则请咨询作者bosswancheng）

> 播放 `XOR`加密流 需要流媒体服务器端自己开发支持（集成规则请咨询作者bosswancheng）

> 播放 `HLS(aes-128-cbc)`加密流 需要流媒体服务器端自己开发支持（集成规则请咨询作者bosswancheng）

## 水印支持

| 加密格式            | 支持情况 |
|:----------------|:-----|
| 全屏水印（平铺）        | 支持   |
| 播放器自定义水印（局部水印）	 | 支持   |
| 截图加自定义水印（局部水印）	 | 支持   |
| 动态水印	           | 支持   |
| 幽灵水印	           | 支持   |
| 截图暗水印（数字水印）	    | 支持   |

> 全屏水印（平铺）只支持文本信息。

> 播放器自定义水印（局部水印）支持 `图片`，`文字`，`自定义HTML`, 支持单个水印，支持多个水印。

> 截图加自定义水印（局部水印）支持`图片`，`文字`，支持单个水印。

> 动态水印只支持文本信息。

## 分辨率支持

支持任意分辨率（720P、1080P、2k、4k）

> SIMD 软解码4k 只支持 3840X2160格式25FPS，4096X2160暂不支持

> 分辨力越高，对于解码要求越高，对于浏览器有要求，对于设备有要求。

## 浏览器支持

> 基础功能支持主流浏览器。

> 会存在部分功能只支持部分浏览器。

> MSE和WebCodecs硬解码H265需要Chrome 107及以上。

> Chrome/Edge 91, Firefox89及之后正式提供的SIMD指令集加速解码(在1080P以上分辨率带来100%-300%的性能提升)。

> Safari 16.4版本之后正式提供的WCS硬解码H264。

> Safari暂不支持SIMD指令集加速解码。

> Chrome/Edge 113 支持 WebGPU

> Chrome/Edge 97 支持 WebTransport


| 浏览器     | 支持情况 |
|:--------|:-----|
| Chrome  | 支持   |
| Firefox | 支持   |
| Edge    | 支持   |
| Safari  | 支持   |
| 统信浏览器   | 支持   |
| 奇安信浏览器  | 支持   |

> 统信浏览器 和 奇安信浏览器 是国产操作系统的浏览器。

> 默认指的都是较新版本的浏览器，如果是较老版本的浏览器，可能会有部分功能不支持。

> 目前播放器的编译支持的浏览器版本是基于：`browserslist 的default`配置，即支持大部分浏览器，如有特殊浏览器支持要求，可修改browserslist配置即可。

目前的支持程度如下：

|平台| 最低支持版本（约） |
|:--------|:-----|
|iOS Safari| ~15.x - 16.x|
|Android Chrome| ~近2个大版本|
|Android WebView| ~近2个大版本|
|Chrome 桌面| ~近2个大版本|
|Edge 桌面| ~近2个大版本|
|Firefox 桌面| ~近2个大版本|
|Safari 桌面 | ~15.x+|

> 如果有ios safari版本过低的需求，可以通过修改browserslist配置，来支持更低版本的ios safari浏览器。

### 对于hevc(265)支持情况

<img src="/hevc-support.png">

[caniuse详情](https://caniuse.com/?search=hevc)

#### chrome

1. Supported for all devices on macOS (>= Big Sur 11.0) and Android (>= 5.0),
2. for devices with hardware support on Windows (>= Windows 8),
3. and for devices with hardware support powered by VAAPI on Linux and ChromeOS

#### edge

1. Supported for all devices on macOS (>= Big Sur 11.0) and Android (>= 5.0) if Edge >= 107,
2. for devices with hardware support on Windows (>= Windows 10 1709) when HEVC video extensions from the Microsoft Store
   is installed

> 可以在浏览器地址栏上面输入：`chrome://gpu/` 或者 `edge://gpu/` 就可以查看显卡支持情况了


<img src="/img/gpu-support.jpg">

### 对于Webassembly支持情况

<img src="/webassembly-support.png">

[caniuse详情](https://caniuse.com/?search=webassembly)

### 对于webassembly SIMD 支持情况

<img src="/webassembly-simd.png">

[caniuse详情](https://caniuse.com/?search=webassembly)

### PC端

根据推荐配置，优先使用 `mse硬解码`，如果不支持 `mse`，会降级到 `wasm simd解码`，如果不支持`wasm simd`，会降级到 `wasm解码`。

| 浏览器     | wasm | wasm simd | mse 解码H264 | mse 解码H265 | webcodecs 解码H264 | webcodecs 解码H265 |
|:--------|:-----|:----------|:-----------|:-----------|:-----------------|:-----------------|
| Chrome  | 支持   | 支持        | 支持         | 支持         | 支持               | 支持               |
| Firefox | 支持   | 支持        | 支持         | 不支持        | 不支持              | 不支持              |
| Edge    | 支持   | 支持        | 支持         | 支持         | 支持               | 支持               |
| Safari  | 支持   | 不支持       | 支持         | 支持         | 支持               | 不支持              |
| 统信浏览器   | 支持   | 支持        | 支持         | 不支持        | 不支持              | 不支持              |
| 奇安信浏览器  | 支持   | 支持        | 支持         | 不支持        | 不支持              | 不支持              |

> mse 解码H265 需要window系统在10及以上，chrome/edge的版本得108及以上才能够支持

> edge支持 硬解码H265 需要额外安装hevc扩展
> 具体见：[edge/chrome开启硬解码](https://blog.csdn.net/water1209/article/details/126959579)

> edge浏览器检查并打开HEVC的支持： [开启HEVC支持](https://blog.csdn.net/liulilittle/article/details/135027589)

> webcodecs 解码H265 需要https环境，同样需要window系统在10及以上，chrome的版本得107及以上，safari的16.4及以上才能够支持。

#### Chromium 内核的 Edge 在 Windows 系统下，额外支持了硬解 HEVC 视频，但必须满足如下条件：

- 操作系统版本必须为 Windows 10 1709（16299.0）及以后版本。
- 安装付费的 HEVC 视频扩展或免费的来自设备制造商的 HEVC 视频扩展且版本号必须大于等于 1.0.50361.0（由于一个存在了一年半以上的
  Bug，老版本存在抖动的
  Bug，Issue：https://techcommunity.microsoft.com/t5/discussions/hevc-video-decoding-broken-with-b-frames/td-p/2077247/page/4）。
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

根据推荐配置，优先使用`mse硬解码(h264+h265)`，如果不支持`mse`，会降级到`wasm simd解码`，如果不支持`wasm simd`，会降级到
`wasm解码`。

| 浏览器    | wasm | wasm simd | mse 解码H264 | mse 解码H265 | webcodecs 解码H264 | webcodecs 解码H265 |
|:-------|:-----|:----------|:-----------|:-----------|:-----------------|:-----------------|
| Chrome | 支持   | 支持        | 支持         | 支持         | 支持               | 支持               |
| 微信内置   | 支持   | 支持        | 支持         | 支持         | 支持               | 支持               |

安卓在硬解码的基础上，可以支持 `H264/H265` `1080p及以上(2k,4k)`的分辨率。前提是机器性能足够。

#### iphone

> iphone自带的浏览器不支持mse，所以只能使用wasm解码（软解码性能要远远弱于PC机，根据CPU性能情况支持1~2路）。

> iphone 16.4.1版本之后，支持webcodecs硬解码h264，但是不支持h265。

> iphone 17.1.x版本之后，支持MSE硬解码h264，但是不支持h265。

| 浏览器    | wasm | wasm simd | mse 解码H264   | mse 解码H265 | webcodecs 解码H264 | webcodecs 解码H265 |
|:-------|:-----|:----------|:-------------|:-----------|:-----------------|:-----------------|
| Safari | 支持   | 不支持       | 支持(17.1.x以上) | 不支持        | 支持(16.4.1以上)     | 不支持              |
| Chrome | 支持   | 不支持       | 支持(17.1.x以上) | 不支持        | 支持(16.4.1以上)     | 不支持              |
| 微信内置   | 支持   | 不支持       | 支持(17.1.x以上) | 不支持        | 支持(16.4.1以上)     | 不支持              |

`IOS`在硬解码的基础上，可以支持 `H264` `1080p及以上(2k,4k)`的分辨率。前提是机器性能足够。

但是对于`H265`的支持，目前只能使用软解码，而且只能使用 wasm 解码 （IOS不支持WASM SIMD解码），导致分辨率只能支持`1080p及以下`。
`2k，4k`的分辨率没法支持。

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

可下载完整的[电脑型号（硬件配置）信息.xlsx](https://jessibuca.com/pro-doc/config.xlsx) 查看

### windows 默认

> windows 7 与 windows 10也是存在差异性的。windows 7的性能会比windows 10/11差一些。

<img src="/configuration.png">


> 2k,4k的视频，需要更高的配置才能够流畅播放。一般默认情况下，只能支持一路2k或者4k的视频。

> “每路所需内存”，大部分是wasm所需要的内存。

### windows 开启多线程模式下

> 多线程依赖https，并且需要额外开启支持才行 见：[开启多线程解码](/pro-doc.html#localhost)


<img src="/configuration-2.png">

### 国产操作系统

> 国产操作系统是指：麒麟、龙芯、飞腾、兆芯等。

<img src="/configuration-3.png">

### 小结

<img src="/configuration-4.png">

> 例如：6路 1080p的视频，所需的内存为：6 * 300 = 1800M，1800/1024 = 1.75G

> 例如：6路 720p的视频，所需的内存为：6 * 200 = 1200M，1200/1024 = 1.17G

>
对于硬解码，如果只是集成显卡的情况，对于1080P的H265视频，4路基本不卡（推荐，gpu的占比基本30%以下），5路，6路都是轻微（gpu的占比基本50%左右），7路卡顿增多（gpu的占比基本60%左右），8路，9路（gpu的占比基本60%-80%）就是整个画面一直在卡了。

> 所以如果想要上4路以上的多屏，就得上独立显卡了。


!!! 如果电脑的解码性能跟不上，可能会导致解码数据堆积，内存会一直上涨，导致网页崩溃。!!!

如果想要测试当前显示器的最大支持路数，可以测试[测试地址](https://jessibuca.com/check-max-play-multi-size.html)

### N卡官方对视频并发解码路数有限制

使用N卡进行视频编解码有着并发数的限制，https://developer.nvidia.com/video-encode-decode-gpu-support-matrix#Encoder，可以看到，GeForce
系列各种显卡大部分限制为 3 路并发。这个限制貌似并不是硬件能力的限制，而是Nvidia为了让更多的人购买高端系列的显卡而做的手脚。
有同志已经制作了补丁，可以打破这一限制，https://github.com/keylase/nvidia-patch 。使用这个补丁后，现在同时进行19路编解码，连续两周，还很稳定。

### 码率

网络带宽方面

#### 240p分辨率：

> 426 × 240（宽 × 高）

这是最低的视频分辨率，适用于低带宽网络环境。在这种情况下，视频的清晰度较低，但传输所需的带宽也较少，通常需要大约
`150 kbps- 200 kbps`的带宽。


#### 360p分辨率：

> 640 × 360（宽 × 高）

这是一种标准的视频分辨率，适用于中等带宽网络环境。视频的清晰度相对较好，但传输所需的带宽也相对较高，通常需要大约
`400 kbps - 800 kbps`的带宽。

#### 480p分辨率：

> 480p（Standard Definition）（SD 标准清晰度）720 × 480（宽 × 高）

这是一种较高的视频分辨率，适用于较好的带宽网络环境。视频的清晰度较高，但传输所需的带宽也较高，通常需要大约
`800 kbps - 1500 kbps`的带宽。对应的摄像头的像素一般是30万像素。

#### 720p分辨率：

> 720p（HD Ready）（高清）1280 × 720（宽 × 高）

这是一种高清视频分辨率，适用于较好的带宽网络环境。视频的清晰度非常好，但传输所需的带宽也相对较高，通常需要大约
`1.5 Mbps - 4 Mbps`的带宽。对应的摄像头的像素一般是100万像素。

推荐的帧率是： `20-25fps`之间

推荐的码率是：
1. 低质量：`1000-1500kbps`
2. 中等质量：`1500-2500kbps`
3. 高质量：`2500-4000kbps`

> 所以码率提高到至少1500kbps

#### 1080p分辨率：

> 1080p（Full HD）（全高清）1920 × 1080（宽 × 高）

这是一种全高清视频分辨率，适用于高速带宽网络环境。视频的清晰度非常高，但传输所需的带宽也非常高，通常需要大约
`4 Mbps - 8 Mbps`的带宽。对应的摄像头的像素一般是200万像素。


帧率(FPS)推荐：
1. 最低配置：15-20fps（适用于静态场景，如办公室）
2. 推荐配置：25-30fps（适用于大多数场景，运动物体较清晰）
3. 高配置：30fps（适用于快速运动场景的监控）

码率(Bitrate)推荐：
1. 低质量场景（办公室、室内静态场景）：
 - 2000-3000kbps
2. 中等质量（一般监控场景）：
 - 3000-4000kbps
3. 高质量（需要清晰细节的场景）：
 - 4000-6000kbps
4. 超高质量（专业视频录制）：
 - 6000-8000kbps以上

小结：

如果是一般监控使用：
- 帧率：25fps
- 码率：4000kbps
- 关键帧间隔：25-50（即1-2秒一个关键帧）

如果是需要高清晰度的专业使用：
- 帧率：30fps
- 码率：6000kbps
- 关键帧间隔：30-60（即1-2秒一个关键帧）


#### 2k分辨率/2.5k分辨率

> QHD/WQHD（Quad HD） 2560 × 1440（宽 × 高）

> DCI 2K（Digital Cinema Initiatives） 2048 × 1080（宽 × 高）

这是一种超高清视频分辨率，适用于高速带宽网络环境。视频的清晰度最佳，但传输所需的带宽也非常高，通常需要大约
`10Mbps - 15Mbps`的带宽。对应的摄像头的像素一般是300万到500万像素。

> 2k 至少10Mbps稳定带宽

> 2.5K 至少15Mbps稳定带宽


##### 2k (2560×1440)
1. 帧率(FPS)推荐：
 - 标准：25-30fps
 - 高品质：30-60fps
2. 码率(Bitrate)推荐：
 - 低质量（基础监控）： 4000-6000kbps
 - 中等质量（标准使用）： 6000-8000kbps
 - 高质量（专业使用）： 8000-12000kbps


##### 2.5k (2880×1620)
1. 帧率(FPS)推荐：
- 标准：25-30fps
- 高品质：30-60fps
2. 码率(Bitrate)推荐：
- 低质量（基础监控）： 6000-8000kbps
- 中等质量（标准使用）： 8000-12000kbps
- 高质量（专业使用）： 12000-16000kbps


#### 4k分辨率

> UHD 4K（Ultra High Definition） 3840 × 2160（宽 × 高）

> DCI 4K（Digital Cinema Initiatives） 4096 × 2160（宽 × 高）

这是一种超高清视频分辨率，适用于高速带宽网络环境。视频的清晰度最佳，但传输所需的带宽也非常高，通常需要大约`25 Mbps`
的带宽。对应的摄像头的像素一般是800万到2000万像素。


1. 帧率(FPS)推荐：
- 标准：25-30fps
- 高品质：30-60fps
2. 码率(Bitrate)推荐：
- 低质量（基础监控）： 8000-12000kbps
- 中等质量（标准使用）： 12000-20000kbps
- 高质量（专业使用）： 20000-40000kbps



#### 8K分辨率

> 8K UHD 7680 × 4320（宽 × 高）

这是一种8K超高清视频分辨率，适用于高速带宽网络环境。视频的清晰度最佳，但传输所需的带宽也非常高，通常需要大约
`20 Mbps - 50 Mbps`，对应的摄像头的像素一般是3300万像素。


#### 小结

> 对于不同的720p 1080p，2k，2.5k， 4k的服务器的出口带宽的要求

我来详细分析下不同分辨率下服务器出口带宽的要求。这里我会按照并发观看人数来计算：

#### 单路视频流带宽需求

##### 720P (1280x720)
- 低质量：1.5Mbps
- 标准质量：2-2.5Mbps
- 高质量：3-4Mbps

##### 1080P (1920x1080)
- 低质量：2-3Mbps
- 标准质量：4Mbps
- 高质量：6-8Mbps

##### 2K (2560x1440)
- 低质量：4-6Mbps
- 标准质量：6-8Mbps
- 高质量：8-12Mbps

##### 2.5K (2880x1620)
- 低质量：6-8Mbps
- 标准质量：8-12Mbps
- 高质量：12-16Mbps

##### 4K (3840x2160)
- 低质量：8-12Mbps
- 标准质量：12-20Mbps
- 高质量：20-40Mbps

#### 服务器出口带宽计算公式
```
所需带宽 = 单路视频码率 × 并发观看人数 × (1 + 冗余系数)
```
冗余系数建议：20%-50%（根据网络质量要求）

#### 具体场景带宽需求示例

假设使用标准质量配置：

##### 小型应用（10个并发）：
- 720P：25-30Mbps
- 1080P：48-60Mbps
- 2K：80-100Mbps
- 2.5K：120-150Mbps
- 4K：200-250Mbps

##### 中型应用（50个并发）：
- 720P：125-150Mbps
- 1080P：240-300Mbps
- 2K：400-500Mbps
- 2.5K：600-750Mbps
- 4K：1000-1250Mbps

##### 大型应用（100个并发）：
- 720P：250-300Mbps
- 1080P：480-600Mbps
- 2K：800-1000Mbps
- 2.5K：1.2-1.5Gbps
- 4K：2-2.5Gbps

#### 推荐码率配置

| 分辨率              |     帧率  | 最小码率       | 推荐码率 |   最大码率|
|:-----------------|:-----|:-----------|:--------|:--------|
| 1280x720(720P)   |  30fps   | 1000 kbps  | 2000 kbps | 3500 kbps|
| 1920x1080(1080P) |  30fps  | 2000 kbps  | 4000 kbps | 6000 kbps|
| 2560x1440(2K)    |  30fps  | 4000 kbps  | 8000 kbps | 12000 kbps|
| 2880x1620(2.5K)  |  30fps  | 6000 kbps  | 12000 kbps| 16000 kbps|
| 3840x2160(4K)    |  30fps  | 8000 kbps  | 16000 kbps|  24000 kbps|

#### 优化建议

1. CDN分发策略：
    - 建议使用CDN分发减轻源站压力
    - 可以考虑多级CDN架构

2. 编码优化：
    - 使用H.265可以节省30-50%带宽
    - 可以考虑使用SVC（可伸缩视频编码）

3. 动态码率调节：
    - 根据网络状况自动调整码率
    - 实现多码率自适应切换

4. 服务器配置建议：
- 小型应用（≤50并发）：
    - 建议带宽：1Gbps
    - 适合：独立服务器或云服务器

- 中型应用（50-200并发）：
    - 建议带宽：2-5Gbps
    - 适合：专线接入或云服务器集群

- 大型应用（>200并发）：
    - 建议带宽：10Gbps以上
    - 必须使用CDN分发
    - 考虑多机房部署

5. 其他注意事项：
- 考虑峰值流量（建议预留50%带宽）
- 考虑网络质量（抖动、延迟、丢包）
- 考虑上行带宽和下行带宽比例
- 考虑不同地域用户访问情况

6. 成本优化建议：
- 使用动态扩缩容
- 采用阶梯计费的带宽包
- 根据实际需求选择合适的分辨率
- 合理使用转码服务

### 对于H265硬解码显卡的支持情况：

<img src="/hevc-1.png">

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

| 运行环境   | 时间   |
|:-------|:-----|
| PC端    | 1s以内 |
| 移动端安卓  | 1s以内 |
| 移动端IOS | 1s以内 |

> 延迟指的是从采集到的数据到播放器播放出来的时间。

> 可以支持超低延迟300ms以内。

## 测试报告：

[单路](https://jessibuca.com/pro-doc/single.pdf)

[多路-4路](https://jessibuca.com/pro-doc/multi.pdf)

[多路-16路-内存消耗](https://jessibuca.com/pro-doc/16multi-1080p.pdf)

[多路-12路-低延迟](https://jessibuca.com/pro-doc/12multi-delay.pdf)

[多路-24路-720p-1080p](https://jessibuca.com/pro-doc/24multi-720-1080.pdf)

## 音频播放器

支持播放`ws-flv`、`http-flv`、`webrtc`格式的音频流。

> 可以支持移动端（平板端）息屏和后台播放。

## 语音通讯

支持在web页面采集麦克风数据，支持编码格式pcm、pcma(g711a)、pcmu(g711u)，支持封装成rtp协议包、jtt协议包。

> 目前仅支持单向语音通讯，不支持双向语音通讯。（仅支持通过ws传输到服务器，不支持通过ws接收音频数据进行播放），根据国标情况，播放端是跟着音视频一起播放的，可以使用音视频播放器进行播放器音频和视频数据。

> 关于
>
jtt协议包的封装格式，可以参考：[jtt协议包](https://jessibuca.com/document.html#%E5%85%B3%E4%BA%8E%E5%9B%BD%E6%A0%87-jtt-%E9%81%93%E8%B7%AF%E8%BF%90%E8%BE%93%E8%BD%A6%E8%BE%86%E5%8D%AB%E6%98%9F%E5%AE%9A%E4%BD%8D%E7%B3%BB%E7%BB%9F%E9%9F%B3%E8%A7%86%E9%A2%91%E9%80%9A%E8%AE%AF%E5%8D%8F%E8%AE%AE)

## PRO AI

[AI 模块](https://jessibuca.com/pro-ai.html)

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

## PRO 扩展模块

[扩展模块](https://jessibuca.com/pro-module.html)

### Mp4 录制（MPEG-4）

利用ffmpeg实现的 mp4录制，支持录制视频和音频。

视频支持的编码格式：`h264`、`h265`、

音频支持的编码格式：`aac`、`mp3`、`pcmu(g711u)`、`pcma(g711a)`

### 直播流、回放(录像)流下载（录制）器

支持 `ws(s)-raw`,`http(s)-flv`,`ws(s)-flv`,`http(s)-fmp4`,`ws(s)-fmp4`,`hls`,`http(s)-h264`,`ws(s)-h264`,`http(s)-h265`,
`ws(s)-h265` 等协议的直播流、回放(录像)流下载（录制）器。

视频支持的编码格式：`h264`、`h265`、
音频支持的编码格式：`aac`、`mp3`、`pcmu(g711u)`、`pcma(g711a)`

支持录制格式 `flv`,`mp4`，`webm`。

### VR 全景播放器(直播流、点播文件)

可以支持VR 360全景视频播放

支持点播文件，支持直播流。


## 支持定制化服务开发

支持一系类和业务耦合的开发需求。

## 支持作者

### 第一作者

<img src="/wx.jpg"><img src="/alipay.jpg">

### V3版本作者

<img src="/wx-pay-wc.jpg" style="width:333px"><img src="/alipay-wc.jpg" style="width:333px">

# 群

<img src="/qrcode.jpeg">

## qq频道

<img src="/qq-qrcode.jpg">

