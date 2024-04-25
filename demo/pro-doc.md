
# 简介
jessibuca pro 是在开源版本的基础上额外支持的深入业务解决方案的。解决了一些痛点，比如H265的硬解码，SIMD软解码加速。
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


以node 为例
```js
app.use((req, res, next) => {
    res.setHeader('Cross-Origin-Opener-Policy', 'same-origin')
    res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp')
    next()
})
```

nginx 配置

```nginx
add_header Cross-Origin-Opener-Policy same-origin;
add_header Cross-Origin-Embedder-Policy require-corp;
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


## 群

<img src="/public/qrcode.jpeg">

## 群
<img src="/public/qrcode-qw.jpeg">

## qq频道
<img src="/public/qq-qrcode.jpg">
