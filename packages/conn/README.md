# 万能连接模块

提供对Http、Websocket、WebTransport、WebRTC连接管理的统一封装。包含

- 自动重连机制
- 流量统计功能
- 提供按需（长度）读取功能（方便协议解析）

## 安装

```
npm i conn
```
## 使用

```ts
import { Connection } from 'conn'

```