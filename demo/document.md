
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

## 常见问题

### 推荐配置


#### http

```js
{
    useMSE:true,
    autoWasm:true
}
```

#### https

```js
{
    useWCS:true,
    autoWasm:true
}
```


### vue、react 推荐

#### vue

#### 关于new Jessibuca 之后的实例绑定

1. 推荐绑定在 `this` 上面，不推荐绑定在`data` 对象上面，不然会触发无效的事件监听。
2. 或者在实例的时候，绑定在`data`上面的时候，命名的时候以 `_` 或者`$` 开头，这样也不会触发无效的事件监听。
在 `vue` 中
> 以 _ 或 $ 开头的属性将不会被组件实例代理，因为它们可能和 Vue 的内置属性、API 方法冲突。你必须以 this.$data._property 的方式访问它们。

见： https://cn.vuejs.org/api/options-state.html#data

```vue
// 可以挂载在Vue上面
Vue.prototype.$player = new Jessibuca({

})
```

```vue
// 也可以挂载在 $options 上面
this.$options.jessibuca = new Jessibuca({

})
```


#### react
#### 关于new Jessibuca 之后的实例绑定

推荐绑定在 `this` 上面，不推荐绑定在`state` 对象上面。


### 是否支持npm（yarn） install 安装

> 暂不支持

因为 项目中用到了`wasm`， node_modules  对于`wasm` 支持度不友好。所以暂不支持。

#### 其他解决方案
可以考虑下把wasm文件编译成base64，然后通过打包合并到js文件中，这样就可以通过npm安装了。

> 但是会增加js的文件大小，所以酌情考虑

> 可以看下`vue-cli-plugin-jessibuca`解决方案


### vue-cli-plugin-jessibuca

jessibuca 没有提供 npm package,只能通过 script 方式引入,所以使用 vue-cli 插件形式自动引入 jessibuca

插件会自动在 html 插入 index.js 的 script 标签,所以可以在代码中直接使用 Jessibuca 全局变量


```
npm install vue-cli-plugin-jessibuca -D
# use yarn
yarn add vue-cli-plugin-jessibuca -D
```

使用
```js
const instance = new window.Jessibuca({})

```





### 关于硬解码和软解码

#### 硬解码

1. `useMSE`和`useWCS`都是硬解码
2. `useMSE` 支持 H264，
3. `useMSE` jessibuca Pro 支持 H265（需要手动开启参数），
4. `useWCS` 只支持H264(浏览器不支持H265)
5. `useMSE` 支持http 和https
6. `useWCS` 只支持https

#### 软解码

1. 支持 H264(低分辨率) 和 H265(低分辨率)
2. jessibuca Pro 支持 H264 和 H265 高分辨率高帧率解码
5. 软解码支持http 和https

如果遇到硬解码失败的时候，会自动切换到wasm软解码

> 单屏情况下，软解码可以比硬解码做到更低的延迟。

> 多屏情况下，因为软解码比较吃CPU 所以在多屏情况下，会出现解码延迟，导致播放延迟，卡顿。

> 多屏情况下，建议使用硬解码。如果硬解码不支持，可以考虑降低屏幕数量。

目前pro对于软解码和硬解码的支持情况，[连接](/pro.html#windows-%E9%BB%98%E8%AE%A4)


### 关于解码（useMSE、useWCS、wasm）优先级

#### useMSE

使用的是浏览器提供的`MediaSource`接口，来进行解码。

- 硬解码
- 兼容性好
- ios safari不支持
- 支持H264和H265解码
- 支持http和https

#### useWCS

使用的是`WebCodec`接口，来进行解码。

- 硬解码
- 支持H264和H265解码
- 支持https
- ios safari不支持
- 兼容性不如mse

#### wasm(simd)

使用的是`webassembly`来进行解码。

- 软解码
- 兼容性好
- 支持H264和H265解码
- 支持http和https

> wasm(simd) 主要是只支持`simd`指令集的浏览器，比如`chrome`，`edge`，`safari`不支持。


#### 优先级

如果同时配置了`useMSE`和`useWCS`，则优先使用`useMSE`，如果`useMSE`不支持，则使用`useWCS`，如果 `useWCS` 不支持，则降级到`wasm`解码。

> useMSE > useWCS > wasm

### 关于是否可以播放rtsp、rtmp协议

#### 回答：浏览器不支持

> 浏览器不支持rtmp:// ，rtsp:// 协议

浏览器只支持，`http(s)://`、 `ws(s)://`、`Webrtc`、`Webtransport` 等协议

因为在js的环境中，无法直接使用tcp或者udp传数据（js没提供接口），而rtsp的流是基于tcp或者udp， 所以纯web的方式目前是没办法直接播放rtsp流的，rtmp也是类似

#### 解决方案（使用M7S）

https://qun.qq.com/qqweb/qunpro/share?_wv=3&_wwv=128&appChannel=share&inviteCode=21lyeGcXwMe&contentID=1qiMyF&businessType=2&from=181174&shareSource=5&biz=ka

### jessibuca.js decoder.js decoder.wasm文件想存放特定地址

一般情况下，建议放置在 `public` 目录下面，如果需要放置在子目录，需要修改的地方有

例如放在 `jessibuca`文件夹

index.html文件
```html
    <script src="./jessibuca/jessibuca.js"></script>
```
对于 new Jessibuca()  的时候

```shell
{
  decoder:'/jessibuca/decoder.js'
}
```


### jessibuca.js decoder.js decoder.wasm文件想通过CDN加载

因为默认情况下 decoder.js 是通过相对路径引入 decoder.wam 文件的。

如果想引用CDN的地址，需要修改成CDN的绝对地址。

所以如果想通过CDN加载，需要修改decoder.js文件

需要配置`decoder` 参数为CDN绝对地址文件。

```js
{
  decoder:'https://your-cdn.com/decoder.js'
}
```

```js
// 修改前 src/worker/index.js

this.decoderWorker = new Worker(player._opt.decoder)

// 修改后 src/worker/index.js
const blob = new Blob([`importScripts("${player._opt.decoder}")`], {"type": 'application/javascript'});
const blobUrl = window.URL.createObjectURL(blob);
this.decoderWorker = new Worker(blobUrl);
```

```js
// 修改前 src/decoder/decoder.js
wasmBinaryFile = 'decoder.wasm';
// 修改后 src/decoder/decoder.js
wasmBinaryFile = 'https://cdn.com/decoder.wasm';
```

然后需要重新执行下 `npm run build` 命令 就可以了。

> Pro版本支持在编译端通过 `rollup.config.js` 配置`WASM 的 CDN`地址。

### 对于渲染元素

#### wasm软解码
默认是通过 `canvas` 进行渲染的

> jessibuca pro 支持 `video` 标签渲染

#### mse 硬解码
默认是通过 `video` 标签进行渲染的

> jessibuca pro 支持 `canvas` 标签渲染

#### webcodecs 硬解码
默认是通过 `canvas` 进行渲染的

> jessibuca pro 支持 `video` 标签渲染

> Pro 支持 `canvas webgl2` 进行渲染的


##### 如果网页中存在大量消耗webgl性能的，会导致播放器不够webgl资源，导致canvas渲染挂掉，出现一个`哭脸表情`的表情。

消耗webgl性能的，比如说，3d背景，地图啥的。

解决方案：
1. 使用video标签渲染。
2. 网页中移除掉些消耗webgl性能的东西。


### 关于延迟丢帧（排除网络延迟）

#### 开源版本

1. 支持WASM智能不花屏丢帧，长时间播放绝不累积延迟。

> 请关闭F12控制台看延迟效果。

#### pro 版本

1. 支持WASM智能不花屏丢帧，长时间播放绝不累积延迟。
2. 支持MSE硬解码智能不花屏丢帧，长时间播放绝不累积延迟。
3. 支持Webcodecs硬解码智能不花屏丢帧，长时间播放绝不累积延迟。


###  对于播放过程中延迟慢慢越来越大的问题

如果是使用的是开源版的，并且是通过wasm解码的，遇到延迟还是慢慢积累，越来越大（从刚开始的0.3到慢慢的几秒），这种情况基本定位出来就是`网络延迟`导致的。

#### 解释网络延迟
请求流的服务器端的出口带宽不够，导致的到客户端的时候，码率不够，导致播放器端收到的数据不够，这个是由于网络问题导致的延迟。

#### 解决方案

1. 优化网络，提高出口带宽。
2. 降低码率，降低码率，降低码率。
3. pro 可以监听到网络延迟，可以等到延迟达到一个阈值，断开连接，重新请求地址（不推荐使用，依然解决不了延迟问题）。


### 播放器的延迟时间

实际测试 videoBuffer设置为100 毫秒，实测延迟300-400毫秒。低于1秒，达到毫秒级低延迟。


### 多分屏超过 6 路不能播放

chrome限制同源http（协议+域名+端口）请求最多6个并发
> 浏览器对同源 HTTP/1.x 连接的并发个数有限制, 几种方式规避这个问题：

1. 通过 WebSocket 协议(chrome下ip会报安全错误，建议域名形式访问，检查下端口范围chrome浏览器是否允许，chrome会默认禁用很多端口)访问直播流，如：播放 WS-FLV 直播流
2. 开启 [HTTP/2.0](https://datatracker.ietf.org/doc/html/rfc7540), 通过 HTTP2协议访问直播流
3. 准备多个域名，每个域名上限6个并发。

#### HTTP/2.0

关于HTTP/2.0的解决方案
1. https://zhuanlan.zhihu.com/p/77803705
2. https://blog.csdn.net/u014552102/article/details/116418790

nginx开启http2

1.https://www.cnblogs.com/flydean/p/15196067.html

### IIS下wasm返回404错误

> 使用IIS作为webserver，程序已经上传到服务器，访问js文件正常，但访问wasm文件返回404错误。

To get rid of the 404 add a new Mime Type for Wasm, it’s not currently in IIS 10 (or below).

Click Start > Run > type InetMgr > expand Sites > select the app > Mime Types > Add:

Extension: .wasm (dot wasm)
MIMEType: application/wasm


### wasm 格式返回错误  Incorrect response MIME type. Expected 'application/wasm'. falling back to arraybuffer instantiation 错误

> Uncaught (in promise) TypeError: Failed to execute 'compile' on 'WebAssembly': Incorrect response MIME type. Expected 'application/wasm'.


> Expected 'application/wasm'., falling back to ArrayBuffer instantiation.
> These warnings refers to incorrect response MIME type of the wasm file.
> In order to fix it, please try to set the MIME filetype to application/wasm
> for the actual wasm file in your server config


> 这个错误通常是由WebAssembly模块加载时失败而导致的。当WebAssembly模块不能成功编译时，JavaScript代码会回退到使用ArrayBuffer实例化来代替。

- 检查浏览器版本是否过旧，尝试更新下浏览器版本。
- 修复下wasm文件的MIME类型，设置为application/wasm

<img src="/public/wasm.png">


类似

```shell
[ERROR] wasm streaming compile failed: TypeError: Failed to execute 'compile' on 'WebAssembly': Incorrect response MIME type. Expected 'application/wasm'.
[ERROR] falling back to ArrayBuffer instantiation
```

因为 从远程服务器加载的Wasm模块文件只有在其HTTP相应结果中被标识为application/wasm类型，才可以被WebAssembly.instantiateStreaming方法正确的编译和处理

查看 network 板块，就可以看到decoder.wasm 的返回格式化， 看下` Response Headers` 下面的`Content-Type` 是否是`application/wasm`

#### 解决方案

1. 用的springboot的tomcat，所以修改tomcat的mime类型，多添加一个wasm的类型
2. 用的是ISS，配置下wasm类型的数据就行了。

Extension: .wasm (dot wasm)
MIMEType: application/wasm

##### apache修改 mime.types，添加

```shell
application/wasm            wasm

```

##### nginx修改mime.types，添加

```shell
application/wasm            wasm;

```
##### 或者 nginx修改nginx.conf，添加

```shell
{
    # 配置 MIME 类型
    types {
        application/wasm wasm;
    }

    # 开启 gzip 压缩
    gzip on;

}
```

#### 通过springBoot 部署的静态资源遇到 `falling back to arraybuffer instantiation` 错误问题

> decoder-pro-simd.js:1 wasm streaming compile failed: CompileError: WebAssembly.instantiateStreaming(): section (code 1, "Type") extends past end of the module (length 11493359, remaining bytes 2877270) @+8

> decoder-pro-simd.js:1 falling back to ArrayBuffer instantiation

检查下是不是通过`maven`方式进行打包的。

> maven 进行打包的时候，使用maven进行资源过滤的时候，会把二进制文件破坏掉。导致内容变大。

https://www.mianshigee.com/note/detail/72131ooi/

##### 解决方案

使用maven进行资源过滤的时候，只要过滤需要过滤的文件，一些二进制文件，比如https证书等，就不要参与资源过滤，否则打包后会破坏文件内容。

<img src="/public/img/maven-wasm.png">


### 优化加载速度

1. 将js程序进行gzip/Brotli压缩
2. 将wasm文件进行gzip/Brotli压缩

> 推荐 Brotli 压缩，Brotli 压缩比 gzip 压缩更高效（提升20%性能），更快速。

### gzip压缩jessibuca.js 和decoder.js 和decoder.wasm 文件

linux(mac)
#### jessibuca.js
```
gzip jessibuca.js
mv jessibuca.js.gz jessibuca.js
```

#### decoder.js

```
gzip 和decoder.js
mv 和decoder.js.gz 和decoder.js
```

#### decoder.wasm

```
gzip 和decoder.wasm
mv 和decoder.wasm.gz 和decoder.wasm
```

windows 系统压缩方法

下载 gzip.exe

[http://gnuwin32.sourceforge.net/downlinks/gzip-bin-zip.php](http://gnuwin32.sourceforge.net/downlinks/gzip-bin-zip.php)

解压后，将 `jessibuca.js` 和 `decoder.js`和 `decoder.wasm` 文件拖到 gzip.exe上，文件就压缩好了，也需要去掉.gz后缀

<img src="/public/img/gzip.png">

### Brotli压缩jessibuca.js 和decoder.js 和decoder.wasm 文件

可以看下解决方案
[https://www.cnblogs.com/densen2014/p/16120778.html](https://www.cnblogs.com/densen2014/p/16120778.html)

<img src="/public/img/br.png">

### 关于WASM压缩优化

压缩是有效提高下载速度的方式，浏览器目前支持的主流压缩格式包括 `gzip` 和 `brotli` 两种。针对wasm包的压缩，`brotli` 算法有显著的优势。

- gzip： 一种流行的压缩文件格式，能有效的降低文件大小。
- brotli： Google 在 2015 年推出的一种压缩方式，相对于 Gzip 约有 20% 的压缩比提升。

### Media Source Extensions 硬解码H265

- Windows系统下,360浏览器可播放使用MSE加速解码H265.
- Windows系统下,win10商店购买hevc解码器后最新edge可硬件加速解码播放H265.

> jessibuca pro 版本已经支持了。欢迎测试使用。http://jessibuca.monibuca.com/player-pro.html

### webcodecs 硬解码H265

#### Chrome/Edge 86及之后

提供的WebCodecs API来进行硬解码,为实验特性,需要手动开启 enable chrome://flags/#enable-experimental-web-platform-features, or pass --enable-blink-features=WebCodecs flag via the command line.

#### Chrome/Edge 94之后
Desktop,Android,Webview中已默认开启!

需要https加载web,播放https/wss-flv流. 如果控制台打印 "WCS is not supported or experimental-web-platform-features not enabled" 请将当前页面使用https访问

> jessibuca pro 版本已经支持了。欢迎测试使用。http://jessibuca.monibuca.com/player-pro.html


### http vs https

#### http

在http 协议里面，是不能播放https 或者 wss 协议的，会报跨域报错。


#### https

在https 协议里面，是不能播放http 或者 ws 协议的，会报跨域报错。

### OffscreenCanvas这个特性需要特殊的环境和硬件支持吗

默认是关闭的. 如果开启需要设置 forceNoOffscreen 为 false 就可以了。

各个浏览器对于OffscreenCanvas支持程度。

https://caniuse.com/?search=OffscreenCanvas

> 该特性是实验性特性，某些版本的浏览器会出现内存无缘无故变大的情况。谨慎使用。
> https://github.com/langhuihui/jessibuca/issues/227

### 多个iframe一起播放视频，如果有一个视频地址播放不了会导致其他地址也无法播放

#### 问题
多个iframe一起播放视频，如果有一个视频地址播放不了会导致其他地址也无法播放(请求被堵塞住了)。

#### 解决方案

1. m7s ui 里面 我也是setTimeout 0去渲染的。或者建议你用Promise.resolve的形式去播放。
2. 如果是http或者ws接口，可以尝试换成https或者wss接口。


### vue3 下面使用Typescript

https://github.com/langhuihui/jessibuca/issues/137

https://github.com/bosscheng/jessibuca-vue-ts-demo


### destroy释放内存

https://github.com/langhuihui/jessibuca/issues/135

> 经测试，放到node+express服务中，16画面轮询跑了14个小时没有崩溃，chrome浏览器内存达到2G左右，destroy优化的效果还是很明显的，感谢大佬！


### 关于pts值

可以的，http://jessibuca.monibuca.com/api.html#stats 监听 stats ,一秒回调一次，

```shell
buf: 当前缓冲区时长，单位毫秒,
fps: 当前视频帧率,
abps: 当前音频码率，单位bit,
vbps: 当前视频码率，单位bit，
ts: 当前视频帧pts，单位毫秒

```

#### 对于jessibuca pro 版本 支持

```shell
buf: 当前缓冲区时长，单位毫秒,
fps: 当前视频帧率,
abps: 当前音频码率，单位bit,
vbps: 当前视频码率，单位bit，
ts: 当前视频帧pts，单位毫秒
... 其他参数,
pTs: 当前播放器的播放时间，从0开始，单位毫秒
```


### 将录制的视频保存在安卓手机相册中，显示的时长为0，并且无法播放。

https://github.com/langhuihui/jessibuca/issues/126

现象：将录制的视频保存在安卓手机相册中，显示的时长为0，并且无法播放。但是在对应的文件路径中找到源文件是能播放的，但是依然不显示时长。


这是录制的是webm 格式的视频，对于移动端的兼容性不是很好。等后续支持录制MP4格式(MPEG-4)的视频录制就可以解决这个问题了。

另外：
> MP4格式支持在IOS VLC播放器显示时长播放，Android VLC播放器无法显示时长播放，PC VLC播放器可以播放


> Jessibuca Pro 可以录制MP4格式(MPEG-4)的视频，就可以解决这个问题了。

### 无音频的flv视频流，无法录制，录制的文件大小都是0

原问题：https://github.com/langhuihui/jessibuca/issues/128

- 1、无音频视频录制不成功，文件大小为0
- 2、静音视频录制不成功，文件大小为0

解决方案：

### 如果没有音频数据

设置 hasAudio 为false 就可以解决了。

> 目前如果声音在静音或者没有音频数据的时候，一定要设置hasAudio，不然MediaRecorder会录制失败。

### 如果只需要播放音频数据

> jessibuca pro 已经有了单独的音频播放器，支持播放音频数据

音频直播流（支持移动端（平板端）息屏和后台播放）

https

https://jessibuca.com/pro/audio-player-demo.html

http

http://jessibuca.monibuca.com/pro/audio-player-demo.html


### 创建单个视频播放卡顿

是指播放器渲染的帧率太低，比如：1s 显示 3～5 帧，或者渲染完一帧后，过很久才渲染下一帧。

- 网络带宽不足

- 播放设备性能不足

- 视频流时间戳问题

#### 网络带宽不足

摄像头 -> 流媒体服务器 -> 播放器

- 摄像头的网络不好，导致推流上行不稳定

- 流媒体服务器的线路质量不好，导致分发不稳定

- 播放器的网络不好，导致拉流下行不稳定

#### 播放设备性能不足

- 增大缓冲区，有助于缓解解码不稳定带来的卡顿
- 尽量硬解码（MSE）(WCS)

#### 视频流时间戳问题

播放器一般是严格根据码流中的音视频的时间戳来做音画同步的，

因此，如果码流中的音视频时间戳出现错误，肯定会影响到播放画面的渲染时机。

> 可以先通过设置 hasAudio: false 来排除音频的问题
> 确保视频流的时间戳也得增加。


### 创建多个以上播放实例会非常卡顿，还会导致页面黑屏

例如 h265,1280*720，wasm 肯定会卡顿的。 建议降低分辨率。还需要增大videoBuffer 大小。

#### 可能存在的问题
1. 分辨率过高
2. 带宽是否跟得上
3. 是否是H265编码

#### 自查
监听下`stats` 事件，查看 `fps` 是否达到了预期的值。

#### h265 优化方案

1. 降低分辨率
2. 增大videoBuffer大小，一般1s，2s，3s都是可以的
3. 设置hasAudio 为false，不demux和decode音频数据。
4. ~~条件允许(支持OffscreenCanvas)也可以配合设置 forceNoOffscreen 为 false  开启离屏渲染模式，提升性能。~~
5. pro版本支持360 或者edge浏览器 H265硬解码。 http://jessibuca.monibuca.com/player-pro.html
6. pro版本支持SIMD解码，尤其是1080p及以上的分辨率，会有很强的效果。http://jessibuca.monibuca.com/player-pro.html
7. 如果是服务器端出口带宽跟不上的情况，增大服务器端出口带宽。


> 某些显卡在支持OffscreenCanvas上面会存在问题，所以谨慎使用。
> https://github.com/langhuihui/jessibuca/issues/227

#### h264 优化方案

1. 降低分辨率
2. 增大videoBuffer大小，一般1s，2s，3s都是可以的
3. 设置hasAudio 为false，不demux和decode音频数据。
4. ~~条件允许(支持OffscreenCanvas)也可以配合设置 forceNoOffscreen 为 false  开启离屏渲染模式，提升性能。~~
5. 如果是https情况下 设置 useWCS 为 true。
6. 如果是http情况下 设置 useMSE 为 true。


> 某些显卡在支持OffscreenCanvas上面会存在问题，所以谨慎使用。
> https://github.com/langhuihui/jessibuca/issues/227



### 播放过程中出现了延迟

#### 对于开源版

- wasm解码`做了`丢帧（消除延迟）`逻辑，保证`前台`长时间在设置的延迟范围内
- mse解码`没有`做丢帧（消除延迟）逻辑
- wcs解码`没有`做丢帧（消除延迟）逻辑

> 浏览器切换到后台（最小化，tab窗口被关闭），导致的窗口不可见的情况，会导致延迟增大。

#### 对于pro版

- wasm解码`做了`丢帧（消除延迟）逻辑，保证`前台和窗口不可见的情况下`长时间在设置的延迟范围内
- mse解码`做了`丢帧（消除延迟）逻辑，保证`前台和窗口不可见的情况下`长时间在设置的延迟范围内
- wcs解码`做了`丢帧（消除延迟）逻辑，保证`前台和窗口不可见的情况下`长时间在设置的延迟范围内

> pro播放器在窗口不可见的情况下是利用黑科技实现的消除延迟的逻辑。

### 播放过程中页面出现崩溃

可能得原因：

1. 分辨力过大
2. 播放的屏幕数量过多
3. 电脑内存过小
4. 有没有打开devTools，配置debug:true
5. 是否H265


开源版只能支持到`720p`的视频，如果超过这个分辨率，就会存在`解码堆积`，时间长了就会容易出现内存堆积，进而导致浏览器崩溃情况。

因为配置了`debug:true`之后 devtools 会打印日志，日志本身就会占用很多内存，也会导致浏览器崩溃。

如果视频是`H265`的，因为其本身的高压缩率，解码端非常依赖硬解码，如果硬解码不支持，就会导致软解码，软解码性能不好，也会导致浏览器崩溃。

> pro 版本可以完美的支持到1080p的视频，甚至更高分辨率的视频，支持硬解码+软解码。

> pro 版本可以完美的支持到H265的视频，支持硬解码+软解码。

#### 解决方案

1. 降低分辨率
2. 降低播放的屏幕数量
3. 增加电脑内存
4. 升级到pro版本
5. 关闭devTools(如果是长时间测试，建议把 `debug:false` 配置起来，因为日志也会存留在内存里面的)
6. H265降级到H264


### 关于浏览器崩溃(sbox_fatal_memory_exceeded)

- 看下本身电脑的内存是否足够，分配给浏览器的内存是否足够。

可以看下 http://jessibuca.monibuca.com/pro.html#%E7%94%B5%E8%84%91%E5%9E%8B%E5%8F%B7%E4%BB%A5%E5%8F%8A%E7%A0%81%E7%8E%87

每路所需的内存情况。

### 可以播放本地视频吗？

回答：不可以，

jessibuca 定位是直播流播放器。

Jessibuca是一款开源的纯H5直播流播放器。

> 所以 暂不支持 本地文件打开。

### 是否可支持倍速播放

pro 已经支持了 [回放(录像)流Demo(支持mse、wcs、simd解码)](https://jessibuca.com/pro-demo.html#tf%E5%8D%A1%E6%B5%81%EF%BC%88%E5%BD%95%E5%83%8F%E6%B5%81%EF%BC%89-24h%E6%97%B6%E9%97%B4%E8%BD%B4)

###  关于延迟造成的原因

可能的原因
- 网络加载的延迟
- 软解码的延迟
- 渲染的延迟

一般来说，如果在用户`网络环境`较好的情况下，渲染由于使用了WebGL，很难造成瓶颈（操作很单一），其中一般会因为软解码性能不足造成不停卡顿及延迟。

优化因为软解码性能不足造成的延迟，我们一般从几个地方着手：

- 视频的profile：相比于main/high而言，baseline不包含B帧，解码消耗更低
- 视频帧率：过高的帧率会造成软解码跟不上，可以试着降低帧率，例如24fps
- 视频码率：码率越高，视频富含的细节越多，也越清晰，但是会消耗更多的解码性能，可以试着降低码率
- 视频分辨率：过高的视频会造成单帧传递的数量极大

#### 解决方案

- 可以使用jessibuca pro 的 simd 解码，尤其正对于HEVC的1080p的解码能力提升很多。
- jessibuca pro 还支持 mse 解码 HEVC(H265)



### 视频颜色变灰色（软解码）

原因

- 视频流的格式 不是 yuv420p

可能的视频格式是：yuvj422p 格式。

可能是webgl 渲染的问题导致的。

> jessibuca pro 支持video 标签渲染数据，不会出现视频颜色变灰色的情况


### 视频渲染发绿（软解码）

#### 原因

- 对于宽度不是8的倍数的时候就会出现这样的问题

原问题： https://github.com/langhuihui/jessibuca/issues/152

例如：540x960 分辨率

在使用WebGL对YUV420P进行渲染时，WebGL图像预处理默认每次取4字节的数据，但是540x960分辨率下的U、V分量宽度是540/2=270不能被4整除，导致绿屏。

#### 解决方案

1. ~~可以通过设置`gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);` 的方式来解决，但是会损耗一部分性能。~~
2. ~~将`openWebglAlignment` 设为 `true`。~~
3. 最新解决方案，程序会自动检查分辨率，如果不是标准的分辨率，会自动更新webgl 渲染规则
4. jessibuca pro 支持video 标签渲染数据，不会出现视频渲染发绿的情况

### 关于如何集成到qiankun这类的微前端中去

需要将jessibuca的`dist`目录下面的文件[`decocer.js`,`decoder.wasm`,`jessbuca.js`]放到`主应用`的`public`目录或者根目录下面。

然后在`子应用`使用的时候，要在index.html 下面通过`script`标签引入主应用路径下面的`jessbuca.js`，在业务代码里面，通过配置`decoder`参数，也是主应用下面的decoder.js地址。

> 注意：`decocer.js`,`decoder.wasm`两个文件必须放同一个目录下面。


### H5 全屏的时候，视频区域并没有全屏的问题

因为H5 全屏采用的是web端的全屏方案

```css
{
    position: fixed;
    z-index: 9999;
    left: 0;
    top: 0;
    right: 0;
    bottom: 0;
    width: 100vw !important;
    height: 100vh !important;
    background: #000;
}
```
所以要保证，jessibuca 的`container`绑定的dom对象，以及上层对象，是否某些属性导致了`position: fixed`失效。

具体研究可看：https://github.com/chokcoco/iCSS/issues/24


### decoder.js 报 Unexpected token '<'错误

1.查看控制台的`network` 面板下面的 `decoder.wasm`文件有没有被正确返回。返回个格式是不是 `application/wasm`格式的。

2.查看控制台的`network` 面板下面的 decoder.js 文件有没有被正确返回。返回个格式是不是 `application/javascript`格式的。（因为配置的路径不对，会存在vue 或者react 项目 直接被返回了index.html 内容了）

4.查看`decoder`参数是否配置的正确，见[decoder参数配置](http://jessibuca.monibuca.com/api.html#decoder)，如果配置错误，会被web服务器以找不到文件，然后返回index.html的内容。

> 需要正确的配置`decoder`参数，播放器默认引用的是根目录下面的`decoder.js`

#### react 解决方案

见 https://github.com/bosscheng/jessibuca-react-demo/tree/v3


#### vue 解决方案

https://github.com/bosscheng/jessibuca-vue-demo/tree/v3

typescript:https://github.com/bosscheng/jessibuca-vue-ts-demo

#### 网友的解决方案

https://blog.csdn.net/nbwgl/article/details/122652003


### 有数据,但是没有画面出来


可能的原因
#### 时间戳导致的

通过监听`stats` 事件，查看下面的`ts` 看是否都是相同的时间戳。

#### 开启调试 debug:true

通过设置` debug:true `，然后重新播放视频源，通过日志查看是否有报错信息。


### g711系列的音频，听起来为啥都是杂音。

确认下是否是服务器端推送音频数据的时候，把g711a 的推 成了g711u的格式，或者反过来了。导致播放器在解码格式的时候，听起来全是杂音。


### Failed to constructor 'Worker': Script at 'file://xxxxxxx'

别用`file`协议启动项目，`file`协议暂不支持`worker`。

使用`http`协议启动，可以配合`nginx`或者`node` 启动。

> pro 支持通过配置参数，只使用mse解码，不启动worker。 见

#### node 启动(解决方案)
通过 jessibuca-vue-demo 中的 preview 进行查看。

https://github.com/bosscheng/jessibuca-vue-demo/blob/v3/preview/preview.js

#### nginx 配置(解决方案)

待补充


### Failed to construct 'Worker': Script at 'https://a.com' cannot be accessed from origin 'https://b.com'

这个错误是由于`同源策略（Same-Origin Policy）`引起的。具体来说，浏览器禁止从一个源（在这个例子中是 https://a.com）访问或加载另一个源（在这个例子中是`https://b.com`）的资源。

同源策略：

> 同源策略是一种浏览器的安全机制，用于防止一个源中的文档或脚本对另一个源中的资源进行不安全的访问。它确保了一个网页只能请求与其同源的资源，避免了潜在的跨站脚本攻击（XSS）。

源的定义：

> 一个源是由协议（protocol）如：`http 或者 https` 、主机（host）如：`localhost 或者 test.com` 和 端口（port）如：`3000和4000` 组成的。如果两个 URL 的协议、主机和端口都相同，那么它们就是同源的。

解决方案：

> CORS（跨域资源共享）是一种机制，它使用额外的 HTTP 头来告诉浏览器，允许一个源（域）的 Web 应用访问另一个源（域）的资源。

例如 node 服务端代码：

```js

const express = require('express');
const app = express();

app.use((req, res, next) => {
  res.header('Access-Control-Allow-Origin', '*'); // 允许所有的源，生产环境建议设置指定的源
  res.header('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE');
  res.header('Access-Control-Allow-Headers', 'Content-Type');
  next();
});


app.listen(80, () => {
  console.log('Server is running on port 80');
});

```

### 对于出现渲染页面直接倒过来180度的解决方案

#### 问题

通过webgl渲染(canvas)的时候，会出现部分机型画面倒挂，一般这种情况都是出现在 `wasm` 渲染模式上面的。

#### 原因
这是由于在部分A卡上面，webgl渲染会存在兼容性bug，导致了画面180度倒挂。


#### 解决方案

1. 如果是h264的源，建议使用MSE 硬解码 通过设置`useMSE:true`,使得渲染元素是video标签。
2. 如果是h265的源，推荐使用 `jessibuca pro` 目前pro 版本支持 `mse` `wasm`  `webcodecs`解码之后通过video标签渲染。
3. 提供一个操作按钮，让用户可以手动的旋转画面，播放器提供了`setRotate`方法，可以通过`setRotate`方法旋转画面。


### chrome无法访问更私有的地址

触发了 Chrome 安全策略 - 私有网络控制（CORS-RFC1918）

升级chrome 91后，默认无法从开放的地址往更私有的地址访问。

比如从公网访问web，播放内网的流媒体地址。

|外网访问内网| http | https |
| --- | --- | --- |
|http| Chorme 94禁止| Chorme 94禁止|
|https| 安全内容加载不安全内容，禁止| 取跨域策略|



```shell

Access to fetch at 'http://192.168.0.2:8000/live/test.flv' from origin 'http://jessibuca.monibuca.com/' has been blocked by CORS policy: The request client is not a secure context and the resource is in more-private address space `private`.
```
打开浏览器的

> chrome://flags/#block-insecure-private-network-requests

将这项设置为关闭

> 将Block insecure private network requests配置禁用掉（Disable）。但是一定要注意，修改了配置后必须点击Chrome此时在右下角出现的“重启”（Restart）按钮才能生效。自己主动关闭浏览器全部页面再打开是不会触发Chrome更新配置的。

###  报错：jessibuca need container option

这是由于初始化播放器的时候，`container` 的 `dom` 还没有生成,这个和生命周期有关系。

#### vue 项目中使用
不能在create 生命周期里面初始化jessibuca，因为这个时候，DOM 还没有生成，所以会报错。

```js
jessibuca
```js
{
    mounted(){
        // init jessibuca
    }
}

```

#### react 项目中使用

```js
jessibuca
```js
{
    componentDidMount(){
        // init jessibuca
    }
}

```

### 是否支持原生、小程序（UniApp,小程序）等

#### 对于UniApp

|Vue2|Vue3|
| --- | --- |
|√|√|


|App	|快应用	|微信小程序	|支付宝小程序	|百度小程序	|字节小程序	|QQ小程序|
| --- | --- | --- | --- | --- | --- | --- |
|×|×|×|×|×|×|×|

|钉钉小程序|	快手小程序	|飞书小程序|	京东小程序|
| --- | --- | --- | --- |
|×|×|×|×|

|H5-Safari	|Android Browser	|微信浏览器(Android)	|QQ浏览器(Android)	|Chrome	|IE|	Edge	|Firefox	|PC-Safari|
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
|√|√|√|√|√|√|√|√|√|


#### 对于小程序

> 例如 微信小程序、快手小程序、钉钉小程序、飞书小程序、京东小程序

只支持内嵌 webview 模式播放。


### 关于超低延迟(300ms)以内

目前想要超低延迟，只能使用wasm解码。目前开源版的超低延迟最多只能支持到`1s`以内

推荐的配置
#### 对于开源版

```
{
    videoBuffer:0.1
}
```

#### 对于PRO

pro 由于使用了解码性能更强的simd解码，所以推荐使用simd 解码来提升解码性能，所以可以做到更低的延迟(300ms以内)。

```
{
    videoBuffer:0.1,
    videoBufferDelay:0.2
    useSIMD:true
}
```


### 对于hevc(265)支持情况

<img src="/public/hevc-support.png">

[caniuse详情](https://caniuse.com/?search=hevc)

#### chrome

1. Supported for all devices on macOS (>= Big Sur 11.0) and Android (>= 5.0),
2. for devices with hardware support on Windows (>= Windows 8),
3. and for devices with hardware support powered by VAAPI on Linux and ChromeOS

#### edge

1. Supported for all devices on macOS (>= Big Sur 11.0) and Android (>= 5.0) if Edge >= 107,
2. for devices with hardware support on Windows (>= Windows 10 1709) when HEVC video extensions from the Microsoft Store is installed


#### 实际测试情况

1. 遇到过两台电脑操作系统，浏览器版本都是一样，cpu 不一样，GPU不一样,4050显卡的所有浏览器都能走硬解码,2060显卡就有谷歌浏览器能走硬解码，预测是 HEVC video extensions 这个插件的兼容性问题。


### 关于PRO提示MSE不支持265解码可能得原因

#### 检查下chrome（edge）的版本号

确保是较新版本。

#### 检查操作系统

##### window

1. 确保GPU 支持 Hevc硬解
2. window10 1709 以前的版本不支持，建议升级到最新版本。
3. 需要安装 `HEVC video extensions` HEVC扩展
4. 或者安装360浏览器（最新版本）

#### mac
1. macOS (>= Big Sur 11.0)
2. Edge >= 107

#### 扩展

[window chrome 如何开启HEVC硬件解码](https://jessibuca.com/document.html#chrome-%E5%A6%82%E4%BD%95%E5%BC%80%E5%90%AFhevc%E7%A1%AC%E4%BB%B6%E8%A7%A3%E7%A0%81)

### chrome 如何开启HEVC硬件解码

解决方案：https://www.nxrte.com/jishu/11365.html

主要就是检测步骤就是：

1. 判断客户机是否支持HEVC硬解码
2. chrome浏览器配置

#### 判断客户机是否支持HEVC（H265）硬解码

首先检查自己的电脑是否支持HEVC硬解码，可以下载dxva checker检测软件，DXVAChecker是一个windows系统PC检测DirectX视频加速的工具，其可检测解码是否支持GPU

#### chrome浏览器配置

1. 首先安装最新版本的google chrome浏览器，打开帮助->关于，查看版本号是否大于104。
2. 地址栏输入：chrome://settings，打开配置页面，搜索”硬件加速”，使用硬件加速开启：
3. 地址栏输入：chrome://flags，搜索hardware，使能Hardware-accelerated video decode硬件解码：
4. 如果chrome浏览器没有快捷方式，建立一个快捷方式，增加启动运行参数：–enable-features=PlatformHEVCDecoderSupport 这样使用此快捷方式打开即可直接加上此运行参数，也可cmd下运行exe加上此运行参数运行，比较麻烦，这里直接添加到快捷方式上，加入方式如下(右键->属性->目标(T) 末尾加个空格，然后赋值上面的参数)：
5. 通过快捷键打开chrome，地址栏输入chrome://gpu,搜索”Video Acceleration”,验证chrome是否开启成功:

### 关于window Hevc是否支持

#### edge/chrome 自查

1.浏览器输入：`chrome://gpu/` 如果edge浏览器就`edge://gpu/`
2.全局搜索下`hevc`关键词

#### 查看设备是否支持

要知道自己的电脑支持什么格式的硬解码，可以下载DXVA Checker
下载地址：https://bluesky-soft.com/en/DXVAChecker.html

也可以直接查询BlueSky的数据库（可直接点击超链接）
- AMD ：https://bluesky-soft.com/en/dxvac/deviceInfo/decoder/amd.html
- Intel ：https://bluesky-soft.com/en/dxvac/deviceInfo/decoder/intel.html
- NVIDIA ：https://bluesky-soft.com/en/dxvac/deviceInfo/decoder/nvidia.html

浏览器通常采用核显加速，同时有独显核显的，参考核显的解码能力。

HEVC硬解支持的硬件较多，Intel第六代酷睿处理器及以后的核显全部支持HEVC，六代之前的部分支持，具体请看BlueSky的数据库。

AV1硬解目前仅限于AMD RX 6000系（除6500XT）、Nvidia 30系、Intel Arc显卡、Intel UHD 700系和Iris 锐炬Xe核显，后续型号应该也会支持AV1。

#### 开启Hevc硬解码

1. 开启HEVC之前需要下载HEVC插件，这个插件可以在微软商店花7块钱购买。

搜索：HEVC视频扩展

2. 也可以直接在网上免费下载，两者都是一样的。
   HEVC视频拓展下载地址：https://www.free-codecs.com/hevc_video_extension_download.htm
   由于以前的bug已经修复，所以可以直接下载最新版的插件，选择x64版本的HEVC Video Extension 1.0.50361下载并安装。
   或者直接下载已经下载好的：[Microsoft.HEVCVideoExtension_2.1.1803.0_neutra.zip](https://jessibuca.com/zip/Microsoft.HEVCVideoExtension_2.1.1803.0_neutra.zip)

感谢：https://www.bilibili.com/read/cv35896480/ 的解决方案。

3. 在地址栏输入edge://flags/ 进入搜索 Choose ANGLE graphics backend 选择 D3D11，选择后重启浏览器再打开。


### 关于遇到报错的时候，如何反馈给作者去定位问题。

> 如果有公网的流，把公网的流发给作者，去复现下这个问题。只是为了复现问题，不会用于其他用途。

#### 开源版

1.初始化播放器的时候，设置`debug:true`
```js
const jessibuca = new Jessibuca({
    // 其他参数
    debug:true
})
```

2.从头开始播放，然后直到出错的时候，把F12控制台(console tab)的所有内容右键（save as）保存下来，以及播放器的配置信息，发给作者。

3.如果条件允许，把出问题的流，保存个flv文件，发给作者。

#### pro版本

1.初始化播放器的时候，设置`debug:true`，`debugLevel:'debug'`

```js
const jessibuca = new JessibucaPro({
    // 其他参数
    debug:true,
    debugLevel:'debug'
})
```

2.从头开始播放，然后直到出错的时候，把F12控制台(console tab)的所有内容右键（save as）保存下来，以及播放器的配置信息，发给作者。

3.如果条件允许，把出问题的流，保存个flv文件，发给作者。

#### 日志下载

<img src="/public/img/save-log.png">

### 关于绿屏和花屏

现象： 播放画面出现图像紊乱，大面积的异常颜色的方块图，或者绿屏现象

可能得原因：
#### 流媒体服务器-> 播放器端
- 网络不好，编码后的数据发不出去，导致丢失参考帧。
- 推过来的流，不是从i帧开始的，会导致首帧解码出现绿屏或者花屏的情况。
- 推过来的流，码流中视频尺寸发生变化。

#### 推流端->流媒体服务器
- 如果是`rtsp协议`推流，因为默认采用的udp，不能保证数据的完整性，可以尝试使用`rtmp协议`推流（使用的是tcp）推流。

#### 播放器端
- 系统低内存，队列里面无法承受更多的帧数据。
- 硬编硬解的兼容性问题

#### 自查

1. 同样的播放地址，用客户端播放器（例如客户端的vlc）播放是否正常，检查是否流本身的问题。
2. 同样的播放地址，用其他浏览器播放是否正常，检查是否浏览器的问题，检查是否浏览器的问题。
3. 同样的播放地址，用其他的web播放器（[video.js](https://videojs.com/),[xgplayer.js](https://h5player.bytedance.com/)）,播放是否有问题，检查是否Jessibuca的问题。

> 千万不要web端跑的是http协议的 ，然后用rtsp协议这样的协议去vlc播放测试，这也是毫无意义的，因为不同的封装协议，不同的传输协议，不同的编码协议，都会导致不同的问题。

> 一定要用同样的协议，同样的封装，同样的编码，同样的传输协议，去测试，这样才能正常是否是流本身的问题导致的绿屏。



### 首屏打开慢

- 网络不好，导致拉流慢。
- 首帧不是I帧，播放器为了等I帧。
- 流媒体服务器的线路质量不好，导致分发不稳定。


### 就是在webview中使用写好的网页，ios工程会找不到Jessibuca这个对象

> 用的cdn方式就可以了。




### aborted(rangeError:webassembly.instance():out of memory: cannot allocate wasm memory for new instance) 错误

> 这个错误是由于WebAssembly实例内存不足而导致的。WebAssembly是一种虚拟机技术，它可以在浏览器中运行高性能的本地代码，但是WebAssembly实例的内存大小是有限制的。

> 通常情况下，WebAssembly实例的内存分配是由JavaScript主线程控制的。当WebAssembly实例需要更多的内存时，JavaScript代码会向操作系统申请更多的内存。然而，在某些情况下，如高密度的计算、大型数据集等场景下，WebAssembly实例会超出内存限制，导致无法继续分配内存，最终抛出"out of memory"错误。

> 要解决这个问题，可以考虑优化WebAssembly代码，减少内存使用量；或者增加JavaScript主线程控制的WebAssembly实例内存限制，具体方法可以根据应用程序需求和实际情况进行调整。

- 检查下系统级别的`内存`和`cpu`和`gup`使用情况
- 检查下是不是播放的码流过大，导致内存不足（开源版单实例能够支持分辨率最大720p，pro版本单实例可以支持到2k级别）


### 关于切换 url 播放地址

为了降低内存溢出的风险，如果要切换播放地址，需要先销毁当前的实例，再创建新的实例。

```js
let jessibuca = new Jessibuca({
// 参数
})

jessibuca.play('url1')

// 切换播放源

jessibuca.destroy();
jessibuca = null;

// 从新new一个出来
let jessibuca = new Jessibuca({
// 参数
})


// 播放新的地址
jessibuca.play('url2')

```


### WebGL: CONTEXT_LOST_WEBGL 错误引发的原因

WebGL的CONTEXT_LOST_WEBGL错误通常表示WebGL上下文（context）已经丢失。当浏览器认为WebGL上下文已不再可用时，将会发生这种错误。以下是一些可能导致WebGL上下文丢失的原因：

- 设备内存不足：在设备内存不足的情况下，浏览器会释放一些资源以获得更多内存，其中可能包括WebGL上下文。

- 用户操作中断：例如窗口被最小化、系统进入睡眠模式等用户操作都有可能导致WebGL上下文丢失。

- 设备性能问题：某些设备可能无法支持WebGL或者性能较低，导致WebGL上下文出现问题。

- 驱动程序问题：某些情况下，驱动程序可能会导致WebGL上下文丢失。

- 系统错误：例如设备故障、硬件损坏等系统错误也可能导致WebGL上下文丢失。

#### 窗口被最小化、系统进入睡眠模式等用户操作都有可能导致WebGL上下文丢失。
当用户窗口被最小化或系统进入睡眠模式时，浏览器会暂停WebGL上下文的渲染和更新。这可能会导致WebGL上下文超时（timeout）并且丢失。如果WebGL上下文被挂起或暂停时间过长，可以认为WebGL上下文已过期或无效，从而导致CONTEXT_LOST_WEBGL错误。

WebGL是一种在Web浏览器中渲染3D图形的技术，需要高帧率和持续更新来保持流畅的体验。如果WebGL上下文被暂停或挂起，它就无法满足要求的性能需求，因此浏览器会释放WebGL上下文，以回收资源和内存。



### video 标签报 PIPELINE_ERROR_DECODE 错误

<img src="/public/img/pipline-error-decode.png">

PIPELINE_ERROR_DECODE 是指视频解码器在解码视频流时发生了错误，这可能是由多种原因导致的，包括但不限于：

- 视频数据缺失关键元素。(可能网络原因导致的帧顺序错乱)
- 浏览器正在同时处理多个视频流，而硬件资源不足以支持同时解码多个视频流。

解决方法可能因具体原因而异，以下是一些可能的解决方法：

- 检查视频数据本身，确保它的格式正确，且不损坏（如果是网络问题，则没法解决，只能通过尝试重播）。
- 如果系统资源不足以支持同时解码多个视频流，则可以尝试降低同时播放的视频数量，或者升级硬件设备。


### 浏览器报：Uncaught RangeError: Array buffer allocation failed

> Uncaught RangeError: Array buffer allocation failed 错误通常是因为尝试分配一个大于 JavaScript 引擎所允许的内存限制的 ArrayBuffer。这个限制因浏览器而异，但通常是几百 MB 到几 GB 不等。


一般这个报错出现在软解码的时候且是多屏的情况下。

本身软解码的性能就不是很好，如果是多屏的情况下，那么内存的消耗就会更大。

解决方法：
- 使用硬解码（pro 支持H264/H265硬解码）
- 降低分辨率
- 降低帧率
- 降低码率
- 降低多屏数量

### 浏览器报：PIPELINE_ERROR_DECODE: Failed to parse H.264 stream

> 表示解码器无法正确解析传入的 H.264 视频流。

#### 这个问题的原因可能是：

- 视频流本身存在问题，可能已经损坏或编码不正确。
- 网络问题导致视频流传输中出现了错误，例如丢包、延迟等。
####  解决此问题的方法包括：

- 检查视频流本身是否存在问题，可以使用其他工具对视频流进行分析或修复。
- 检查网络连接，确保网络连接稳定，且视频流传输过程中没有出现问题。可以尝试使用其他网络或更改网络配置来解决问题。

### 浏览器报：PIPELINE_ERROR_DECODE: Failed to send video packet for decoding: {timestamp=568800000 duration=40000 size=240812 is_key_frame=1 encrypted=0}

> 解码器无法接收并解析视频数据包。

#### 这个问题的原因可能是：

- 视频流本身存在问题，可能已经损坏或编码不正确。
- 网络问题导致视频流传输中出现了错误，例如丢包、延迟等。
####  解决此问题的方法包括：

- 检查视频流本身是否存在问题，可以使用其他工具对视频流进行分析或修复。
- 检查网络连接，确保网络连接稳定，且视频流传输过程中没有出现问题。可以尝试使用其他网络或更改网络配置来解决问题。

### 关于 play() failed because the user didn't interact with the document first. 错误

背景：用户希望打开页面的时候就直接自动播放`带音频`视频（单屏或者多屏），但是浏览器的自动播放策略是，必须是用户手动触发了事件之后，才能自动播放。

会抛出`DOMException: play() failed because the user didn't interact with the document first. https://goo.gl/xX8pDD` 错误。

> 手动刷新页面也会出现这个异常。

> 这个报错是浏览器的规范，浏览器规定，必须要用户主动触发才能播放带音频的视频。

优先使用`canvas`进行渲染或者静音状态。这样就可以规避掉浏览器的规范了。

> mse、wcs是硬解码，wasm是软解码


#### 解决方案

1. 静音状态下播放,添加一个交互事件，让用户手动触发下，再去播放视频。
2. 浏览器允许点击连接跳转打开页面允许自动播放并支持声音。

可以看下demo实现

https
https://jessibuca.com/pro/demo-auto-play.html

http
http://jessibuca.monibuca.com/pro/demo-auto-play.html

### 关于浏览器报：The AudioContext was not allowed to start. It must be resumed (or created) after a user gesture on the page. 错误

背景：用户希望打开页面的时候就直接自动播放`带音频`视频（单屏或者多屏）,软解码音频的时候。但是浏览器的自动播放策略是，必须是用户手动触发了事件之后，才能自动播放。

浏览器会抛出：`The AudioContext was not allowed to start. It must be resumed (or created) after a user gesture on the page. https://goo.gl/7K7WLu` 错误

#### 解决方案

1. 静音状态下播放,添加一个交互事件，让用户手动触发下，再去播放视频。
2. 浏览器允许点击连接跳转打开页面允许自动播放并支持声音。

可以看下demo实现

https
https://jessibuca.com/pro/demo-auto-play.html

http
http://jessibuca.monibuca.com/pro/demo-auto-play.html


### 浏览器报：SBOX FATAL MEMORY EXCEEDED 错误

#### 原因：

触及 Chrome 沙箱内存上限，主动崩溃。

#### 上限：

chrome 源码

```
int64_t physical_memory = base::SysInfo::AmountOfPhysicalMemory();if (sandbox_type == SandboxType::kGpu && physical_memory > 64 * GB) {
memory_limit = 64 * GB;
} else if (sandbox_type == SandboxType::kGpu && physical_memory > 32 * GB) {
memory_limit = 32 * GB;
} else if (physical_memory > 16 * GB) {
memory_limit = 16 * GB;
} else if (physical_memory > 8 * GB) {
memory_limit = 8 * GB;
}
```
一般来说，16G 内存电脑，沙箱上限为 8G。

注意： 多个标签页，同一个域名，一般情况下会使用同一个进程，也就是 8G 内存多个标签页共用。

#### 解决方案：

第一种: 增加内存到 24G 或者 32G，能使沙箱上限增加到 16G。

第二种：增加命令行：--no-sandbox 禁用沙箱，不足之处在于浏览器会给出提示，说关闭了沙箱不稳定，不安全。

第三种：很有可能存在内存泄漏，一般一段时间后，出现崩溃，应该是某些资源一直在创建，建议从代码逻辑中查找下原因。

#### 更深入的讨论：

https://www.likecs.com/ask-306316.html；

https://help.thingjs.com/hc/kb/article/1555089/


### 监听请求流的失效(404)或者500报错

可以监听`play`方法的`catch`

```js
jessibuca.play(url).catch((err) => {
    // err 就是错误信息
})
```

> 注意：这个是初次请求的时候，如果流失效，会触发`catch`，如果流有效，但是后面流失效了，不会触发`catch`。

> 播放过程中流发生500报错，会触发`error`事件。

> 播放过程中由于网络切换（网络动荡），导致流失效，会触发`error`事件。


### 理解loadingTimeout 和 delayTimeout

#### loadingTimeout
loadingTimeout 是指在`播放器在请求url的时候`，接口是返回200状态码了，但是数据还迟迟没有推送给web端 ，如果在`loadingTimeout`时间内，没有收到流数据，则会抛出`loadingTimeout`错误。

#### delayTimeout
delayTimeout 是指在`播放器播放过程中`，如果在`delayTimeout`时间内，没有收到流数据，则会抛出`delayTimeout`错误。

#### loadingTimeoutReplay(delayTimeoutReplay) 与 loadingTimeoutReplayTimes(delayTimeoutReplayTimes)

> 如果在`loadingTimeout`时间内，没有收到流数据，则会抛出`loadingTimeout`错误，如果设置了`loadingTimeoutReplay`，则会重新播放，会重试`loadingTimeoutReplayTimes`次。


### 关于移动端（H5）切换网络的时候，播放器会触发什么事件。


#### http请求
会触发`fetchError`事件

```js

jessibuca.on("fetchError", function (msg) {
    console.log('fetchError:', msg)
})
```

> pro 版本只需要监听一个事件 playFailedAndPaused 即可


#### websocket请求

会触发`websocketError`事件


```js
jessibuca.on("websocketError", function (msg) {
console.log('websocketError:', msg)
})
```

> pro 版本只需要监听一个事件 playFailedAndPaused 即可


#### 小结

或者可以通过监听`error`错误事件，来监听所有的错误事件。

```js
jessibuca.on("error", function (error) {
    console.log('error:', error)

    if (error === jessibuca.ERROR.fetchError || error === jessibuca.ERROR.websocketError) {
        // 这里统一的做重连。

        jessibuca.destroy();
        jessibuca = null;
        jessibuca = new Jessibuca();
        jessibuca.play(url);
    }
})

```


> pro 版本只需要监听一个事件 playFailedAndPaused 即可

```js

jessibuca.on("playFailedAndPaused", function (msg) {
    console.log('playFailedAndPaused:', msg)
    // 直接重新播放失效地址。
    jessibuca.play(url);
})

```


### 浏览器报：Uncaught RuntimeError: memory access out of bounds

“Uncaught RuntimeError: memory access out of bounds” 错误通常表示您在 WebAssembly 中访问了越界的内存地址。
这个错误通常发生在您尝试访问超出 WebAssembly 内存大小的地址时。


WebAssembly 通过线性内存来存储和处理数据。线性内存是 WebAssembly 虚拟机中的一个连续的字节数组，通过指针来进行访问。
如果您的 WebAssembly 模块试图读取或写入超出线性内存的末尾或开头的地址，就会触发 “memory access out of bounds” 错误。

#### 可能得原因

##### 系统资源限制：

如果您的系统资源（例如内存或 CPU）不足，也可能会导致 WebAssembly 运行时错误。

##### 解码视频需要的内存过大：

如果申请的内存不够大的情况下，播放的视频流过大，就会出现这个错误。

##### 流数据越界了

流数据本身就是越界的，导致解码的时候，越界了。给得流数据本身就是有问题，不是完整的流数据。

#### 解决方案

如果是`解码视频需要的内存过大`问题导致的，可以尝试初始化wasm的时候，分配更大的内存。

如果是`系统资源限制`导致的，可以尝试增加系统资源，或者降低视频的分辨率。

如果是`流数据越界了`，可以尝试检查下流数据本身是否存在问题。

真实客户案例：[案例一](https://jessibuca.com/document-demo-1.html)


### 多屏需求

#### 如果不需要播放音频

可以设置`hasAudio`为`false`，这样就不会解码音频数据了，可以提升性能。



### 黑屏，但是网速一直有数据


检查下播放地址，有没有带文件后缀，目前播放器是根据后缀来分析协议的，例如`.flv`后缀分析成flv格式。

对于地址如果没有带任何后缀的，播放器会默认识别为`m7s`的私有格式。

需要检查下 `isFlv` 参数。

#### 没有文件后缀，但是希望解析成flv协议。

配置`isFlv:true` 就行了。

#### 其他可能性

如果不是上述可能的原因，就看下`f12`控制栏上面是否有报错信息。

> 建议开启`debug:true` 参数，这样可以看到更多的日志信息。


### 支持浏览器打开连接立即播放视频

在浏览器的规范里面，是不允许自动播放的，必须要用户主动触发才行。

所以说，硬解码是没法支持的。

> 可以用软解码去实现

配置如下参数下：

```
useMSE:false,
useWCS:false
```

> 但是声音这块也是没法支持的，因为声音是需要借助浏览器提供的API去播放。


### Websocket 1006 异常断连

1006 是websocket的一个异常码，表示连接异常断开。

| 状态码 | 名称              | 描述                |
|-------|-----------------|-------------------|
| 1006 | CLOSE_ABNORMAL  | 用于期望收到状态码时连接非正常关闭 |

> WebSocket 关闭状态码 1006 是由于服务器在接收到客户端的连接请求后，在建立连接前发生了错误导致连接失败。


#### 可能的原因

1. 在客户端和WebSocket服务器之间的全双工连接中，有时候连接上可能没有数据流。在这个时候，网络中介可能中止连接。

> 就是可能会在一段时间内没有数据流，导致网络中介认为连接已经断开了。

> 也有可能是播放器端进程卡住了，导致接受推流的速度变慢，导致流媒体推流端推流到播放器变慢，甚至直接没法接收到流媒体传输过来的数据，导致网络中介认为没有流数据了，为连接已经断开了，也有可能是服务器端检测到堆积量过大，从而断开了ws连接，从而导致浏览器抛出了1006 错误。

> 有可能是本地的网络带宽上限要低于流媒体服务器端推流的码率，比如流媒体服务器端推流的码率是2M，而本地的网络带宽只有1M，这样就会有1M的数据堆积没法到达播放器端，导致服务器端堆积过多就会断开连接，然后播放器抛出了1006 错误。

> 通讯层(浏览器底层)断连了，但是应用层还是连接着，这个时候浏览器就会抛出1006错误。

2. 大多情况都是因为websocket 连接在nginx 配置的 proxy_read_timeout 内没有收到数据，nginx主动发起的连接断开（不是客户端主动断开，也不是服务端主动断开的）

> client->proxy->ws-server 如果proxy和ws-server之间通信有问题 client就会收到1006错误码。

3. 网络连接问题：网络中断、防火墙设置等因素可能导致WebSocket连接异常关闭。

4. 在播放倍率流的时候，如果服务器端是高倍率推流，比如8倍，这个时候如果电脑的性能跟不上，就会导致解封装和解码跟不上，因为js是单线程的，会导致解码和解封装的速度跟不上，导致堆积量过大，从而堵塞了接收流数据，从而触发了服务器端数据堆积过大，从而从物理层断开ws连接，从而导致浏览器抛出了1006 错误。



#### AI 回复的:
1. 服务器端程序崩溃或异常关闭：如果WebSocket服务器在处理连接请求时崩溃或异常关闭，连接将被重置，导致1006错误。
2. 客户端网络连接问题：客户端与服务器之间的网络连接出现故障，如网络断开、防火墙拦截等，也可能导致连接重置并出现1006错误。
3. 服务器端资源限制：如果服务器端资源受限，如内存不足、线程数达到上限等，可能导致服务器关闭连接以释放资源，从而引发1006错误。
4. WebSocket服务器配置问题：WebSocket服务器配置错误，如端口号不正确、认证设置不正确等，也可能导致连接重置并出现1006错误。

#### 排查

##### 服务器端原因：

这个错误通常是由于服务器端的问题导致的，比如服务器端的程序出现了 bug 或者服务器端的硬件出现了故障。

这种情况下可以考虑检查服务器端的程序和硬件是否正常工作, 查看`服务器端的日志`和`监控数据`来找出问题所在。

##### 网络原因

检查网络是否正常，网络是否稳定，网络是否有丢包，网络是否有延迟等。


#### 解决方案
1. 需要在nginx加入一段proxy的timeout超时设置，加了500s

2. `Pro播放器`支持内部检测到1006错误，会内部自动重连。

#### 资料

1. https://zhuanlan.zhihu.com/p/351747258

### 关于播放器内部自定义DOM

业务需要有自己的dom在播放器内部，例如

```html
<div id="container"></div>
```

当初始化播放器，传递 `container` 参数的时候，播放器会在 `container` 内部创建播放器的`DOM`

```html
<div id="container">
    // 播放器自己初始化的dom元素
    <canvas></canvas>
    <video></video>
</div>
```

如果业务需在播放器内部有自己的`DOM`，可以直接在`container`内部创建，播放器会自动识别并不会覆盖。


```html
<div id="container">
    // 播放器自己初始化的dom元素
    <canvas></canvas>
    <video></video>
    // 业务的dom
    <div class="your-dom">业务自己的dom</div>
</div>

```


小结： 所以在初始化播放器的时候，业务可以通过在container内部创建自己的dom。

```html

<div id="container">
    // 业务的dom
    <div class="your-dom">业务自己的dom</div>
</div>
```

待播放器初始化的时候，播放器只会在container内部创建自己的dom，不会覆盖业务自己的dom。

> 调用播放器destroy() 方法的时候，播放器内部也只会销毁掉播放器创建的dom，不会销毁业务自己创建的dom。

###  直播流播放完了，能监听到吗？

回答：不能

因为播放器也不知道流什么时候结束，播放器是被动的接受流数据的，所以播放器也不知道流什么时候结束。

解决方案：

1. 可以通过业务层去监听流是否结束。
2. 可以通过监听超时事件（不建议）。


### 关于当前直播流正在播的时间点

目前播放器能够监听到的时间是从流里面的`时间戳`，`stats`中的`ts`。

> 这个是流里面的时间戳，可能是相对时间戳，也可能是绝对时间戳。

如果业务上面需要获取`当前直播流正在播的时间点` 这需求，目前只能结合业务然后结合`ts`作为相对时间来计算。

1. 从服务器获取当前播放的时间点，
2. 监听播放器的stats事件，获取到`ts`，缓存一个开始时间点，通过最新的`ts`减去开始时间点，就是当前播放的时间点。



### UniApp 或者内嵌其他App（XX小程序） 里面webview，需要截图下载或者录制视频下载。

> 由于webview的限制，无法像浏览器那样直接截图，或者录制视频可以通过a.download 下载。

解决方案：

对于截图：

可以通过`screenshot(filename, format, quality,'base64')`返回base64数据，然后通过`jsbridge`传给app，app再通过`base64`转成图片，然后保存到本地。

> XX小程序可以通过`postMessage` 将base64传给小程序，小程序再使用系统级别api保存到本地。

对于录制视频：

目前 开源版暂不支持录制的视频返回`blob`格式。

> pro版本支持录制视频返回`blob`格式。 可以通过`jsbridge`传给app，app再通过`blob`转成视频，然后保存到本地。

> XX小程序可以通过`postMessage` 将blob转换成ArrayBuffer传给小程序，小程序再通过系统级别api保存到本地。


### 火狐(firefox)，chrome，等浏览器报ws地址连接不上

可能得原因：

1. 如果ws地址是`IP`的话，检查下`端口`是否是浏览器禁用的端口端。
2. 检查下`https` 下面是否请求的 `wss`地址，`http`下面是否请求的`ws`地址。


### 关于 Uncaught (in promise) DOMException: BodyStreamBuffer was aborted 错误

这个是当发起`http(s)`请求视频资源的时候，调用 `destroy()`销毁播放器的时候，worker线程调用了`abort()`方法，导致的浏览器抛出的错误。

> 这个异常暂时无法捕获到，不影响业务逻辑，可以先无视掉。

### 关于 Uncaught (in promise) DOMException: The user aborted a request. 错误

这个是当发起`http(s)`请求视频资源的时候，调用 `destroy()`销毁播放器的时候，主线程调用了`abort()`方法，导致的浏览器抛出的错误。

> 这个异常暂时无法捕获到，不影响业务逻辑，可以先无视掉。


### 关于 Uncaught (in promise) RuntimeError: Aborted(CompileError: WebAssembly.instantiate(): section (code 1, "Type") extends past end of the module (length 11493359, remaining bytes 2961839) @+8). Build with -sASSERTIONS for more info. 错误

这是由于`decoder.js` 和`decoder.wasm` 不匹配导致的。

解决方案：

每次都要全量替换`decoder.js` 和`decoder.wasm`，不能只替换其中一个。

> 记得需要清除下浏览器缓存（f12-> Network Tab（网络） -> 勾选 Disable cache(禁用缓存) 选项 ），然后刷新页面


### 关于iframe 页面里面有jessibuca 播放器，点击全屏按钮报：fullscreen request error TypeError Disallowed by permissions policy 错误

这个是由于浏览器的安全策略导致的。

iframe默认不允许全屏, 如果内嵌了video那么控制条上将不显示全屏按钮, 同理dom申请全屏事件也是不允许的。

解决方案：

通过添加allowfullscreen属性可以开启全屏功能

```html
<iframe allowfullscreen src=""></iframe>
```

这样就可以触发全屏了。


### 关于IOS不能系统全屏


IOS 全屏效果

<img src="/public/ios-fullscreen.png">


> IOS是不存在全屏API的，调用全屏会进入系统播放模式

解决方案：

可以通过配置`useWebFullscreen:true` 使用css全屏的方式进行模拟（即网页内全屏）。

参考demo: http://jessibuca.monibuca.com/mobile-fullscreen.html

#### IOS实现全屏效果

1. 业务层自己修改 `container`的宽高 + `resize()` 方法实现全屏效果

> 缺点：没法使用到播放器提供的底部控制栏，因为控制栏不会跟着变化。

2. 参数`useWebFullScreen`配置为`true`

> 播放器会检测当前环境是否支持系统级别的全屏方法，如果不支持，则会使用web全屏

> `jessibucaPro播放器`内部会自动判断，根据当前环境是否支持系统级别的全屏方法，来降级选择使用web全屏。



### Android端webview全屏调用无效问题

> android webView内默认是没有实现视频全屏的，调动dom.requestFullscreen没有任何响应，这个会表现为点击全屏按钮无效


解决方案：

该问题的解决必须依赖native端的开发，具体实现请参考以下方式WebView 实现全屏播放视频的示例代码

https://cloud.tencent.com/developer/article/1741520


### Android端webView灰色按钮（默认的播放按钮）问题

<img src="/public/img/android-webview-play-icon.png">

> android端自动起播在首帧出来之前会有一个灰色的播放按钮闪现，不同的手机或者android版本会略有不同，这个是webview中video内置的poster导致，前端无法隐藏

解决方案：

方案一：找android webView的开发同学，参考以下方式实现隐藏 HTML5 video remove overlay play icon

https://stackoverflow.com/questions/18271991/html5-video-remove-overlay-play-icon

https://www.mengke.me/blog/202312/Remove_Android_WebView_video_poster

You can hide this picture. For example:

```java
WebView mWebView = (WebView) findViewById(R.id.web_view);
mWebView.setWebChromeClient(new WebChromeClientCustomPoster());
```

Chrome client class:

```java
private class WebChromeClientCustomPoster extends WebChromeClient {

    @Override
    public Bitmap getDefaultVideoPoster() {
        return Bitmap.createBitmap(10, 10, Bitmap.Config.ARGB_8888);
    }
}
```

方案二：设置Video的poster属性为一个透明的图片,或者'noposter'

```
// 透明 base64
<video poster="data:image/gif;base64,R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7" />
// or
<video poster="https://via.placeholder.com/1x1" />
// or
<video poster="noposter" />
```
> poster="" 直接给空字符串会被忽略，所以需要设置一个透明的图片或者noposter

### IOS端无法内联播放（行内播放）

> 对于webrtc模式下

> canvas 渲染不存在这样的问题

ios10以下不支持内联播放

解决方案：

如果在webview内该属性不生效，则说明webview没有开启该属性，请找自己app native开发同学给webview容器对应的setting设置为true, 具体实现参考一下文档

https://developer.apple.com/documentation/uikit/uiwebview/1617960-allowsinlinemediaplayback

https://developer.apple.com/documentation/webkit/wkwebviewconfiguration/1614793-allowsinlinemediaplayback


### 关于报“Too many active WebGL contexts. Oldest context will be lost” 错误

这个错误是由于浏览器的限制导致的。

对于每个不同的浏览器

- Google Chrome: 通常情况下，Chrome 的上限是 16 个 WebGL 上下文。这是一个相对较低的限制，如果超过这个数量，会出现 “Too many active WebGL contexts” 错误。

- Mozilla Firefox: Firefox 也有一个上限，通常在 16 到 32 个 WebGL 上下文之间。这个上限可以在 about:config 中的 webgl.context-creation.max-ctx 设置中进行调整。

- Microsoft Edge: Microsoft Edge 的上限通常与 Chromium 类似，大约为 16 个 WebGL 上下文。

- Safari: Safari 在 macOS 上的上限通常比较高，可以达到几百个，但仍然受到系统资源的限制。

解决方案：

1. 降低多屏数量
2. 优先使用mse解码+video渲染
3. 可以使用pro版本，pro 版本支持所有解码器（wasm+mse+webcodec）都可以使用video标签渲染，所以不会有任何限制情况。

### 关于报“RuntimeError: abort(OOM). Build with -s ASSERTIONS=1 for more info” 错误

这个错误是wasm的内存不足导致的。

解决方案：

1.提高wasm申请的内存大小。
2.降级播放流的分辨率。


### chrome chunk_demuxer_error_append_failed 错误

"chrome chunk_demuxer_error_append_failed" 错误通常与视频播放相关。这个错误表示 Chrome 浏览器的 chunk demuxer（分段解复用器）无法将一些视频片段拼接在一起以播放视频。

这个错误通常是由于视频文件本身的问题引起的，例如视频文件损坏或者格式不受支持。也可能是由于网络连接问题，例如视频文件未完全下载或下载过程中发生了错误。


> 开源版软解码（wasm）最高能支持的分辨率是720p的。

> pro版本软解码（wasm simd）最高能支持的分辨率是4k的。



### failed to execute 'fetch' on 'workerGlobalScope' : failed to parse url from decoder.wasm 错误

这个错误是由于`decoder.wasm`文件下载失败导致的。

> 一般这种情况都是配置了，`decoder:cdn url` 缘故导致的，并没有修改decoder.js 里面引用的decoder.wasm的url。

解决方案：见

https://jessibuca.com/document.html#jessibuca-js-decoder-js-decoder-wasm%E6%96%87%E4%BB%B6%E6%83%B3%E9%80%9A%E8%BF%87cdn%E5%8A%A0%E8%BD%BD


### The play() request was interrupted because video-only background media was paused to save power 错误

通常发生在网页应用尝试自动播放视频时，但浏览器出于节能目的暂停了视频的播放。

可以看下[chrome的自动播放策略](https://developer.chrome.com/blog/autoplay?hl=zh-cn)

1. 浏览器的自动播放策略：许多现代浏览器，尤其是移动设备上的浏览器，会限制在不同条件下自动播放媒体内容，尤其是如果媒体内容没有与用户的互动。这是为了节约数据和电池。
2. 视频内容的属性：如果视频是静音的或不包含音频轨道，某些浏览器可能会允许自动播放。但如果视频包含音频，且页面没有得到用户的明确互动（如点击），浏览器可能会阻止自动播放。
3. 电源节约模式：在某些设备上，如果启用了电源节约模式，浏览器可能会限制背景媒体的播放，以减少电量消耗。

解决方案：

1. 用户交互：确保在用户与页面互动（如点击按钮）后再播放视频。
2. 静音视频：如果视频不需要音频，可以尝试将其设置为静音。
3. 检查浏览器设置：用户可以查看浏览器的隐私或安全设置，看看是否有限制自动播放媒体的选项。
4. 检查设备设置：用户可以查看设备的电源设置，看看是否有限制自动播放媒体的选项。
5. 顶级帧可以将自动播放权限委托给其 `iframe`，以允许有声自动播放(测试了没有啥效果)。


```html

<!-- 允许自动播放 -->
<iframe src="https://cross-origin.com/myvideo.html" allow="autoplay">

<!-- 允许自动播放和全屏 -->
<iframe src="https://cross-origin.com/myvideo.html" allow="autoplay; fullscreen">
```
> 测试了iframe感觉没啥效果，还是会触发 `play() failed because the user didn't interact with the document first` 异常

可以看在[权限策略](https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Permissions_Policy)

### 播放内网https地址报错(ERR_CERT_COMMON_NAME_INVALID 错误)

> 一般这种情况是浏览器端不认可https证书的缘故。

解决方案：

方案1：修改自签名证书

https://www.dyxmq.cn/network/err_cert_common_name_invalid.html

方案2：

通过chrome 浏览器设置 `隐私和安全` -> `允许显示不安全内容` 配置让浏览器端认可这个内网https证书。

<img src="/public/img/https-setting.png">


### 苹果IOS系统webglcontextlost问题（ios内核的bug）

> 在IOS手机 解码视频流的时候，第一次可以正常播放，但只要IOS手机熄屏，再重新唤醒，就会一直播放失败，无论换哪个浏览器都不行。安卓手机则一切正常。

> 重新打开新的窗口也不行。

> 只能通过手动关闭safari / chrome 浏览器，重新打开才行。

> 16.7.x 和 17.0.x 都有机会遇到

解决方案： 升级到最新的17.1.x版本。

参考：https://blog.csdn.net/s18813688772/article/details/134203931


### 点播H264、H265需求（点播mp4、Hls）

> jessibuca 是流播放器，目前暂不支持点播的逻辑。


解决方案：

1. [video.js](https://videojs.com/)
2. [xgplayer.js](https://h5player.bytedance.com/)
3. [JessibucaProVod](https://jessibuca.com/player-pro-vod.html) 正在开发中，敬请期待。


### 测试的时候遇到请求的连接（播放地址）跨域报错

#### 方法1：使用扩展程序
安装`CORS`扩展: 在`Chrome Web Store`中搜索并安装一个允许跨域请求的扩展程序，如“`CORS Unblock`”或“`Allow CORS: Access-Control-Allow-Origin`”。

启用扩展程序: 安装完成后，在浏览器扩展程序栏中找到该扩展并启用。

配置扩展程序: 根据需要配置扩展程序的设置，以允许特定的跨域请求。

> 如果chrome没法安装成功，可以在Edge浏览器已经安装也是可以的。插件名称：CORS Unblock



#### 方法2：修改浏览器启动参数
关闭所有Chrome实例: 确保所有Chrome窗口都已关闭。

修改启动快捷方式: 右击Chrome的启动快捷方式，选择“属性”。

添加参数: 在“目标”字段中，Chrome.exe后添加参数 `--disable-web-security --user-data-dir=[某个文件夹路径]。`例如: "C:\Program Files (x86)\Google\Chrome\Application\chrome.exe" --disable-web-security --user-data-dir=C:\ChromeDevSession。

重启Chrome: 使用修改后的快捷方式启动Chrome。


### 公网访问内网地址的时候报跨域错误

比如在`https://jessibuca.com` 访问 `http://192.168.xxx.xxx/test.flv` 地址，会报`the request client is not a secure context and the resource is in more-private address space 'private'` 错误

<img src="/public/img/play-192-error.png">

#### 方法1：修改flags 参数
打开  `chrome://flags/` 搜索 `Block insecure private network requests`，将其设置为 `Disabled`，然后重启浏览器。

<img src="/public/img/block-insecure-setting.png">

#### 方法2：使用扩展程序
安装`CORS`扩展: 在`Chrome Web Store`中搜索并安装一个允许跨域请求的扩展程序，如“`CORS Unblock`”或“`Allow CORS: Access-Control-Allow-Origin`”。

启用扩展程序: 安装完成后，在浏览器扩展程序栏中找到该扩展并启用。

配置扩展程序: 根据需要配置扩展程序的设置，以允许特定的跨域请求。


### 关于播放器地址不带协议后缀的判断逻辑

播放地址带协议后缀的：`https://test.com/play/xxxx.flv`

播放地址不带协议后缀的：`https://test.com/play/xxxx`

> 播放器不会检查?参数里面携带的后缀，只会检查地址里面的后缀。例如：`https://test.com/play/xxxx?format=flv` 这种不会被检查成flv格式。

#### 开源版

开源版目前支持`.flv`，还有 `m7s` 私有格式。 如果播放的地址不带`.flv` 例如:`https://test.com/play/xxxx` 播放器默认会按照`m7s`格式解析播放的。

如果 `https://test.com/play/xxxx`  这种播放地址需要按照`flv` 协议解封装，则需要通过配置参数`isFlv:true`就行了。

#### Pro 版本

Pro版本支持所有的格式，如果播放的地址不带后缀，播放器会默认识别为`m7s`的私有格式。

同理，如下想法按照某种特定格式解析播放的，例如 `flv`协议解析,则需要配置`isFlv:true`就行了，例如 `裸流的格式`， 则需要配置`isNakedFlow:true`就行了

> 推荐如果没有协议后缀（例如.flv）的播放地址，通过指定解析协议参数来播放。


### 关于更新播放窗口大小

1. 修改`your-container`的宽高
2. 调用播放器的`resize()`方法。


> your-container 是new Jessibuca() 的 container 参数

```js
const jessibuca = new Jessibuca({
    container: document.getElementById('your-container')
})
```

### 关于在H5中实现横竖屏自适应

> 这个需要业务层借助resize() 方法自己实现

1. 监听到横竖屏切换事件
2. 修改`your-container`的宽高
3. 调用播放器的`resize()`方法。

> your-container 是new Jessibuca() 的 container 参数

```js
const jessibuca = new Jessibuca({
    container: document.getElementById('your-container')
})
```

监听横竖屏切换事件

```js
window.addEventListener('orientationchange', function () {
    // 横屏
    if (window.orientation === 90 || window.orientation === -90) {
        // 修改your-container的宽高
        // 调用播放器的resize()方法
    } else {
        // 竖屏
        // 修改your-container的宽高
        // 调用播放器的resize()方法
    }
})
```

### RuntimeError： Aborted(compileError:Webassembly.instantiate(): expected magic word 00 61 73 6d, found 3c 21 44 4f @+0)

一般这种报错是`decoder.js` 对应的`decoder.wasm`胶水文件错误导致的。

> decoder.js 和 decoder.wasm 胶水文件是一一对应的，不同版本的不能混在一起使用。

如果发现这个错误，检查下是不是缓存原因导致的两个文件的版本没有一一对应上。如果不是，可以去官网或者github 上下载最新版本的jessibuca，全量替换更新下就行了。

> 记得需要清除下浏览器缓存（f12-> Network Tab（网络） -> 勾选 Disable cache(禁用缓存) 选项 ），然后刷新页面


### WebRTC

> WebRTC标准是不支持h265的。

> jessibuca pro 版本结合M7S已经支持了。欢迎测试使用。 http://jessibuca.monibuca.com/player-pro.html


#### 关于播放webrtc 的 H265格式的视频

目前 `pro版本` 是支持`[M7S流媒体服务器](https://m7s.live/)`来播放 webrtc 的 H265格式的视频。

> 同时也支持音频

是借助DataChannel实现的。

可以看网友的基于DataChannel 实现的 H265 的方案。

https://juejin.cn/post/7215608036394614844

> 当然 pro 可以做到1s以内的更低延迟。

> 对于ZLMediaKit，目前官网版本是不支持DataChannel的，需要自己实现。如需要对接集成，可以联系Pro作者：bosswancheng


### 播放器内部的样式发生变形或者class 丢失

可能得原因：

- 播放器样式被其他样式覆盖了,检查下是否有全局样式覆盖了播放器的样式（播放器内部会有`video`,`canvas`标签）。
- container 设置了双向绑定，导致class丢失。

推荐的 vue 写法

```vue
<template>
    <div class="wrap">
      <!--  不要绑定任何的 :class :style 样式  -->
        <div ref="container" ></div>
    </div>
</template>
<script>
export default {
    name: 'App',
    mounted() {
        // 通过 ref 获取到 dom 节点
        const dom = this.$refs['container']
        // 通过 dom 节点获取到 player 实例
        const player = new window.Jessibuca({
            container: dom,
        })
    }
}
</script>
```

推荐的 react 写法

```jsx
import React, { useEffect, useRef } from 'react'

export default function App() {
    const container = useRef(null)
    useEffect(() => {
        const player = new window.Jessibuca({
            container: container.current,
        })
    }, [])
    return (
        <div className="wrap">
            <div ref={container}></div>
        </div>
    )
}

```


### 当container窗口发生变化的时候，播放器如何自适应

播放器提供一个`resize()` 方法。当外界窗口发生变化的时候，调用该方法即可。

```js
const player = new window.Jessibuca({
    container: dom,
})



// 当container的宽高发生变化的时候，调用resize方法
player.resize();
```

### 出现页面崩溃之后，问题定位

#### 是否开启了`devTools`

首先检查下是否开启了`devTools`。

因为devtools 会打印日志，日志本身就会占用很多内存，也会导致浏览器崩溃。

#### 检查崩溃日志

通过`chrome://crashes/`查看崩溃日志

#### 如果是必先的

可以通过

<img src="/img/crash-1.png">

<img src="/img/crash-2.png">

观察下内存是否一致增加。

### 安卓webview 下面的一些问题

> 在项目中，会有在webview嵌入的网页中播放视频的需求，会在部分手机上出现白屏或有声音无画面等问题，并且存在全屏按钮点击无效果的问题。


借鉴：https://www.cnblogs.com/hwb04160011/p/13960585.html

#### 播放视频白屏、无画面问题解决
原因是WebView播放视频时可能需要硬件加速才可以正常播放视频，关闭硬件加速可能在部分手机上出现白屏、无画面、无法暂停等问题。解决方法就是开启硬件加速：

在AndroidManifest.xml的application中声明HardwareAccelerate属性
在AndroidManifest.xml的对应activity中声明HardwareAccelerate属性
在使用WebView的代码前添加如下代码：

```java
window.setFlags(WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED,
    WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED)
```
#### 视频不能全屏问题解决

原因是WebView的视频全屏事件是需要客户端完成后续逻辑的，网页点击全屏按钮会触发WebChromeClient的onShowCustomView方法，全屏后缩回来会触发onHideCustomView方法，在这两个方法中我们要对视频进行一个全屏放大的处理。

在我们的WebView使用之前需要添加的代码如下：

```java
    var fullscreenContainer: FrameLayout? = null
    var customViewCallback: WebChromeClient.CustomViewCallback? = null
    val mWebChromeClient = object : WebChromeClient() {
        override fun onShowCustomView(view: View?, callback: CustomViewCallback?) {
            super.onShowCustomView(view, callback)
            showCustomView(view, callback)
        }

        override fun onHideCustomView() {
            super.onHideCustomView()
            hideCustomView()
        }
    }

    /**
     * 显示自定义控件
     */
    private fun showCustomView(view: View?, callback: WebChromeClient.CustomViewCallback?) {
        if (fullscreenContainer != null) {
            callback?.onCustomViewHidden()
            return
        }

        fullscreenContainer = FrameLayout(context).apply { setBackgroundColor(Color.BLACK) }
        customViewCallback = callback
        fullscreenContainer?.addView(view)
        val decorView = (context as? Activity)?.window?.decorView as? FrameLayout
        (context as? Activity)?.requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
        decorView?.addView(fullscreenContainer)
    }

    /**
     * 隐藏自定义控件
     */
    private fun hideCustomView() {
        if (fullscreenContainer == null) {
            return
        }

        val decorView = (context as? Activity)?.window?.decorView as? FrameLayout
        (context as? Activity)?.requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT
        fullscreenContainer?.removeAllViews()
        decorView?.removeView(fullscreenContainer)
        fullscreenContainer = null
        customViewCallback?.onCustomViewHidden()
        customViewCallback = null
    }

```

最终在WebView使用时，为WebView设置WebChromeClient：

```java
    webView.webChromeClient = mWebChromeClient
```


### PC电脑端播放视频，整个显示器会突然白屏一下

1. 看下浏览器有没有关闭硬解码。
2. 看下显卡驱动是不是好久没有更新了。

> 显卡驱动太老的话，会有可能导致硬解码出现问题，导致整个显示器画面白屏下的。

### wasm编译打包 之后报：uncaught referenceError:module is not defined

 一般是因为wasm没有编译成功，导致的。

### video 抛出 PIPELINE_ERROR_DECODE: video decode error! 错误

### video 抛出 PIPELINE_ERROR_COULD_NOT_RENDER 错误

### video 抛出 PIPELINE_ERROR_DECODE: VDA Error 4 错误


### video 抛出  PIPELINE_ERROR_COULD_NOT_RENDER 错误

这是因为访问的是受限制的端口

> 例如 10080 端口


解决方案：

1. 这个方法就很简单了，直接改网站监听的端口就行了，不要使用受限制的端口即可。（推荐）
2. 配置 `--explicitly-allowed-ports=10080` 参数，允许访问受限制ed端口。（强烈不推荐）


### 关于：The play() request was interrupted by a call to pause() 报错

这是因为在发起 video标签的 `play()` 还没有返回结果的时候，调用了`pause()`方法导致的。

因为 `play()` 方法是是个`Promise`,所以需要等待`play()`方法返回结果之后，再调用`pause()`方法。


### video 抛出 Unmuting failed and the element was paused instead because the user didn't interact with the document before

这个错误是由于浏览器的自动播放策略导致的，浏览器不允许播放带有音频的视频。

#### 对于硬解码
如果业务系统是设置了打开页面的时候，就自动进行播放视频的话，建议不要将`isNotMute`参数设置为`true` 在硬解码的时候，浏览器会抛出以上异常。

#### 对于软解码
软解码的时候，由于调用的是 audioContext 进行播放音频数据，不会影响视频播放。

#### 解决方案

不要将`isNotMute`参数设置为`true`，通过监听start事件，然后根据当前场景判断是否调用 `cancelMute()` 方法

```js
jessibuca.on('start', () => {
    /**
     * 0：网页通过点击链接、地址栏输入、表单提交、脚本操作等方式加载，相当于常数performance.navigation.TYPE_NAVIGATE。
     * 1：网页通过“重新加载”按钮或者location.reload()方法加载，相当于常数performance.navigation.TYPE_RELOAD。
     * 2：网页通过“前进”或“后退”按钮加载，相当于常数performance.navigation.TYPE_BACK_FORWARD。
     * 255：任何其他来源的加载，相当于常数performance.navigation.TYPE_RESERVED。
     */
    if (performance.navigation.type === 0) {
        // 这里如果是地址栏输入（书签）打开，貌似也会进去，奈何浏览器也不允许这种逻辑进行自动播放音频，播放器内部会降级到软解码去。。
        jessibuca.cancelMute();
        console.log('volume',jessibuca.getVolume());
    }
})
```

参考demo:[demo-auto-play.html](https://jessibuca.com/pro/demo-auto-play.html)


### 浏览器抛出 Unmuting failed and the element was paused instead because the user didn't interact with the document before

这是由于 浏览器的安全机制，在某些情况下

1. 网页通过“前进”或“后退”按钮加载
2. 网页通过“重新加载”按钮或者location.reload()方法加载
3. 网页通过`地址栏输入`或者`书签`加载

这些情况下，浏览器会认为用户没有主动操作，所以会禁止自动播放。

> 浏览器允许通过`点击链接`方式打开的页面自动播放。

参考demo:[demo-auto-play.html](https://jessibuca.com/pro/demo-auto-play.html)

### window.Jessibuca is not a constructor 错误

这种错误一般都是 `jessibuca.js` 没有加载成功导致的。

检查F12 -> 网络(network) -> 找到加载的 `jessibuca.js` 文件,然后看下 `response` 返回的内容是否是正常的。

> 如果路径配置的不对的话，会存在vue 或者react 项目 直接被返回了index.html 内容了



### 关于硬解码或者video标签渲染自动播放

背景：用户希望打开页面的时候就直接自动播放视频（单屏或者多屏），但是浏览器的自动播放策略是，必须是用户手动触发了事件之后，才能自动播放。

会抛出`DOMException: play() failed because the user didn't interact with the document first. https://goo.gl/xX8pDD` 错误。

#### 解决方案

1. 添加一个交互事件，让用户手动触发下，再去播放视频。
2. 使用`wcs`解码(在https环境下)，然后使用`canvas`标签渲染。
3. 使用wasm(simd) 软解码，然后使用`canvas`标签渲染。

### 页面首次加载超时检测

目前播放器的默认配置是

```
{
    loadingTimeout: 10,
    loadingTimeoutReplay:true,
    loadingTimeoutReplayTimes:3
}
```

> 如果想要如果想无限次重试，可以设置loadingTimeoutReplayTimes为-1

### 页面播放过程中超时检测

目前播放器的默认配置是

```
{
    heartTimeout: 10,
    heartTimeoutReplay:true,
    heartTimeoutReplayTimes:3
}
```

> 如果想要如果想无限次重试，可以设置heartTimeoutReplayTimes为-1


### 播放的时候就有声音

分两种业务场景

1.希望页面打开的时候，就自动播放，并且带有声音
2.通过交互点击事件打开播放器，带有声音

#### 页面打开，刷新的时候页面的时候，就需要自动播放，并且带有声音


> 如果是通过点击链接、脚本操作等方式加载页面，可以通过js程序去掉静音。

> “重新加载”按钮或者location.reload()方法加载,通过“前进”或“后退”按钮加载 这种是不会带有声音播放的。


目前推荐的方案是：借助`performance.navigation.type 判断是否满足加载声音的条件`。来实现自动播放(带声音)

```js
 jessibuca.on('start', () => {
    /**
     * 0：网页通过点击链接、地址栏输入、表单提交、脚本操作等方式加载，相当于常数performance.navigation.TYPE_NAVIGATE。
     * 1：网页通过“重新加载”按钮或者location.reload()方法加载，相当于常数performance.navigation.TYPE_RELOAD。
     * 2：网页通过“前进”或“后退”按钮加载，相当于常数performance.navigation.TYPE_BACK_FORWARD。
     * 255：任何其他来源的加载，相当于常数performance.navigation.TYPE_RESERVED。
     */
    if (performance.navigation.type === 0) {
        jessibuca.cancelMute();
    }
})
```

可以监听点击事件来解除静音

```js

$container.addEventListener('click', function () {
    jessibuca.cancelMute()
}, false)
```

可以看下demo:[demo-auto-play.html](https://jessibuca.com/pro/demo-auto-play.html)

#### 通过交互点击事件打开播放器，带有声音

播放器默认播放的时候，是静音播放的。所以如果想播放的时候解除静音，则需要配置`isNotMute:true`就可以了。

可以看下demo:[demo-play-not-mute.html](https://jessibuca.com/pro/demo-play-not-mute.html)


### 关于初始化webgl失败的可能性

1. 浏览器不支持webgl。
2. 浏览器支持webgl，但是被禁用了。
3. 如果是套壳在QT等环境下，可能会有webgl初始化失败的情况。检查是否选择了正确的显卡，或者显卡驱动是否正常。

#### 浏览器如何禁用/开启 webgl

要在Chrome浏览器中禁用WebGL，您可以按照以下步骤操作：

1. 打开Chrome浏览器并输入以下地址：chrome://flags。
2. 在Chrome Flags页面中，搜索框中输入"webgl"，以查找与WebGL相关的标志。
3. 找到名为"WebGL"的选项，并将其设置为"Disabled"。
4. 关闭Chrome浏览器，并重新启动它以使更改生效。


完成上述步骤后，Chrome浏览器将禁用WebGL功能。请注意，这将影响所有网站上使用WebGL的内容，包括3D图形和游戏等。


### 关于切换分辨率

目前pro版本支持配置分辨率参数，会在底部UI 展示，当点击分辨率的时候，会抛出事件，然后业务层监听到事件，通过调用播放器的`play(url)` 方法来实现分辨率的切换逻辑。

体验demo：[https://jessibuca.com/pro/demo-control-dom.html](https://jessibuca.com/pro/demo-control-dom.html)


### 关于播放webrtc 报： Failed to execute 'setRemoteDescription' on 'RTCPeerConnection': Failed to parse SessionDescription.  Duplicate a=msid lines detected

解决方案：https://blog.csdn.net/dualvencsdn/article/details/137049065



### 浏览器播放视频过程中，整个显示器会突然的白屏下

> 大概率是显卡驱动问题，可以尝试升级显卡驱动，或者降级浏览器版本。

可以看下这个解决方案：
https://blog.csdn.net/DYxiao666/article/details/136072932


### Failed to execute 'requestfullscreen' on 'Element': APl can only be initiated by a user gesture.

这个错误是因为全屏操作必须是用户手动触发的，不能是程序触发的。


### 如何在electron中使用 jessibuca 播放视频

> 在electron中这样就是以file:///路径格式加载，而浏览器加载wasm必须以web形式加载。否则控制台报错：

而使用Electron开发App肯定是希望离线部署的，所以也不能部署到cdn来加载。

解决方法很简单，就是用nodejs创建一个http static file 服务即可。

解决方案：

####  安装 node-static

```
npm i node-static
```

#### 创建静态文件服务器


```js
const { app, BrowserWindow } = require('electron')
const static = require('node-static');
const http = require('http');
// 将程序目录下的js目录设为webroot目录
const file = new (static.Server)(__dirname+"/js");

app.whenReady().then(() => {
  http.createServer(function (req, res) {
    file.serve(req, res);
    // 在本机上监听8080端口提供服务
  }).listen(8888, "127.0.0.1");
  createWindow()
})
```
#### 引用jessibuca

将 jessibuca 的 `dist`文件夹下面的  `js文件`和`wasm文件` 放到 `/js`目录下。

> pro 的文件夹是 pro/js 文件夹

#### 在html中引用

```html

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
<!--    注意：这个需要注释掉 -->
<!--    <meta http-equiv="Content-Security-Policy" content="default-src 'self'; script-src 'self'">-->
<!--    <meta http-equiv="X-Content-Security-Policy" content="default-src 'self'; script-src 'self'">-->
    <title>Document</title>
    <script type="text/javascript" src="http://localhost:8080/jessibuca.js"></script>
</head>
<body>
    <div id="container"></div>
    <script>
        //
        var jessibuca = new Jessibuca({
            container: document.getElementById('container'),
        })

        jessibuca.play('http://xxx.xxx.xxx.xxx:8080/xxx.flv')
    </script>

</body>
```


### 如何开启electron硬解码Hevc（H265）

https://github.com/StaZhu/enable-chromium-hevc-hardware-decoding/blob/main/README.zh_CN.md

> 如果是 Electron 20 (Chromium 104)，则已集成好 Mac, Windows 平台的 HEVC 硬解功能，在启动时执行 app.commandLine.appendSwitch('enable-features', 'PlatformHEVCDecoderSupport') 即可启用硬解。若要集成软解，方法同上述 Chromium 教程相同。

见：https://www.cnblogs.com/gnz48/p/16422304.html


### 如何验证视频播放是否走硬解？

打开 `chrome://gpu`, 搜索 `Video Acceleration Information`, 如果能看到 `Decode hevc main` 和 `Decode hevc main 10` (macOS 还会显示 `Decode hevc main still-picture` 和 `Decode hevc range extensions`) 说明支持硬解（这里 macOS 是个例外，显示仅代表支持 `VideoToolbox` 解码，至于是否硬解取决于 GPU 支持情况)。

打开 `chrome://media-internals` 并尝试播放一些 HEVC 视频 (测试页面)，如果最终使用的 Decoder 是 `VDAVideoDecoder` 或 `D3D11VideoDecoder` 或 `VaapiVideoDecoder` 说明走了硬解（这里 macOS 是个例外，macOS Big Sur 以上版本，在不支持的 GPU 上，VideoToolbox 会自动 fallback 到软解，性能相比 FFMPEG 软解更好，Decoder 同样为 `VDAVideoDecoder`）, 如果 Decoder 是 `FFMpegVideoDecoder` 说明走的是软解。

#### MAC
如果是 Mac，请打开 活动监视器并搜索 `VTDecoderXPCService`, 如果播放时进程的 CPU 利用率大于0说明走了硬解（或软解)。
#### Windows
如果是 Windows，请打开 任务管理器 并切换到 性能 - GPU 面板，如果 `Video Decoding` 的利用率大于0说明走了硬解。

### 为什么我的显卡支持，但仍无法使用硬解？

1. 操作系统版本过低
2. 显卡驱动版本有问题
3. 特定硬件有问题

#### 操作系统版本过低

##### Windows

请确保操作系统版本大于等于 `Windows 8`，这是因为 Chromium 的 `D3D11VideoDecoder` 仅支持 Windows 8 以上系统，在 Windows 8 以下操作系统使用 `VDAVideoDecoder` 进行硬解。而 `VDAVideoDecoder` 基于 `Media Foundation` 实现，`Media Foundation` 对于 HEVC 硬解的支持（`需要安装 HEVC视频扩展 插件`），系统版本需大于 `Windows 10 1709`。

##### macOS

请确保操作系统版本大于等于 `Big Sur`，这是因为`CMVideoFormatDescriptionCreateFromHEVCParameterSets API`，在 Big Sur 以下版本有兼容问题。


#### 显卡驱动版本有问题

部分显卡驱动版本有 BUG，导致被禁用使用`D3D11VideoDecoder`，因此若你确保 GPU 支持 HEVC 硬解，请先更新到最新版本显卡驱动再尝试。


#### 特定硬件有问题

部分硬解有 BUG，导致被禁用 `D3D11VideoDecoder`，这种情况没什么办法解决，只能软解。


### JbFro container has been created and can not be created again

这个错误的原因通常是调用`destroy()` 方法不对导致的。

> destroy() 返回的是Promise

解决方案
```js

await jessibuca.destroy();

// 重置解码器
```
或者
```js
jessibuca.destroy().then(()=>{
  // 重置播放器。

}).catch(()=>{

})
```


### window.jessibuca is not a constructor

这个错误通常是因为`jessibuca.js`没有加载成功导致的。

排查：

查看html页面的`script`标签是否引入了`jessibuca.js`文件。

```html
<script src="jessibuca.js"></script>
```
> 确保`jessibuca.js`文件的路径是正确的。能够访问到。返回的是正常的js文件。而不是html文件（Nginx配置当访问资源404的时候会默认返回index.html内容）。

检查：f12 打开控制台，然后切换到network tab选项卡，然后找到`jessibuca.js`文件，看下`response`返回的内容是否是正常的js文件。


### 播放器是否支持IPv6 播放地址

播放器不限制播放的地址，只要是浏览器所在环境支持的地址，播放器都是支持的。

> 前提得保证所在web页面环境能够正常访问IPv6地址，请求能够被响应。

### Mixed Content: The page at 'https://jessibuca.com' was loaded over HTTPS, but requested an insecure resource 'http://xxx.com/xxx.flv'. This request has been blocked; the content must be served over HTTPS.

这个错误是因为页面是https，但是请求的资源是http，浏览器不允许这种请求。

解决方案：

1. 使用 http://jessibuca.monibuca.com/ 地址 代替 https://jessibuca.com 地址

同理，如果是http页面，请求的资源是https，也会报同样的错误。

解决方案

1. 使用 https://jessibuca.com 地址 代替 http://jessibuca.monibuca.com 地址


### webview环境下，PC和安卓能够正常播放，IOS环境下播放器黑屏无法播放

> pc 和 安卓的环境下播放正常（走的是硬解码）。

> 如果安卓下面走的软解码，也会出现黑屏的情况。

> IOS 现状就是黑屏，然后vconsole也没有啥报错信息。

大概率是[wasm 格式返回错误 Incorrect response MIME type. Expected 'application/wasm'. falling back to arraybuffer instantiation 错误](/document.html#wasm-格式返回错误-incorrect-response-mime-type-expected-application-wasm-falling-back-to-arraybuffer-instantiation-错误) 这个原因导致的。

### 加载视频等待画面时长过长

可能的原因：

1. 检查下请求地址是否正常，是否有返回数据，以及相应的时长。
2. 检查下首帧是否推送的I帧数据，如果没有I帧数据，会导致等待画面时长过长。


### wasm 报："failed to asynchronously prepare wasm: Error: WebAssembly.Module doesn't parse at byte xxx: invalid opcode xxx, in function at index xxx" 异常

完整的错误信息：

> "failed to asynchronously prepare wasm: Error: WebAssembly.Module doesn't parse at byte xxx: invalid opcode xxx, in function at index xxx"
> "Aborted(Error: WebAssembly.Module doesn't parse at byte xxx: invalid opcode xxx, in function at index xxx)

Unhandled Promise Rejection: Error: Aborted (Error: WebAssembly.Module doesn't parse at byte 659: invalid opcode 192, in function at index101). Build with -sASSERTlONS for more info, (evaluating 'new WebAssembly,RuntimeErrorle)'

### 在已经使用硬解码基础上，播放多路视频，会出现卡顿，内存开始飙升

> 在播放1路到4路的情况下，硬解码是没有问题的，但是播放到5路以上，就会出现卡顿，内存飙升的情况。

这种情况大概率是因为显卡的解码性能跟不上导致的。

解决方案
1. 升级显卡
2. 降低分辨率/帧率
3. 降低播放路数



## 支持作者

### 第一作者
<img src="/public/wx.jpg"><img src="/public/alipay.jpg">

### V3版本作者
<img src="/public/wx-pay-wc.jpg" style="width:333px"><img src="/public/alipay-wc.jpg" style="width:333px">


### 群
<img src="/public/qrcode.jpeg">

### 群
<img src="/public/qrcode-qw.jpeg">


## qq频道
<img src="/public/qq-qrcode.jpg">

