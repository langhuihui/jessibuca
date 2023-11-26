<template>
    <div class="root">
        <div class="container-shell">
            <div class="container-shell-title">jessibuca demo player <span class="tag-version" v-if="version">({{
                    version
                }})</span></div>
            <div id="container" ref="container"></div>
            <div class="input">
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
                /><span>开启日志</span>

            </div>

            <div class="input">
                <div>
                    <input
                        type="checkbox"
                        v-model="isFlv"
                        @change="restartPlay()"
                    /><span>设置Flv格式</span>
                    <input
                        type="checkbox"
                        v-model="controlAutoHide"
                        @change="restartPlay()"
                    /><span>控制栏自动隐藏(移动端不支持)</span>
                </div>

            </div>
            <div class="input">
                <span>解码器：</span>
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
                    v-model="useWasm"
                    ref="vod"
                    @change="restartPlay('wasm')"
                /><span>wasm</span>
            </div>
            <div class="input">
                <div>输入URL：</div>
                <input
                    placeholder="支持ws-raw/ws-flv/http-flv协议"
                    type="input"
                    autocomplete="on"
                    ref="playUrl"
                    value=""
                />
                <button v-if="!playing" @click="play">播放</button>
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
                    <span v-if="fps" style="margin-left: 10px">FPS：{{ fps }}</span>
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
import {VERSION} from "./version";

function isMobile() {
    return (/iphone|ipad|android.*mobile|windows.*phone|blackberry.*mobile/i.test(window.navigator.userAgent.toLowerCase()));
}

function isPad() {
    return (/ipad|android(?!.*mobile)|tablet|kindle|silk/i.test(window.navigator.userAgent.toLowerCase()));
}

export default {
    name: "DemoPlayer",
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
            fps: '',
            volume: 1,
            rotate: 0,
            useWCS: false,
            useMSE: false,
            useWasm: true,
            useOffscreen: false,
            recording: false,
            isDebug: true,
            recordType: 'webm',
            scale: 0,
            vConsole: null,
            controlAutoHide: true,
            isFlv: false,
        };
    },
    mounted() {
        if ((isMobile() || isPad()) && window.VConsole) {
            this.vConsole = new window.VConsole();
        }
        this.version = VERSION === '#VERSION#' ? '' : VERSION;
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
            const jessibuca = new window.Jessibuca(
                Object.assign(
                    {
                        container: this.$refs.container,
                        videoBuffer: Number(this.$refs.buffer.value), // 缓存时长
                        isResize: false,
                        useWCS: this.useWCS,
                        useMSE: this.useMSE,
                        wcsUseVideoRender: this.useWCS,
                        text: "",
                        // background: "bg.jpg",
                        loadingText: "疯狂加载中...",
                        // hasAudio:false,
                        debug: this.isDebug,
                        hotKey: true,
                        // hasAudio:false,
                        controlAutoHide: this.controlAutoHide,
                        supportDblclickFullscreen: true,
                        showBandwidth: this.showBandwidth, // 显示网速
                        operateBtns: {
                            fullscreen: this.showOperateBtns,
                            screenshot: this.showOperateBtns,
                            record: this.showOperateBtns,
                            play: this.showOperateBtns,
                            audio: this.showOperateBtns,
                        },
                        forceNoOffscreen: !this.useOffscreen,
                        isNotMute: true,
                        timeout: 10,
                        recordType: 'mp4',
                        isFlv: this.isFlv,
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
                _this.fps = stats.fps;
            })

            jessibuca.on('kBps', function (kBps) {
                console.log('kBps', kBps);
            });

            // jessibuca.on("play", () => {
            //     this.playing = true;
            //     this.loaded = true;
            //     this.quieting = jessibuca.isMute();
            // });

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
                this.$options.jessibuca.play(this.$refs.playUrl.value).then(() => {
                    console.log('play success');
                    this.playing = true;
                    this.loaded = true;
                    this.quieting = jessibuca.isMute();
                }).catch((err) => {
                    console.log('play error', err);
                });
            }
        },
        mute() {
            this.$options.jessibuca.mute();
        },
        cancelMute() {
            this.$options.jessibuca.cancelMute();
        },

        pause() {
            this.$options.jessibuca.pause().then(() => {
                console.log('pause success');
                this.playing = false;
                this.err = "";
                this.performance = "";
            }).catch((err) => {
                console.log('pause error', err);
            });

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


        restartPlay(type) {

            if (type === 'mse') {
                this.useWCS = false;
                this.useOffscreen = false;
                this.useWasm = false;
            } else if (type === 'wcs') {
                this.useMSE = false
                this.useWasm = false;
            } else if (type === 'offscreen') {
                this.useMSE = false
            } else if (type === 'wasm') {
                this.useWCS = false;
                this.useMSE = false;
                this.useOffscreen = false;
            }

            this.destroy();
            setTimeout(() => {
                this.play();
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

<style>
.page {
    background: url(/bg.jpg);
    background-repeat: no-repeat;
    background-position: top;
}
</style>
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
    width: 640px;
    height: 398px;
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


@media (max-width: 720px) {
    #container {
        width: 90vw;
        height: 52.7vw;
    }
}
</style>
