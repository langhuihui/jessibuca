
# 哈哈
<Rice/>


## 如何播放g711a/u的文件

1. 下载 Audacity 客户端
2. 然后File(文件) -> Import(导入)-> Raw Data(二进制)

然后通过配置选择

<img src="/img/g711-config-1.png">


<img src="/img/g711-config-2.png">

> g711a/u 是标准的8k采样率的

> 编码格式那边选择a-Law 或者 u-Law 就行了

## 关于麦克风权限问题

### NotAllowedError: Permission denied by system

1. 检查当前url页面是否localhost或者https协议，然后看下这个页面是否赋予了麦克风权限（tab页面层面）
2. 如果是localhost或者https协议，那么检查下浏览器是否有麦克风权限（系统层面）
3. 如果是https协议，那么检查下是否有证书问题
4. 如果是localhost，那么检查下是否有其他页面占用了麦克风权限


#### 更改网站的摄像头和麦克风使用权限

可以看： https://support.google.com/chrome/answer/2693767?hl=zh-Hans&co=GENIE.Platform%3DDesktop#zippy=


#### 如何设置浏览器的麦克风

可以看：http://faq-cs.alo7.com/%E5%A6%82%E4%BD%95%E8%AE%BE%E7%BD%AE%E6%B5%8F%E8%A7%88%E5%99%A8%E7%9A%84%E9%BA%A6%E5%85%8B%E9%A3%8E.html


#### 选择默认麦克风

可以看： https://support.google.com/chrome/answer/2693767?hl=zh-Hans&co=GENIE.Platform%3DDesktop#zippy=



## 播放音频数据

> 配套websocket测试服务器

首先下载测试服务器

下载地址：[jessibuca-pro-talk-websocket-test-server](https://github.com/bosscheng/jessibuca-pro-talk-websocket-test-server)

然后安装依赖
```
npm install
```

然后运行起来
```
node server.js
```

然后配置参数，允许接收websocket信息，并播放。

支持 pcm/g711a/g711u 格式。

> 注意封装格式需要选“裸数据”

然后输入ws地址：`ws://localhost:8080/`，然后点击连接按钮。就可以测试了


