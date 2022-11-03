# 介绍
基于Stream流式编程的流解封装模块

# 使用

```ts
import { Demuxer } from 'jv4-demuxer'
import OPut from 'oput'
const oput = new OPut()

const demuxer = new Demuxer({
  type: 'flv',
  read: (need) => oput.read(need)
})

// 读取数据
const inputPipe = new ReadableStream({
  start(controller) {
    dc.onmessage = (evt) => {
      controller.enqueue(evt.data)
    }
  }
}).pipeTo(new WritableStream(oput))

// 视频可读流
const videoOuput = demuxer.videoReadable.pipeTo(new WriteableStream({
  write(chunk: EncodedVideoChunkInit) {
    console.log('video', chunk)
  }
}))

// 音频可读流
const audioOuput = demuxer.audioReadable.pipeTo(new WriteableStream({
  write(chunk: EncodedAudioChunkInit) {
    console.log('audio', chunk)
  }
}))
```
```