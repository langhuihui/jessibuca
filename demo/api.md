# API

## Jessibuca(options)

options 支持的参数有：

### container
- **类型**：`DOM` or `string`
- **用法**：

播放器容器。

> 若为 string ，则底层调用的是 document.querySelector(container)

源码
```js

let $container = options.container;
if (typeof options.container === 'string') {
    $container = document.querySelector(options.container);
}
if (!$container) {
    throw new Error('Jessibuca need container option');
    return;
}
```


### videoBuffer
- **类型**：`number`
- **默认值**：`1`
- **用法**：
设置最大缓冲时长，单位秒，播放器会自动消除延迟。

### decoder
- **类型**：`string`
- **默认值**：`decoder.js`
- **用法**：

worker地址

> 默认引用的是根目录下面的decoder.js文件 ，decoder.js 与 decoder.wasm文件必须是放在同一个目录下面。


### forceNoOffscreen
- **类型**：`boolean`
- **默认值**：true
- **用法**：

是否不使用离屏模式（提升渲染能力）


### hiddenAutoPause
- **类型**：`boolean`
- **默认值**：false
- **用法**：

> 见

是否开启当页面的'visibilityState'变为'hidden'的时候，自动暂停播放。


### hasAudio
- **类型**：`boolean`
- **默认值**：true
- **用法**：

是否有音频，如果设置`false`，则不对音频数据解码，提升性能。


### rotate
- **类型**：`number`
- **默认值**：0
- **用法**：

设置旋转角度，只支持，0(默认) ，180，270 三个值。

### isResize
- **类型**：`boolean`
- **默认值**：`true`
- **用法**：

1. 当为`true`的时候：视频画面做等比缩放后,高或宽对齐canvas区域,画面不被拉伸,但有黑边。 等同于 `setScaleMode(1)`
2. 当为`false`的时候：视频画面完全填充canvas区域,画面会被拉伸。等同于 `setScaleMode(0)`



### isFullResize
- **类型**：`boolean`
- **默认值**：`false`
- **用法**：
1. 当为`true`的时候：视频画面做等比缩放后,完全填充canvas区域,画面不被拉伸,没有黑边,但画面显示不全。等同于 `setScaleMode(2)`


### isFlv
- **类型**：`boolean`
- **默认值**：`false`
- **用法**：
1. 当为`true`的时候：ws协议不检验是否以.flv为依据，进行协议解析。





### debug
- **类型**：`boolean`
- **默认值**：`false`
- **用法**：
是否开启控制台调试打印

### timeout
- **类型**：`number`
- **默认值**：`10`
- **用法**：

1. 设置超时时长, 单位秒
2. 在连接成功之前(loading)和播放中途(heart),如果超过设定时长无数据返回,则回调timeout事件


### heartTimeout
- **类型**：`number`
- **默认值**：`10`
- **用法**：

1. 设置超时时长, 单位秒
2. 在连接成功之前,如果超过设定时长无数据返回,则回调timeout事件

### loadingTimeout
- **类型**：`number`
- **默认值**：`10`
- **用法**：

1. 设置超时时长, 单位秒
2. 在连接成功之前,如果超过设定时长无数据返回,则回调timeout事件


### supportDblclickFullscreen
- **类型**：`boolean`
- **默认值**：`false`
- **用法**：
是否支持屏幕的双击事件，触发全屏，取消全屏事件。

### showBandwidth
- **类型**：`boolean`
- **默认值**：`false`
- **用法**：
是否显示网速

### operateBtns
- **类型**：`object`
- **默认值**：`{
            fullscreen:false,
            screenshot: false,
            play: false,
            audio: false,
            record:false
            }`
- **用法**：
配置操作按钮
其中

1. fullscreen 是否显示全屏按钮
2. screenshot 是否显示截图按钮
3. play 是否显示播放暂停按钮
4. audio 是否显示声音按钮
5. record 是否显示录制按钮

### keepScreenOn
- **类型**：`boolean`
- **默认值**：`false`
- **用法**：
开启屏幕常亮，在手机浏览器上, canvas标签渲染视频并不会像video标签那样保持屏幕常亮。

### isNotMute
- **类型**：`boolean`
- **默认值**：`false`
- **用法**：
是否开启声音，默认是关闭声音播放的。

### loadingText
- **类型**：`string`
- **默认值**：``
- **用法**：
加载过程中文案。

### background
- **类型**：`string`
- **默认值**：``
- **用法**：
背景图片。

### useMSE
- **类型**：`boolean`
- **默认值**：`false`
- **用法**：
是否开启MediaSource硬解码

> 视频编码只支持H.264视频（Safari on iOS不支持）

> 不支持 forceNoOffscreen 为 false (开启离屏渲染)

### useWCS
- **类型**：`boolean`
- **默认值**：`false`
- **用法**：
  是否开启Webcodecs硬解码

> 视频编码只支持H.264视频 (需在chrome 94版本以上，需要https或者localhost环境)

> 支持 forceNoOffscreen 为 false （开启离屏渲染）


### hotKey
- **类型**：`boolean`
- **默认值**：`false`
- **用法**：
  是否开启键盘快捷键

> 目前支持的键盘快捷键有：esc -> 退出全屏；arrowUp -> 声音增加；arrowDown -> 声音减少；

## 方法

### setDebug(flag)
- **参数**：
   - `{boolean} flag`
- **用法**：
是否开启控制台调试打印
```js
// 开启
jessibuca.setDebug(true)
// 关闭
jessibuca.setDebug(false)
```

### mute()
- **用法**：
静音
```js
jessibuca.mute()
```

### cancelMute()
- **用法**：
取消静音
```js
jessibuca.cancelMute()
```

### audioResume()
- **用法**：
留给上层用户操作来触发音频恢复的方法。

iPhone，chrome等要求自动播放时，音频必须静音，需要由一个真实的用户交互操作来恢复，不能使用代码。

https://developers.google.com/web/updates/2017/09/autoplay-policy-changes



### setTimeout(time)
- **参数**：
   - `{number} time`
- **用法**：

设置超时时长, 单位秒
在连接成功之前和播放中途,如果超过设定时长无数据返回,则回调timeout事件

```js
jessibuca.setTimeout(10)

jessibuca.on('timeout',function(){
    //
});
```

### setScaleMode(mode)
- **参数**：
   - `{number} mode`

- **用法**：

1. `0` 视频画面完全填充canvas区域,画面会被拉伸  等同于参数 `isResize` 为false
2. `1` 视频画面做等比缩放后,高或宽对齐canvas区域,画面不被拉伸,但有黑边 等同于参数 `isResize` 为true
3. `2` 视频画面做等比缩放后,完全填充canvas区域,画面不被拉伸,没有黑边,但画面显示不全 等同于参数 `isFullResize` 为true

```js
jessibuca.setScaleMode(0)

jessibuca.setScaleMode(1)

jessibuca.setScaleMode(2)
```

### pause()
- **返回**：
    - `{Promise}`
- **用法**：
暂停播放
```js
jessibuca.pause().then(()=>{
    console.log('pause success')

    jessibuca.play().then(()=>{

    }).catch((e)=>{

    })

}).catch((e)=>{
    console.log('pause error',e);
})

```
可以在pause 之后，再调用 `play()`方法就继续播放之前的流。

### close()
- **用法**：
关闭视频,不释放底层资源
```js
jessibuca.close();
```


### destroy()
- **用法**：
关闭视频，释放底层资源
```js
jessibuca.destroy()
```

### clearView()
- **用法**：
清理画布为黑色背景
```js
jessibuca.clearView()
```

### play([url])
- **参数**：
   - `{string} url`

- **返回**：
  - `{Promise}`
- **用法**：
播放视频
```js

jessibuca.play('url').then(()=>{
    console.log('play success')
}).catch((e)=>{
    console.log('play error',e)
})
//
jessibuca.play()
```

### resize()
- **用法**：
重新调整视图大小

### setBufferTime(time)
- **参数**：
   - `{number} time`

- **用法**：
设置最大缓冲时长，单位秒，播放器会自动消除延迟。

等同于 `videoBuffer` 参数。

```js
// 设置 200ms 缓冲
jessibuca.setBufferTime(0.2)
```

### setRotate(deg)
- **参数**：
   - `{number} deg`
- **用法**：
设置旋转角度，只支持，0(默认) ，180，270 三个值。

> 可用于实现监控画面小窗和全屏效果，由于iOS没有全屏API，此方法可以模拟页面内全屏效果而且多端效果一致。

```js
jessibuca.setRotate(0)

jessibuca.setRotate(90)

jessibuca.setRotate(270)
```



### setVolume(volume)
- **参数**：
   - `{number} volume`

- **用法**：

1. 设置音量大小，取值0 — 1
2. 当为0时，完全无声
3. 当为1时，最大音量，默认值

> 区别于 mute 和 cancelMute 方法，虽然设置setVolume(0) 也能达到 mute方法，但是mute 方法是不调用底层播放音频的，能提高性能。而setVolume(0)只是把声音设置为0 ，以达到效果。

```js
jessibuca.setVolume(0.2)

jessibuca.setVolume(0)

jessibuca.setVolume(1)
```

### hasLoaded()
- **返回值**：`boolean`
- **用法**：
返回是否加载完毕
```js
var result = jessibuca.hasLoaded()
console.log(result) // true
```

### setKeepScreenOn()
- **用法**：
开启屏幕常亮，在手机浏览器上, canvas标签渲染视频并不会像video标签那样保持屏幕常亮。
H5目前在chrome\edge 84, android chrome 84及以上有原生亮屏API, 需要是https页面
其余平台为模拟实现，此时为兼容实现，并不保证所有浏览器都支持

```js
jessibuca.setKeepScreenOn()
```
### setFullscreen(flag)
- **参数**：
   - `{boolean} flag`

- **用法**：
全屏(取消全屏)播放视频
```js

jessibuca.setFullscreen(true)
//
jessibuca.setFullscreen(false)
```


### screenshot(filename, format, quality,type)
- **参数**：
    - `{string} filename`
    - `{string} format`
    - `{number} quality`
    - `{string} type`
- **用法**：

截图，调用后弹出下载框保存截图
1. filename: 可选参数, 保存的文件名, 默认 `时间戳`
2. format : 可选参数, 截图的格式，可选png或jpeg或者webp ,默认 `png`
3. quality: 可选参数, 当格式是jpeg或者webp时，压缩质量，取值0 ~ 1 ,默认 `0.92`
4. type: 可选参数, 可选download或者base64或者blob，默认`download`

```js

jessibuca.screenshot("test","png",0.5)


const base64 = jessibuca.screenshot("test","png",0.5,'base64')


const fileBlob = jessibuca.screenshot("test",'blob')

```

### startRecord(fileName,fileType)
- **参数**：
    - `{string} filename`
    - `{string} fileType`

- **用法**：
  开始录制。
1. fileName: 可选，默认时间戳
2. fileType: 可选，默认webm，支持webm 和mp4 格式


```js
jessibuca.startRecord('xxx','webm')
```


### stopRecordAndSave()

- **用法**：
  暂停录制并下载。
```js
jessibuca.stopRecordAndSave()
```



### isPlaying()
- **返回值**：`boolean`
- **用法**：
  返回是否正在播放中状态。
```js
var result = jessibuca.isPlaying()
console.log(result) // true
```

### isMute()
- **返回值**：`boolean`
- **用法**：
  返回是否静音。
```js
var result = jessibuca.isMute()
console.log(result) // true
```


### isRecording()
- **返回值**：`boolean`
- **用法**：
  返回是否正在录制。
```js
var result = jessibuca.isRecording()
console.log(result) // true
```

### on(event, callback)
- **参数**：
    - `{string} event`
    - `{function} callback`
- **用法**：
监听方法

```js

jessibuca.on("load",function(){console.log('load')})
```

## 事件

### load
监听 jessibuca 初始化事件。

```js

jessibuca.on("load",function(){console.log('load')})
```

### timeUpdate
视频播放持续时间，单位ms
```js
jessibuca.on('timeUpdate',function (ts) {console.log('timeUpdate',ts);})
```

### videoInfo
当解析出视频信息时回调，2个回调参数
1. width：视频宽
2. height：视频高
```js

jessibuca.on("videoInfo",function(data){console.log('width:',data.width,'height:',data.width)})
```

### audioInfo
当解析出音频信息时回调，2个回调参数
1. numOfChannels：声频通道
2. sampleRate 采样率

```js

jessibuca.on("audioInfo",function(data){console.log('numOfChannels:',data.numOfChannels,'sampleRate',data.sampleRate)})
```

### log
信息，包含错误信息
```js

jessibuca.on("log",function(data){console.log('data:',data)})
```

### error
错误信息

目前已有的错误信息：
1. jessibuca.ERROR.playError ;播放错误，url 为空的时候，调用play方法
2. jessibuca.ERROR.fetchError ;http 请求失败
3. jessibuca.ERROR.websocketError;  websocket 请求失败
4. jessibuca.ERROR.webcodecsH265NotSupport; webcodecs 解码 h265 失败
5. jessibuca.ERROR.mediaSourceH265NotSupport; mediaSource 解码 h265 失败
6. jessibuca.ERROR.wasmDecodeError ; wasm 解码失败




```js

jessibuca.on("error",function(error){
    if(error === jessibuca.ERROR.fetchError){
        //
    }
    else if(error === jessibuca.ERROR.webcodecsH265NotSupport){
        //
    }
    console.log('error:',error)
})
```

### kBps
当前网速， 单位KB 每秒1次,
```js

jessibuca.on("kBps",function(data){console.log('kBps:',data)})
```

### start
渲染开始
```js

jessibuca.on("start",function(){console.log('start render')})
```


### timeout
当设定的超时时间内无数据返回,则回调

1. jessibuca.TIMEOUT.loadingTimeout ; 同loadingTimeout
2. jessibuca.TIMEOUT.delayTimeout  ; 同delayTimeout

```js

jessibuca.on("timeout",function(error){console.log('timeout:',error)})
```

### loadingTimeout
当play()的时候，如果没有数据返回，则回调
```js

jessibuca.on("loadingTimeout",function(){console.log('timeout')})
```


### delayTimeout
当播放过程中，如果超过timeout之后没有数据渲染，则抛出异常。
```js

jessibuca.on("delayTimeout",function(){console.log('timeout')})
```



### fullscreen
当前是否全屏
```js
jessibuca.on("fullscreen",function(flag){console.log('is fullscreen',flag)})
```
### play
触发播放事件
```js
jessibuca.on("play",function(flag){console.log('play')})
```
### pause
触发暂停事件
```js
jessibuca.on("pause",function(flag){console.log('pause')})
```

### mute
触发声音事件，返回boolean值
```js
jessibuca.on("mute",function(flag){console.log('is mute',flag)})
```
### stats
流状态统计，流开始播放后回调，每秒1次。

1. buf: 当前缓冲区时长，单位毫秒,
2. fps: 当前视频帧率,
3. abps: 当前音频码率，单位bit,
4. vbps: 当前视频码率，单位bit，
5. ts:当前视频帧pts，单位毫秒
```js
jessibuca.on("stats",function(s){console.log("stats is",s)})
```
### performance
渲染性能统计，流开始播放后回调，每秒1次。

- 0: 表示卡顿
- 1: 表示流畅
- 2: 表示非常流程

```js
jessibuca.on("performance",function(performance){console.log("performance is",performance)})
```

### recordStart
录制开始的事件

```js
jessibuca.on("recordStart",function(){console.log("record start")})
```

### recordEnd
录制结束的事件

```js
jessibuca.on("recordEnd",function(){console.log("record end")})
```

### recordingTimestamp
录制的时候，返回的录制时长，1s一次

```js
jessibuca.on("recordingTimestamp",function(timestamp){console.log("recordingTimestamp is",timestamp)})
```

### playToRenderTimes
监听调用play方法 经过 初始化-> 网络请求-> 解封装 -> 解码 -> 渲染 一系列过程的时间消耗

```js
jessibuca.on("playToRenderTimes",function(times){console.log("playToRenderTimes is",times)})
```
数据结构如下。
```
{
    playInitStart: '', //1
    playStart: '', // 2
    streamStart: '', //3
    streamResponse: '', // 4
    demuxStart: '', // 5
    decodeStart: '', // 6
    videoStart: '', // 7
    playTimestamp: '',// playStart- playInitStart
    streamTimestamp: '',// streamStart - playStart
    streamResponseTimestamp: '',// streamResponse - streamStart
    demuxTimestamp: '', // demuxStart - streamResponse
    decodeTimestamp: '', // decodeStart - demuxStart
    videoTimestamp: '',// videoStart - decodeStart
    allTimestamp: '' // videoStart - playInitStart
}

```




