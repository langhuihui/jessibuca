## 哈哈
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
## 简介

对于播放webrtc协议，在web端其实就是个sdp的交换，然后通过webrtc的api进行音视频的传输。

目前对于不同的流媒体服务器，`POST`请求获取`sdp`的方式是不一样的.

### SRS

例如 http://r.ossrs.net/rtc/v1/play/live/livestream 这个接口返回的是一个json格式的数据，里面包含了sdp的信息。

那播放器接受的地址就是 webrtc://r.ossrs.net/rtc/v1/play/live/livestream

播放器内部会发起POST请求获取sdp信息，然后进行webrtc的连接。请求地址是：http://r.ossrs.net/rtc/v1/play/live/livestream

> webrtc:// 只是为了告诉播放器内部，这个是webrtc协议播放的，实际上播放器内部会发起POST请求获取sdp信息，然后进行webrtc的连接。

> 最终的地址还是需要依据流媒体服务器的获取sdp的接口来确定。

> 就是把获取sdp的接口地址，放到webrtc:// 后面，播放器内部会发起POST请求获取sdp信息，然后进行webrtc的连接。

### ZLMediaKit

例如 http://127.0.0.1/index/api/webrtc?app=live&stream=test&type=play 这个接口返回的是一个json格式的数据，里面包含了sdp的信息。

那播放器接受的地址就是 webrtc://127.0.0.1/index/api/webrtc?app=live&stream=test&type=play

播放器内部会发起POST请求获取sdp信息，然后进行webrtc的连接。请求地址是：http://127.0.0.1/index/api/webrtc?app=live&stream=test&type=play

> webrtc:// 只是为了告诉播放器内部，这个是webrtc协议播放的，实际上播放器内部会发起POST请求获取sdp信息，然后进行webrtc的连接。

> 最终的地址还是需要依据流媒体服务器的获取sdp的接口来确定。

> 就是把获取sdp的接口地址，放到webrtc:// 后面，播放器内部会发起POST请求获取sdp信息，然后进行webrtc的连接。
