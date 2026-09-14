# Document (常见问题)
<Rice/>


## PageSpy配置（官方）

### https配置

```html
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" />
    <meta name="theme-color" content="#000000" />
    <link rel="manifest" href="%PUBLIC_URL%/manifest.json" />
    <link rel="shortcut icon" href="%PUBLIC_URL%/favicon.ico" />
    <title>React App</title>

    <!-- 使用第一步：引入 SDK 文件 -->
      <script crossorigin="anonymous" src="https://spy.jessibuca.com/page-spy/index.min.js"></script>

    <!-- 使用第二步：实例化 PageSpy -->
    <script>
      // 实例化的参数是可选的（通过 ESM 方式使用时，必须填写 api 和 clientOrigin）
      window.$pageSpy = new PageSpy({
          api: "spy.jessibuca.com",
          clientOrigin: "https://spy.jessibuca.com",
          enableSSL: true,
      });
      // PageSpy 应该已经在屏幕右下角等你了，赶紧试试吧！👉
    </script>
  </head>

  <body>
    <noscript> You need to enable JavaScript to run this app. </noscript>
    <div id="root"></div>
  </body>
</html>

```

### http配置
```html
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" />
    <meta name="theme-color" content="#000000" />
    <link rel="manifest" href="%PUBLIC_URL%/manifest.json" />
    <link rel="shortcut icon" href="%PUBLIC_URL%/favicon.ico" />
    <title>React App</title>

    <!-- 使用第一步：引入 SDK 文件 -->
      <script crossorigin="anonymous" src="http://spy.jessibuca.com:6752/page-spy/index.min.js"></script>
      <!-- 使用第二步：实例化 PageSpy -->
    <script>
      // 实例化的参数是可选的（通过 ESM 方式使用时，必须填写 api 和 clientOrigin）
      window.$pageSpy = new PageSpy({
          api: "spy.jessibuca.com:6752",
          clientOrigin: "http://spy.jessibuca.com:6752",
          enableSSL: false,
      });
      // PageSpy 应该已经在屏幕右下角等你了，赶紧试试吧！👉
    </script>
  </head>

  <body>
    <noscript> You need to enable JavaScript to run this app. </noscript>
    <div id="root"></div>
  </body>
</html>

```


