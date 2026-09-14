
# 哈哈
<Rice/>


## 关于flv点播支持的依赖条件


需要把 `duration`,`width`,`height`,`keyframes` 字段放metaData中，这样才能在播放器中获取到这些信息。


### keyframes

其中 `keyframes` 是一个数组，每个元素是一个对象，包含了 `filepositions` 和 `times` 字段，`filepositions` 是关键帧在文件中的位置，`times` 是关键帧的时间戳。

`times` 时间点的单位是`秒`。`filepositions` 文件位置的单位是`字节`，位置是关键帧的起始地址，即 previous tagsize。

```
{
  duration: 60, // 时长 单位秒
  width: 640, // 宽
  height: 360, // 高
  keyframes: [
    {
      filepositions: [0, 100, 200, 300], // 关键帧在文件中的位置
      times: [0, 10, 20, 30] // 关键帧的时间戳,单位秒
    }
  ]
}
```



