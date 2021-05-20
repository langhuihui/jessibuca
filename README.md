---
home: true
heroImage: /logo.png
heroText: Jessibuca
tagline: 纯H5直播流播放器
actionText: 打开播放器
actionLink: /player.md
sidebarDepth: 2
sidebar: auto
features:
- title: 无插件
  details: 基于ASM.js/WebAssembly(wasm)实现的纯JavaScript直播播放器
- title: 低延时
  details: 能够在 PC\Android\iOS 浏览器Webview 内实现2秒以内低延迟直播播放
- title: 支持H265
  details: 软解码H.264/H.265+AAC/PCMA/PCMU流，WebGL视频渲染，WebAudio音频播放。
footer: GPL Licensed | Copyright © 2020-present dexter
---
# 简介

Jessibuca是一款开源的纯H5直播流播放器，通过Emscripten将音视频解码库编译成Js（ams.js/wasm)运行于浏览器之中。兼容几乎所有浏览器，可以运行在PC、手机、微信中，无需额外安装插件。

## 功能
- 支持解码H.264视频(Baseline, Main, High Profile全支持，支持解码B帧视频)
- 支持解码H.265视频（flv id == 12）
- 支持解码AAC音频(LC,HE,HEv2 Profile全支持)
- 支持解码PCMA音频以及PCMU音频格式
- 可设置播放缓冲区时长，可设置0缓冲极限低延迟（网络抖动会造成卡顿现象）
- 支持智能不花屏丢帧，长时间播放绝不累积延迟。
- 可创建多个播放实例
- 程序精简，经CDN加速，GZIP压缩（实际下载500k），加载速度更快
- 同时支持http-flv和websocket-flv协议以及websocket-raw私有协议（裸数据，传输量更小，需要搭配Monibuca服务器）
注：以http-flv请求时，存在跨域请求的问题，需要设置access-control-allow-origin, websocket-flv默认不存在此问题
- 支持HTTPS/WSS加密视频传输，保证视频内容传输安全
- 手机浏览器内打开视频不会变成全屏播放

## 本地测试

安装 vuepress (npm install -g vuepress)
执行 vuepress dev .

## 使用方法
自动播放http-flv格式
```html
<div id="container" style="width:800px;height:600px"></div>
<script src="./renderer.js"></script>
<script>
  var container = document.getElementById("container");
  var jessibuca = new Jessibuca({ container, decoder: "worker.js" ,videoBuffer:0.2});
  jessibuca.onLoad = function () {
      this.play("http://localhost/live/user1.flv")
  }
</script>
```
程序控制播放websocket-raw格式
```html
<div id="container" style="width:800px;height:600px"></div>
<script src="./renderer.js"></script>
<button onclick="play">播放</button>
<script>
  var container = document.getElementById("container");
  var jessibuca = new Jessibuca({ container, decoder: "worker.js" ,videoBuffer:0.2});
  function play(){
    jessibuca.play("ws://localhost/live/user1")
  }
</script>
```
## API
[API](/api.md)

## 源码目录结构

- obj 存放emscripten编译好的ffmpeg解码库的字节码库
- .vuepress/public 存放编译输出的js和wasm文件

## 编译

编译命令是 python make.py --wasm

## 基本原理

<img src="/tech.png">
