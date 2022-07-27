---
sidebar: false
---
<ProDemoPlayer/>

> 注意：pro 试用开发包(10分钟断开, 刷新页面继续测试)
>
> 注意：pro 试用开发包(10分钟断开, 刷新页面继续测试)
>
> 注意：pro 试用开发包(10分钟断开, 刷新页面继续测试)


# 其他demo

## 2x2
#### https
[https://jessibuca.com/pro/2x2-demo.html](https://jessibuca.com/pro/2x2-demo.html)

#### http
[http://jessibuca.monibuca.com/pro/2x2-demo.html](http://jessibuca.monibuca.com/pro/2x2-demo.html)

## 2x2 simd
#### https
[https://jessibuca.com/pro/2x2-simd-demo.html](https://jessibuca.com/pro/2x2-simd-demo.html)

#### http
[http://jessibuca.monibuca.com/pro/2x2-simd-demo.html](http://jessibuca.monibuca.com/pro/2x2-simd-demo.html)


## 3x3
#### https
[https://jessibuca.com/pro/3x3-demo.html](https://jessibuca.com/pro/3x3-demo.html)

#### http
[http://jessibuca.monibuca.com/pro/3x3-demo.html](http://jessibuca.monibuca.com/pro/3x3-demo.html)

## 3x3 simd
#### https
[https://jessibuca.com/pro/3x3-simd-demo.html](https://jessibuca.com/pro/3x3-simd-demo.html)

#### http
[http://jessibuca.monibuca.com/pro/3x3-simd-demo.html](http://jessibuca.monibuca.com/pro/3x3-simd-demo.html)




# MSE H265硬解码

Windows系统下,360浏览器可播放使用MSE加速解码H265。

Windows系统下,win10商店购买hevc解码器后最新edge可硬件加速解码播放H265。

- 使用360浏览器测试
- 使用win10商店购买hevc解码器
- 使用 chrome canary 并开启 `--enable-features=PlatformHEVCDecoderSupport` 进行测试

## mac

```shell
open /Applications/Google\ Chrome\ Canary.app --args --enable-features=PlatformHEVCDecoderSupport
```
## window
通过配置启动项

```shell
--enable-features=PlatformHEVCDecoderSupport
```

# 支持音视频流（TF卡流）的倍数播放

- 支持TF卡流的倍数播放
- 支持自定义帧率(fps)
- 支持音频变速不变调
> 如需要请联系作者定制开发

# 支持语音通讯
支持采集PCM/G711A/G711U格式的数据、支持采样率16000Hz或8000Hz，采样精度32bits或者16bits，支持单通道或双通道。

> 如需要请联系作者定制开发

# 支持crypto解密播放
> 如需要请联系作者定制开发

# 支持webrtc标准流播放
请使用 m7s 提供的webrtc 地址播放

> 如：webrtc://${hostName}${port}/webrtc/play/${streamPath}


# 支持WebTransport协议播放。
请使用 m7s 提供的WebTransport 地址播放

> 如：wt://${hostName}${wtPort}/play/${streamPath}


# 联系作者

<img src="/public/wx.jpg"><img src="/public/alipay.jpg">
