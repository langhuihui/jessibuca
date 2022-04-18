# Document

## 常见问题

### 延迟时间

实际测试 videoBuffer设置为100 毫秒，实测延迟300-400毫秒。低于1秒，达到毫秒级低延迟。

### 多分屏超过 6 路不能播放

chrome限制同源http请求最多6个并发
> 浏览器对同源 HTTP/1.x 连接的并发个数有限制, 几种方式规避这个问题：

1. 通过 WebSocket 协议访问直播流，如：播放 WS-FLV 直播流
2. 开启 HTTPS, 通过 HTTPS 协议访问直播流

### IIS下wasm返回404错误

> 使用IIS作为webserver，程序已经上传到服务器，访问js文件正常，但访问wasm文件返回404错误。

To get rid of the 404 add a new Mime Type for Wasm, it’s not currently in IIS 10 (or below).

Click Start > Run > type InetMgr > expand Sites > select the app > Mime Types > Add:

Extension: .wasm (dot wasm)
MIMEType: application/wasm


### 优化加载速度

1. 将js程序进行gzip压缩


### Media Source Extensions 硬解码H265
Windows系统下,360浏览器可播放使用MSE加速解码H265, win10商店购买hevc解码器后最新edge可硬件加速解码播放H265.


### webcodecs

#### Chrome86及之后

提供的WebCodecs API来进行硬解码,为实验特性,需要手动开启 enable chrome://flags/#enable-experimental-web-platform-features, or pass --enable-blink-features=WebCodecs flag via the command line.

#### Chrome94之后
Desktop,Android,Webview中已默认开启!

需要https加载web,播放https/wss-flv流. 如果控制台打印 "WCS is not supported or experimental-web-platform-features not enabled" 请将当前页面使用https访问


### http vs https

#### http

在http 协议里面，是不能播放https 或者 wss 协议的，会报跨域报错。


#### https

在https 协议里面，是不能播放http 或者 ws 协议的，会报跨域报错。

### WebRTC

webrtc目前只是本地实现了，还需要服务器端支持才行。

WebRTC标准是不支持h265的。

### OffscreenCanvas这个特性需要特殊的环境和硬件支持吗

默认是关闭的. 如果开启需要设置 forceNoOffscreen 为 false 就可以了。

各个浏览器对于OffscreenCanvas支持程度。

https://caniuse.com/?search=OffscreenCanvas

### 多个iframe一起播放视频，如果有一个视频地址播放不了会导致其他地址也无法播放

#### QUESTION
多个iframe一起播放视频，如果有一个视频地址播放不了会导致其他地址也无法播放。


#### ANSWER

m7s ui 里面 我也是setTimeout 0去渲染的。或者建议你用Promise.resolve的形式去播放。

### MSE加速解码H265

“Windows系统下,360浏览器可播放使用MSE加速解码H265, win10商店购买hevc解码器后最新edge可硬件加速解码播放H265"

### vue3 下面使用Typescript

https://github.com/langhuihui/jessibuca/issues/137

https://github.com/bosscheng/jessibuca-vue-ts-demo


### destroy释放内存

https://github.com/langhuihui/jessibuca/issues/135

> 经测试，放到node+express服务中，16画面轮询跑了14个小时没有崩溃，chrome浏览器内存达到2G左右，destroy优化的效果还是很明显的，感谢大佬！


### 关于视频卡顿

可以通过设置videoBuffer 变大些，一般1s，2s，3s都是可以的


### 关于黑屏

在设置了videBuffer 为1s 之后， useMSE 和useWCS 下面会有1s的黑屏，wasm下面首屏会第一时间加载出来。但是页面会卡顿1s 。

### 对于wasm

我是先拿到i帧去解码，然后就播放了。然后后面的数据进行缓存。这段时间内是不播放解码出来的视频数据的


### 关于pts值

可以的，http://jessibuca.monibuca.com/api.html#stats 监听 stats ,一秒回调一次，

buf: 当前缓冲区时长，单位毫秒,
fps: 当前视频帧率,
abps: 当前音频码率，单位bit,
vbps: 当前视频码率，单位bit，
ts:当前视频帧pts，单位毫秒

### 将录制的视频保存在安卓手机相册中，显示的时长为0，并且无法播放。

https://github.com/langhuihui/jessibuca/issues/126

现象：将录制的视频保存在安卓手机相册中，显示的时长为0，并且无法播放。但是在对应的文件路径中找到源文件是能播放的，但是依然不显示时长。


这是录制的是webm 格式的视频，对于移动端的兼容性不是很好。等后续支持录制MP4格式(MPEG-4)的视频录制就可以解决这个问题了。

另外：
> MP4格式支持在IOS VLC播放器显示时长播放，Android VLC播放器无法显示时长播放，PC VLC播放器可以播放

### 创建多个以上播放实例会非常卡顿，还会导致页面黑屏

例如 h265,1280*720，wasm 肯定会卡顿的。 建议降低分辨率。还需要增大videoBuffer 大小。

#### h265 优化方案

1. 降低分辨率
2. 增大videoBuffer大小
3. 设置hasAudio 为false，不demux和decode音频数据。
4. 条件允许(支持OffscreenCanvas)也可以配合设置 forceNoOffscreen 为 false  开启离屏渲染模式，提升性能。

#### h264 优化方案

1. 降低分辨率
2. 增大videoBuffer大小
3. 设置hasAudio 为false，不demux和decode音频数据。
4. 条件允许(支持OffscreenCanvas)也可以配合设置 forceNoOffscreen 为 false  开启离屏渲染模式，提升性能。
5. 如果是https情况下 设置 useWCS 为 true。
6. 如果是http情况下 设置 useMSE 为 true。（暂不支持 forceNoOffscreen 参数）

> 某些显卡在支持OffscreenCanvas上面会存在问题，所以谨慎使用。




### 可以播放本地视频吗？

不可以，jessibuca 定位是直播流播放器。


### 是否可支持倍速播放

开源版不支持，可以支持定制开发。


###  对于延迟

可能的原因
- 网络加载的延迟
- 软解码的延迟
- 渲染的延迟

一般来说，如果在用户网络环境较好的情况下，渲染由于使用了WebGL，很难造成瓶颈（操作很单一），其中一般会因为软解码性能不足造成不停卡顿及延迟。

优化因为软解码性能不足造成的延迟，我们一般从几个地方着手：

- 视频的profile：相比于main/high而言，baseline不包含B帧，解码消耗更低
- 视频帧率：过高的帧率会造成软解码跟不上，可以试着降低帧率，例如24fps
- 视频码率：码率越高，视频富含的细节越多，也越清晰，但是会消耗更多的解码性能，可以试着降低码率
- 视频分辨率：过高的视频会造成单帧传递的数量极大

### 视频颜色变灰色（软解码）

原因

- 视频流的格式 不是 yuv420p

可能的视频格式是：yuvj422p 格式。


### 视频渲染发绿（软解码）

#### 原因

- 对于宽度不是8的倍数的时候就会出现这样的问题

原问题： https://github.com/langhuihui/jessibuca/issues/152

例如：540x960 分辨率

在使用WebGL对YUV420P进行渲染时，WebGL图像预处理默认每次取4字节的数据，但是540x960分辨率下的U、V分量宽度是540/2=270不能被4整除，导致绿屏。

#### 解决方案

1. 可以通过设置`gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);` 的方式来解决，但是会损耗一部分性能。
2. `openWebglAlignment` 设为 `true`
