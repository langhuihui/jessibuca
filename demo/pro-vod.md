
# 哈哈
<Rice/>


## MP4放进去以后一直加载不成功怎么办（有报错信息）

###  播放器会报：“cannot find moov or mdat box” 错误

检查下文件格式是否正确：目前遇到了海康的NVR录制的MP4文件，使用ffmpeg查看了一下，其实不是MP4格式的文件，文件头是MP4的，但是实际内容是PS格式的。用Mp4Box.js解析的时候会报错。



### 报错：“Could not find tag for codec pcm_alaw in stream #1, codec not currently supported in container”
如果出现了报错：

```
[mp4 @ 0x7fb737f05600] Could not find tag for codec pcm_alaw in stream #1, codec not currently supported in container
[out#0/mp4 @ 0x7fb737f04440] Could not write header (incorrect codec parameters ?): Invalid argument
```

这个错误表明在尝试将视频重新封装时遇到了音频编解码器兼容性的问题。具体来说，源文件中的音频编码格式是 PCM A-law (pcm_alaw)，而 MP4 容器格式默认不支持这种未压缩的音频编码。
让我们修改一下命令来解决这个问题。我建议将音频转换为 MP4 容器广泛支持的 AAC 编码格式：

```
./ffmpeg -i input.mp4 -vcodec copy -acodec aac -movflags faststart -y output.mp4

```


## MP4放进去以后一直加载不成功怎么办（无报错信息）

检查下Network的是否正在大量下载数据，如果是这样，那就是可能服务器不支持Range请求，导致需要等到完整的文件下载完毕才能播放。


检查下network 有没有报：`ERR_CONTENT_LENGTH_MISMATCH` 错误

错误的返回格式：

<img src="/img/range-error.png">

解决方法：

1. 服务器支持Range请求即可。

正确的返回格式：

<img src="/img/range-success.png">




## MP4 播放报错：`cannot parse segments`

一般这种情况，就是缺少web播放的核心mp4box 盒子。

主要是缺少：

1. stsd
2. stts
2. stsc
3. stsz
4. stco
5. stss


| box 名称     | 全称                         | 解决的问题               | 里面存的核心信息                             | 缺失后的真实影响（Web 端）                    | 重要级别  |
|------------| -------------------------- | ------------------- | ------------------------------------ |------------------------------------| ----- |
| **stsd**   | Sample Description Box     | sample 用什么编码        | codec、profile、extradata（avcC / hvcC） | **完全无法解码**                         | ⭐⭐⭐⭐⭐ |
| **stts**   | Decoding Time to Sample    | 一帧播多久（DTS）          | sample_delta 时间表                     | **时间轴不存在，几乎不播**                    | ⭐⭐⭐⭐⭐ |
| **stsc**   | Sample to Chunk            | 一个 chunk 有几个 sample | chunk → sample 数                     | **结构级损坏，无法拆帧**                     | ⭐⭐⭐⭐⭐ |
| **stsz**   | Sample Size                | 每个 sample 多大        | sample_size / size[]                 | **帧边界丢失，必挂**                       | ⭐⭐⭐⭐⭐ |
| **stco**   | Chunk Offset               | chunk 在文件哪          | chunk 偏移字节数                          | **读不到数据，必挂**                       | ⭐⭐⭐⭐⭐ |
| **stss**   | Sync Sample                | 哪些是关键帧              | I / IDR sample 编号                    | 点播业务不能播，没法seek，video标签需要完整下载才能seek | ⭐⭐☆☆☆ |


#### 如何自查缺少哪些盒子

可以通过`mp4box.js` 来解析mp4文件，查看报错信息。

访问地址：https://gpac.github.io/mp4box.js/test/filereader.html

正常能播的mp4文件：
<img src="/img/mp4box-1.png">

缺少关键盒子的mp4文件：

> 缺少stss 盒子

<img src="/img/mp4box-2.png">
