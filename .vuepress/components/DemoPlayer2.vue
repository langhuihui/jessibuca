<template>
    <div class="root">
        <div class="container-shell">
            <div id="container" ref="container"></div>
            <div class="input">
                <div>输入URL：</div>
                <input autocomplete="on" ref="playUrl"
                       value=""/>
                <button v-if="!playing" @click="play">播放</button>
                <button v-else @click="pause">停止</button>
            </div>
            <div class="option">
                <span>缓冲:</span>
                <input style="width:50px" type="number" ref="buffer" value="0.2" @change="changeBuffer">
                <input type="checkbox" ref="wasm" @change="changeWasm"><span>wasm</span>
                <select ref="vc" @change="changeVC">
                    <option selected>h264</option>
                    <option>h265</option>
                </select>
            </div>
        </div>
    </div>
</template>
<script>
    import Jessibuca from "./rendererV2";

    export default {
        name: "DemoPlayer",
        props: {},
        data() {
            return {
                jessibuca: null,
                wasm: false,
                vc: "ff",
                playing: false,
                quieting: true, // mute
                err: "",
                speed: 0,
                muting: true,
            };
        },
        computed: {
            decoder() {
                return this.vc + (this.wasm ? "_wasm" : "") + ".js"
            },
            speedShow() {

            }
        },
        watch: {
            decoder(v) {
                if (this.jessibuca) {
                    this.jessibuca.destroy();
                }
                this.create()
                this.playing = false;
            }
        },
        mounted() {
            this.create()
            window.onerror = msg => (this.err = msg);
        },
        destroyed() {
            this.jessibuca.destroy();
        },
        methods: {
            create() {
                this.jessibuca = new Jessibuca({
                    container: this.$refs.container,
                    decoder: this.decoder,
                    videoBuffer: Number(this.$refs.buffer.value),
                    isResize: false,
                    text: '',
                    background: 'https://seopic.699pic.com/photo/40011/0709.jpg_wh1200.jpg',
                    loadingText: '加载中',
                    debug: true,
                    // isFullResize: true
                });
                this.jessibuca.onLog = msg => console.log('onLog', msg);
                this.jessibuca.onLoad = msg => console.log('onLoad');
                this.jessibuca.onRecord = msg => console.log('onRecord', msg);
                this.jessibuca.onPause = () => console.log('onPause');
                this.jessibuca.onPlay = () => console.log('onPlay');
                this.jessibuca.onFullscreen = msg => console.log('onFullscreen', msg);
                this.jessibuca.onMute = msg => console.log('onMute', msg);

                this.jessibuca.on('load', function () {
                    console.log('on load');
                });

                this.jessibuca.on('log', function (msg) {
                    console.log('on log', msg);
                });
                this.jessibuca.on('record', function (msg) {
                    console.log('on record:', msg);
                });
                this.jessibuca.on('pause', function () {
                    console.log('on pause');
                });
                this.jessibuca.on('play', function () {
                    console.log('on play');
                });
                this.jessibuca.on('fullscreen', function (msg) {
                    console.log('on fullscreen', msg);
                });

                this.jessibuca.on('mute', function (msg) {
                    console.log('on mute', msg);
                });

                this.jessibuca.on('mute', function (msg) {
                    console.log('on mute2', msg);
                });

                this.jessibuca.on('audioInfo', function (msg) {
                    console.log('audioInfo', msg);
                });

                this.jessibuca.on('bps', function (bps) {
                    console.log('bps', bps);
                });

                console.log(this.jessibuca);
            },
            play() {
                this.jessibuca.onPlay = () => (this.playing = true);
                if (this.$refs.playUrl.value) {
                    this.jessibuca.play(this.$refs.playUrl.value);
                }
            },
            mute() {
                this.jessibuca.mute();
                this.muting = true;
            },
            cancelMute() {
                this.jessibuca.cancelMute();
                this.muting = false;
            },

            pause() {
                this.jessibuca.pause();
                this.playing = false;
                this.err = "";
            },


            changeVC() {
                this.vc = ["ff", "libhevc_aac"][this.$refs.vc.selectedIndex]
            },

            changeWasm() {
                this.wasm = this.$refs.wasm.checked
            },

            changeBuffer() {
                this.jessibuca.setBufferTime(Number(this.$refs.buffer.value));
            }
        }
    };
</script>
<style>
    .root {
        display: flex;
        place-content: center;
    }

    .container-shell {
        background: gray;
        padding: 30px 4px 50px 4px;
        border: 2px solid black;
        width: auto;
        position: relative;
        border-radius: 5px;
        box-shadow: 0 10px 20px;
    }

    .container-shell:before {
        content: "jessibuca demo player";
        position: absolute;
        color: darkgray;
        top: 4px;
        left: 10px;
        text-shadow: 1px 1px black;
    }

    #container {
        background: rgb(13, 14, 27);
        width: 640px;
        height: 398px;
    }

    .input {
        position: absolute;
        display: flex;
        bottom: 15px;
        left: 10px;
        right: 10px;
        color: white;
        place-content: stretch;
    }

    .input input {
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
