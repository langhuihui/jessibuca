
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


## MP4放进去以后一直加载不成功怎么办

确认moov box 是否在mdat box之前

> 其实就是按照fmp4格式封装就行了。


使用FFMpeg 确定 moov位置:

```
ffprobe 视频.mp4 -v trace 2>&1 | grep 'mdat\|moov'
```

输出如下，type:'moov' 在 type:'mdat'之前就是正常的，否则就是错误。

```
[mov,mp4,m4a,3gp,3g2,mj2 @ 0x7fd30d004400] type:'moov' parent:'root' sz: 7993 36 815638
[mov,mp4,m4a,3gp,3g2,mj2 @ 0x7fd30d004400] type:'mvhd' parent:'moov' sz: 108 8 7985
[mov,mp4,m4a,3gp,3g2,mj2 @ 0x7fd30d004400] type:'trak' parent:'moov' sz: 5480 116 7985
[mov,mp4,m4a,3gp,3g2,mj2 @ 0x7fd30d004400] type:'trak' parent:'moov' sz: 2268 5596 7985
[mov,mp4,m4a,3gp,3g2,mj2 @ 0x7fd30d004400] type:'udta' parent:'moov' sz: 129 7864 7985
[mov,mp4,m4a,3gp,3g2,mj2 @ 0x7fd30d004400] type:'mdat' parent:'root' sz: 807609 8037 815638
```

解决方法

mp4 将moov box前置（不转码方法）

```
ffmpeg -i input.mp4 \
-vcodec copy \
-acodec copy \
-movflags faststart \
-y video.mp4
```


大体确定MOOV的范围 调整coreProbePart参数大小(0-1 = 0%-100%)

见图
<img src="/public/img/vod-coreProbePart.png">
