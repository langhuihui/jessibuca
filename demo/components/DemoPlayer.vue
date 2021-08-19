<template>
  <div class="root">
    <div class="container-shell">
      <div class="container-shell-title">jessibuca demo player <span class="tag-version" v-if="version">({{
          version
        }})</span></div>
      <div id="container" ref="container"></div>
      <div class="input">
        <div>输入URL：</div>
        <input
            autocomplete="on"
            ref="playUrl"
            value="ws://localhost:8080/jessica/live/rtc"
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
          <span v-if="performance">性能：{{ performance }}</span>
        </div>
      </div>
      <div class="option">
        <span>缓冲(秒):</span>
        <input
            style="width: 50px"
            type="number"
            ref="buffer"
            value="0.2"
            @change="changeBuffer"
        />
        <!-- <input
          type="checkbox"
          v-model="vod"
          ref="vod"
          @change="restartPlay"
        /><span>点播模式</span> -->
        <input
            type="checkbox"
            ref="offscreen"
            v-model="forceNoOffscreen"
            @change="restartPlay"
        /><span>禁用离屏渲染</span>
        <input type="checkbox" ref="resize" @change="changeResize"/><span
      >禁止画面拉伸</span
      >
      </div>
    </div>
  </div>
</template>
<script>
import {VERSION} from "./version";

export default {
  name: "DemoPlayer",
  props: {},
  data() {
    return {
      jessibuca: null,
      version: '',
      wasm: false,
      vc: "ff",
      playing: false,
      quieting: true,
      loaded: false, // mute
      showOperateBtns: false,
      showBandwidth: false,
      err: "",
      speed: 0,
      performance: "",
      volume: 1,
      rotate: 0,
      vod: false,
      forceNoOffscreen: false,
    };
  },
  mounted() {
    this.version = VERSION === '#VERSION#' ? '' : VERSION;
    this.create();
    window.onerror = (msg) => (this.err = msg);
  },
  unmounted() {
    this.jessibuca.destroy();
  },
  methods: {
    create(options) {
      options = options || {};
      this.jessibuca = new window.Jessibuca(
          Object.assign(
              {
                container: this.$refs.container,
                videoBuffer: Number(this.$refs.buffer.value), // 缓存时长
                isResize: false,
                text: "",
                // background: "bg.jpg",
                loadingText: "加载中",
                // hasAudio:false,
                debug: true,
                showBandwidth: this.showBandwidth, // 显示网速
                operateBtns: {
                  fullscreen: this.showOperateBtns,
                  screenshot: this.showOperateBtns,
                  play: this.showOperateBtns,
                  audio: this.showOperateBtns,
                },
                vod: this.vod,
                forceNoOffscreen: this.forceNoOffscreen,
                isNotMute: false,
              },
              options
          )
      );
      // this.jessibuca.onLog = (msg) => console.log("onLog", msg);
      // this.jessibuca.onLoad = (msg) => console.log("onLoad");
      // this.jessibuca.onRecord = (msg) => console.log("onRecord", msg);
      // this.jessibuca.onPause = () => console.log("onPause");
      // this.jessibuca.onPlay = () => console.log("onPlay");
      // this.jessibuca.onFullscreen = (msg) => console.log("onFullscreen", msg);
      // this.jessibuca.onMute = (msg) => console.log("onMute", msg);
      // this.jessibuca.onInitSize = () => console.log("onInitSize");
      var _this = this;
      this.jessibuca.on("load", function () {
        console.log("on load");
      });

      this.jessibuca.on("log", function (msg) {
        console.log("on log", msg);
      });
      this.jessibuca.on("record", function (msg) {
        console.log("on record:", msg);
      });
      this.jessibuca.on("pause", function () {
        console.log("on pause");
        _this.playing = false;
      });
      this.jessibuca.on("play", function () {
        console.log("on play");
        _this.playing = true;
      });
      this.jessibuca.on("fullscreen", function (msg) {
        console.log("on fullscreen", msg);
      });

      this.jessibuca.on("mute", function (msg) {
        console.log("on mute", msg);
        _this.quieting = msg;
      });

      this.jessibuca.on("mute", function (msg) {
        console.log("on mute2", msg);
      });

      this.jessibuca.on("audioInfo", function (msg) {
        console.log("audioInfo", msg);
      });

      this.jessibuca.on("bps", function (bps) {
        // console.log('bps', bps);
      });
      let _ts = 0;
      this.jessibuca.on("timeUpdate", function (ts) {
        // console.log('timeUpdate,old,new,timestamp', _ts, ts, ts - _ts);
        _ts = ts;
      });

      this.jessibuca.on("videoInfo", function (info) {
        console.log("videoInfo", info);
      });

      this.jessibuca.on("error", function (error) {
        console.log("error", error);
      });

      this.jessibuca.on("timeout", function () {
        console.log("timeout");
      });

      this.jessibuca.on('start', function () {
        console.log('start');
      })

      // this.jessibuca.on("stats", function (stats) {
      //   console.log('stats', JSON.stringify(stats));
      // });

      this.jessibuca.on("performance", function (performance) {
        var show = "卡顿";
        if (performance === 2) {
          show = "非常流畅";
        } else if (performance === 1) {
          show = "流畅";
        }
        _this.performance = show;
      });
      this.jessibuca.on('buffer', function (buffer) {
        console.log('buffer', buffer);
      })

      this.jessibuca.on('stats', function (stats) {
        console.log('stats', stats);
      })

      this.jessibuca.on('kBps', function (kBps) {
        console.log('kBps', kBps);
      });

      // 显示时间戳 PTS
      this.jessibuca.on('videoFrame', function () {

      })

      //
      this.jessibuca.on('metadata', function () {

      });


      // console.log(this.jessibuca);
    },
    play() {
      // this.jessibuca.onPlay = () => (this.playing = true);
      this.jessibuca.on("play", () => {
        this.playing = true;
        this.loaded = true;
        this.quieting = this.jessibuca.quieting;
      });

      if (this.$refs.playUrl.value) {
        if (this.jessibuca.hasLoaded()) {
          this.jessibuca.play(this.$refs.playUrl.value);
        } else {
          this.jessibuca.on("load", () => {
            this.jessibuca.play(this.$refs.playUrl.value);
          });
        }
      }
    },
    mute() {
      this.jessibuca.mute();
    },
    cancelMute() {
      this.jessibuca.cancelMute();
    },

    pause() {
      this.jessibuca.pause();
      this.playing = false;
      this.err = "";
      this.performance = "";
    },
    volumeChange() {
      this.jessibuca.setVolume(this.volume);
    },
    rotateChange() {
      this.jessibuca.setRotate(this.rotate);
    },
    destroy() {
      if (this.jessibuca) {
        this.jessibuca.destroy();
      }
      this.create();
      this.playing = false;
      this.loaded = false;
      this.performance = "";
    },

    fullscreen() {
      this.jessibuca.setFullscreen(true);
    },

    screenShot() {
      this.jessibuca.screenshot();
    },

    changeWasm() {
      this.wasm = this.$refs.wasm.checked;
    },

    restartPlay() {
      this.destroy();
      this.play();
    },

    changeBuffer() {
      this.jessibuca.setBufferTime(Number(this.$refs.buffer.value));
    },

    changeResize() {
      const value = this.$refs.resize.checked ? 1 : 0;
      this.jessibuca.setScaleMode(value);
    },
    changeVod() {
      const value = this.$refs.vod.checked ? 1 : 0;
      this.jessibuca.setVod(value);
    },
    changeOffscreen() {
      const value = this.$refs.offscreen.checked ? 1 : 0;
      this.jessibuca.setNoOffscreen(value);
    },
  },
};
</script>
<style>
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
  margin-top: 10px;
  color: white;
  place-content: stretch;
}

.input2 {
  bottom: 0px;
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
