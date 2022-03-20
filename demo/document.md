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


### 创建多个以上播放实例会非常卡顿，还会导致页面黑屏

例如 h265,1280*720，wasm 肯定会卡顿的。 建议降低分辨率。还需要增大videoBuffer 大小。


### 可以播放本地视频吗？

不可以，jessibuca 定位是直播流播放器。


### 是否可支持倍速播放

开源版不支持，可以支持定制开发。
