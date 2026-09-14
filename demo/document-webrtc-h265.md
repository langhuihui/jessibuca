# WebRTC H.265 (HEVC) 浏览器支持情况
<Rice/>

# WebRTC H.265 (HEVC) 浏览器支持情况

> 调研时间：2025年5月

## 总结表

| 浏览器 | 最低版本 | 支持平台 | 状态 |
|--------|---------|----------|------|
| Chrome | 136+ | Windows, macOS, Android, Linux(VAAPI) | ✅ 原生支持，需硬件编解码 |
| Safari | 18.0+ | macOS, iOS, iPadOS | ✅ 原生支持 |
| Edge | — | — | ❌ 未启用 H.265 发送 |
| Firefox | — | — | ❌ 无计划支持 |

## Chrome / Chromium 系列

**Chrome 136+**（2025年4月底稳定版）原生支持 WebRTC H.265，无需任何 flag 或特殊配置。

### 平台要求

- **Windows**：开箱即用（需要硬件支持 HEVC 编解码，现代显卡基本都支持）
- **macOS**：开箱即用（Apple 芯片和较新 Intel Mac 均有硬件支持）
- **Linux**：需要 VAAPI 支持（即需要硬件解码驱动配合）
- **Android**：Chrome 136+ 同样支持，依赖设备硬件编解码能力

### 旧版本手动启用

Chrome 135 及以下版本可以通过手动开启 flag 来启用：

```bash
# macOS
open -a "Google Chrome" --args --enable-features=WebRtcAllowH265Send,WebRtcAllowH265Receive

# Windows
start chrome --enable-features=WebRtcAllowH265Send,WebRtcAllowH265Receive
```

### 重要限制

H.265 在 Chrome WebRTC 中**仅支持硬件编解码**，不提供软件编解码回退。如果设备没有 HEVC 硬件编解码器，SDP 中不会出现 H.265 codec。

## Safari

**Safari 18.0+**（2024年9月随 macOS Sequoia / iOS 18 发布）默认启用 WebRTC H.265 支持。

- **macOS**：Safari 18.0+ 原生支持
- **iOS / iPadOS**：Safari 18.0+ 原生支持

Safari 17.x 时期可以在"设置 → Feature Flags"中手动开启 "WebRTC H265 codec" 来体验。

## Microsoft Edge

**Edge 目前不支持 WebRTC H.265 发送（编码）。**

虽然 Edge 基于 Chromium，但微软在 Edge 136 中并未启用此功能。Edge 可能支持 H.265 接收（解码），但发送端被禁用，造成与 Chrome 的互操作性差异。

参考：https://learn.microsoft.com/en-us/answers/questions/5875799/edge-webrtc-h265-support

## Firefox

**不支持，且没有计划支持。**

Mozilla 的立场是不会在 WebRTC 中加入 H.265，主要原因是 HEVC 的专利授权费用复杂且昂贵，Mozilla 更倾向于推动免版税的 AV1。

## 其他浏览器

- **Thorium**（第三方 Chromium 分支）：原生支持
- **Opera / Brave 等 Chromium 系**：理论上跟随 Chromium 内核版本，136+ 应该支持，但需各自确认是否启用

## 关键注意事项

1. **硬件依赖**：WebRTC H.265 依赖硬件编解码能力，不像 H.264 有 OpenH264 软件编码器兜底。即使浏览器版本够了，老旧设备如果没有 HEVC 硬件支持也无法使用。

2. **兼容性风险**：不同 H.265 实现之间可能不兼容，在 SFU（Selective Forwarding Unit）广播场景中使用 H.265 需谨慎。

3. **Codec 协商建议**：建议设置 codec 优先级回退策略，例如 `h265 → av1 → h264 → vp8`，确保在不支持 H.265 的端也能正常播放。

## 参考链接

- https://vdo.ninja/h265 （浏览器兼容性检测工具）
- https://chris.hiszpanski.name/posts/is-webrtc-hevc-supported/
- https://github.com/bluenviron/mediamtx/discussions/4396
- https://issues.chromium.org/341986313
