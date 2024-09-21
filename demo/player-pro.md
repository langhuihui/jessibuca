---
sidebar: false
---

<ProDemoPlayer/>

> 注意：pro 试用开发包(1小时断开, 刷新页面继续测试)
>
> 注意：pro 试用开发包(1小时断开, 刷新页面继续测试)
>
> 注意：pro 试用开发包(1小时断开, 刷新页面继续测试)
>
> 如需要购买PRO版本可以联系添加作者微信：bosswancheng


> 完整离线包下载地址：[下载地址](https://jessibuca.com/pro.zip)
>
> 需要本地挂载在Nginx（或者自己搭建一个web-server服务）上面才能跑起来。

> Demo 列表
[Http](http://jessibuca.monibuca.com/pro/index.html)

[Https](https://jessibuca.com/pro/index.html)

# Pro 介绍文档

> Pro版本主要的功能，与开源版的差异性。

[Pro 文档](/pro.html)

# 支持19种直播格式：(live/test为streamPath)

1. ws(s)-raw 即ws(s)://host-name:port/jessica/`live/test` (该协议只对接了[monibuca服务器](https://monibuca.com),其他服务器需要额外对接)
2. ws(s)-flv 即ws(s)://host-name:port/jessica/`live/test`.flv（chrome下ip会报安全错误，建议域名形式访问，检查下端口范围chrome浏览器是否允许，chrome会默认禁用很多端口）
3. http(s)-flv 即http(s)://host-name:port/hdl/`live/test`.flv
4. Hls 即http(s)://host-name:port/hls/`live/test`.m3u8 (支持H264/H265)
5. WebTransport 即wt://host-name:port/play/`live/test` (该协议只对接了[monibuca服务器](https://monibuca.com),其他服务器需要额外对接)
6. Webrtc 即 webrtc://host-name:port/webrtc/play/`live/test` (支持H264/H265, 仅支持https://或者http://localhost环境)
7. Webrtc-zlmediakit 即 webrtc://host-name:port/index/api/webrtc?app=live&stream=`stream-name`&type=play  (支持H264, 仅支持https://或者http://localhost环境)
8. Webrtc-srs 即 webrtc://host-name:port/rtc/v1/play/`live/test`  (支持H264, 仅支持https://或者http://localhost环境)
9. Webrtc-others 即 webrtc://host-name:port/`live/test` (支持H264, 仅支持https://或者http://localhost环境)
10. http(s)-fmp4 即http(s)://host-name:port/your-path/`live/test`.(f)mp4
11. ws(s)-fmp4 即ws(s)://host-name:port/your-path/`live/test`.(f)mp4
12. http(s)-h264 即http(s)://host-name:port/jessica/`live/test`.h264
13. ws(s)-h264 即ws(s)://host-name:port/jessica/`live/test`.h264
14. http(s)-h265 即http(s)://host-name:port/jessica/`live/test`.h265
15. ws(s)-h265 即ws(s)://host-name:port/jessica/`live/test`.h265
16. ws(s)-mpeg4 即ws(s)://host-name:port/your-path/`live/test`.mpeg4
17. http(s)-mpeg4 即http(s)://host-name:port/your-path/`live/test`.mpeg4
18. artc-aliyun Web Rtc 即artc://host-name:port/xxxx(根据阿里云的播放地址来)
19. http(s)-ts 即http(s)://host-name:port/your-path/`live/test`.ts (mpeg-ts 支持H264/H265)
- 注意http协议会有跨域问题，需要设置cors头
- 协议同时支持https、wss
- 同时支持H264和H265编码格式
- 支持mpeg4编码格式
- 支持webcodecs硬解码(H264+H265)和MSE硬解码(H264+H265)
- 支持HLS(H264+H265)软解码、硬解码
- 支持`m7s` webrtc(H264+H265(软解码、硬解码))，
- 支持`zlmediakit` webrtc(H264)
- 支持`srs` webrtc(H264)
- 支持`others` webrtc(H264)
- 支持加密流(国标SM4、XOR加密、m7s加密流)
- 支持裸流(H264+H265)
- 支持Fmp4格式(H264+H265)
- 支持mpeg-ts格式(H264+H265)
- 支持阿里云Web Rtc播放 [阿里云Web Rtc](https://help.aliyun.com/zh/live/user-guide/web-rts-sdk-overview?spm=a2c4g.11186623.0.i1)

<Rice/>

# 支持两种点播格式：(vod/test为streamPath)

1. http(s)-mp4 即http(s)://host-name:port/hdl/`vod/test`.mp4 (支持H264/H265)
2. http(s)-hls 即http(s)://host-name:port/hdl/`vod/test`.m3u8 (支持H264/H265)

- 支持H264和H265编码格式
- 支持硬解码(MediaSource、Webcodec)和软解码(Wasm,Wasm(simd))
- 支持支持加密文件(国标SM4、XOR加密、m7s加密)

# 群
<img src="/public/qrcode.jpeg">


## qq频道
<img src="/public/qq-qrcode.jpg">

