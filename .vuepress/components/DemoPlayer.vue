
<template>
  <div class="root">
    <div class="container-shell">
      <div id="container" ref="container"></div>
      <div class="input">
        <div>输入URL：</div>
        <input autocomplete="on" ref="playUrl" />
        <button v-if="!playing" @click="play">播放</button>
        <button v-else @click="stop">停止</button>
        <button @click="fullscreen">全屏</button>
        <button v-if="!quieting" @click="quiet">静音</button>
        <button v-else @click="playAudio">声音</button>
        <button @click="screenshots">截图</button>
      </div>
      <div class="err" v-show="!playing">{{err}}</div>
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
import Jessibuca from "./renderer";
export default {
  name: "DemoPlayer",
  data() {
    return {
      jessibuca: null,
      wasm:false,
      vc:"ff",
      playing: false,
      quieting:false, // mute
      err: ""
    };
  },
  computed:{
    decoder(){
      return this.vc+(this.wasm?"_wasm":"")+".js"
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
    create(){
      this.jessibuca = new Jessibuca({
        container: this.$refs.container,
        decoder: this.decoder,
        videoBuffer: Number(this.$refs.buffer.value),
        contextOptions:{
          preserveDrawingBuffer:true
        }
      });
      this.jessibuca.onLog = msg=>(this.err=msg);
    },
    play() {
      this.jessibuca.onPlay = () => (this.playing = true);
      this.jessibuca.play(this.$refs.playUrl.value);
      this.err = "loading";
    },
    stop() {
      this.jessibuca.close();
      this.playing = false;
      this.err = "";
    },
    fullscreen() {
      this.jessibuca.fullscreen = true;
    },
    quiet(){
      this.quieting = true;
      this.jessibuca.audioEnabled(false);
    },
    playAudio(){
      this.quieting = false;
      this.jessibuca.audioEnabled(true);
    },
    screenshots(){
      if (!(this.jessibuca && this.jessibuca.canvasElement)) {
        return;
      }
      const dataURL = this.jessibuca.canvasElement.toDataURL('image/png');
      console.log(dataURL);
      this._downloadImg(this._dataURLToFile(dataURL));
    },
    _dataURLToFile(dataURL){
      const arr = dataURL.split(",");
      const bstr = atob(arr[1]);
      const type = arr[0].replace("data:", "").replace(";base64", "")
      let n = bstr.length, u8arr = new Uint8Array(n);
      while (n--) {
        u8arr[n] = bstr.charCodeAt(n);
      }
      return new File([u8arr], 'file', {type});
    },

    _downloadImg(content){
      const aLink = document.createElement("a");
      aLink.download = '' + new Date().getTime();
      aLink.href = URL.createObjectURL(content);
      aLink.click();
      URL.revokeObjectURL(content);
    },

    changeVC(){
      this.vc = ["ff","libhevc_aac"][this.$refs.vc.selectedIndex]
    },
    changeWasm(){
      this.wasm = this.$refs.wasm.checked
    },
    changeBuffer(){
      this.jessibuca.decoderWorker.postMessage({ cmd: "setVideoBuffer", time: Number(this.$refs.buffer.value) })
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
  height: 375px;
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
  top:4px;
  right:10px;
  display: flex;
  place-content: center;
}
.option span{
  color: white;
}
@media (max-width: 720px) {
  #container {
    width: 90vw;
    height: 52.7vw;
  }
}
</style>
