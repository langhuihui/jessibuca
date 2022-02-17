# Document

## 常见问题

### 多分屏超过 6 路不能播放

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
