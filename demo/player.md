---
sidebar: false
---

<DemoPlayer/>

- 支持3种格式：
1. ws-raw 即ws://localhost/live/test (该协议只能对接[monibuca服务器](https://monibuca.com))
2. ws-flv 即ws://localhost/live/test.flv
3. http-flv 即http://localhost/live/test.flv
- 注意http协议会有跨域问题，需要设置cors头
- 协议同时支持https、wss
- 同时支持H264和H265编码格式
- 支持webcodecs和MSE硬解码(H264格式)

