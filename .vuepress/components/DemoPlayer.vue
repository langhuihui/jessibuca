
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
      </div>
      <div class="err" v-show="!playing">{{err}}</div>
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
      decoder: "ff",
      playing: false,
      err: ""
    };
  },
  watch: {
    decoder(v) {
      if (this.jessibuca) {
        this.jessibuca.destroy();
      }
      this.jessibuca = new Jessibuca({
        container: this.$refs.container,
        decoder: v + ".js",
        videoBuffer: 0.2
      });
      this.jessibuca.onPlay = () => (this.playing = true);
    }
  },
  mounted() {
    this.jessibuca = new Jessibuca({
      container: this.$refs.container,
      decoder: this.decoder + ".js",
      videoBuffer: 0.2
    });
    this.jessibuca.onPlay = () => (this.playing = true);
    onerror = msg => (this.err = msg);
  },
  destroyed() {
    this.jessibuca.destroy();
  },
  methods: {
    play() {
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
@media (max-width: 720px) {
  #container {
    width: 90vw;
    height: 52.7vw;
  }
}
</style>