---
layout: home
hero:
  name: Jessibuca
  text: |-
    纯 H5 低延迟
    直播播放器
  tagline: 开源版 · 无插件 · H.264 / H.265 软解 · 1 秒内延迟
  image:
    src: /logo.png
    alt: Jessibuca
  actions:
    - theme: brand
      text: 打开播放器
      link: /player
    - theme: alt
      text: 查看文档
      link: /document
    - theme: alt
      text: Pro 版本
      link: /player-pro
features:
  - icon: ⚡
    title: 无插件
    details: 纯 JavaScript 实现。H.264 / H.265 走 WASM 软解，H.264 还可走 MSE / WebCodecs。
  - icon: ⏱️
    title: 1 秒内低延迟
    details: PC、Android、iOS WebView 均可做到一秒以内的直播延迟（开源版 WASM 建议 720P 内）。
  - icon: 📡
    title: 开源协议
    details: 开源版支持 ws-raw（M7S 私有格式）、HTTP-FLV、WS-FLV。HLS / 裸流 / WebRTC 见 Pro。
  - icon: 🎬
    title: H.264 / H.265 软解
    details: 开源版 H.265 为 WASM 软解。H.265 MSE / WebCodecs 硬解仅 Pro 支持。
  - icon: 🎛️
    title: 原子化控制栏
    details: 加载、播放、音量、截图、全屏、流量显示均可按需开关。开源版可录 WebM。
  - icon: 🖥️
    title: 多实例解码
    details: 支持多画面同时播放，WebWorker 解码。1080P+ SIMD / 多线程仅 Pro 支持。
---
