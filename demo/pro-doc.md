
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



## 关于如何配置使用simd多线程解码



### localhost

不限制 `http`或者 `https`协议

需要在线申请一个origin trial token
https://developer.chrome.com/origintrials/?utm_source=devtools#/view_trial/303992974847508481

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
## 群

<img src="/public/qrcode.jpeg">

## 群
<img src="/public/qrcode-qw.jpeg">

## qq频道
<img src="/public/qq-qrcode.jpg">
