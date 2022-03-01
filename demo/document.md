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


