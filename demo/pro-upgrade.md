# 哈哈
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

# 开源版 升级到 Pro 版本

## 替换文件 （软解码+硬解码）

> 只需要无脑的将`pro/js`下面的所有js和wasm文件替换到原本的开源版`js`目录下即可。

需要将原本开源版的

- jessibuca.js
- decoder.js
- decoder.wasm

替换成 `pro/js`下面的所有js和wasm文件

文件介绍

- jessibuca-pro-demo.js 或 jessibuca-pro-multi-demo.js   // 主文件(需要通过script标签引入)

- decoder-pro.js  //  worker解码器(这个库无需引入,播放器内部会自己引入)
- decoder-pro.wasm // worker解码器胶水文件(这个库无需引入,播放器内部会自己引入)

- decoder-pro-simd.js // worker-simd解码器(这个库无需引入,播放器内部会自己引入)
- decoder-pro-simd.wasm // worker-simd解码器胶水文件(这个库无需引入,播放器内部会自己引入)

- decoder-pro-hard.js // worker硬解码解封装+audio解码(这个库无需引入,播放器内部会自己引入)
- decoder-pro-hard-not-wasm.hs // worker硬解码解封装(这个库无需引入,播放器内部会自己引入)
- decoder-pro-audio.js // worker音频解码器(这个库无需引入,播放器内部会自己引入)
- decoder-pro-audio.wasm // worker音频解码器胶水文件(这个库无需引入,播放器内部会自己引入)

- decoder-pro-mt.js // worker多线程解码器(这个库无需引入,播放器内部会自己引入)
- decoder-pro-mt.wasm // worker多线程解码器胶水文件(这个库无需引入,播放器内部会自己引入)
- decoder-pro-mt-worker.js // worker多线程解码器(这个库无需引入,播放器内部会自己引入)

- decoder-pro-simd-mt.jd // worker-simd多线程解码器(这个库无需引入,播放器内部会自己引入)
- decoder-pro-simd-mt.wasm // worker-simd多线程解码器胶水文件(这个库无需引入,播放器内部会自己引入)
- decoder-pro-simd-mt-worker.js // worker-simd多线程解码器(这个库无需引入,播放器内部会自己引入)

> 如果有多屏需求，可以将`jessibuca-pro.js`替换成`jessibuca-pro-multi.js`。

> `jessibuca-pro.js` 与`jessibuca-pro-multi.js`只需要引用一个即可（不需要两个同时引用）。

> `decoder-pro-simd.js`是simd解码器（适用于高分辨率解码）（） （）


## 替换文件（只使用硬解码）

需要将原本开源版的

- jessibuca.js
- decoder.js
- decoder.wasm

替换成

- jessibuca-pro-demo.js 或 jessibuca-pro-multi-demo.js   // 主文件(需要通过script标签引入)
- decoder-pro-hard-not-wasm.js // worker硬解码解封装(这个库无需引入,播放器内部会自己引入)


## js以及decoder的配置

将原本html 里面引用的 `jessibuca.js` 替换成 `jessibuca-pro-demo.js` 或者 `jessibuca-pro-multi-demo.js`

```html
<html>
    <head>
<!--        <script src="./js/jessibuca.js"></script>-->
        <script src="./js/jessibuca-pro-demo.js"></script>
    </head>
</html>

```

将配置参数 解码器`decoder` 替换成 pro解码器

原本的开源版的
```js
// 原本的
const jessibuca = new Jessibuca({
    // ...
    decoder:'your-path/decoder.js'
})
```


```js
const jessibucaPro = new JessibucaPro({
    // ...
    decoder:'your-path/decoder-pro.js'
})
```


## 替换方法

将原本 `new Jessibuca()` 的地方替换成 `new JessibucaPro()`

```js
// 原本的
const jessibuca = new Jessibuca({
    // ...
})
```

```js
// 替换成
const jessibucaPro = new JessibucaPro({
    // ...
})
```
```js
// 替换成 多屏
const jessibucaProMulti = new JessibucaProMulti({
    // ...
})
```


需要将原本开源版的 `destroy()` 从同步方法，修改为异步方法

```js
// 原本的
jessibuca.destroy()
```

```js

// 替换成
await jessibucaPro.destroy()
// 或者
jessibucaPro.destroy().then(() => {
    // ...
})
```

将原本的判断 `hasLoaded()` 和监听`load`事件

```js
// 原本的
if (jessibuca.hasLoaded()) {
    // ...
    jessibuca.play('url');
} else {
    jessibuca.on(Jessibuca.EVENTS.load, () => {
        // ...
        jessibuca.play('url');
    })
}
```

```js
// 替换成
jessibuca.play('url');
```



### vue 里面

在组件里面的`beforeDestroy`里面调用`destroy`方法

```js

// 原本的
beforeDestroy() {
    this.jessibuca.destroy()
}

// 替换成
async beforeDestroy() {
    await this.jessibucaPro.destroy()
}
```


### react 里面

在组件里面的`componentWillUnmount`里面调用`destroy`方法

```js

// 原来的
componentWillUnmount() {
    this.jessibuca.destroy()
}

// 替换成
async componentWillUnmount() {
    await this.jessibucaPro.destroy()
}

```
