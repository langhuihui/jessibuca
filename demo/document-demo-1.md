# Document (常见问题)
-
-
-
-
-
-
-
-
-
-
<Rice/>

## 常见问题(案例)


###  wasm 文件抛出 Uncaught RuntimeError: memory access out of bounds 异常

客户反馈问题：

<img src="/public/img/doc-demo/1.png">

这里可以知道，是解码`音频`数据的时候报错的。音频格式是 `pcma`的数据，每一帧的时间戳戳是80ms，每一帧的数据长度是640字节。这里的问题是，解码的时候，解码器解码的数据长度不对，导致内存越界。

然后通过分析流数据发现

<img src="/public/img/doc-demo/2.jpg">


<img src="/public/img/doc-demo/3.jpg">

流的数据是有些大于640字节的，这里就是导致内存越界的原因。






