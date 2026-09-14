
# 哈哈
<Rice/>

重要的事情说三遍：

1. 看清楚是`http/ws` 还是`https/wss`的测试地址，如果是`http/ws`的地址用 http://jessibuca.monibuca.com/ 域名测试 如果是`https/wss`的地址用 https://jessibuca.com/ 测试。
2. 看清楚是`http/ws` 还是`https/wss`的测试地址，如果是`http/ws`的地址用 http://jessibuca.monibuca.com/ 域名测试 如果是`https/wss`的地址用 https://jessibuca.com/ 测试。
3. 看清楚是`http/ws` 还是`https/wss`的测试地址，如果是`http/ws`的地址用 http://jessibuca.monibuca.com/ 域名测试 如果是`https/wss`的地址用 https://jessibuca.com/ 测试。


> 测试地址都是公网地址，可能会不稳定，或者失效，或者限制域名访问，请自行测试。

> 测试地址都是公网地址，可能会不稳定，或者失效，或者限制域名访问，请自行测试。

> 测试地址都是公网地址，可能会不稳定，或者失效，或者限制域名访问，请自行测试。


## 离线版下载

开源版的离线下载包：https://jessibuca.com/dist.zip

Pro版本的离线下载包：https://jessibuca.com/pro.zip

需要本地挂个Nginx打开，或者用node启动一个静态服务器，或者用本地的http-server打开。

## 直播测试地址

### flv

#### H264

##### node-media 854*480

https://live.nodemedia.cn:8443/live/b480_264.flv

https://live.nodemedia.cn:8443/live/bbb_264.flv

> 需要通过本地local域名访问，公网会校验域名

##### vigokit 当前时间 (1040 * 634)

wss://live.vigokit.com:8443/live/time.flv

> 需要通过本地local域名访问，公网会校验域名

##### vigokit 1280 * 720
wss://live.vigokit.com:8443/live/tcl_264.flv

https://live.nodemedia.cn:8443/live/tcl_264.flv

> 需要通过本地local域名访问，公网会校验域名

##### volcfcdnrd 有 sei 数据（1920 * 1080）

http://pull-demo.volcfcdnrd.com/live/st-4536523_yzmhde.flv

##### node-media 4k 3840 * 2160
https://live.nodemedia.cn:8443/live/sony_4k_264.flv

> 需要通过本地local域名访问，公网会校验域名

##### 西瓜视频


https://pull-demo.volcfcdnrd.com/live/st-4536523_yzmhde.flv

http://pull-demo.volcfcdnrd.com/live/st-4536523_yzmhde.flv

#### H265

##### node-media 854*480

https://live.nodemedia.cn:8443/live/b480_265.flv

> 需要通过本地local域名访问，公网会校验域名

##### vigokit 1280 * 720
wss://live.vigokit.com:8443/live/tcl_265.flv

https://live.nodemedia.cn:8443/live/tcl_265.flv

> 需要通过本地local域名访问，公网会校验域名

##### 西瓜视频（1920 * 1080）

###### http
http://pull-demo.volcfcdnrd.com/live/st-4536521_yzmuhevcd.flv

###### https
https://pull-demo.volcfcdnrd.com/live/st-4536521_yzmuhevcd.flv

> 需要通过本地local域名访问，公网会校验域名


### hls

#### H264



##### 西瓜视频（1280 * 720） m3u8 + ts

https://sf1-cdn-tos.huoshanstatic.com/obj/media-fe/xgplayer_doc_video/hls/xgplayer-demo.m3u8

https://pull-demo.volcfcdnrd.com/live/st-4536523_yzmhde.m3u8

http://pull-demo.volcfcdnrd.com/live/st-4536523_yzmhde.m3u8


##### 七牛云 1080p

https://live-mikudemo.cloudvdn.com/mikudemo/timestamps.m3u8

#### H265


## 点播测试地址

点播文件的测试地址是：

#### https:

1. https://jessibuca.com/player-pro.html
2. https://jessibuca.com/pro-demo.html#demo

#### http:

1. http://jessibuca.monibuca.com/player-pro.html
2. http://jessibuca.monibuca.com/pro-demo.html#demo


### hls

#### h264
https://vod.lyhuicheng.com/20230719/oc5MCoON/index.m3u8

##### AES-128 加密
https://playertest.longtailvideo.com/adaptive/aes-with-tracks/master.m3u8

##### （1280,720）m3u8 + ts
https://test-streams.mux.dev/x36xhzz/x36xhzz.m3u8

https://test-streams.mux.dev/x36xhzz/url_2/193039199_mp4_h264_aac_ld_7.m3u8

https://test-streams.mux.dev/x36xhzz/url_4/193039199_mp4_h264_aac_7.m3u8

https://test-streams.mux.dev/x36xhzz/url_6/193039199_mp4_h264_aac_hq_7.m3u8

https://test-streams.mux.dev/x36xhzz/url_8/193039199_mp4_h264_aac_fhd_7.m3u8 // 1080p

#### h265

##### 西瓜视频（854 * 480） m3u8 + ts
https://voddemo-play.volcvod.com/8d1417d814b24fee8b8b0c2747f303e5/main.m3u8?a=0&auth_key=1765632280-d3569ddc77d445ab8647746220f67766-0-0222f8289976f07f8c91c230753ce911&br=384&bt=384&cd=0%7C0%7C0&ch=0&cr=1&cs=&cv=1&dr=0&ds=2&er=0&l=202212142122085BF19BF20712C413A9A2&lr=&mime_type=video_mp4&net=0&pl=0&qs=0&rc=amg6c2o0aTg6ZTQzNGRnM0ApZzlnZzNlPGRlNzZmZTg5NWdfZy9gMHFrYTBgLS1kYy9zczQxNi4wYmItLTVgYTFjXjU6Yw%3D%3D&vl=&vr=

##### (768* 320) m3u8 + ts

https://cdn-static.m-m10010.com/demo/hls/output.m3u8


### flv

#### h264
360p  西瓜视频测试地址

https://sf1-cdn-tos.huoshanstatic.com/obj/media-fe/xgplayer_doc_video/flv/xgplayer-demo-360p.flv

360p  flvplayer.js 测试地址

https://flvplayer.js.org/assets/video/weathering-with-you.flv

#### h265
360p 网友提供的测试地址

https://cdn-static.m-m10010.com/demo/h265_aac-1.flv

### mp4

#### h265

##### 4k(25p)

###### Main Profile (SDR)
https://lf-tk-sg.ibytedtos.com/obj/tcs-client-sg/resources/hevc_4k25P_main_1.mp4

https://lf-tk-sg.ibytedtos.com/obj/tcs-client-sg/resources/hevc_4k25P_main_2.mp4

##### 720p
https://ks3-cn-beijing.ksyun.com/ksplayer/h265/mp4_resource/jinjie_265.mp4

##### 360p
https://www.zzsin.com/h265webjs2/example/hevc_test_moov_set_head_16s.mp4

#### h264

480 * 360 -------- artplayer.js使用的测试文件

https://artplayer.org/assets/sample/video.mp4

920 * 400  ------- videojs使用的测试文件

https://vjs.zencdn.net/v/oceans.mp4


640 * 360 -------
https://d2zihajmogu5jn.cloudfront.net/elephantsdream/ed_hd.mp4


960 * 540 -------
https://cdn.zwplayer.cn/media/VMAP9lxJvRpgn5sP3lV6rQ9qkzQmh5psggso3185.mp4

测试地址

https://raw.githubusercontent.com/nickdesaulniers/netfix/gh-pages/demo/frag_bunny.mp4

西瓜视频

http://lf3-static.bytednsdoc.com/obj/eden-cn/nupenuvpxnuvo/xgplayer_doc/xgplayer-demo-720p.mp4



## vr 点播测试地址

### http
http://jessibuca.monibuca.com/video/vr.mp4

### https
https://jessibuca.com/video/vr.mp4
