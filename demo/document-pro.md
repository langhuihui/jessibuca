# Document (常见问题)
<Rice/>

## 常见问题


### initDecoderWorkerTimeout 异常

这个报错是加载decoder worker 解码器超时导致的。

#### 检查

查看 `F12` 控制台的 `network` 选项卡，看看解码器的加载情况。

一般的解码器有：

- decoder-pro-simd.js
- decoder-pro-f-simd.js
- decoder-pro.js
- decoder-pro-hard-not-wasm.js
- decoder-pro-hard.js
- decoder-pro-old.js


主要是看这些文件是否加载成功，并且成功返回正确的内容。

看下对应的 `wasm` 胶水文件是否加载成功，并且成功返回正确的内容。
