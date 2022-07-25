<template>
    <div class="root">
        <div class="container-shell">
            <div class="container-shell-title">jessibuca Pro demo player <span class="tag-version" v-if="version">({{
                    version
                }})</span></div>
            <div class="option">
                <span>缓冲(秒):</span>
                <input
                    style="width: 50px"
                    type="number"
                    ref="buffer"
                    value="0.2"
                    @change="changeBuffer"
                />
                <input
                    type="checkbox"
                    v-model="isDebug"
                    ref="isDebug"
                    @change="restartPlay"
                /><span>日志</span>
                <input
                    type="checkbox"
                    v-model="useMSE"
                    ref="vod"
                    @change="restartPlay('mse')"
                /><span>MediaSource</span>
                <input
                    type="checkbox"
                    v-model="useWCS"
                    ref="vod"
                    @change="restartPlay('wcs')"
                /><span>webcodecs</span>
                <input
                    type="checkbox"
                    v-model="useSIMD"
                    ref="vod"
                    @change="restartPlay('simd')"
                /><span>SIMD</span>
            </div>
            <div id="container" ref="container"></div>
            <div class="input">
                <div>输入URL：</div>
                <input
                    placeholder="支持 hls/ws-raw/ws-flv/http-flv协议"
                    type="input"
                    autocomplete="on"
                    ref="playUrl"
                    value="https://flv.bdplay.nodemedia.cn/live/bbb_265.flv"
                />
                <template v-if="!playing">
                    <button v-if="playType === '' || playType === 'play'" @click="play">播放</button>
                    <button v-if="playType === '' || playType === 'playback'" @click="playback">播放录像流</button>
                </template>
                <button v-else @click="pause">停止</button>
            </div>
            <div class="input" v-if="loaded" style="line-height: 30px">
                <button @click="destroy">销毁</button>
                <button v-if="quieting" @click="cancelMute">取消静音</button>
                <template v-else>
                    <button @click="mute">静音</button>
                    音量
                    <select v-model="volume" @change="volumeChange">
                        <option value="1">100</option>
                        <option value="0.75">75</option>
                        <option value="0.5">50</option>
                        <option value="0.25">25</option>
                    </select>
                </template>
                <span>旋转</span>
                <select v-model="rotate" @change="rotateChange">
                    <option value="0">0</option>
                    <option value="90">90</option>
                    <option value="270">270</option>
                </select>

                <button @click="fullscreen">全屏</button>
                <button @click="screenShot">截图</button>
                <button @click="screenshotWatermark1">截图(水印文字)</button>
                <button @click="screenshotWatermark2">截图(水印图片)</button>
                <div style="line-height: 30px">
                    <input
                        type="checkbox"
                        ref="operateBtns"
                        v-model="showOperateBtns"
                        @change="restartPlay"
                    /><span>操作按钮</span>
                    <input
                        type="checkbox"
                        ref="operateBtns"
                        v-model="showBandwidth"
                        @change="restartPlay"
                    /><span>网速</span>
                    <span v-if="performance">性能：{{ performance }}</span>
                </div>
            </div>
            <div class="input" v-if="loaded">
                <input
                    type="checkbox"
                    ref="offscreen"
                    v-model="useOffscreen"
                    @change="restartPlay('offscreen')"
                /><span>离屏渲染</span>

                <select v-model="scale" @change="scaleChange">
                    <option value="0">完全填充(拉伸)</option>
                    <option value="1">等比缩放</option>
                    <option value="2">完全填充(未拉伸)</option>
                </select>
                <button v-if="!playing" @click="clearView">清屏</button>
                <template v-if="playing">
                    <select v-model="recordType">
                        <option value="webm">webm</option>
                        <option value="mp4">mp4</option>
                    </select>
                    <button v-if="!recording" @click="startRecord">录制</button>
                    <button v-if="!recording" @click="stopAndSaveRecord">暂停录制</button>
                </template>

            </div>
        </div>
    </div>
</template>
<script>
export default {
    name: "ProDemoPlayer",
    jessibuca: null,
    props: {},
    data() {
        return {
            version: '',
            wasm: false,
            playing: false,
            quieting: true,
            loaded: false, // mute
            showOperateBtns: true,
            showBandwidth: true,
            hotKey: true,
            err: "",
            speed: 0,
            performance: "",
            volume: 1,
            rotate: 0,
            useWCS: false,
            useMSE: false,
            useSIMD: false,
            useOffscreen: false,
            recording: false,
            isDebug: false,
            recordType: 'webm',
            scale: 0,
            vConsole: null,
            playType: ''
        };
    },
    mounted() {
        this.vConsole = new window.VConsole();
        this.create();
        window.onerror = (msg) => (this.err = msg);
    },
    unmounted() {
        if (this.$options && this.$options.jessibuca) {
            this.$options.jessibuca.destroy();
        }
        this.vConsole.destroy();
    },
    methods: {
        create(options) {
            options = options || {};
            const jessibuca = new window.JessibucaPro(
                Object.assign(
                    {
                        container: this.$refs.container,
                        decoder: '/pro/decoder.js',
                        videoBuffer: Number(this.$refs.buffer.value), // 缓存时长
                        isResize: false,
                        useWCS: this.useWCS,
                        useMSE: this.useMSE,
                        useSIMD: this.useSIMD,
                        wcsUseVideoRender: this.useWCS,
                        text: "",
                        // background: "bg.jpg",
                        loadingText: "疯狂加载中...",
                        // hasAudio:false,
                        debug: this.isDebug,
                        hotKey: true,
                        // hasAudio:false,
                        supportDblclickFullscreen: true,
                        showBandwidth: this.showBandwidth, // 显示网速
                        qualityConfig: ['普清', '高清', '超清', '4K', '8K'],
                        operateBtns: {
                            fullscreen: this.showOperateBtns,
                            screenshot: this.showOperateBtns,
                            record: this.showOperateBtns,
                            play: this.showOperateBtns,
                            audio: this.showOperateBtns,
                            ptz: this.showOperateBtns,
                            quality: this.showOperateBtns
                        },
                        vod: this.vod,
                        forceNoOffscreen: !this.useOffscreen,
                        isNotMute: true,
                        timeout: 10,
                        watermarkConfig: {
                            image: {
                                src: 'http://jessibuca.monibuca.com/jessibuca-logo.png',
                                width: 150,
                                height: 48
                            },
                            // text:{
                            //     content:'jessibuca-pro'
                            // },
                            right: 10,
                            top: 10
                        },
                    },
                    options
                )
            );

            this.$options.jessibuca = jessibuca;
            var _this = this;
            jessibuca.on("load", function () {
                console.log("on load");
            });

            jessibuca.on("log", function (msg) {
                console.log("on log", msg);
            });
            jessibuca.on("record", function (msg) {
                console.log("on record:", msg);
            });
            jessibuca.on("pause", function () {
                console.log("on pause");
                _this.playing = false;
            });
            jessibuca.on("play", function () {
                console.log("on play");
                _this.playing = true;
            });
            jessibuca.on("fullscreen", function (msg) {
                console.log("on fullscreen", msg);
            });

            jessibuca.on("mute", function (msg) {
                console.log("on mute", msg);
                _this.quieting = msg;
            });

            jessibuca.on("mute", function (msg) {
                console.log("on mute2", msg);
            });

            jessibuca.on("audioInfo", function (msg) {
                console.log("audioInfo", msg);
            });

            jessibuca.on("bps", function (bps) {
                // console.log('bps', bps);
            });
            // let _ts = 0;
            // this.jessibuca.on("timeUpdate", function (ts) {
            //     console.log('timeUpdate,old,new,timestamp', _ts, ts, ts - _ts);
            //     _ts = ts;
            // });

            jessibuca.on("videoInfo", function (info) {
                console.log("videoInfo", info);
            });

            jessibuca.on("error", function (error) {
                console.log("error", error);
            });

            jessibuca.on("timeout", function () {
                console.log("timeout");
            });

            jessibuca.on('start', function () {
                console.log('frame start');
            })

            jessibuca.on("performance", function (performance) {
                var show = "卡顿";
                if (performance === 2) {
                    show = "非常流畅";
                } else if (performance === 1) {
                    show = "流畅";
                }
                _this.performance = show;
            });
            jessibuca.on('buffer', function (buffer) {
                console.log('buffer', buffer);
            })

            jessibuca.on('stats', function (stats) {
                console.log('stats', stats);
            })

            jessibuca.on('kBps', function (kBps) {
                console.log('kBps', kBps);
            });

            jessibuca.on("play", () => {
                this.playing = true;
                this.loaded = true;
                this.quieting = jessibuca.isMute();
            });

            jessibuca.on('recordingTimestamp', (ts) => {
                console.log('recordingTimestamp', ts);
            })

            jessibuca.on('playToRenderTimes', (times) => {
                console.log(times);
            })

            // this.play();
            // console.log(this.jessibuca);
        },
        play() {
            // this.jessibuca.onPlay = () => (this.playing = true);


            if (this.$refs.playUrl.value) {
                this.$options.jessibuca.play(this.$refs.playUrl.value)
                this.playType = 'play'
            }
        },
        playback() {
            const playList = [{"start": 1653840000, "end": 1653841624}, {
                "start": 1653841634,
                "end": 1653843420
            }, {"start": 1653843429, "end": 1653843958}, {"start": 1653843967, "end": 1653845688}, {
                "start": 1653845698,
                "end": 1653846480
            }, {"start": 1653846490, "end": 1653847199}, {"start": 1653847208, "end": 1653848531}, {
                "start": 1653848541,
                "end": 1653850863
            }, {"start": 1653850872, "end": 1653853371}, {"start": 1653853381, "end": 1653857885}, {
                "start": 1653857894,
                "end": 1653858352
            }, {"start": 1653858362, "end": 1653860545}, {"start": 1653860554, "end": 1653861080}, {
                "start": 1653861090,
                "end": 1653862017
            }, {"start": 1653862026, "end": 1653863812}, {"start": 1653863822, "end": 1653865325}, {
                "start": 1653865335,
                "end": 1653867374
            }, {"start": 1653867383, "end": 1653867698}, {"start": 1653867707, "end": 1653868816}, {
                "start": 1653868826,
                "end": 1653872829
            }, {"start": 1653872838, "end": 1653877527}, {"start": 1653877537, "end": 1653879799}, {
                "start": 1653879809,
                "end": 1653881953
            }, {"start": 1653881963, "end": 1653885397}, {"start": 1653885407, "end": 1653886894}, {
                "start": 1653886904,
                "end": 1653890591
            }, {"start": 1653890600, "end": 1653894360}, {"start": 1653894370, "end": 1653903276}, {
                "start": 1653903286,
                "end": 1653912848
            }, {"start": 1653912858, "end": 1653914424}, {"start": 1653914433, "end": 1653915002}, {
                "start": 1653915011,
                "end": 1653918125
            }, {"start": 1653918135, "end": 1653921622}, {"start": 1653921631, "end": 1653924609}, {
                "start": 1653924618,
                "end": 1653926399
            }]

            if (this.$refs.playUrl.value) {
                this.$options.jessibuca.playback(this.$refs.playUrl.value, {
                    playList
                })
                this.playType = 'playback'
            }
        },
        mute() {
            this.$options.jessibuca.mute();
        },
        cancelMute() {
            this.$options.jessibuca.cancelMute();
        },

        pause() {
            this.$options.jessibuca.pause();
            this.playing = false;
            this.err = "";
            this.performance = "";
        },
        volumeChange() {
            this.$options.jessibuca.setVolume(this.volume);
        },
        rotateChange() {
            this.$options.jessibuca.setRotate(this.rotate);
        },
        destroy() {
            if (this.$options.jessibuca) {
                this.$options.jessibuca.destroy();
            }
            this.create();
            this.playType = '';
            this.playing = false;
            this.loaded = false;
            this.performance = "";
        },

        fullscreen() {
            this.$options.jessibuca.setFullscreen(true);
        },

        clearView() {
            this.$options.jessibuca.clearView();
        },

        startRecord() {
            const time = new Date().getTime();
            this.$options.jessibuca.startRecord(time, this.recordType);
        },

        stopAndSaveRecord() {
            this.$options.jessibuca.stopRecordAndSave();
        },


        screenShot() {
            this.$options.jessibuca.screenshot();
        },
        screenshotWatermark1() {
            this.$options.jessibuca.screenshotWatermark({
                text: {
                    content: 'jessibuca-pro',
                    fontSize: '40',
                    color: 'red'
                },
                right: 20,
                top: 40
            });
        },
        screenshotWatermark2() {
            this.$options.jessibuca.screenshotWatermark({
                image: {
                    src: 'data:image/png;base64,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',
                    width: 60,
                    height: 60
                },
                right: 20,
                top: 20
            });
        },


        restartPlay(type) {

            if (type === 'mse') {
                this.useWCS = false;
                this.useOffscreen = false;
            } else if (type === 'wcs') {
                this.useMSE = false
                this.useSIMD = false;
            } else if (type === 'offscreen') {
                this.useMSE = false
            } else if (type === 'simd') {
                this.useMSE = false;
                this.useWCS = false;
            }

            this.destroy();
            setTimeout(() => {
                if (this.playType === 'play') {
                    this.play();
                } else if (this.playType === 'playback') {
                    this.playback();
                } else {
                    this.play();
                }
            }, 100)
        },

        changeBuffer() {
            this.$options.jessibuca.setBufferTime(Number(this.$refs.buffer.value));
        },

        scaleChange() {
            this.$options.jessibuca.setScaleMode(this.scale);
        },
    },
};
</script>
<style scoped>
.root {
    display: flex;
    place-content: center;
    margin-top: 3rem;
}

.container-shell {
    position: relative;
    backdrop-filter: blur(5px);
    background: hsla(0, 0%, 50%, 0.5);
    padding: 30px 4px 10px 4px;
    /* border: 2px solid black; */
    width: auto;
    position: relative;
    border-radius: 5px;
    box-shadow: 0 10px 20px;
}

.container-shell-title {
    position: absolute;
    color: darkgray;
    top: 4px;
    left: 10px;
    text-shadow: 1px 1px black;
}

.tag-version {
}

#container {
    background: rgba(13, 14, 27, 0.7);
    width: 900px;
    height: 568px;
}

.input {
    display: flex;
    align-items: center;
    margin-top: 10px;
    color: white;
    place-content: stretch;
}

.input2 {
    bottom: 0px;
}

.input input[type='input'] {
    flex: auto;
}

.err {
    position: absolute;
    top: 40px;
    left: 10px;
    color: red;
}

.option {
    position: absolute;
    top: 4px;
    right: 10px;
    display: flex;
    place-content: center;
    align-items: center;
    font-size: 12px;
}

.option span {
    color: white;
}

.page {
    background: url(/bg.jpg);
    background-repeat: no-repeat;
    background-position: top;
}

@media (max-width: 720px) {
    #container {
        width: 90vw;
        height: 52.7vw;
    }
}
</style>
