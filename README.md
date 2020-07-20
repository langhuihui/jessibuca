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
  details: 软解码H.264/H.265+AAC/MP3/Speex流，WebGL视频渲染，WebAudio音频播放。
footer: GPL Licensed | Copyright © 2020-present dexter
---
# 简介

Jessibuca是一款开源的纯H5直播流播放器，通过Emscripten将音视频解码库编译成Js（ams.js/wasm)运行于浏览器之中。兼容几乎所有浏览器，可以运行在PC、手机、微信中，无需额外安装插件。

## 功能
- 支持解码H.264视频(Baseline, Main, High Profile全支持，支持解码B帧视频)
- 支持解码H.265视频（flv id == 12）
- 支持解码AAC音频(LC,HE,HEv2 Profile全支持)
- 支持解码MP3音频以及Speex音频格式
- 可设置播放缓冲区时长，可设置0缓冲极限低延迟（网络抖动会造成卡顿现象）
- 支持智能不花屏丢帧，长时间播放绝不累积延迟。
- 可创建多个播放实例
- 程序精简，经CDN加速，GZIP压缩（实际下载500k），加载速度更快
- 同时支持http-flv和websocket-flv协议以及websocket-raw私有协议（裸数据，传输量更小，需要搭配Monibuca服务器）
注：以http-flv请求时，存在跨域请求的问题，需要设置access-control-allow-origin, websocket-flv默认不存在此问题
- 支持HTTPS/WSS加密视频传输，保证视频内容传输安全
- 手机浏览器内打开视频不会变成全屏播放
## 使用方法
自动播放http-flv格式
```html
<div id="container" style="width:800px;height:600px"></div>
<script src="./renderer.js"></script>
<script>
  var container = document.getElementById("container");
  var jessibuca = new Jessibuca({ container, decoder: "ff.js" ,videoBuffer:0.2});
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
  var jessibuca = new Jessibuca({ container, decoder: "ff.js" ,videoBuffer:0.2});
  function play(){
    jessibuca.play("ws://localhost/live/user1")
  }
</script>
```

## 源码目录结构

- obj 存放emscripten编译好的各种解码库的字节码库
- public 存放编译输出的js和wasm文件以及renderer.js
- thirdparty 各种第三方解码库的代码（已修改）和编译脚本 

## 编译

编译命令是python make.py 加上参数构成

参数包括
- -v 视频解码库，有效值为ff（ffmpeg）、libhevc、libde265 如果不传参数代表使用avc库（只支持h264 的baseline）
- -a 音频解码库，有效值为mp3、speex、aac，如果不传参数则无音频解码（ffmpeg自带aac解码）
- --wasm 表示编译成WebAssembly格式
- -o 代表输出文件路径，默认值是public/Jessibuca.js

示例:
ffmpeg（h264-aac）
```bash
python make.py -v ff -o public/ff.js
```
avc-mp3组合
```bash
python make.py -a mp3 -o public/avc_mp3.js
```
libhevc-aac组合
```bash
python make.py -v libhevc -a aac -o public/h265_aac.js
```
## 基本原理

<img src="/tech.png">