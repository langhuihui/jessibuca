
# 简介
jessibuca pro 是在开源版本的基础上额外支持的深入业务解决方案的。解决了一些痛点，比如H265的硬解码，SIMD软解码加速。
<Rice/>


## 开源版升级到 PRO 体验版

联系添加作者微信：bosswancheng



## 关于如何配置使用 wasm/simd多线程解码



### localhost

不限制 `http`或者 `https`协议

需要在线申请一个chrome的 `origin trial token`
https://developer.chrome.com/origintrials/?utm_source=devtools#/view_trial/303992974847508481

> 需要自备机场才能访问的到。

> 例如本地的地址为：http://localhost:8080，申请的时候，需要填写`http://localhost:8080`就可以了。

然后配置到`index.html`中

```html
    <meta http-equiv="origin-trial" content="your origin trial token">
```

### 服务器

需要`https`协议下。否则无法使用。

需要在服务器上配置`cross-origin-isolated`头


#### 以node 为例
```js
app.use((req, res, next) => {
    res.setHeader('Cross-Origin-Opener-Policy', 'same-origin')
    res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp')
    next()
})
```

可以跑下官方demo

[https://github.com/bosscheng/jessibuca-pro-wasm-multi-thread-demo](https://github.com/bosscheng/jessibuca-pro-wasm-multi-thread-demo)

#### nginx 配置

```nginx

// 需要放在路由规则里面
add_header Cross-Origin-Opener-Policy same-origin;
add_header Cross-Origin-Embedder-Policy require-corp;
```

案例：

```nginx
server {
    listen 443 ssl;
    server_name xx.xx.xx.xx;

    #证书路径
    ssl_certificate server.crt;
    ssl_certificate_key server.key;

    # SSL增强配置
    ssl_protocols TLSv1.2 TLSv1.3;
    SSL_ciphers 'ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256';
    ssl_prefer server_ciphers on;
    ssl_session_cache shared:SSL:10m;
    ssl_session_timeout 10m;

    # same origin 配置
    add_header Cross-Origin-0pener-Policy same-origin;
    add_header Cross-Origin-Embedder-Policy require-corp;

    # 全局CORS配置
    add_header Access-Control-Allow-Origin "*";

    #项目路径
    root xxxx/xxxxx/xxxx;
    index index.html index.htm;

    # 路由规则
    location / {
        try_files $uri $uri/ /index.html;
        add_header Access-Control-Allow-Origin "*" always:
        add_header Cross-Origin-0pener-Policy same-origin;
        add_header Cross-Origin-Embedder-Policy require-corp;
        autoindex on;
    }

    # 静态资源缓存
    location ~* \.(js|css|png|jpg|jpeg|gif|ico|svg)$ {
        expires 30d;
        add_header Access-Control-Allow-Origin "*" always;
        add_header Cross-Origin-0pener-Policy same-origin;
        add_header Cross-Origin-Embedder-Policy require-corp;
    }
}



```




### -enable-features=SharedArrayBuffer启动 Chrome

您可以使用命令行标志--enable-features=SharedArrayBuffer启动 Chrome，从而在不启用跨域隔离的情况下启用SharedArrayBuffer。

谷歌浏览器版本 >= 92.

在window系统下，可以在快捷方式的目标中添加 `--enable-features=SharedArrayBuffer`

或者通过执行脚本

```shell
chrome --enable-features=SharedArrayBuffer
```


``` bat
/*&cls
@echo off
rem 在指定快捷方式的目标项后面添加/增加一个参数值
title %#% +%@%%$%%$% %z%
cd /d "%~dp0"
rem 指定快捷方式的路径

for /f "tokens=2*" %%a in ('reg query "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" /v Desktop') do set Desktop=%%b
rem 通过注册表查找出桌面所在的位置，并将位置路径定义为变量Desktop

set "lnkfile=%Desktop%\Google Chrome.lnk"
rem 指定添加的参数
set "arguments= --enable-features=SharedArrayBuffer"
cscript -nologo -e:jscript "%~f0" "%lnkfile%" "%arguments%"
echo;%#% +%@%%$%%$% %z%
pause
exit
*/
var ws=new ActiveXObject('WScript.Shell');
var lnk=ws.CreateShortcut(WSH.Arguments(0));
lnk.Arguments=WSH.Arguments(1);
lnk.Save();
```

将文件保存为 `chrome.bat`，双击运行即可。

或者直接下载 [chrome.bat](https://jessibuca.com/public/zip/set-chrome.zip)

成功后通过桌面的快捷方式打开谷歌，将默认开启SharedArrayBuffer特性。

如果因系统权限问题开启失败，你可以通过以下途径手动开启：

1. 在桌面上右键点击谷歌浏览器图标
2. 选择属性，选择“快捷方式”

看到目标栏的内容类似于 `"C:\Program Files\Google\Chrome\Application\chrome.exe"`，在后面添加 `--enable-features=SharedArrayBuffer`，然后点击确定。

`"C:\Program Files\Google\Chrome\Application\chrome.exe" --enable-features=SharedArrayBuffer`

保存后重启浏览器，通过该快捷方式启动浏览器，将自动启用多线程特性


### 检查是否成功

打开浏览器的F12开发者工具，选择控制台，输入以下代码：

```js
console.log(typeof SharedArrayBuffer)
```

如果输出结果为 `function`，则表示配置成功，可以使用多线程解码特性。如图：

<img src="/img/shared-array-buffer-support.png">


如果是 `undefined`，则表示配置失败，无法使用多线程解码特性。如图：

<img src="/img/shared-array-buffer-not-support.png">

## 关于直播流，回放流，特殊回放流/点播flv，点播文件四者的差异性

- 业务上面直播流是实时的，
- 回放流一般是接的摄像头存储的文件，通过流的方式拉取过来的，
- 特殊回放流/点播flv 一般是存储的flv文件，通过流的方式拉取过来的，播放器端没法控制拉流的速率，只能被动的接收数据。
- 点播文件是静态的，目前播放器支持的点播文件格式是hls和mp4。

### 直播流

特点：

- 播放器被动接收流数据
- 实时性强，延迟低，会触发丢帧逻辑
- 一般是直接对接摄像头或者obs推流过去的
- 不支持倍率播放
- 不支持seek逻辑
- 支持暂停和恢复播放（通过断开流实现）

### 回放流

特点：

- 播放器被动接收流数据
- 不会触发丢帧逻辑，数据会缓存在播放内
- 一般是接的摄像头存储的文件，通过流的方式拉取过来的
- 支持倍率播放（依赖服务器端配合实现）
- 支持seek逻辑（依赖服务器端配合实现）
- 支持暂停和恢复播放（继续接收数据，缓存在播放器内部）


### 特殊回放流/点播flv

特点：
- 播放器被动接收流数据（无法控制倍率）
- 不会触发丢帧逻辑，数据会缓存在播放内
- 不支持seek逻辑
- 支持倍率播放（本地实现）
- 支持监听播放结束（本地实现）
- 支持暂停和恢复播放（本地实现）

> 播放文件不易过大，因为数据都是缓存在播放器内部的，文件过大会占用大量内存。

### 点播文件

特点：
- 播放器主动拉取文件数据
- 不会触发丢帧逻辑，数据会缓存在播放内
- 一般是静态的文件，支持hls和mp4格式
- 支持倍率播放（播放器内部自己实现）
- 支持seek逻辑（播放器内部自己实现）
- 支持暂停和恢复播放（播放器内部自己实现）


## 群

<img src="/qrcode.jpeg">

## qq频道
<img src="/qq-qrcode.jpg">
