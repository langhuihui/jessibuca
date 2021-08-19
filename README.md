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

- 执行yarn 或者npm i
- 执行yarn dev 或者 npm run dev

## API
[API](/api.md)

## 源码目录结构

- obj 存放emscripten编译好的ffmpeg解码库的字节码库
- demo/public 存放编译输出的js和wasm文件
- src 存放js源码

## 打包js

执行yarn build 或者 npm run build

## 引用关系
src/index.js 引用了jessibuca.js和worker.js 目的是将两个js打包到一起，实际执行中会根据当前是否为主线程来判断是否加载worker线程（把自己加载到webworker中）
worker.js 会引用 demo/public/ff.js （该文件由编译C++文件所得，用于加载wasm）

## 编译C++代码

执行yarn build:wasm 或者 npm run build:wasm

## 基本原理

<img src="/demo/public/tech.png">
