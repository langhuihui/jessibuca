<template>
  <div class="root">
    <div class="container-shell">
      <div id="container" class="container" ref="container"></div>
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
        <button v-if="isMute" @click="cancelMute">取消静音</button>
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
        <input type="checkbox" ref="vod" @change="changeVod" /><span
          >点播模式</span
        >
        <input type="checkbox" ref="offscreen" @change="changeOffscreen" /><span
          >禁用离屏渲染</span
        >
        <input type="checkbox" ref="resize" @change="changeResize" /><span
          >自适应</span
        >
      </div>
    </div>
  </div>
</template>
<script setup>
import Jessibuca from "./renderer";

import { ref, onMounted, onUnmounted } from "vue";
let playing = ref(false);
let loaded = ref(false);
let showOperateBtns = ref(false);
let showBandwidth = ref(false);
let err = ref("");
let performance = ref("");
let volume = ref(1);
let rotate = ref(0);
let container = ref();
let buffer = ref();
let resize = ref();
let vod = ref();
let offscreen = ref();
let playUrl = ref();
let jessibuca = null;
function create() {
  jessibuca = new Jessibuca({
    container: container.value,
    videoBuffer: Number(buffer.value.value), // 缓存时长
    isResize: false,
    text: "",
    // background: "bg.jpg",
    loadingText: "加载中",
    debug: true,
    showBandwidth: showBandwidth.value, // 显示网速
    operateBtns: {
      fullscreen: showOperateBtns.value,
      screenshot: showOperateBtns.value,
      play: showOperateBtns.value,
      audio: showOperateBtns.value,
    },
  });
  jessibuca.onLog = (msg) => console.log("onLog", msg);
  jessibuca.onLoad = (msg) => console.log("onLoad");
  jessibuca.onRecord = (msg) => console.log("onRecord", msg);
  jessibuca.onPause = () => console.log("onPause");
  jessibuca.onPlay = () => console.log("onPlay");
  jessibuca.onFullscreen = (msg) => console.log("onFullscreen", msg);
  jessibuca.onMute = (msg) => console.log("onMute", msg);
  jessibuca.onInitSize = () => console.log("onInitSize");
  // jessibuca.onTimeUpdate = (ts)=> console.log('onTimeUpdate',ts);
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
    playing.value = false;
  });
  jessibuca.on("play", function () {
    console.log("on play");
    playing.value = true;
  });
  jessibuca.on("fullscreen", function (msg) {
    console.log("on fullscreen", msg);
  });

  jessibuca.on("mute", function (msg) {
    console.log("on mute", msg);
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
  let _ts = 0;
  jessibuca.on("timeUpdate", function (ts) {
    // console.log('timeUpdate,old,new,timestamp', _ts, ts, ts - _ts);
    _ts = ts;
  });

  jessibuca.on("videoInfo", function (info) {
    console.log("videoInfo", info);
  });

  jessibuca.on("error", function (error) {
    console.log("error", error);
  });

  jessibuca.on("timeout", function () {
    console.log("timeout");
  });

  jessibuca.on("stats", function (stats) {
    // console.log('stats',JSON.stringify(stats));
  });

  jessibuca.on("performance", function (p) {
    var show = "卡顿";
    if (p === 2) {
      show = "非常流畅";
    } else if (p === 1) {
      show = "流畅";
    }
    // console.log('stats',show);
    performance.value = show;
  });
}
onMounted(create);
onUnmounted(() => {
  if (jessibuca) jessibuca.destroy();
});
function isMute() {
  return !jessibuca || jessibuca.quieting;
}
function play() {
  // jessibuca.onPlay = () => (playing.value = true);
  jessibuca.on("play", () => {
    playing.value = true;
    loaded.value = true;
  });

  if (playUrl.value.value) {
    if (jessibuca.hasLoaded()) {
      jessibuca.play(playUrl.value.value);
    } else {
      jessibuca.on("load", () => {
        jessibuca.play(playUrl.value.value);
      });
    }
  }
}
function mute() {
  jessibuca.mute();
}
function cancelMute() {
  jessibuca.cancelMute();
}
function pause() {
  jessibuca.pause();
  playing.value = false;
  err.value = "";
  performance.value = "";
}
function volumeChange() {
  jessibuca.setVolume(volume.value);
}
function rotateChange() {
  jessibuca.setRotate(rotate.value);
}
function destroy() {
  if (jessibuca) {
    jessibuca.destroy();
  }
  create();
  playing.value = false;
  loaded.value = false;
  performance.value = "";
}
function fullscreen() {
  jessibuca.setFullscreen(true);
}
function screenShot() {
  jessibuca.screenshot();
}
function restartPlay() {
  destroy();
  play();
}
function changeBuffer() {
  jessibuca.setBufferTime(Number(buffer.value.value));
}
function changeResize() {
  jessibuca.setScaleMode(resize.value.checked ? 1 : 0);
}
function changeVod() {
  jessibuca.setVod(vod.value.checked ? 1 : 0);
}
function changeOffscreen() {
  jessibuca.setNoOffscreen(offscreen.value.checked ? 1 : 0);
}
</script>
<style>
.root {
  display: flex;
  place-content: center;
}

.container-shell {
  backdrop-filter: blur(5px);
  background: hsla(0, 0%, 50%, 0.5);
  padding: 30px 4px 10px 4px;
  /* border: 2px solid black; */
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

.container {
  background: rgba(13, 14, 27, 0.5);
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

@media (max-width: 720px) {
  .container {
    width: 90vw;
    height: 52.7vw;
  }
}
</style>
