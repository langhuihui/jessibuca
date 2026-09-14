# 哈哈
<Rice/>

# 开源版 升级到 Pro 版本

## 替换文件

> 只需要无脑的将`pro/js`下面的所有js和wasm文件替换到原本的开源版`js`目录下即可。

[pro体验版zip包](https://jessibuca.com/pro.zip)

需要将原本开源版的

- jessibuca.js
- decoder.js
- decoder.wasm

替换成 `pro/js`下面的所有js和wasm文件


## js以及decoder的配置

将原本html 里面引用的 `jessibuca.js` 替换成 `jessibuca-pro-demo.js` 或者 `jessibuca-pro-multi-demo.js`

> 如果是delay-time 版本，则需要替换成 `jessibuca-pro-delay-demo.js`

```html
<html>
    <head>
<!--        <script src="./js/jessibuca.js"></script>-->
        <script src="./js/jessibuca-pro-demo.js"></script>
    </head>
</html>

```

或者

```html
<html>
    <head>
<!--        <script src="./js/jessibuca.js"></script>-->
        <script src="./js/jessibuca-pro-delay-time.js"></script>
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
// 替换成
const jessibucaPro = new JessibucaPro({
    // ...
    decoder:'your-path/decoder-pro.js' // 配置decoder-pro.js pro 专用解码器
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

> jessibuca 开源版如果是v3.2.x 版本以上就不需要做任何修改，因为v3.2.x 版本以上的开源版已经是异步方法了。

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
