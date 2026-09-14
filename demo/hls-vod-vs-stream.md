# 哈哈
<Rice/>

## hls 直播流和点播流的区别


### 点播文件
M3U8 是一种基于文本的播放列表文件格式，通常用于 HTTP Live Streaming (HLS) 流媒体传输。以下是一个标准的 M3U8 文件示例：

```plaintext
#EXTM3U
#EXT-X-VERSION:3
#EXT-X-TARGETDURATION:10
#EXT-X-MEDIA-SEQUENCE:0

#EXTINF:9.009,
http://example.com/segment0.ts
#EXTINF:9.009,
http://example.com/segment1.ts
#EXTINF:9.009,
http://example.com/segment2.ts
#EXTINF:9.009,
http://example.com/segment3.ts
#EXTINF:9.009,
http://example.com/segment4.ts

#EXT-X-ENDLIST
```

#### 解释：
1. **#EXTM3U**：文件头，表示这是一个 M3U8 文件。
2. **#EXT-X-VERSION:3**：指定 HLS 协议的版本，这里是版本 3。
3. **#EXT-X-TARGETDURATION:10**：指定每个媒体段的最大持续时间（秒），这里是 10 秒。
4. **#EXT-X-MEDIA-SEQUENCE:0**：指定第一个媒体段的序列号，这里是 0。
5. **#EXTINF:9.009,**：指定下一个媒体段的持续时间（秒），这里是 9.009 秒。
6. **http://example.com/segment0.ts**：媒体段的 URL。
7. **#EXT-X-ENDLIST**：表示播放列表的结束。

#### 注意：
- 每个媒体段（`.ts` 文件）的 URL 可以是绝对路径或相对路径。
- 如果流是直播流，可能不会有 `#EXT-X-ENDLIST` 标签，因为播放列表会不断更新。

这个示例是一个简单的点播流媒体播放列表，包含了几个媒体段和一个结束标记。


### 直播流

对于 **直播流** 的 M3U8 文件，格式会有所不同，因为直播流是动态生成的，播放列表会不断更新。直播流的 M3U8 文件通常不会包含 `#EXT-X-ENDLIST` 标签，而是通过不断追加新的媒体段来实现实时播放。

以下是一个典型的 **直播流 M3U8 文件** 示例：

```plaintext
#EXTM3U
#EXT-X-VERSION:3
#EXT-X-TARGETDURATION:10
#EXT-X-MEDIA-SEQUENCE:123

#EXTINF:9.009,
http://example.com/live/segment123.ts
#EXTINF:9.009,
http://example.com/live/segment124.ts
#EXTINF:9.009,
http://example.com/live/segment125.ts
#EXTINF:9.009,
http://example.com/live/segment126.ts
```

#### 直播流的特点：
1. **没有 `#EXT-X-ENDLIST` 标签**：
    - 直播流是动态生成的，播放列表会不断更新，因此不会标记结束。

2. **`#EXT-X-MEDIA-SEQUENCE`**：
    - 表示当前播放列表中第一个媒体段的序列号。每次更新播放列表时，序列号会递增。
    - 例如，上面的示例中，序列号是 `123`，表示当前播放列表从第 123 个媒体段开始。

3. **动态更新**：
    - 播放列表会定期更新，旧的媒体段会被移除，新的媒体段会被追加到播放列表中。
    - 客户端（如播放器）会定期重新加载 M3U8 文件以获取最新的媒体段。

4. **`#EXT-X-TARGETDURATION`**：
    - 表示每个媒体段的最大持续时间（以秒为单位）。客户端会根据这个值来调整缓冲策略。

5. **媒体段 URL**：
    - 媒体段的 URL 通常是动态生成的，例如基于时间戳或序列号。

---

#### 直播流的工作流程：
1. **客户端请求 M3U8 文件**：
    - 客户端（如播放器）首先请求 M3U8 文件，获取当前的播放列表。

2. **客户端下载并播放媒体段**：
    - 客户端根据 M3U8 文件中的 URL 下载媒体段（`.ts` 文件）并播放。

3. **客户端定期更新 M3U8 文件**：
    - 客户端会定期重新请求 M3U8 文件，检查是否有新的媒体段。
    - 如果发现新的媒体段，客户端会继续下载并播放。

4. **服务器动态更新 M3U8 文件**：
    - 服务器会不断生成新的媒体段，并更新 M3U8 文件，移除旧的媒体段，追加新的媒体段。

---

#### 示例：更新后的 M3U8 文件
假设直播流继续生成新的媒体段，更新后的 M3U8 文件可能如下：

```plaintext
#EXTM3U
#EXT-X-VERSION:3
#EXT-X-TARGETDURATION:10
#EXT-X-MEDIA-SEQUENCE:124

#EXTINF:9.009,
http://example.com/live/segment124.ts
#EXTINF:9.009,
http://example.com/live/segment125.ts
#EXTINF:9.009,
http://example.com/live/segment126.ts
#EXTINF:9.009,
http://example.com/live/segment127.ts
```

- 可以看到，`#EXT-X-MEDIA-SEQUENCE` 更新为 `124`，表示播放列表从第 124 个媒体段开始。
- 旧的媒体段（`segment123.ts`）被移除，新的媒体段（`segment127.ts`）被追加。

---

### 总结：
- 直播流的 M3U8 文件是动态更新的，没有结束标记。
- 客户端需要定期重新加载 M3U8 文件以获取最新的媒体段。
- 服务器会不断生成新的媒体段并更新播放列表，移除旧的媒体段。

这种机制使得 HLS 直播流能够实现低延迟的实时视频传输。
