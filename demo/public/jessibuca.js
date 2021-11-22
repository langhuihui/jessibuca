(function (global, factory) {
    typeof exports === 'object' && typeof module !== 'undefined' ? module.exports = factory() :
    typeof define === 'function' && define.amd ? define(factory) :
    (global = typeof globalThis !== 'undefined' ? globalThis : global || self, global.jessibuca = factory());
})(this, (function () { 'use strict';

    // 播放协议
    const PLAYER_PLAY_PROTOCOL = {
      websocket: 0,
      fetch: 1
    };
    const DEMUX_TYPE = {
      flv: 'flv',
      m7s: 'm7s'
    }; // default player options

    const DEFAULT_PLAYER_OPTIONS = {
      videoBuffer: 0.5,
      vod: false,
      isResize: true,
      isFullResize: false,
      isFlv: false,
      debug: false,
      timeout: 30,
      supportDblclickFullscreen: false,
      showBandwidth: false,
      //
      keepScreenOn: false,
      isNotMute: false,
      hasAudio: true,
      hasVideo: true,
      operateBtns: {
        fullscreen: false,
        screenshot: false,
        play: false,
        audio: false,
        recorder: false
      },
      hasControl: false,
      loadingText: '',
      background: '',
      decoder: 'decoder.js',
      url: '',
      //
      rotate: 0,
      text: '',
      forceNoOffscreen: true,
      // 默认是不采用
      hiddenAutoPause: false,
      protocol: PLAYER_PLAY_PROTOCOL.fetch,
      demuxType: DEMUX_TYPE.flv,
      //
      useWCS: false,
      //
      useEMS: false,
      //
      useOffscreen: false //

    };
    const WORKER_CMD_TYPE = {
      init: 'init',
      initVideo: 'initVideo',
      render: 'render',
      playAudio: 'playAudio',
      print: 'print',
      printErr: 'printErr',
      initAudio: 'initAudio',
      kBps: 'kBps',
      decode: 'decode',
      audioCode: 'audioCode',
      videoCode: 'videoCode'
    };
    const MEDIA_TYPE = {
      audio: 1,
      video: 2
    };
    const FLV_MEDIA_TYPE = {
      audio: 8,
      video: 9
    };
    const WORKER_SEND_TYPE = {
      init: 'init',
      decode: 'decode',
      audioDecode: 'audioDecode',
      videoDecode: 'videoDecode',
      close: 'close'
    }; //

    const EVENTS = {
      fullscreen: 'fullscreen',
      webFullscreen: 'webFullscreen',
      decoderWorkerInit: 'decoderWorkerInit',
      play: 'play',
      playing: 'playing',
      pause: 'pause',
      mute: 'mute',
      load: 'load',
      loading: 'loading',
      videoInfo: 'videoInfo',
      timeUpdate: 'timeUpdate',
      audioInfo: "audioInfo",
      log: 'log',
      error: "error",
      kBps: 'kBps',
      timeout: 'timeout',
      stats: 'stats',
      performance: "performance",
      record: 'record',
      recording: 'recording',
      recordingTimestamp: 'recordingTimestamp',
      buffer: 'buffer',
      videoFrame: 'videoFrame',
      start: 'start',
      metadata: 'metadata',
      resize: 'resize',
      streamRate: 'streamRate',
      streamEnd: 'streamEnd',
      streamSuccess: 'streamSuccess',
      streamMessage: 'streamMessage',
      streamError: 'streamError',
      volumechange: 'volumechange',
      frameStart: 'frameStart'
    };
    const JESSIBUCA_EVENTS = {
      load: EVENTS.load,
      timeUpdate: EVENTS.timeUpdate,
      videoInfo: EVENTS.videoInfo,
      audioInfo: EVENTS.audioInfo,
      error: EVENTS.error,
      kBps: EVENTS.kBps,
      log: EVENTS.log,
      start: EVENTS.frameStart,
      timeout: EVENTS.timeout,
      fullscreen: EVENTS.fullscreen,
      play: EVENTS.play,
      pause: EVENTS.pause,
      mute: EVENTS.mute,
      stats: EVENTS.stats,
      performance: EVENTS.performance
    };
    const EVENTS_ERROR = {
      fetchError: "fetchError",
      websocketError: 'websocketError',
      websocketCloseSuccess: 'websocketCloseSuccess',
      websocketClosedByError: 'websocketClosedByError'
    };
    const WEBSOCKET_STATUS = {
      notConnect: 'notConnect',
      open: 'open',
      close: 'close',
      error: 'error'
    };
    const SCREENSHOT_TYPE = {
      download: 'download',
      base64: 'base64',
      blob: 'blob'
    };
    const VIDEO_ENC_TYPE = {
      7: 'H264',
      //
      12: 'H265' //

    };
    const AUDIO_ENC_TYPE = {
      10: 'AAC',
      7: 'ALAW',
      8: 'MULAW'
    };
    const CONTROL_HEIGHT = 40;
    const SCALE_MODE_TYPE = {
      full: 0,
      //  视频画面完全填充canvas区域,画面会被拉伸
      auto: 1,
      // 视频画面做等比缩放后,高或宽对齐canvas区域,画面不被拉伸,但有黑边
      fullAuto: 2 // 视频画面做等比缩放后,完全填充canvas区域,画面不被拉伸,没有黑边,但画面显示不全

    };

    class Debug {
      constructor(master) {
        this.log = (name, ...args) => {
          if (master._opt.debug) {
            console.log(`Jessibuca: [${name}]`, ...args);
          }
        };

        this.warn = (name, ...args) => {
          if (master._opt.debug) {
            console.warn(`Jessibuca: [${name}]`, ...args);
          }
        };

        this.error = (name, ...args) => {
          if (master._opt.debug) {
            console.error(`Jessibuca: [${name}]`, ...args);
          }
        };
      }

    }

    class Events {
      constructor(master) {
        this.destroys = [];
        this.proxy = this.proxy.bind(this);
        this.master = master;
      }

      proxy(target, name, callback, option = {}) {
        if (Array.isArray(name)) {
          return name.map(item => this.proxy(target, item, callback, option));
        }

        target.addEventListener(name, callback, option);

        const destroy = () => target.removeEventListener(name, callback, option);

        this.destroys.push(destroy);
        return destroy;
      }

      destroy() {
        this.master.debug.log(`Events`, 'destroy');
        this.destroys.forEach(event => event());
      }

    }

    var property$1 = (player => {
      Object.defineProperty(player, 'rect', {
        get: () => {
          return player.$container.getBoundingClientRect();
        }
      });
      ['bottom', 'height', 'left', 'right', 'top', 'width'].forEach(key => {
        Object.defineProperty(player, key, {
          get: () => {
            return player.rect[key];
          }
        });
      });
    });

    function createCommonjsModule(fn, module) {
    	return module = { exports: {} }, fn(module, module.exports), module.exports;
    }

    var screenfull = createCommonjsModule(function (module) {
    /*!
    * screenfull
    * v5.1.0 - 2020-12-24
    * (c) Sindre Sorhus; MIT License
    */
    (function () {

    	var document = typeof window !== 'undefined' && typeof window.document !== 'undefined' ? window.document : {};
    	var isCommonjs = module.exports;

    	var fn = (function () {
    		var val;

    		var fnMap = [
    			[
    				'requestFullscreen',
    				'exitFullscreen',
    				'fullscreenElement',
    				'fullscreenEnabled',
    				'fullscreenchange',
    				'fullscreenerror'
    			],
    			// New WebKit
    			[
    				'webkitRequestFullscreen',
    				'webkitExitFullscreen',
    				'webkitFullscreenElement',
    				'webkitFullscreenEnabled',
    				'webkitfullscreenchange',
    				'webkitfullscreenerror'

    			],
    			// Old WebKit
    			[
    				'webkitRequestFullScreen',
    				'webkitCancelFullScreen',
    				'webkitCurrentFullScreenElement',
    				'webkitCancelFullScreen',
    				'webkitfullscreenchange',
    				'webkitfullscreenerror'

    			],
    			[
    				'mozRequestFullScreen',
    				'mozCancelFullScreen',
    				'mozFullScreenElement',
    				'mozFullScreenEnabled',
    				'mozfullscreenchange',
    				'mozfullscreenerror'
    			],
    			[
    				'msRequestFullscreen',
    				'msExitFullscreen',
    				'msFullscreenElement',
    				'msFullscreenEnabled',
    				'MSFullscreenChange',
    				'MSFullscreenError'
    			]
    		];

    		var i = 0;
    		var l = fnMap.length;
    		var ret = {};

    		for (; i < l; i++) {
    			val = fnMap[i];
    			if (val && val[1] in document) {
    				for (i = 0; i < val.length; i++) {
    					ret[fnMap[0][i]] = val[i];
    				}
    				return ret;
    			}
    		}

    		return false;
    	})();

    	var eventNameMap = {
    		change: fn.fullscreenchange,
    		error: fn.fullscreenerror
    	};

    	var screenfull = {
    		request: function (element, options) {
    			return new Promise(function (resolve, reject) {
    				var onFullScreenEntered = function () {
    					this.off('change', onFullScreenEntered);
    					resolve();
    				}.bind(this);

    				this.on('change', onFullScreenEntered);

    				element = element || document.documentElement;

    				var returnPromise = element[fn.requestFullscreen](options);

    				if (returnPromise instanceof Promise) {
    					returnPromise.then(onFullScreenEntered).catch(reject);
    				}
    			}.bind(this));
    		},
    		exit: function () {
    			return new Promise(function (resolve, reject) {
    				if (!this.isFullscreen) {
    					resolve();
    					return;
    				}

    				var onFullScreenExit = function () {
    					this.off('change', onFullScreenExit);
    					resolve();
    				}.bind(this);

    				this.on('change', onFullScreenExit);

    				var returnPromise = document[fn.exitFullscreen]();

    				if (returnPromise instanceof Promise) {
    					returnPromise.then(onFullScreenExit).catch(reject);
    				}
    			}.bind(this));
    		},
    		toggle: function (element, options) {
    			return this.isFullscreen ? this.exit() : this.request(element, options);
    		},
    		onchange: function (callback) {
    			this.on('change', callback);
    		},
    		onerror: function (callback) {
    			this.on('error', callback);
    		},
    		on: function (event, callback) {
    			var eventName = eventNameMap[event];
    			if (eventName) {
    				document.addEventListener(eventName, callback, false);
    			}
    		},
    		off: function (event, callback) {
    			var eventName = eventNameMap[event];
    			if (eventName) {
    				document.removeEventListener(eventName, callback, false);
    			}
    		},
    		raw: fn
    	};

    	if (!fn) {
    		if (isCommonjs) {
    			module.exports = {isEnabled: false};
    		} else {
    			window.screenfull = {isEnabled: false};
    		}

    		return;
    	}

    	Object.defineProperties(screenfull, {
    		isFullscreen: {
    			get: function () {
    				return Boolean(document[fn.fullscreenElement]);
    			}
    		},
    		element: {
    			enumerable: true,
    			get: function () {
    				return document[fn.fullscreenElement];
    			}
    		},
    		isEnabled: {
    			enumerable: true,
    			get: function () {
    				// Coerce to boolean in case of old WebKit
    				return Boolean(document[fn.fullscreenEnabled]);
    			}
    		}
    	});

    	if (isCommonjs) {
    		module.exports = screenfull;
    	} else {
    		window.screenfull = screenfull;
    	}
    })();
    });
    screenfull.isEnabled;

    var events = (player => {
      //
      player.on(EVENTS.load, () => {
        player.debug.log('player', 'has loaded');
        player._hasLoaded = true;
      }); //

      player.on(EVENTS.play, () => {
        player.loading = false;
      }); //

      player.on(EVENTS.fullscreen, value => {
        if (value) {
          try {
            screenfull.request(player.$container).then(() => {}).catch(e => {
              player.webFullscreen = true;
            });
          } catch (e) {
            player.webFullscreen = true;
          }
        } else {
          try {
            screenfull.exit().then(() => {
              if (player.$borderSelect) {
                player.$borderSelect.style.display = 'block';
              }
            }).catch(() => {
              player.webFullscreen = false;
            });
          } catch (e) {
            player.webFullscreen = false;
          }
        }
      });
      player.on(EVENTS.webFullscreen, value => {
        if (value) {
          player.$container.classList.add('webmediaplayer-fullscreen-web');
          const {
            clientHeight: bodyHeight,
            clientWidth: bodyWidth
          } = document.body;
          const {
            clientHeight: playerHeight,
            clientWidth: playerWidth
          } = player.video.$videoElement;
          const bodyRatio = bodyWidth / bodyHeight;
          const playerRatio = playerWidth / playerHeight;
          const needSpin = bodyRatio < playerRatio;

          if (needSpin) {
            const scale = Math.min(bodyHeight / playerWidth, bodyWidth / playerHeight);
            player.video.$videoElement.style.transform = `rotate(90deg) scale(${scale},${scale})`;
          }
        } else {
          player.$container.classList.remove('webmediaplayer-fullscreen-web');
          player.video.$videoElement.style.transform = null;
        }
      }); //

      player.on(EVENTS.resize, () => {
        player.video.resize();
      });

      if (player._opt.debug) {
        Object.keys(EVENTS).forEach(key => {
          player.on(EVENTS[key], value => {
            player.debug.log('player events', EVENTS[key], value);
          });
        });
      }
    });

    function noop() {}
    function supportOffscreen($canvas) {
      return typeof $canvas.transferControlToOffscreen === 'function';
    }
    function supportOffscreenV2() {
      return typeof OffscreenCanvas !== "undefined";
    }
    function createContextGL($canvas) {
      let gl = null;
      const validContextNames = ["webgl", "experimental-webgl", "moz-webgl", "webkit-3d"];
      let nameIndex = 0;

      while (!gl && nameIndex < validContextNames.length) {
        const contextName = validContextNames[nameIndex];

        try {
          let contextOptions = {
            preserveDrawingBuffer: true
          };
          gl = $canvas.getContext(contextName, contextOptions);
        } catch (e) {
          gl = null;
        }

        if (!gl || typeof gl.getParameter !== "function") {
          gl = null;
        }

        ++nameIndex;
      }

      return gl;
    }
    function dataURLToFile(dataURL = '') {
      const arr = dataURL.split(",");
      const bstr = atob(arr[1]);
      const type = arr[0].replace("data:", "").replace(";base64", "");
      let n = bstr.length,
          u8arr = new Uint8Array(n);

      while (n--) {
        u8arr[n] = bstr.charCodeAt(n);
      }

      return new File([u8arr], 'file', {
        type
      });
    }
    function downloadImg(content, fileName) {
      const aLink = document.createElement("a");
      aLink.download = fileName;
      aLink.href = URL.createObjectURL(content);
      aLink.click();
      URL.revokeObjectURL(content);
    }
    function now() {
      return new Date().getTime();
    }
    (() => {
      try {
        if (typeof WebAssembly === "object" && typeof WebAssembly.instantiate === "function") {
          const module = new WebAssembly.Module(Uint8Array.of(0x0, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00));
          if (module instanceof WebAssembly.Module) return new WebAssembly.Instance(module) instanceof WebAssembly.Instance;
        }
      } catch (e) {}

      return false;
    })();
    function clamp(num, a, b) {
      return Math.max(Math.min(num, Math.max(a, b)), Math.min(a, b));
    }
    function getNowTime() {
      if (performance && typeof performance.now === 'function') {
        return performance.now();
      }

      return Date.now();
    }
    function calculationRate(callback) {
      let totalSize = 0;
      let lastTime = getNowTime();
      return size => {
        totalSize += size;
        const thisTime = getNowTime();
        const diffTime = thisTime - lastTime;

        if (diffTime >= 1000) {
          callback(totalSize / diffTime * 1000);
          lastTime = thisTime;
          totalSize = 0;
        }
      };
    }

    function supportWCS() {
      return "VideoEncoder" in window;
    }
    function formatVideoDecoderConfigure(avcC) {
      let codecArray = avcC.subarray(1, 4);
      let codecString = "avc1.";

      for (let j = 0; j < 3; j++) {
        let h = codecArray[j].toString(16);

        if (h.length < 2) {
          h = "0" + h;
        }

        codecString += h;
      }

      return {
        codec: codecString,
        description: avcC
      };
    }
    function isFullScreen() {
      return document.isFullScreen || document.mozIsFullScreen || document.webkitIsFullScreen;
    }

    class Emitter {
      on(name, fn, ctx) {
        const e = this.e || (this.e = {});
        (e[name] || (e[name] = [])).push({
          fn,
          ctx
        });
        return this;
      }

      once(name, fn, ctx) {
        const self = this;

        function listener(...args) {
          self.off(name, listener);
          fn.apply(ctx, args);
        }

        listener._ = fn;
        return this.on(name, listener, ctx);
      }

      emit(name, ...data) {
        const evtArr = ((this.e || (this.e = {}))[name] || []).slice();

        for (let i = 0; i < evtArr.length; i += 1) {
          evtArr[i].fn.apply(evtArr[i].ctx, data);
        }

        return this;
      }

      off(name, callback) {
        const e = this.e || (this.e = {});

        if (!name) {
          Object.keys(e).forEach(key => {
            delete e[key];
          });
          delete this.e;
          return;
        }

        const evts = e[name];
        const liveEvents = [];

        if (evts && callback) {
          for (let i = 0, len = evts.length; i < len; i += 1) {
            if (evts[i].fn !== callback && evts[i].fn._ !== callback) liveEvents.push(evts[i]);
          }
        }

        if (liveEvents.length) {
          e[name] = liveEvents;
        } else {
          delete e[name];
        }

        return this;
      }

    }

    var createWebGL = (gl => {
      var vertexShaderScript = ['attribute vec4 vertexPos;', 'attribute vec4 texturePos;', 'varying vec2 textureCoord;', 'void main()', '{', 'gl_Position = vertexPos;', 'textureCoord = texturePos.xy;', '}'].join('\n');
      var fragmentShaderScript = ['precision highp float;', 'varying highp vec2 textureCoord;', 'uniform sampler2D ySampler;', 'uniform sampler2D uSampler;', 'uniform sampler2D vSampler;', 'const mat4 YUV2RGB = mat4', '(', '1.1643828125, 0, 1.59602734375, -.87078515625,', '1.1643828125, -.39176171875, -.81296875, .52959375,', '1.1643828125, 2.017234375, 0, -1.081390625,', '0, 0, 0, 1', ');', 'void main(void) {', 'highp float y = texture2D(ySampler,  textureCoord).r;', 'highp float u = texture2D(uSampler,  textureCoord).r;', 'highp float v = texture2D(vSampler,  textureCoord).r;', 'gl_FragColor = vec4(y, u, v, 1) * YUV2RGB;', '}'].join('\n');
      var vertexShader = gl.createShader(gl.VERTEX_SHADER);
      gl.shaderSource(vertexShader, vertexShaderScript);
      gl.compileShader(vertexShader);

      if (!gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS)) {
        console.log('Vertex shader failed to compile: ' + gl.getShaderInfoLog(vertexShader));
      }

      var fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
      gl.shaderSource(fragmentShader, fragmentShaderScript);
      gl.compileShader(fragmentShader);

      if (!gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS)) {
        console.log('Fragment shader failed to compile: ' + gl.getShaderInfoLog(fragmentShader));
      }

      var program = gl.createProgram();
      gl.attachShader(program, vertexShader);
      gl.attachShader(program, fragmentShader);
      gl.linkProgram(program);

      if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
        console.log('Program failed to compile: ' + gl.getProgramInfoLog(program));
      }

      gl.useProgram(program);
      var vertexPosBuffer = gl.createBuffer();
      gl.bindBuffer(gl.ARRAY_BUFFER, vertexPosBuffer);
      gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([1, 1, -1, 1, 1, -1, -1, -1]), gl.STATIC_DRAW);
      var vertexPosRef = gl.getAttribLocation(program, 'vertexPos');
      gl.enableVertexAttribArray(vertexPosRef);
      gl.vertexAttribPointer(vertexPosRef, 2, gl.FLOAT, false, 0, 0);
      var texturePosBuffer = gl.createBuffer();
      gl.bindBuffer(gl.ARRAY_BUFFER, texturePosBuffer);
      gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([1, 0, 0, 0, 1, 1, 0, 1]), gl.STATIC_DRAW);
      var texturePosRef = gl.getAttribLocation(program, 'texturePos');
      gl.enableVertexAttribArray(texturePosRef);
      gl.vertexAttribPointer(texturePosRef, 2, gl.FLOAT, false, 0, 0);

      function _initTexture(name, index) {
        var textureRef = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, textureRef);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.bindTexture(gl.TEXTURE_2D, null);
        gl.uniform1i(gl.getUniformLocation(program, name), index);
        return textureRef;
      }

      var yTextureRef = _initTexture('ySampler', 0);

      var uTextureRef = _initTexture('uSampler', 1);

      var vTextureRef = _initTexture('vSampler', 2);

      return {
        render: function (w, h, y, u, v) {
          gl.viewport(0, 0, w, h);
          gl.activeTexture(gl.TEXTURE0);
          gl.bindTexture(gl.TEXTURE_2D, yTextureRef);
          gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, w, h, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, y);
          gl.activeTexture(gl.TEXTURE1);
          gl.bindTexture(gl.TEXTURE_2D, uTextureRef);
          gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, w / 2, h / 2, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, u);
          gl.activeTexture(gl.TEXTURE2);
          gl.bindTexture(gl.TEXTURE_2D, vTextureRef);
          gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, w / 2, h / 2, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, v);
          gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
        },
        destroy: function () {
          try {
            gl.deleteProgram(program);
            gl.deleteBuffer(vertexPosBuffer);
            gl.deleteBuffer(texturePosBuffer);
            gl.deleteTexture(yTextureRef);
            gl.deleteTexture(uTextureRef);
            gl.deleteBuffer(vTextureRef);
          } catch (e) {// console.error(e);
          }
        }
      };
    });

    class CanvasVideoLoader extends Emitter {
      constructor(player) {
        super();
        this.player = player;
        const $canvasElement = document.createElement("canvas");
        $canvasElement.style.position = "absolute";
        $canvasElement.style.top = 0;
        $canvasElement.style.left = 0;
        this.$videoElement = $canvasElement;
        player.$container.appendChild(this.$videoElement);
        this.contextGl = null;
        this.contextGlRender = null;
        this.contextGlDestroy = null;
        this.bitmaprenderer = null;
        this.videoInfo = {
          width: '',
          height: '',
          encType: '',
          encTypeCode: ''
        };
        this.player.debug.log('CanvasVideo', 'init');
      } //


      updateVideoInfo(data) {
        if (data.encTypeCode) {
          this.videoInfo.encTypeCode = data.encTypeCode;
          this.videoInfo.encType = VIDEO_ENC_TYPE[data.encTypeCode];
        }

        if (data.width) {
          this.videoInfo.width = data.width;
        }

        if (data.height) {
          this.videoInfo.height = data.height;
        }
      }

      _initContextGl() {
        this.contextGl = createContextGL(this.$videoElement);
        const webgl = createWebGL(this.contextGl);
        this.contextGlRender = webgl.render;
        this.contextGlDestroy = webgl.destroy;
      }

      initCanvasViewSize() {
        this.$videoElement.width = this.videoInfo.width;
        this.$videoElement.height = this.videoInfo.height;

        if (!this.player._opt.useWCS) {
          this._initContextGl();
        }

        this.resize();
      }

      _supportOffscreen() {
        return supportOffscreen(this.$videoElement) && this.player._opt.useOffscreen;
      } //


      bindOffscreen() {
        if (this._supportOffscreen()) {
          this.bitmaprenderer = this.$videoElement.getContext('bitmaprenderer');
        }
      }

      render(msg) {
        if (this._supportOffscreen()) {
          this.bitmaprenderer && this.bitmaprenderer.transferFromImageBitmap(msg.buffer);
        } else if (this.player._opt.useWCS) {
          this.$videoElement.drawImage(msg.videoFrame, 0, 0, this.$videoElement.width, this.$videoElement.height);
        } else {
          this.contextGlRender(this.$videoElement.width, this.$videoElement.height, msg.output[0], msg.output[1], msg.output[2]);
        }
      }

      screenshot(filename, format, quality, type) {
        filename = filename || now();
        type = type || SCREENSHOT_TYPE.download;
        const formatType = {
          png: 'image/png',
          jpeg: 'image/jpeg',
          webp: 'image/webp'
        };
        let encoderOptions = 0.92;

        if (!formatType[format] && SCREENSHOT_TYPE[format]) {
          type = format;
          format = 'png';
          quality = undefined;
        }

        if (typeof quality === "string") {
          type = quality;
          quality = undefined;
        }

        if (typeof quality !== 'undefined') {
          encoderOptions = Number(quality);
        }

        const dataURL = this.$videoElement.toDataURL(formatType[format] || formatType.png, encoderOptions);
        const file = dataURLToFile(dataURL);

        if (type === SCREENSHOT_TYPE.base64) {
          return dataURL;
        } else if (type === SCREENSHOT_TYPE.blob) {
          return file;
        } else if (type === SCREENSHOT_TYPE.download) {
          downloadImg(file, filename);
        }
      } //


      clearView() {
        if (this.contextGl) {
          this.contextGl.clear(this.contextGl.COLOR_BUFFER_BIT);
        } else if (this.player._opt.useWCS) ;
      }

      resize() {
        this.player.debug.log('canvasVideo', 'resize');
        const width = this.player.width;
        let height = this.player.height;

        if (this.player._opt.hasControl) {
          height -= CONTROL_HEIGHT;
        }

        const resizeWidth = this.$videoElement.width;
        const resizeHeight = this.$videoElement.height;
        const wScale = width / resizeWidth;
        const hScale = height / resizeHeight;
        let scale = wScale > hScale ? hScale : wScale; //

        if (!this.player._opt.isResize) {
          if (wScale !== hScale) {
            scale = wScale + ',' + hScale;
          }
        } //


        if (this.player._opt.isFullResize) {
          scale = wScale > hScale ? wScale : hScale;
        }

        let transform = "scale(" + scale + ")";
        this.$videoElement.style.transform = transform;
        this.$videoElement.style.left = (width - resizeWidth) / 2 + "px";
        this.$videoElement.style.top = (height - resizeHeight) / 2 + "px";
      }

      destroy() {
        if (this.contextGl) {
          this.contextGl = null;
        }

        if (this.contextGlRender) {
          this.contextGlDestroy && this.contextGlDestroy();
          this.contextGlRender = null;
        }

        if (this.bitmaprenderer) {
          this.bitmaprenderer = null;
        }

        this.videoInfo = {
          width: '',
          height: '',
          encType: '',
          encTypeCode: ''
        };
        this.player.$container.removeChild(this.$videoElement);
        this.player.debug.log(`CanvasVideoLoader`, 'destroy');
      }

    }

    class Video {
      constructor(player) {
        const Loader = Video.getLoaderFactory();
        return new Loader(player);
      }

      static getLoaderFactory() {
        return CanvasVideoLoader;
      }

    }

    class AudioContextLoader extends Emitter {
      constructor(player) {
        super();
        this.bufferList = [];
        this.player = player;
        this.scriptNode = null;
        this.hasInitScriptNode = false;
        this.audioContextChannel = null;
        this.audioContext = new (window.AudioContext || window.webkitAudioContext)(); //

        this.gainNode = this.audioContext.createGain(); // Get an AudioBufferSourceNode.
        // This is the AudioNode to use when we want to play an AudioBuffer

        const source = this.audioContext.createBufferSource(); // set the buffer in the AudioBufferSourceNode

        source.buffer = this.audioContext.createBuffer(1, 1, 22050); // connect the AudioBufferSourceNode to the
        // destination so we can hear the sound

        source.connect(this.audioContext.destination); // noteOn as start
        // start the source playing

        if (source.noteOn) {
          source.noteOn(0);
        } else {
          source.start(0);
        }

        this.audioBufferSourceNode = source; //

        this.mediaStreamAudioDestinationNode = this.audioContext.createMediaStreamDestination(); //

        this.audioEnabled(true); // default setting 0

        this.gainNode.gain.value = 0;
        this.playing = false;
        this.audioInfo = {
          encType: '',
          encTypeCode: '',
          channels: '',
          sampleRate: ''
        };
        this.player.debug.log('AudioContext', 'init');
      }

      updateAudioInfo(data) {
        if (data.encTypeCode) {
          this.audioInfo.encTypeCode = data.encTypeCode;
          this.audioInfo.encType = AUDIO_ENC_TYPE[data.encTypeCode];
        }

        if (data.channels) {
          this.audioInfo.channels = data.channels;
        }

        if (data.sampleRate) {
          this.audioInfo.sampleRate = data.sampleRate;
        }
      } //


      get isPlaying() {
        return this.playing;
      }

      get isMute() {
        return this.gainNode.gain.value === 0 && this.isStateSuspended();
      }

      get volume() {
        return this.gainNode.gain.value;
      }

      get bufferSize() {
        return this.bufferList.length;
      }

      initScriptNode() {
        if (this.hasInitScriptNode) {
          return;
        }

        const channels = this.audioInfo.channels;
        const scriptNode = this.audioContext.createScriptProcessor(1024, 0, channels);
        this.playing = true;

        scriptNode.onaudioprocess = audioProcessingEvent => {
          const outputBuffer = audioProcessingEvent.outputBuffer;

          if (this.bufferList.length && this.playing) {
            const buffer = this.bufferList.shift();

            for (let channel = 0; channel < channels; channel++) {
              const b = buffer[channel];
              const nowBuffering = outputBuffer.getChannelData(channel);

              for (let i = 0; i < 1024; i++) {
                nowBuffering[i] = b[i] || 0;
              }
            }
          }
        };

        scriptNode.connect(this.gainNode);
        this.scriptNode = scriptNode;
        this.gainNode.connect(this.audioContext.destination);
        this.gainNode.connect(this.mediaStreamAudioDestinationNode);
        this.hasInitScriptNode = true;
      }

      mute(flag) {
        if (flag) {
          this.setVolume(0);
          this.audioEnabled(false);
          this.clear();
        } else {
          this.audioEnabled(true);
        }
      }

      setVolume(volume) {
        volume = parseFloat(volume).toFixed(2);

        if (isNaN(volume)) {
          return;
        }

        this.audioEnabled(true);
        volume = clamp(volume, 0, 1);
        this.gainNode.gain.value = volume;
        this.gainNode.gain.setValueAtTime(volume, this.audioContext.currentTime);
        this.player.emit(EVENTS.volumechange, this.player.volume);
      }

      closeAudio() {
        if (this.hasInitScriptNode) {
          this.scriptNode && this.scriptNode.disconnect(this.gainNode);
          this.gainNode && this.gainNode.disconnect(this.audioContext.destination);
          this.gainNode && this.gainNode.disconnect(this.mediaStreamAudioDestinationNode);
        }

        this.clear();
      } // 是否播放。。。


      audioEnabled(flag) {
        if (flag) {
          if (this.audioContext.state === 'suspended') {
            // resume
            this.audioContext.resume();
          }
        } else {
          if (this.audioContext.state === 'running') {
            // suspend
            this.audioContext.suspend();
          }
        }
      }

      isStateRunning() {
        return this.audioContext.state === 'running';
      }

      isStateSuspended() {
        return this.audioContext.state === 'suspended';
      }

      clear() {
        // 全部清空。
        while (this.bufferList.length) {
          this.bufferList.shift();
        }
      }

      play(buffer, ts) {
        this.bufferList.push(buffer);
      }

      pause() {
        this.playing = false;
        this.clear();
      }

      resume() {
        this.playing = true;
      }

      destroy() {
        this.closeAudio();
        this.audioContext.close();
        this.audioContext = null;
        this.gainNode = null;

        if (this.scriptNode) {
          this.scriptNode.onaudioprocess = noop;
          this.scriptNode = null;
        }

        this.audioBufferSourceNode = null;
        this.mediaStreamAudioDestinationNode = null;
        this.hasInitScriptNode = false;
      }

    }

    class Audio {
      constructor(player) {
        const Loader = Audio.getLoaderFactory();
        return new Loader(player);
      }

      static getLoaderFactory() {
        return AudioContextLoader;
      }

    }

    class FetchLoader extends Emitter {
      constructor(player) {
        super();
        this.player = player;
        this.playing = false;
        this.abortController = new AbortController(); //

        this.streamRate = calculationRate(rate => {
          player.emit(EVENTS.streamRate, rate);
        });
        player.debug.log('FetchStream', 'init');
      }

      fetchStream(url) {
        const {
          demux
        } = this.player;
        fetch(url, {
          signal: this.abortController.signal
        }).then(res => {
          const reader = res.body.getReader();
          this.emit(EVENTS.streamSuccess);

          const fetchNext = () => {
            reader.read().then(({
              done,
              value
            }) => {
              if (done) {
                demux.close();
              } else {
                this.streamRate(value.byteLength);
                this.playing = true;
                demux.dispatch(value);
                fetchNext();
              }
            }).catch(e => {
              demux.close();
              this.playing = false; // 这边会报用户 aborted a request 错误。

              this.emit(EVENTS_ERROR.fetchError, e);
            });
          };

          fetchNext();
        }).catch(e => {
          this.emit(EVENTS_ERROR.fetchError, e);
        });
      }

      destroy() {
        if (this.abortController) {
          this.abortController.abort();
          this.abortController = null;
        }

        this.streamRate = null;
      }

    }

    class WebsocketLoader extends Emitter {
      constructor(player) {
        super();
        this.player = player;
        this.socket = null;
        this.socketStatus = WEBSOCKET_STATUS.notConnect;
        this.playing = false;
        this.wsUrl = null; //

        this.streamRate = calculationRate(rate => {
          player.emit(EVENTS.streamRate, rate);
        });
      }

      _createWebSocket() {
        const player = this.player;
        const {
          debug,
          events: {
            proxy
          },
          demux
        } = player;
        this.socket = new WebSocket(this.wsUrl);
        this.socket.binaryType = 'arraybuffer';
        proxy(this.socket, 'open', () => {
          this.emit(EVENTS.streamSuccess);
          debug.log('websocketLoader', 'socket open');
          this.socketStatus = WEBSOCKET_STATUS.open;
        });
        proxy(this.socket, 'message', event => {
          const message = new Uint8Array(event.data);
          this.streamRate(message.byteLength);

          this._handleMessage(message);
        });
        proxy(this.socket, 'close', () => {
          debug.log('websocketLoader', 'socket close');
          this.emit(EVENTS.streamEnd);
          this.socketStatus = WEBSOCKET_STATUS.close;
        });
        proxy(this.socket, 'error', error => {
          debug.log('websocketLoader', 'socket error');
          this.emit(EVENTS_ERROR.websocketError, error);
          this.socketStatus = WEBSOCKET_STATUS.error;
          demux.close();
          debug.log('websocketLoader', `socket error:`, error);
        });
      } //


      _handleMessage(message) {
        const {
          demux
        } = this.player;
        demux.dispatch(message);
      }

      fetchStream(url) {
        this.wsUrl = url;

        this._createWebSocket();
      }

      destroy() {
        if (this.socket) {
          this.socket.close();
          this.socket = null;
        }

        this.socketStatus = WEBSOCKET_STATUS.notConnect;
        this.playing = false;
        this.off();
        this.player.debug.log('websocketLoader', 'destroy');
      }

    }

    class Stream {
      constructor(player) {
        const Loader = Stream.getLoaderFactory(player._opt.protocol);
        return new Loader(player);
      }

      static getLoaderFactory(protocol) {
        if (protocol === PLAYER_PLAY_PROTOCOL.fetch) {
          return FetchLoader;
        } else if (protocol === PLAYER_PLAY_PROTOCOL.websocket) {
          return WebsocketLoader;
        }
      }

    }

    class RecordRTCLoader {
      constructor(player) {
        this.player = player;
        this.recording = false;
      }

      setFileName() {}

      startRecord() {}

      stopRecordAndSave() {}

      destroy() {}

    }

    class Recorder {
      constructor(player) {
        const Loader = Recorder.getLoaderFactory();
        return new Loader(player);
      }

      static getLoaderFactory() {
        return RecordRTCLoader;
      }

    }

    class DecoderWorker {
      constructor(player) {
        this.player = player;
        this.decoderWorker = new Worker(player._opt.decoder);
        const {
          debug,
          events: {
            proxy
          }
        } = player;

        this.decoderWorker.onmessage = event => {
          const msg = event.data;

          switch (msg.cmd) {
            case WORKER_CMD_TYPE.init:
              debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.init);
              this.player.emit(EVENTS.load);
              this.player.emit(EVENTS.decoderWorkerInit);

              this._initWork();

              break;

            case WORKER_CMD_TYPE.videoCode:
              this.player.video.updateVideoInfo({
                encTypeCode: msg.code
              });
              break;

            case WORKER_CMD_TYPE.audioCode:
              this.player.audio.updateAudioInfo({
                encTypeCode: msg.code
              });
              break;

            case WORKER_CMD_TYPE.initVideo:
              debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.initVideo, `width:${msg.w},height:${msg.h}`);
              this.player.video.updateVideoInfo({
                width: msg.w,
                height: msg.h
              });
              this.player.video.initCanvasViewSize();
              this.player.video.bindOffscreen();
              break;

            case WORKER_CMD_TYPE.initAudio:
              debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.initAudio, `channels:${msg.channels},sampleRate:${msg.sampleRate}`);
              this.player.audio.updateAudioInfo(msg);
              this.player.audio.initScriptNode(msg);
              break;

            case WORKER_CMD_TYPE.render:
              // debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.render, `msg ts:${msg.ts}`);
              if (this.player.loading) {
                this.player.emit(EVENTS.frameStart);
                this.player.loading = false;
              }

              if (!this.player.playing) {
                this.player.playing = true;
              }

              this.player.video.render(msg);
              break;

            case WORKER_CMD_TYPE.playAudio:
              // debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.playAudio, `msg ts:${msg.ts}`);
              // 只有在 playing 的时候。
              if (this.player.playing) {
                this.player.audio.play(msg.buffer, msg.ts);
              }

              break;

            default:
              player[msg.cmd] && player[msg.cmd](msg);
          }
        };

        player.debug.log('decoderWorker', 'init');
      }

      _initWork() {
        this.decoderWorker.postMessage({
          cmd: WORKER_SEND_TYPE.init,
          opt: JSON.stringify(this.player._opt),
          sampleRate: this.player.audio.audioContext.sampleRate
        });
      }

      decodeVideo(arrayBuffer, ts, isIFrame) {
        const options = {
          type: MEDIA_TYPE.video,
          ts: Math.max(ts, 0),
          isIFrame
        }; // this.player.debug.log('decoderWorker', 'decodeVideo');

        this.decoderWorker.postMessage({
          cmd: WORKER_SEND_TYPE.decode,
          buffer: arrayBuffer,
          options
        }, [arrayBuffer.buffer]);
      } //


      decodeAudio(arrayBuffer, ts) {
        const options = {
          type: MEDIA_TYPE.audio,
          ts: Math.max(ts, 0)
        }; // this.player.debug.log('decoderWorker', 'decodeAudio');

        this.decoderWorker.postMessage({
          cmd: WORKER_SEND_TYPE.decode,
          buffer: arrayBuffer,
          options
        }, [arrayBuffer.buffer]);
      }

      destroy() {
        this.player.debug.log(`decoderWorker`, 'destroy');
        this.decoderWorker.postMessage({
          cmd: WORKER_SEND_TYPE.close
        });
        this.decoderWorker.terminate();
        this.decoderWorker = null;
        this.player = null;
      }

    }

    class FlvLoader {
      constructor(player) {
        this.player = player;

        const input = this._inputFlv();

        this.flvDemux = this.dispatchFlvData(input);
        player.debug.log('FlvDemux', 'init');
      }

      dispatch(data) {
        this.flvDemux(data);
      }

      *_inputFlv() {
        yield 9;
        const tmp = new ArrayBuffer(4);
        const tmp8 = new Uint8Array(tmp);
        const tmp32 = new Uint32Array(tmp);
        const player = this.player;
        const {
          decoderWorker,
          webcodecsDecoder
        } = player;

        while (true) {
          tmp8[3] = 0;
          const t = yield 15;
          const type = t[4];
          tmp8[0] = t[7];
          tmp8[1] = t[6];
          tmp8[2] = t[5];
          const length = tmp32[0];
          tmp8[0] = t[10];
          tmp8[1] = t[9];
          tmp8[2] = t[8];
          let ts = tmp32[0];

          if (ts === 0xFFFFFF) {
            tmp8[3] = t[11];
            ts = tmp32[0];
          }

          const payload = yield length;

          switch (type) {
            case FLV_MEDIA_TYPE.audio:
              if (player._opt.hasAudio) {
                decoderWorker.decodeAudio(payload, ts);
              }

              break;

            case FLV_MEDIA_TYPE.video:
              if (player._opt.hasVideo) {
                const isIframe = payload[0] >> 4 === 1;

                if (player._opt.useWCS) {
                  webcodecsDecoder.decodeVideo(payload, ts, isIframe);
                } else {
                  decoderWorker.decodeVideo(payload, ts, isIframe);
                }
              }

              break;
          }
        }
      }

      dispatchFlvData(input) {
        let need = input.next();
        let buffer = null;
        return value => {
          let data = new Uint8Array(value);

          if (buffer) {
            let combine = new Uint8Array(buffer.length + data.length);
            combine.set(buffer);
            combine.set(data, buffer.length);
            data = combine;
            buffer = null;
          }

          while (data.length >= need.value) {
            let remain = data.slice(need.value);
            need = input.next(data.slice(0, need.value));
            data = remain;
          }

          if (data.length > 0) {
            buffer = data;
          }
        };
      }

      close() {}

      destroy() {}

    }

    class M7sLoader {
      constructor(player) {
        this.player = player;
        player.debug.log('M7sDemux', 'init');
      }

      dispatch(data) {
        const player = this.player;
        const {
          decoderWorker
        } = player;
        const dv = new DataView(data);
        const type = dv.getUint8(0);
        const ts = dv.getUint32(1, false);

        switch (type) {
          case MEDIA_TYPE.audio:
            if (player._opt.hasAudio) {
              const payload = new Uint8Array(data, 5);
              decoderWorker.decodeAudio(payload, ts);
            }

            break;

          case MEDIA_TYPE.video:
            if (player._opt.hasVideo) {
              if (dv.byteLength > 5) {
                const payload = new Uint8Array(data, 5);
                const isIframe = dv.getUint8(5) >> 4 === 1;
                decoderWorker.decodeVideo(payload, ts, isIframe);
              }
            }

            break;
        }
      }

      close() {}

      destroy() {}

    }

    class Demux {
      constructor(player) {
        const Loader = Demux.getLoaderFactory(player._opt.demuxType);
        return new Loader(player);
      }

      static getLoaderFactory(type) {
        if (type === DEMUX_TYPE.m7s) {
          return M7sLoader;
        } else if (type === DEMUX_TYPE.flv) {
          return FlvLoader;
        }
      }

    }

    class WebcodecsDecoder extends Emitter {
      constructor(player) {
        super();
        this.player = player;
        this.hasInit = false;
        this.isInitInfo = false;
        this.decoder = null;
        this.initDecoder();
      }

      initDecoder() {
        const _this = this;

        this.decoder = new VideoDecoder({
          output(videoFrame) {
            _this.handleDecode(videoFrame);
          },

          error(error) {
            _this.handleError(error);
          }

        });
      }

      handleDecode(videoFrame) {
        if (!this.isInitInfo) {
          this.player.video.updateVideoInfo({
            width: videoFrame.codedWidth,
            height: videoFrame.codedHeight
          });
          this.player.video.initCanvasViewSize();
          this.isInitInfo = true;
        }

        this.player.video.render({
          videoFrame
        }); // release resource

        setTimeout(function () {
          if (videoFrame.close) {
            videoFrame.close();
          } else {
            videoFrame.destroy();
          }
        }, 100);
      }

      handleError(error) {}

      decodeVideo(payload, ts, isIframe) {
        if (!this.hasInit) {
          if (isIframe && payload[1] === 0) {
            const videoCodec = payload[0] & 0x0F;
            this.player.emit(EVENTS.videoInfo, {
              encTypeCode: videoCodec
            });
            const config = formatVideoDecoderConfigure(payload.slice(5));
            this.decoder.configure(config);
            this.hasInit = true;
          }
        } else {
          const chunk = new EncodedVideoChunk({
            data: payload.slice(5),
            timestamp: ts,
            type: isIframe ? 'key' : 'delta'
          });
          this.decoder.decode(chunk);
        }
      }

      destroy() {
        this.decoder.close();
        this.decoder = null;
      }

    }

    const iconsMap = {
      play: '播放',
      pause: '暂停',
      audio: '',
      mute: '',
      screenshot: '截图',
      loading: '加载',
      fullscreen: '全屏',
      fullscreenExit: '退出全屏',
      record: '录制',
      recordStop: '停止录制'
    };
    var icons = Object.keys(iconsMap).reduce((icons, key) => {
      icons[key] = `
    <i class="jessibuca-icon jessibuca-icon-${key}"></i>
    <span class="icon-title-tips"><span class="icon-title">${iconsMap[key]}</span></span>
`;
      return icons;
    }, {});

    var template = ((player, control) => {
      player.$container.classList.add('jessibuca-controls-show');
      const options = player._opt;
      player.$container.insertAdjacentHTML('beforeend', `
            ${options.poster ? `<div class="jessibuca-poster" style="background-image: url(${options.poster})"></div>` : ''}
            <div class="jessibuca-loading">${icons.loading}</div>
            <div class="jessibuca-controls">
                <div class="jessibuca-controls-bottom">
                    <div class="jessibuca-controls-left">
                        <div class="jessibuca-controls-item"></div>
                    </div>
                    <div class="jessibuca-controls-right">
                         <div class="jessibuca-controls-item jessibuca-volume">
                         ${icons.audio}
                         ${icons.mute}
                         <div class="jessibuca-volume-panel-wrap">
                              <div class="jessibuca-volume-panel">
                                     <div class="jessibuca-volume-panel-handle"></div>
                              </div>
                         </div>
                         </div>
                         <div class="jessibuca-controls-item jessibuca-screenshot">${icons.screenshot}</div>
                         <div class="jessibuca-controls-item jessibuca-record">${icons.record}</div>
                         <div class="jessibuca-controls-item jessibuca-record-stop">${icons.recordStop}</div>
                         <div class="jessibuca-controls-item jessibuca-fullscreen">${icons.fullscreen}</div>
                         <div class="jessibuca-controls-item jessibuca-fullscreen-exit">${icons.fullscreenExit}</div>
                    </div>
                </div>
            </div>
        `);
    });

    var property = ((player, control) => {
      Object.defineProperty(control, 'controls', {
        get: () => {
          return player.$player.classList.contains('jessibuca-controls-show');
        },
        set: type => {
          if (type) {
            player.$player.classList.add('jessibuca-controls-show');
          } else {
            player.$player.classList.remove('jessibuca-controls-show');
          }
        }
      });
      Object.defineProperty(control, 'controlsRect', {
        get: () => {
          return control.$controls.getBoundingClientRect();
        }
      });
    });

    function styleInject(css, ref) {
      if ( ref === void 0 ) ref = {};
      var insertAt = ref.insertAt;

      if (!css || typeof document === 'undefined') { return; }

      var head = document.head || document.getElementsByTagName('head')[0];
      var style = document.createElement('style');
      style.type = 'text/css';

      if (insertAt === 'top') {
        if (head.firstChild) {
          head.insertBefore(style, head.firstChild);
        } else {
          head.appendChild(style);
        }
      } else {
        head.appendChild(style);
      }

      if (style.styleSheet) {
        style.styleSheet.cssText = css;
      } else {
        style.appendChild(document.createTextNode(css));
      }
    }

    var css_248z$1 = "@keyframes rotation{0%{-webkit-transform:rotate(0deg)}to{-webkit-transform:rotate(1turn)}}.jessibuca-container .jessibuca-icon{cursor:pointer;width:16px;height:16px}.jessibuca-container .jessibuca-player .jessibuca-poster{position:absolute;z-index:10;left:0;top:0;right:0;bottom:0;height:100%;width:100%;background-position:50%;background-repeat:no-repeat;background-size:contain;pointer-events:none}.jessibuca-container .jessibuca-player .jessibuca-loading{display:flex;justify-content:center;align-items:center;position:absolute;z-index:20;left:0;top:0;right:0;bottom:0;width:100%;height:100%;pointer-events:none}.jessibuca-container .jessibuca-player .jessibuca-controls{background-color:#161616;display:flex;flex-direction:column;justify-content:flex-end;position:absolute;z-index:40;left:0;right:0;bottom:0;height:40px;padding-left:30px;padding-right:13px;font-size:14px;color:#fff;opacity:0;visibility:hidden;transition:all .2s ease-in-out;-webkit-user-select:none;user-select:none}.jessibuca-container .jessibuca-player .jessibuca-controls .jessibuca-controls-item{position:relative;display:flex;justify-content:center;padding:0 17px}.jessibuca-container .jessibuca-player .jessibuca-controls .jessibuca-controls-item:hover .icon-title-tips{visibility:visible;opacity:1}.jessibuca-container .jessibuca-player .jessibuca-controls .jessibuca-fullscreen-exit,.jessibuca-container .jessibuca-player .jessibuca-controls .jessibuca-icon-mute,.jessibuca-container .jessibuca-player .jessibuca-controls .jessibuca-microphone-close,.jessibuca-container .jessibuca-player .jessibuca-controls .jessibuca-ptz-active,.jessibuca-container .jessibuca-player .jessibuca-controls .jessibuca-record-stop{display:none}.jessibuca-container .jessibuca-player .jessibuca-controls .jessibuca-icon-audio,.jessibuca-container .jessibuca-player .jessibuca-controls .jessibuca-icon-mute{z-index:1}.jessibuca-container .jessibuca-player .jessibuca-controls .jessibuca-controls-bottom{display:flex;justify-content:space-between;height:100%}.jessibuca-container .jessibuca-player .jessibuca-controls .jessibuca-controls-bottom .jessibuca-controls-left,.jessibuca-container .jessibuca-player .jessibuca-controls .jessibuca-controls-bottom .jessibuca-controls-right{display:flex;align-items:center}.jessibuca-container .jessibuca-player.jessibuca-controls-show .jessibuca-controls{opacity:1;visibility:visible}.jessibuca-container .jessibuca-player.jessibuca-hide-cursor *{cursor:none!important}.jessibuca-container.jessibuca-fullscreen-web{position:fixed;z-index:9999;left:0;top:0;right:0;bottom:0;width:100%!important;height:100%!important;background:#000}.jessibuca-container .jessibuca-icon-loading{width:50px;height:50px;background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%;animation:rotation 1s linear infinite}.jessibuca-container .jessibuca-icon-screenshot{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAE5UlEQVRoQ+1YW2sdVRT+1s7JxbsoVkEUrIIX0ouz15zYNA+N1RdtQfCltlUfvLbqL/BCwZ8grbHtizQqPojgBSr0JkiMmT2nxgapqBURtPVCq7HxJCeZJVPmxDlzZubMmXOSEsnAvOy917fXt9e39tp7E5b4R0vcfywTuNgRbBgBx3HuJqLVzPzmYjprjHkcwAlmLqXNm4XAISLaSESPaq2HF4OE67rbRGRYRA7btn1fbgLGmKsA/Azg0gBkGzO/vZAkHMd5hIiqc5wHcCMz/5k0Z2oExsfHV1QqldPAf8lORNu11m8tBAljzFYAYWxRSl1vWdZvuQj4RsYYF4AVBlgIOVVlE55HRIxt23ZuCfmGjuOsJ6LPoiAistW27XfaEYmIbOYhPc9bXywWR1oiEJDYQkR1zrYjEjGyqfqbKd8a7kJVtLgQ+30i8pht2wfyRKIdmJkJBPkQTbILfudJ7CTZNBvVpggEcgpvc/ML38zESbLJsxBNE/A9biX0rdjGyTQXgbxyapdsarb0PMlXtWnGoXbKpm0Essqp3bJpK4E0OXmed3+hUBDP8w5FI91M0rdcyLLILElOCbaZilSWeXMncRx4klTCY1spfG3dhZJWx3GcDUR0EEB3ZMw0ET2gtT6SZWWzjmlrBIJCl0hAKfWgZVmHszqXZVxbCSxpCS2JJA6umIhe8ZKKVLPbaBJ+S9toqVRa53nedgAbAKwIwH4FcAzAa0R0l4i8F7PPz189k6RFRA+LyNcAXojDV0oNW5b1eW4Cxpg9AHZkSaaa6hhzb065uDSCH2LmRB8Sk9gY4293g43Qo/1pV80m8yQMfZSZ781cB1zXHRKRZ2IMpgD8A+DamL4ZItqitX4/jbQx5iEA7wLoihn3V/ACckWMJN/QWj9b1x5tGBsbW6uUOh5pPy0iL3Z2dn6ilJqanp5ep5TaJSLhF4NppdRNaU8gPmapVLrO87yfIoXuWyJ6uVKp+HmFjo6OQSJ6FcBtYT+UUmstyxqvkWuUgDFmP4AnQu2/e563qlgs+u9DNZ8xZhRAX7VRRPbath0XuXk7Y8xeAE+FgL6fnJzsHRwcLIfBR0ZGLunq6poAsDLUvp+Zw7b1r9PGmJMAbg8Z7WDmoThZuK67WkS+DD18fcPMdzSQUBR/EzN/nIC/SUQ+DPXV4dclsTHmHAD/SfHCNzc3t7Kvr++HJKeMMacA3BL0nyuXyzcPDAxMxo0fHR29slAo/Ajg6qD/fE9Pzw29vb1/x42fmJi4vFwu+5G/LOg/y8zXNJLQ2dAES5JANMQ7mfn1jBI6ycx3NiMhItqstf4oAX+ziHwQ6qvDj5NQNIn/ALCKmX+JSeIvABRD7fuY+ekGBPYBeDI05tTMzExvf3+/vz2Hk91/ET8RSeI6/DoCpVJpjed5fmKGvzMAXpqdnT3oed5Ud3d3v4jsAqBr9Ei0Rmv9VRqBBPzvROQVETnq2xJRdRu9tRF+bCVOKWT+Kvl/TSIFk6SW/LAjKfjV5K8rZABi8dOOEv7FI7Z8x6zwEWbemLbyMfJr5qiSiJ96oclymBOR3bZtP9+M89WxxpjdAHY2sN3DzM8ljWl4I3Nd9x7/OE1ENcdpETnmH3e11n41zv0l4J8RkU+J6AAz+xtF4teQQG7PFslwmcAiLfSyhC72Qv9/I/Avns2OT7QJskoAAAAASUVORK5CYII=\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-screenshot:hover{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAED0lEQVRoQ+2ZycsdRRTFf2ejqHFAMQqiYBTUoElUHLNx3GgCgpuYRF2o0UT9CxwQ/BMkMSbZSKLiQgQHUDCJgjiAxiEiESdEcJbEedgcKaj3UV+/6q7u/jovPPkK3qbr1ql76p5bt6qemPKmKfefeQKHOoLFCNg+H1gi6fFJOmv7VmCvpD1N87Yh8ApwNXCzpB2TIGF7DRDm2inpmt4EbB8LfAMcGUHWSHryYJKwfRMwmuMP4BRJv9TN2RgB2wuB72BWsq+V9MTBIGF7NZBiGzhJ0o+9CIRBtt8FLqgADC6nRDbpVO9Iuqi3hCKB5cDrGZDVkp4aIhIV2aSQyyW9MScCkcQqIOfsnCORkc3I31b5VtyFRmg1IQ7dt0ja3icSQ2C2JhAjUU2ykd+dE7tBNp2i2olAJJFuc+nCt564QTadF6IzgUhiVGiqyinKaQjZpJP2ItBXTkPJZhACXeU0pGwGI9BWTkPLZlACBTldG4o5EA6E1dY66edcyNrs8Q36zg1vVaTazNs7iXPgDVJJzYs7VRvHRzaDEohyugJ4CTi84sg/wHWSdnVxsGQ7aQLXS9pZcqpL/6AEplpCU5HE8YpJ9YrXUKQ6baN1+HPaRm1fBqwFQnKGK2ZoPwCvAo8Ai4FnMpPMHMwapHUj8DFwbw3+Dklv9iZgexOwvktSRduxU2VDlErwmyXV+lCbxLbDdndlCT3TX3vV7JgnKfRuSVflfMkSsL0ZuDMz4E/gL+CETN+/wCpJzzaRtn0D8DRwWMbu1/gCcnSm7zFJd1W/jxGwvQx4r2IYnlbuA14GAomQFw8B6YtBKFSnNj2BxEJ3IvB1pdB9CjwQ8yqYhcg/DJxZ8WOZpA/SbzkC24DbEqOfgPMkBRKzmu23gEuSj1sk5SI3Y2J7C3BHMuZz4FxJf6fgto8APgIWJd+3SUrHjr9O294HnJUMWi8pSGqs2V4CvJ88fH0i6eyChKr4KyS9WIO/Ang+6RvDz0XgABCeFEdtkaQv65yy/QVweuwPY0+T9FuNQ8cAXwHHxf7wdHiypN9r7BfEl8GjYv9+SceXJLQ/mSDYTh2Baog3SHq0pYT2STqno4RWSnqhBn8l8FzSN4bfJol/jkn8bXUS228DFyfft0paVyCwFbg9sQkSDEkctueZZju8iO+tJPEYfo7A0piYKd73wP3xnB+20cvjNnphxdmlkj4sEMjhfwY8COyOY0fb6Bkl/K6FLKxS+M1KpDhJY8mvrG5doRwlf66QZfGbjhLh4pEt35kV3iUp/IvTunU8qtTil/7gaHOY2yjpntaez9b5RmBDYewmSXfX2RRvZLYvbThOh+NuqMa9Ww1+yLnXgO2SwkZR24oEens2oYHzBCa00PMSOtQL/f+NwH+Hg8hAnbrYgQAAAABJRU5ErkJggg==\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-record{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAC+UlEQVRoQ+1ZS2sTURT+zlDJYE3XSq219QHVuEjnJDT+Bff9Abqw2voAEfGxqygUqWhVFHGl/yMLu9BwByxk5SNI66ML6U7axjhHbmhgWiftncxoOiV3FcI53z3f/e65594zhIQPSnj86BBot4IdBToKRFyBnbeFlFIScVEiuYvIWC6Xe2YK8pcC7SYA4CMzH4mDQBXAqilQBDsLQLfPf9FxnF4i8kwwmypARI+Wl5dvmIBEsUmlUkNE9NaHsVCpVAZGR0d/m+A2JSAid3K53E0TkCg2pVKpz7KseR/GfKVSGYxMAMA0M1+JEpyJb6lUOm5ZVnkrAsVisaunp+esiByr1Wp3R0ZGvmifzZK4XQQWHMc52MgBpdQuAOcAXABwuB400ZTjONdaIjA7O5u2bVsnWU1EujzP+5nP5xdMVjvIJkCBD8x8VCm1G8AYgAkAAxt8Z5j5YmgCSqlTAJ4D2OcD/AXgATNfbYVEAIFPIvKKiE4D6GuCea8xX6gtpJT6DmBvECgRFRzHeROWRAABE4iWCbwHEFhkPM/L5vP5dyaz+23+KwHXdR3P854S0YG1ILSCuthNMfNM2OC1/RYENLY+ygcBnPfht6ZAA6BYLNr6dyqVokKhsGpaNQ2TWJstreXaE2aed133sojcj41AKyvdzCdAgSXLsk4MDw9/a/i4rntbRPxFNZoC/5jAV2be759DKTUJ4FZSFFi0bbs/k8noy2R9dAjEuWU2YgXkQOK3kD6BMsysi2Z9JC2Jdcw/ALzwPO+xvmcl7Rj177JVEbkO4BARjSflFDJJuW1dBxJPoCIiL4noDIB1BS0pW6j+oJmbm+uuVqvjRKQfLr0bZHnIzJf0f6HeAybahrUJqAPruhLlcnnPysqKfpXp11n/Gv62zoHAroS+AafT6QkiGrIsazKbzX7eVIHEt1US39gCkOzWYthkjNE+tuZujDGZQ8XRXn8N4KT5lLFZ6uaYPt+nwyDuvC80YdhvB9uOAu1WoaNAR4GIK/AHvdr+QAexB7EAAAAASUVORK5CYII=\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-record:hover{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAACfUlEQVRoQ+2ZSYsUQRCFvycK4nJXXEbHBdwO4kn/gv9CD467ICIutxEFkREdFUU86T/xojcPntyQcT2INw+uISFVkD1Wd2dWlU7nUHlqisiX+fJFZGREi8yHMt8/HYG5VrBToFOg4QnMPxcyM2t4KE2nT0i6EwvylwIjQOCFpE1tEPgGfI0FamC3AFgazP8IrJL0KwZzkAI3gLMxIA1ttgCPA4w3wHpJP2NwBxG4KOlcDEgTGzNbA8wEGP57vA0CU5JONtlczFwz2wY8HUbAzBYCB4CtwCVJb33OIAXmioC70LoyBsxsEXAQOApsLIhelnS6FgEzW+5BBvwA/FS+SPJFa40KBZ5L2mxmS4AJ4IjHxCzwaUnHkgmY2V7gLrAyAPwOXJN0qg6DCgIvgQfAPsDjo2pcKddLciEz+wCs6AO6W9KjVBIVBGIgahN4BvRLMjslPYlZPbT53wR2AbeBtcUmXEFPdh5U06mbd/shBBzbr/Jx4FCAX0+BEsDMFocEYrNmFcE+BD4XsXZL0oyZnQCutkagzkn3m1NBwDe/Q9L74MAuFEqUn5op8I8JvJO0elacTALnc1HAH3Njkvwx+WeYWUegTa/pwaqIgexdyIN4uyRPmqULZRXEvulPwD3gpr+zcrtGQxfzRHYG2AAczuUWiom3kc4D2RN4BdwH9gM9CS0XFyoLGu9UuN974eIFVDiuSzruH5LqgRhtU20q8kBPV8LMlhVVmVdnYwX+SMdAZVeieAF7eeltmElJr4cpkH1bJfvGVvatxdR4bMu+teZuWxtKxWncXn8I7EldtQV7vz79fp9KwZp//9CksB8F206BuVahU6BToOEJ/Ab7+KdABdTt8AAAAABJRU5ErkJggg==\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-recordStop{background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-recordStop:hover{background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-fullscreen{background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-fullscreen:hover{background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-fullscreenExit{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAADd0lEQVRoQ+2Zz2sdVRTHv+fJBDW6anDVXen6wZszYxYBiYgtFGst3VSDunKjpS0GpUlqfjVpsVVs6aaL0or4YxMVFCJZ2ZLdPUP+gq5bQnTxtNAkfTnlhnnlkmQy9yV9780rudt77tzv5/y4v4bQ4Y06XD/2ANodwec/AiJygJnvtdvTWfPnRkBEJAiCN8rl8kMfiPn5+Ve7u7v3rays0Orq6lJfX99/PuN2auMDoAD+BvA2M6/mTWSMOUtE48D6AjHGzN/kjdlNvy+AnWOOmQ/lTSYiEwDOWzsimgrDcCRvzG76GwGw8/zJzO9sN6GInAMwbW1UdSSKoqndCMwb6wNwGsB39Q+p6h/M/C4R2dTa1AoHYBWKyCkA1+pqiWi2Wq0e7e/vf7yRoJAAKcQggMtuJKIoOtoxACnE0/xOi/SXMAxPuhCFjUBdpIjYVWXSEf0TM3/g9BeriDMKdSPEz8z8vrU1xgwT0YXCrEJZy1iSJKOqOub0/8jMA0mSfKKqNwoPkHp7ioiGHIhRIvpHVa93BEBa2JcAfOlALAHo6RgAKzRJkk9V1S6xL7kpV4idOM31taxaIKJHqmpPnMMA9hcOQES2PDJkAT1XAAC+ZebPfWB3auNzmLObVsNRUNUXVHUujuM7OxXnMy4XwOcj29mIyOuq+lapVGrYCelKpkEQ3CyXy4tbzdN0AGPMxr2iYZ+sra3FcRybtgCIiK2BKw2rdgaUSqWoUqlIkQAepFDdAF7cBq5ERI9rtdr1OI7tmE2t6SmUEYFHAEaexYW/1QC2EF+ru5GIvg7D0D2GNJxprQY4o6qv1I/b6SpzOYqiLxpWng5oOQAzXxWRWwA+dkRfYOb1p5hGW6sBJpn5KytSRG4D+KguWFXHoyhy7xdeLC0F2ChSRL4H8OFuINoKYIUbY34gogHH3eeZef1K6tPaDpCm068A3nMEDzHzxY4BUNWSiPxORO6z5aDPPlGICNQ9bYyZIaLjjudzIQoFkKbTbwCO+UI0HcB9J/LdeY0xs0R02IGYYObRrWqiFQCfEZEtSHsfmGZm+4qxbbM/hQD8BeBNa0hEM2EYnmgLgP3lFARBT1dXly4vL//b29tbzQNIU+llAHeJaLFSqRzJes5vegR8xGbZLCwsHKzVav8z8/0sm0ID+MDvAfh4qZk2exFopnd9vv0ELrXBQO7fD10AAAAASUVORK5CYII=\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-fullscreenExit:hover{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAC/ElEQVRoQ+2Zy49NQRCHvx+ReK6IlZ34E7CUiCAR4xEbTLCyQRATYswwb2IQZDYWgojHZpCQECts+ResiQwLj0RClNSkb9Lu3HtPz7mZc8+V6eXt6tP1VVV3VdcVbT7U5vozC9BqD/7/HjCzlZLet9rS9fbP9ICZvQPWSfqRAmFmS4ClMHm+JiR9S1mXVyYFwIBXwEZJv7I2MrPjQH8A6JN0OWtNM/OpAL7HS0mbsjYzswGgN8gNS+rJWtPM/HQAfJ9nkrY22tDMTgMjQaZH0nAzCmatTQE4ClyNPvQU2CbJQ2vKKB2Aa2hmR4DrkbbPgQ5Jv6sJSgkQILqA0dgTkjraBiBAxPHtPz2UtDuGKK0HKkqamd8qg5HS9yXtjebLdYjrHNRqiAeS9gQvnQGGSnML1bvGzOwc0BfN35PUaWYHgRulBwjW9ju+O4JwqM/AWFsABIgLwKkIYgJY1jYAAeJQuGIXVIVcKTKxh8WfBin9J+AVpx/eFWUEqFkyNACKp0rhgWYArkg6kQibSyylmPOklQdibijBX+fSLHFRJkDid+qKmdlaYENOI0zeEcBNSZ9qbVIEQHWuyGOTNZLetgrAz8ClPFpHa1ZL8rf5lFGEB2oBfAxQi4D5DeDmAP7mGJPka0oD4LnDr9imH/xFe8AP4vLIjBclxWXItCOtaIBjwOKo3HaFRyWdnLbmYUHhAJKumdkt4ECk9JCkSitmWixFAwxKOjt5uZvdBvZH2vZLit8XSSBFA/yjpJndAfY1A9FSgOCJu0BnBNErqfIkzfRCywECxCNgR6Rtt6TzmdqHBmyKXG4ZM4sTWc04NzNPWE+AuG3ZlZInSuGBinXMbBzYGVkrE6JUACGcHgPbUyGKAIj7REmZ18y897o5ghiQ5E/bltRChwE/kF7Xj0jyLkbDYWbzgBfA+iA4LmlXqwD8LydvszjAF0lfswBCKC0E3gBeP22p186f8RBKUbaejJmtAr5L+lBPptQAKfCzAClWmkmZWQ/MpHVTvv0X9iFAQGQyevIAAAAASUVORK5CYII=\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-audio{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAACrUlEQVRoQ+2ZPYgTURCAZzbBXJnCeL2Cnb87b9MEtPBUrrMQFAtrtT5/ClGs9LBWWz0RtbBUFCF4oJDsbO68wsLA2YqQSmLlvpEHu7IuMdlLcus+yUKKhJfZ+ebnvZl5CJY/aLn+MAP41x7M1QPMfFtr/crzvHfTAs8FoNPp1LTWzwHgqIg0lFLvrQHwfX8BER8DwC6jNCIecF13wwoA3/dvIuKNpLJa60Oe560XGoCZd4rICiKeTCtaeABmPg4AJmRqg6xcaABmvg4At4aFRyEBhoVM4UMoCplHADCfJTEL5YEsIVNID5iQAYCHALCYxeq5b6PMfF5EBAAEESthGK7W6/XPRpFWq7W3VCqtZg2ZcT3g+/6i4zjzIlLSWn/yPO/DIGMNLCWY2Sj/+xGRK0qpZfNDEASnROTFVi0fr8+aA8z8Ld6KEfGt67oLYwMAwEUium8EREn7OgeAjwCwPyo/nrque3YSgAtE9GDaAM1mc65arc4Zuf1+P2w0Gt9jJZl5DQAORt+fENG5wgEw8zUAMB/zbBBRwyqAIAjuiMjlSOlNItpjFUCqWl0josMzgChR/9hGAWBbknjmAdPhDdqa0gfZzAMJKyVP4v8hhJYRcSni+0JEu63ahZj5anyQici6UuqIVQDdbrfS6/UqRulyufyTiH5sF8AlIro37VpoWEHIzGZ2tM+sEZFnSqkzk9RCS0R01wjIsZz+mug53hDRia0AnI4bGgDYISItz/M2jYC8Gpp2u30MEWuO4zha665Sqp0ZYFStX/iWchRAItFGzoHSsrJ2ZFl1mHg6bfVYJeGJv85CC++BpIJZ5kSFC6G0ha0e7mYJqcJ7IOkRay84UhD2XjHFIFZf8iW9YcYoYRi+tO6aNeupOs66iU/icV46zf/MAKZpzXFk/QL+JG1PUPhRiQAAAABJRU5ErkJggg==\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-audio:hover{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAACSElEQVRoQ+2Zu4sUQRCHf5+C+gf4yBXMfMYHGvjCzEBQDIzV+HwEohipGKupD0QNDE8UEwUFTe68wEDhTMVUMFJ+0tArzbjs9u3Ojt0wBR0M9MzUV1XdXVWNKhcq1189wP/2YKcesH1d0nPgdVvgnQDY3iTpqaT9kuaAt9UA2D4o6aGkzVHpXcByFQC2r0q60lB2D7BUNIDtjZIeSDoyRNGyAWwfiiET4n6YlAtg+7Kka2PCozyAMSHT5CkLIIbMfUlbMhdmOQCZIVOeB2LI3JN0NNPq6bTZe8D2aUmOY72kN8DnoIXt7eF5FSEzkQdsB+OEsFwr6RPwbpixhqYStoPyqVwAbkaAY5KeTWD5wStZHrD9XdJgK34FhBP9H8kFOAvciQBhn3/RAcBHSTvjfx4DJ6cBOAPcbRvA9gZJYQT5DfwYKGl7UdLu+PwIOFUiwCVJYQRZBuZqA7gh6XxUegXYVhtAmq0uAnt7gLhQm9vorBZx74Hcc6D3QLKH/z2JGyVnlYs4pCfzEe4rsLW2XehicpAtAftqAwiZbhhBfgE/ZwVwDrjddi40KiG0HXpHO+KcJ8CJaXKheeBWBOgqnf6W1BwvgcOrATieFDTrJL0HViJAVwXNgVgPrJH0BfiQDTDKtREiNK7KLSnHASQLLacP1PxcVkWWq8PU3emq2yqJJ0b1Qsv2QKpdZp+orBBqmrfq5m5mSJXtgUZI1XnB0YCo94opCal6L/ka3ghtlIXqrllzT9VJ5k19Ek/y0zbf6QHatOYk3/oDujC8QMWgjf4AAAAASUVORK5CYII=\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-mute{background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-mute:hover{background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-text{font-size:14px;width:30px}.jessibuca-container .jessibuca-quality-menu-list{position:absolute;left:50%;bottom:100%;visibility:hidden;opacity:0;transform:translateX(-50%);transition:visibility .3s,opacity .3s;background-color:rgba(0,0,0,.5);border-radius:4px}.jessibuca-container .jessibuca-quality-menu-list.jessibuca-quality-menu-shown{visibility:visible;opacity:1}.jessibuca-container .icon-title-tips{pointer-events:none;position:absolute;left:50%;bottom:100%;visibility:hidden;opacity:0;transform:translateX(-50%);transition:visibility .3s ease 0s,opacity .3s ease 0s;background-color:rgba(0,0,0,.5);border-radius:4px}.jessibuca-container .icon-title{display:inline-block;padding:5px 10px;font-size:12px;white-space:nowrap;color:#fff}.jessibuca-container .jessibuca-quality-menu{padding:8px 0}.jessibuca-container .jessibuca-quality-menu-item{display:block;height:25px;margin:0;padding:0 10px;cursor:pointer;font-size:14px;text-align:center;width:50px;color:hsla(0,0%,100%,.5);transition:color .3s,background-color .3s}.jessibuca-container .jessibuca-quality-menu-item:hover{background-color:hsla(0,0%,100%,.2)}.jessibuca-container .jessibuca-quality-menu-item:focus{outline:none}.jessibuca-container .jessibuca-quality-menu-item.jessibuca-quality-menu-item-active{color:#2298fc}.jessibuca-container .jessibuca-volume-panel-wrap{position:absolute;left:50%;bottom:100%;visibility:hidden;opacity:0;transform:translateX(-50%) translateY(30%);transition:visibility .3s,opacity .3s;background-color:rgba(0,0,0,.5);border-radius:4px;height:120px;width:50px;overflow:hidden}.jessibuca-container .jessibuca-volume-panel-wrap.jessibuca-volume-panel-wrap-show{visibility:visible;opacity:1}.jessibuca-container .jessibuca-volume-panel{cursor:pointer;position:absolute;top:15px;height:60px;width:50px;overflow:hidden}.jessibuca-container .jessibuca-volume-panel-handle{position:absolute;top:0;left:50%;width:12px;height:12px;border-radius:12px;margin-left:-6px;background:#fff}.jessibuca-container .jessibuca-volume-panel-handle:before{bottom:-54px;background:#fff}.jessibuca-container .jessibuca-volume-panel-handle:after{bottom:6px;background:hsla(0,0%,100%,.2)}.jessibuca-container .jessibuca-volume-panel-handle:after,.jessibuca-container .jessibuca-volume-panel-handle:before{content:\"\";position:absolute;display:block;left:50%;width:3px;margin-left:-1px;height:60px}\n/*# sourceMappingURL=data:application/json;base64,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Q3BDLGlCQUFvQixDQUV0QixpQ0FDRSxvQkFBcUIsQ0FDckIsZ0JBQWlCLENBQ2pCLGNBQWUsQ0FDZixrQkFBbUIsQ0FDbkIsVUFBYyxDQUVoQiw2Q0FDRSxhQUFnQixDQUVsQixrREFDRSxhQUFjLENBQ2QsV0FBWSxDQUNaLFFBQVMsQ0FDVCxjQUFlLENBQ2YsY0FBZSxDQUNmLGNBQWUsQ0FDZixpQkFBa0IsQ0FDbEIsVUFBVyxDQUNYLHdCQUErQixDQUMvQix5Q0FBaUQsQ0FDakQsd0RBQ0UsbUNBQTRDLENBQzlDLHdEQUNFLFlBQWUsQ0FDakIscUZBQ0UsYUFBZ0IsQ0FFcEIsa0RBQ0UsaUJBQWtCLENBQ2xCLFFBQVMsQ0FDVCxXQUFZLENBQ1osaUJBQWtCLENBQ2xCLFNBQVUsQ0FDViwwQ0FBMkMsQ0FDM0MscUNBQTJDLENBQzNDLCtCQUFvQyxDQUNwQyxpQkFBa0IsQ0FDbEIsWUFBYSxDQUNiLFVBQVcsQ0FDWCxlQUFrQixDQUNsQixtRkFDRSxrQkFBbUIsQ0FDbkIsU0FBWSxDQUVoQiw2Q0FDRSxjQUFlLENBQ2YsaUJBQWtCLENBQ2xCLFFBQVMsQ0FDVCxXQUFZLENBQ1osVUFBVyxDQUNYLGVBQWtCLENBRXBCLG9EQUNFLGlCQUFrQixDQUNsQixLQUFNLENBQ04sUUFBUyxDQUNULFVBQVcsQ0FDWCxXQUFZLENBQ1osa0JBQW1CLENBQ25CLGdCQUFpQixDQUNqQixlQUFrQixDQUNsQiwyREFDRSxZQUFhLENBQ2IsZUFBa0IsQ0FDcEIsMERBQ0UsVUFBVyxDQUNYLDZCQUFzQyxDQUN4QyxxSEFDRSxVQUFXLENBQ1gsaUJBQWtCLENBQ2xCLGFBQWMsQ0FDZCxRQUFTLENBQ1QsU0FBVSxDQUNWLGdCQUFpQixDQUNqQixXQUFjIiwiZmlsZSI6InN0eWxlLnNjc3MiLCJzb3VyY2VzQ29udGVudCI6WyJAa2V5ZnJhbWVzIHJvdGF0aW9uIHtcbiAgZnJvbSB7XG4gICAgLXdlYmtpdC10cmFuc2Zvcm06IHJvdGF0ZSgwZGVnKTsgfVxuICB0byB7XG4gICAgLXdlYmtpdC10cmFuc2Zvcm06IHJvdGF0ZSgzNjBkZWcpOyB9IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1pY29uIHtcbiAgY3Vyc29yOiBwb2ludGVyO1xuICB3aWR0aDogMTZweDtcbiAgaGVpZ2h0OiAxNnB4OyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtcGxheWVyIC5qZXNzaWJ1Y2EtcG9zdGVyIHtcbiAgcG9zaXRpb246IGFic29sdXRlO1xuICB6LWluZGV4OiAxMDtcbiAgbGVmdDogMDtcbiAgdG9wOiAwO1xuICByaWdodDogMDtcbiAgYm90dG9tOiAwO1xuICBoZWlnaHQ6IDEwMCU7XG4gIHdpZHRoOiAxMDAlO1xuICBiYWNrZ3JvdW5kLXBvc2l0aW9uOiBjZW50ZXIgY2VudGVyO1xuICBiYWNrZ3JvdW5kLXJlcGVhdDogbm8tcmVwZWF0O1xuICBiYWNrZ3JvdW5kLXNpemU6IGNvbnRhaW47XG4gIHBvaW50ZXItZXZlbnRzOiBub25lOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtcGxheWVyIC5qZXNzaWJ1Y2EtbG9hZGluZyB7XG4gIGRpc3BsYXk6IGZsZXg7XG4gIGp1c3RpZnktY29udGVudDogY2VudGVyO1xuICBhbGlnbi1pdGVtczogY2VudGVyO1xuICBwb3NpdGlvbjogYWJzb2x1dGU7XG4gIHotaW5kZXg6IDIwO1xuICBsZWZ0OiAwO1xuICB0b3A6IDA7XG4gIHJpZ2h0OiAwO1xuICBib3R0b206IDA7XG4gIHdpZHRoOiAxMDAlO1xuICBoZWlnaHQ6IDEwMCU7XG4gIHBvaW50ZXItZXZlbnRzOiBub25lOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtcGxheWVyIC5qZXNzaWJ1Y2EtY29udHJvbHMge1xuICBiYWNrZ3JvdW5kLWNvbG9yOiAjMTYxNjE2O1xuICBkaXNwbGF5OiBmbGV4O1xuICBmbGV4LWRpcmVjdGlvbjogY29sdW1uO1xuICBqdXN0aWZ5LWNvbnRlbnQ6IGZsZXgtZW5kO1xuICBwb3NpdGlvbjogYWJzb2x1dGU7XG4gIHotaW5kZXg6IDQwO1xuICBsZWZ0OiAwO1xuICByaWdodDogMDtcbiAgYm90dG9tOiAwO1xuICBoZWlnaHQ6IDQwcHg7XG4gIHBhZGRpbmctbGVmdDogMzBweDtcbiAgcGFkZGluZy1yaWdodDogMTNweDtcbiAgZm9udC1zaXplOiAxNHB4O1xuICBjb2xvcjogI2ZmZjtcbiAgb3BhY2l0eTogMDtcbiAgdmlzaWJpbGl0eTogaGlkZGVuO1xuICB0cmFuc2l0aW9uOiBhbGwgMC4ycyBlYXNlLWluLW91dDtcbiAgdXNlci1zZWxlY3Q6IG5vbmU7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1wbGF5ZXIgLmplc3NpYnVjYS1jb250cm9scyAuamVzc2lidWNhLWNvbnRyb2xzLWl0ZW0ge1xuICAgIHBvc2l0aW9uOiByZWxhdGl2ZTtcbiAgICBkaXNwbGF5OiBmbGV4O1xuICAgIGp1c3RpZnktY29udGVudDogY2VudGVyO1xuICAgIHBhZGRpbmc6IDAgMTdweDsgfVxuICAgIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtcGxheWVyIC5qZXNzaWJ1Y2EtY29udHJvbHMgLmplc3NpYnVjYS1jb250cm9scy1pdGVtOmhvdmVyIC5pY29uLXRpdGxlLXRpcHMge1xuICAgICAgdmlzaWJpbGl0eTogdmlzaWJsZTtcbiAgICAgIG9wYWNpdHk6IDE7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1wbGF5ZXIgLmplc3NpYnVjYS1jb250cm9scyAuamVzc2lidWNhLW1pY3JvcGhvbmUtY2xvc2Uge1xuICAgIGRpc3BsYXk6IG5vbmU7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1wbGF5ZXIgLmplc3NpYnVjYS1jb250cm9scyAuamVzc2lidWNhLWljb24tbXV0ZSB7XG4gICAgZGlzcGxheTogbm9uZTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXBsYXllciAuamVzc2lidWNhLWNvbnRyb2xzIC5qZXNzaWJ1Y2EtcHR6LWFjdGl2ZSB7XG4gICAgZGlzcGxheTogbm9uZTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXBsYXllciAuamVzc2lidWNhLWNvbnRyb2xzIC5qZXNzaWJ1Y2EtZnVsbHNjcmVlbi1leGl0IHtcbiAgICBkaXNwbGF5OiBub25lOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtcGxheWVyIC5qZXNzaWJ1Y2EtY29udHJvbHMgLmplc3NpYnVjYS1yZWNvcmQtc3RvcCB7XG4gICAgZGlzcGxheTogbm9uZTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXBsYXllciAuamVzc2lidWNhLWNvbnRyb2xzIC5qZXNzaWJ1Y2EtaWNvbi1hdWRpbywgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1wbGF5ZXIgLmplc3NpYnVjYS1jb250cm9scyAuamVzc2lidWNhLWljb24tbXV0ZSB7XG4gICAgei1pbmRleDogMTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXBsYXllciAuamVzc2lidWNhLWNvbnRyb2xzIC5qZXNzaWJ1Y2EtY29udHJvbHMtYm90dG9tIHtcbiAgICBkaXNwbGF5OiBmbGV4O1xuICAgIGp1c3RpZnktY29udGVudDogc3BhY2UtYmV0d2VlbjtcbiAgICBoZWlnaHQ6IDEwMCU7IH1cbiAgICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXBsYXllciAuamVzc2lidWNhLWNvbnRyb2xzIC5qZXNzaWJ1Y2EtY29udHJvbHMtYm90dG9tIC5qZXNzaWJ1Y2EtY29udHJvbHMtbGVmdCB7XG4gICAgICBkaXNwbGF5OiBmbGV4O1xuICAgICAgYWxpZ24taXRlbXM6IGNlbnRlcjsgfVxuICAgIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtcGxheWVyIC5qZXNzaWJ1Y2EtY29udHJvbHMgLmplc3NpYnVjYS1jb250cm9scy1ib3R0b20gLmplc3NpYnVjYS1jb250cm9scy1yaWdodCB7XG4gICAgICBkaXNwbGF5OiBmbGV4O1xuICAgICAgYWxpZ24taXRlbXM6IGNlbnRlcjsgfVxuXG4uamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXBsYXllci5qZXNzaWJ1Y2EtY29udHJvbHMtc2hvdyAuamVzc2lidWNhLWNvbnRyb2xzIHtcbiAgb3BhY2l0eTogMTtcbiAgdmlzaWJpbGl0eTogdmlzaWJsZTsgfVxuXG4uamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXBsYXllci5qZXNzaWJ1Y2EtaGlkZS1jdXJzb3IgKiB7XG4gIGN1cnNvcjogbm9uZSAhaW1wb3J0YW50OyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyLmplc3NpYnVjYS1mdWxsc2NyZWVuLXdlYiB7XG4gIHBvc2l0aW9uOiBmaXhlZDtcbiAgei1pbmRleDogOTk5OTtcbiAgbGVmdDogMDtcbiAgdG9wOiAwO1xuICByaWdodDogMDtcbiAgYm90dG9tOiAwO1xuICB3aWR0aDogMTAwJSAhaW1wb3J0YW50O1xuICBoZWlnaHQ6IDEwMCUgIWltcG9ydGFudDtcbiAgYmFja2dyb3VuZDogIzAwMDsgfVxuXG4uamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWljb24tbG9hZGluZyB7XG4gIHdpZHRoOiA1MHB4O1xuICBoZWlnaHQ6IDUwcHg7XG4gIGJhY2tncm91bmQ6IHVybChcIi4uL2Fzc2V0cy9sb2FkaW5nLnBuZ1wiKSBuby1yZXBlYXQgY2VudGVyO1xuICBiYWNrZ3JvdW5kLXNpemU6IDEwMCUgMTAwJTtcbiAgYW5pbWF0aW9uOiByb3RhdGlvbiAxcyBsaW5lYXIgaW5maW5pdGU7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1pY29uLXNjcmVlbnNob3Qge1xuICBiYWNrZ3JvdW5kOiB1cmwoXCIuLi9hc3NldHMvc2NyZWVuc2hvdC5wbmdcIikgbm8tcmVwZWF0IGNlbnRlcjtcbiAgYmFja2dyb3VuZC1zaXplOiAxMDAlIDEwMCU7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1pY29uLXNjcmVlbnNob3Q6aG92ZXIge1xuICAgIGJhY2tncm91bmQ6IHVybChcIi4uL2Fzc2V0cy9zY3JlZW5zaG90LWhvdmVyLnBuZ1wiKSBuby1yZXBlYXQgY2VudGVyO1xuICAgIGJhY2tncm91bmQtc2l6ZTogMTAwJSAxMDAlOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtaWNvbi1yZWNvcmQge1xuICBiYWNrZ3JvdW5kOiB1cmwoXCIuLi9hc3NldHMvcmVjb3JkLnBuZ1wiKSBuby1yZXBlYXQgY2VudGVyO1xuICBiYWNrZ3JvdW5kLXNpemU6IDEwMCUgMTAwJTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWljb24tcmVjb3JkOmhvdmVyIHtcbiAgICBiYWNrZ3JvdW5kOiB1cmwoXCIuLi9hc3NldHMvcmVjb3JkLWhvdmVyLnBuZ1wiKSBuby1yZXBlYXQgY2VudGVyO1xuICAgIGJhY2tncm91bmQtc2l6ZTogMTAwJSAxMDAlOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtaWNvbi1yZWNvcmRTdG9wIHtcbiAgYmFja2dyb3VuZDogdXJsKFwiLi4vYXNzZXRzL3JlY29yZC1zdG9wLnBuZ1wiKSBuby1yZXBlYXQgY2VudGVyO1xuICBiYWNrZ3JvdW5kLXNpemU6IDEwMCUgMTAwJTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWljb24tcmVjb3JkU3RvcDpob3ZlciB7XG4gICAgYmFja2dyb3VuZDogdXJsKFwiLi4vYXNzZXRzL3JlY29yZC1zdG9wLWhvdmVyLnBuZ1wiKSBuby1yZXBlYXQgY2VudGVyO1xuICAgIGJhY2tncm91bmQtc2l6ZTogMTAwJSAxMDAlOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtaWNvbi1mdWxsc2NyZWVuIHtcbiAgYmFja2dyb3VuZDogdXJsKFwiLi4vYXNzZXRzL2Z1bGxzY3JlZW4ucG5nXCIpIG5vLXJlcGVhdCBjZW50ZXI7XG4gIGJhY2tncm91bmQtc2l6ZTogMTAwJSAxMDAlOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtaWNvbi1mdWxsc2NyZWVuOmhvdmVyIHtcbiAgICBiYWNrZ3JvdW5kOiB1cmwoXCIuLi9hc3NldHMvZnVsbHNjcmVlbi1ob3Zlci5wbmdcIikgbm8tcmVwZWF0IGNlbnRlcjtcbiAgICBiYWNrZ3JvdW5kLXNpemU6IDEwMCUgMTAwJTsgfVxuXG4uamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWljb24tZnVsbHNjcmVlbkV4aXQge1xuICBiYWNrZ3JvdW5kOiB1cmwoXCIuLi9hc3NldHMvZXhpdC1mdWxsc2NyZWVuLnBuZ1wiKSBuby1yZXBlYXQgY2VudGVyO1xuICBiYWNrZ3JvdW5kLXNpemU6IDEwMCUgMTAwJTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWljb24tZnVsbHNjcmVlbkV4aXQ6aG92ZXIge1xuICAgIGJhY2tncm91bmQ6IHVybChcIi4uL2Fzc2V0cy9leGl0LWZ1bGxzY3JlZW4taG92ZXIucG5nXCIpIG5vLXJlcGVhdCBjZW50ZXI7XG4gICAgYmFja2dyb3VuZC1zaXplOiAxMDAlIDEwMCU7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1pY29uLWF1ZGlvIHtcbiAgYmFja2dyb3VuZDogdXJsKFwiLi4vYXNzZXRzL2F1ZGlvLnBuZ1wiKSBuby1yZXBlYXQgY2VudGVyO1xuICBiYWNrZ3JvdW5kLXNpemU6IDEwMCUgMTAwJTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWljb24tYXVkaW86aG92ZXIge1xuICAgIGJhY2tncm91bmQ6IHVybChcIi4uL2Fzc2V0cy9hdWRpby1ob3Zlci5wbmdcIikgbm8tcmVwZWF0IGNlbnRlcjtcbiAgICBiYWNrZ3JvdW5kLXNpemU6IDEwMCUgMTAwJTsgfVxuXG4uamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWljb24tbXV0ZSB7XG4gIGJhY2tncm91bmQ6IHVybChcIi4uL2Fzc2V0cy9tdXRlLnBuZ1wiKSBuby1yZXBlYXQgY2VudGVyO1xuICBiYWNrZ3JvdW5kLXNpemU6IDEwMCUgMTAwJTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWljb24tbXV0ZTpob3ZlciB7XG4gICAgYmFja2dyb3VuZDogdXJsKFwiLi4vYXNzZXRzL211dGUtaG92ZXIucG5nXCIpIG5vLXJlcGVhdCBjZW50ZXI7XG4gICAgYmFja2dyb3VuZC1zaXplOiAxMDAlIDEwMCU7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1pY29uLXRleHQge1xuICBmb250LXNpemU6IDE0cHg7XG4gIHdpZHRoOiAzMHB4OyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtcXVhbGl0eS1tZW51LWxpc3Qge1xuICBwb3NpdGlvbjogYWJzb2x1dGU7XG4gIGxlZnQ6IDUwJTtcbiAgYm90dG9tOiAxMDAlO1xuICB2aXNpYmlsaXR5OiBoaWRkZW47XG4gIG9wYWNpdHk6IDA7XG4gIHRyYW5zZm9ybTogdHJhbnNsYXRlWCgtNTAlKTtcbiAgdHJhbnNpdGlvbjogdmlzaWJpbGl0eSAzMDBtcywgb3BhY2l0eSAzMDBtcztcbiAgYmFja2dyb3VuZC1jb2xvcjogcmdiYSgwLCAwLCAwLCAwLjUpO1xuICBib3JkZXItcmFkaXVzOiA0cHg7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1xdWFsaXR5LW1lbnUtbGlzdC5qZXNzaWJ1Y2EtcXVhbGl0eS1tZW51LXNob3duIHtcbiAgICB2aXNpYmlsaXR5OiB2aXNpYmxlO1xuICAgIG9wYWNpdHk6IDE7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmljb24tdGl0bGUtdGlwcyB7XG4gIHBvaW50ZXItZXZlbnRzOiBub25lO1xuICBwb3NpdGlvbjogYWJzb2x1dGU7XG4gIGxlZnQ6IDUwJTtcbiAgYm90dG9tOiAxMDAlO1xuICB2aXNpYmlsaXR5OiBoaWRkZW47XG4gIG9wYWNpdHk6IDA7XG4gIHRyYW5zZm9ybTogdHJhbnNsYXRlWCgtNTAlKTtcbiAgdHJhbnNpdGlvbjogdmlzaWJpbGl0eSAzMDBtcyBlYXNlIDBzLCBvcGFjaXR5IDMwMG1zIGVhc2UgMHM7XG4gIGJhY2tncm91bmQtY29sb3I6IHJnYmEoMCwgMCwgMCwgMC41KTtcbiAgYm9yZGVyLXJhZGl1czogNHB4OyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5pY29uLXRpdGxlIHtcbiAgZGlzcGxheTogaW5saW5lLWJsb2NrO1xuICBwYWRkaW5nOiA1cHggMTBweDtcbiAgZm9udC1zaXplOiAxMnB4O1xuICB3aGl0ZS1zcGFjZTogbm93cmFwO1xuICBjb2xvcjogd2hpdGU7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1xdWFsaXR5LW1lbnUge1xuICBwYWRkaW5nOiA4cHggMDsgfVxuXG4uamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXF1YWxpdHktbWVudS1pdGVtIHtcbiAgZGlzcGxheTogYmxvY2s7XG4gIGhlaWdodDogMjVweDtcbiAgbWFyZ2luOiAwO1xuICBwYWRkaW5nOiAwIDEwcHg7XG4gIGN1cnNvcjogcG9pbnRlcjtcbiAgZm9udC1zaXplOiAxNHB4O1xuICB0ZXh0LWFsaWduOiBjZW50ZXI7XG4gIHdpZHRoOiA1MHB4O1xuICBjb2xvcjogcmdiYSgyNTUsIDI1NSwgMjU1LCAwLjUpO1xuICB0cmFuc2l0aW9uOiBjb2xvciAzMDBtcywgYmFja2dyb3VuZC1jb2xvciAzMDBtczsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXF1YWxpdHktbWVudS1pdGVtOmhvdmVyIHtcbiAgICBiYWNrZ3JvdW5kLWNvbG9yOiByZ2JhKDI1NSwgMjU1LCAyNTUsIDAuMik7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1xdWFsaXR5LW1lbnUtaXRlbTpmb2N1cyB7XG4gICAgb3V0bGluZTogbm9uZTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXF1YWxpdHktbWVudS1pdGVtLmplc3NpYnVjYS1xdWFsaXR5LW1lbnUtaXRlbS1hY3RpdmUge1xuICAgIGNvbG9yOiAjMjI5OEZDOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2Etdm9sdW1lLXBhbmVsLXdyYXAge1xuICBwb3NpdGlvbjogYWJzb2x1dGU7XG4gIGxlZnQ6IDUwJTtcbiAgYm90dG9tOiAxMDAlO1xuICB2aXNpYmlsaXR5OiBoaWRkZW47XG4gIG9wYWNpdHk6IDA7XG4gIHRyYW5zZm9ybTogdHJhbnNsYXRlWCgtNTAlKSB0cmFuc2xhdGVZKDMwJSk7XG4gIHRyYW5zaXRpb246IHZpc2liaWxpdHkgMzAwbXMsIG9wYWNpdHkgMzAwbXM7XG4gIGJhY2tncm91bmQtY29sb3I6IHJnYmEoMCwgMCwgMCwgMC41KTtcbiAgYm9yZGVyLXJhZGl1czogNHB4O1xuICBoZWlnaHQ6IDEyMHB4O1xuICB3aWR0aDogNTBweDtcbiAgb3ZlcmZsb3c6IGhpZGRlbjsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXZvbHVtZS1wYW5lbC13cmFwLmplc3NpYnVjYS12b2x1bWUtcGFuZWwtd3JhcC1zaG93IHtcbiAgICB2aXNpYmlsaXR5OiB2aXNpYmxlO1xuICAgIG9wYWNpdHk6IDE7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS12b2x1bWUtcGFuZWwge1xuICBjdXJzb3I6IHBvaW50ZXI7XG4gIHBvc2l0aW9uOiBhYnNvbHV0ZTtcbiAgdG9wOiAxNXB4O1xuICBoZWlnaHQ6IDYwcHg7XG4gIHdpZHRoOiA1MHB4O1xuICBvdmVyZmxvdzogaGlkZGVuOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2Etdm9sdW1lLXBhbmVsLWhhbmRsZSB7XG4gIHBvc2l0aW9uOiBhYnNvbHV0ZTtcbiAgdG9wOiAwO1xuICBsZWZ0OiA1MCU7XG4gIHdpZHRoOiAxMnB4O1xuICBoZWlnaHQ6IDEycHg7XG4gIGJvcmRlci1yYWRpdXM6IDEycHg7XG4gIG1hcmdpbi1sZWZ0OiAtNnB4O1xuICBiYWNrZ3JvdW5kOiAjZmZmOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2Etdm9sdW1lLXBhbmVsLWhhbmRsZTo6YmVmb3JlIHtcbiAgICBib3R0b206IC01NHB4O1xuICAgIGJhY2tncm91bmQ6ICNmZmY7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS12b2x1bWUtcGFuZWwtaGFuZGxlOjphZnRlciB7XG4gICAgYm90dG9tOiA2cHg7XG4gICAgYmFja2dyb3VuZDogcmdiYSgyNTUsIDI1NSwgMjU1LCAwLjIpOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2Etdm9sdW1lLXBhbmVsLWhhbmRsZTo6YmVmb3JlLCAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXZvbHVtZS1wYW5lbC1oYW5kbGU6OmFmdGVyIHtcbiAgICBjb250ZW50OiAnJztcbiAgICBwb3NpdGlvbjogYWJzb2x1dGU7XG4gICAgZGlzcGxheTogYmxvY2s7XG4gICAgbGVmdDogNTAlO1xuICAgIHdpZHRoOiAzcHg7XG4gICAgbWFyZ2luLWxlZnQ6IC0xcHg7XG4gICAgaGVpZ2h0OiA2MHB4OyB9XG4iXX0= */";
    styleInject(css_248z$1);

    class Control {
      constructor(player) {
        this.player = player;
        template(player);
        property(player, this);
        this.player.debug.log('Control', 'init');
      }

      autoSize() {}

      destroy() {
        this.player.debug.log('control', 'destroy');

        if (this.$poster) {
          this.player.$container.removeChild(this.$poster);
        }

        this.player.$container.removeChild(this.$loading);
        this.player.$container.removeChild(this.$ptzControl);
        this.player.$container.removeChild(this.$controls);
        this.player = null;
      }

    }

    var css_248z = ".jessibuca-container.jessibuca-fullscreen-web{position:fixed;z-index:9999;left:0;top:0;right:0;bottom:0;width:100%!important;height:100%!important;background:#000}\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInN0eWxlLnNjc3MiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUEsOENBQ0UsY0FBZSxDQUNmLFlBQWEsQ0FDYixNQUFPLENBQ1AsS0FBTSxDQUNOLE9BQVEsQ0FDUixRQUFTLENBQ1Qsb0JBQXNCLENBQ3RCLHFCQUF1QixDQUN2QixlQUFrQiIsImZpbGUiOiJzdHlsZS5zY3NzIiwic291cmNlc0NvbnRlbnQiOlsiLmplc3NpYnVjYS1jb250YWluZXIuamVzc2lidWNhLWZ1bGxzY3JlZW4td2ViIHtcbiAgcG9zaXRpb246IGZpeGVkO1xuICB6LWluZGV4OiA5OTk5O1xuICBsZWZ0OiAwO1xuICB0b3A6IDA7XG4gIHJpZ2h0OiAwO1xuICBib3R0b206IDA7XG4gIHdpZHRoOiAxMDAlICFpbXBvcnRhbnQ7XG4gIGhlaWdodDogMTAwJSAhaW1wb3J0YW50O1xuICBiYWNrZ3JvdW5kOiAjMDAwOyB9XG4iXX0= */";
    styleInject(css_248z);

    class Player extends Emitter {
      constructor(container, options) {
        super();
        this.$container = container;
        this._opt = Object.assign({}, DEFAULT_PLAYER_OPTIONS, options);

        if (this._opt.useWCS) {
          this._opt.useWCS = supportWCS();
        }

        if (!this._opt.forceNoOffscreen) {
          if (!supportOffscreenV2()) {
            this._opt.forceNoOffscreen = true;
            this._opt.useOffscreen = false;
          } else {
            this._opt.useOffscreen = true;
          }
        }

        this._opt.hasControl = this._hasControl();
        property$1(this);
        events(this);
        this._loading = true;
        this._hasLoaded = false;
        this.debug = new Debug(this);
        this.events = new Events(this);
        this.video = new Video(this);
        this.audio = new Audio(this);
        this.recorder = new Recorder(this);
        this.decoderWorker = new DecoderWorker(this);
        this.stream = null;
        this.demux = null;

        if (this._opt.useWCS) {
          this.webcodecsDecoder = new WebcodecsDecoder(this);
        } //


        if (this._opt.hasControl) {
          this.control = new Control(this);
        }

        this.debug.log('options', this._opt);
      }

      set fullscreen(value) {
        this.emit(EVENTS.fullscreen, value);
      }

      get fullscreen() {
        return isFullScreen() || this.webFullscreen;
      }

      set webFullscreen(value) {
        this.emit(EVENTS.webFullscreen, value);
      }

      get webFullscreen() {
        return this.$container.classList.contains('jessibuca-fullscreen-web');
      }

      get loaded() {
        return this._hasLoaded;
      } //


      set playing(value) {
        if (this.playing !== value) {
          this.emit(EVENTS.playing, value);
          this.emit(EVENTS.volumechange, this.volume);
        }
      }

      get playing() {
        return this.stream.playing;
      }

      get volume() {
        return this.audio.volume;
      }

      set loading(value) {
        this._loading = value;
        this.emit(EVENTS.loading);
      }

      get loading() {
        return this._loading;
      }

      set recording(value) {
        if (value) {
          this.recorder.startRecord();
        } else {
          this.recorder.stopRecordAndSave();
        }
      }

      get recording() {
        return this.recorder.recording;
      }

      startRecord(fileName) {
        this.recorder.setFileName(fileName); //

        this.recording = true;
      }

      stopRecordAndSave() {
        this.recording = false;
      }
      /**
       *
       * @param options
       */


      updateOption(options) {
        this._opt = Object.assign({}, this._opt, options);
      } //


      init() {
        return new Promise((resolve, reject) => {
          if (!this.stream) {
            this.stream = new Stream(this);
          }

          if (!this.demux) {
            this.demux = new Demux(this);
          }

          if (!this.decoderWorker) {
            this.decoderWorker = new DecoderWorker(this);
            this.once(EVENTS.decoderWorkerInit, () => {
              resolve();
            });
          } else {
            resolve();
          }
        });
      }

      play(url) {
        return new Promise((resolve, reject) => {
          this.init().then(() => {
            this.stream.fetchStream(url); //

            this.stream.once(EVENTS_ERROR.fetchError, error => {
              reject(error);
            }); //

            this.stream.once(EVENTS.streamSuccess, () => {
              resolve();
            });
          }).catch(e => {
            reject(e);
          });
        });
      } //


      close() {
        //
        if (this.stream) {
          this.stream.destroy();
          this.stream = null;
        }

        if (this.demux) {
          this.demux.destroy();
          this.demux = null;
        } //


        if (this.decoderWorker) {
          this.decoderWorker.destroy();
          this.decoderWorker = null;
        }

        if (this.webcodecsDecoder) {
          this.webcodecsDecoder.destroy();
          this.webcodecsDecoder = null;
        }
      }

      pause() {
        return new Promise((resolve, reject) => {
          this.close(); //

          this.playing = false;
          this.audio.pause();
        });
      } // 恢复播放


      resume() {}

      _hasControl() {
        let result = false;
        let hasBtnShow = false;
        Object.keys(this._opt.operateBtns).forEach(key => {
          if (this._opt.operateBtns[key]) {
            hasBtnShow = true;
          }
        });

        if (this._opt.showBandwidth || this._opt.text || hasBtnShow) {
          result = true;
        }

        return result;
      }

      destroy() {
        if (this.events) {
          this.events.destroy();
          this.events = null;
        }

        if (this.decoderWorker) {
          this.decoderWorker.destroy();
          this.decoderWorker = null;
        }

        if (this.video) {
          this.video.destroy();
          this.video = null;
        }

        if (this.audio) {
          this.audio.destroy();
          this.audio = null;
        }

        if (this.stream) {
          this.stream.destroy();
          this.stream = null;
        }

        if (this.recorder) {
          this.recorder.destroy();
          this.recorder = null;
        }

        if (this.control) {
          this.control.destroy();
          this.control = null;
        } // 其他没法解耦的，通过 destroy 方式


        this.emit('destroy'); // 接触所有绑定事件

        this.off();
        this.debug.log('play', 'destroy end');
      }

    }

    class Jessibuca extends Emitter {
      constructor(options) {
        super();
        let _opt = options;
        let $container = options.container;

        if (typeof options.container === 'string') {
          $container = document.querySelector(options.container);
        }

        if (!$container) {
          throw new Error('Jessibuca need container option');
        }

        $container.classList.add('jessibuca-container');
        delete _opt.container;
        this._opt = _opt;
        this.$container = $container;
        this.href = null;
        this.events = new Events(this);
        this.player = new Player($container, _opt);

        this._bindEvents();
      }

      _bindEvents() {
        // 对外的事件
        Object.keys(JESSIBUCA_EVENTS).forEach(key => {
          this.player.on(EVENTS[key], value => {
            this.emit(key, value);
          });
        });
      }
      /**
       * 是否开启控制台调试打印
       * @param value {Boolean}
       */


      setDebug(value) {
        this.player.updateOption({
          isDebug: !!value
        });
      }
      /**
       *
       */


      mute() {
        this.player.audio.mute(true);
      }
      /**
       *
       */


      cancelMute() {
        this.player.audio.mute(false);
      }
      /**
       *
       * @param value {number}
       */


      setVolume(value) {
        this.player.audio.setValue(value);
      }
      /**
       *
       */


      audioResume() {
        this.player.audio.audioEnabled(true);
      }
      /**
       * 设置超时时长, 单位秒 在连接成功之前和播放中途,如果超过设定时长无数据返回,则回调timeout事件
       * @param value {number}
       */


      setTimeout(time) {
        this.player.updateOption({
          timeout: Number(time)
        });
      }
      /**
       *
       * @param type {number}: 0,1,2
       */


      setScaleMode(type) {
        type = Number(type);
        let options = {
          isFullResize: false,
          isResize: false
        };

        switch (type) {
          case SCALE_MODE_TYPE.full:
            options.isFullResize = false;
            options.isResize = false;
            break;

          case SCALE_MODE_TYPE.auto:
            options.isFullResize = false;
            options.isResize = true;
            break;

          case SCALE_MODE_TYPE.fullAuto:
            options.isFullResize = true;
            options.isResize = true;
            break;
        }

        this.player.updateOption(options);
        this.player.video.resize();
      }
      /**
       *
       */


      pause() {
        this.player.pause();
      }
      /**
       *
       */


      close() {
        this.player.close();
      }
      /**
       *
       */


      clearView() {
        this.player.video.clearView();
      }
      /**
       *
       * @param url {string}
       * @returns {Promise<unknown>}
       */


      play(url) {
        return new Promise((resolve, reject) => {
          if (!url && !this._opt.url) {
            reject();
            return;
          }

          if (url) {
            // url 相等的时候。
            if (this._opt.url && url === this._opt.url) {
              resolve();
              return;
            } else {
              this._opt.url = url; //  新的url

              const isHttp = url.indexOf("http") === 0; //

              const protocol = isHttp ? PLAYER_PLAY_PROTOCOL.fetch : PLAYER_PLAY_PROTOCOL.websocket; //

              const demuxType = isHttp || url.indexOf(".flv") !== -1 || this._opt.isFlv ? DEMUX_TYPE.flv : DEMUX_TYPE.m7s;
              this.player.updateOption({
                protocol,
                demuxType
              });

              if (this.hasLoaded()) {
                this.player.play(url).then(() => {
                  resolve();
                }).catch(() => {
                  this.player.close();
                  reject();
                });
              } else {
                this.player.once(EVENTS.load, () => {
                  this.player.play(url).then(() => {
                    resolve();
                  }).catch(() => {
                    this.player.close();
                    reject();
                  });
                });
              }
            }
          } else {
            //  url 不存在的时候
            //  就是从 play-> pause -> play
            this.player.resume();
          }
        });
      }

      resize() {
        this.player.video.resize();
      }
      /**
       *
       * @param time {number}
       */


      setBufferTime(time) {
        time = Number(time);

        if (this.player) ; else {
          this._opt.videoBuffer = time;
        }
      }
      /**
       *
       * @param deg {number}
       */


      setRotate(deg) {
        deg = parseInt(deg, 10);
        const list = [0, 90, 270];

        if (this._opt.rotate === deg || list.indexOf(deg) === -1) {
          return;
        }

        this.player.updateOption({
          rotate: deg
        });
        this.resize();
      }

      hasLoaded() {
        return this.player.loaded;
      }

      setKeepScreenOn() {
        this.player.updateOption({
          keepScreenOn: true
        });
      }

      useWCS() {
        this._opt.useWCS = supportWCS();
      }
      /**
       *
       * @param flag {Boolean}
       */


      setFullscreen(flag) {
        const fullscreen = !!flag;

        if (this.player.fullscreen !== fullscreen) {
          this.player.fullscreen = fullscreen;
        }
      }
      /**
       *
       * @param filename {string}
       * @param format {string}
       * @param quality {number}
       * @param type {string} download,base64,blob
       */


      screenshot(filename, format, quality, type) {
        return this.player.video.screenshot(filename, format, quality, type);
      }
      /**
       *
       * @returns {Boolean}
       */


      isPlaying() {
        return this.player.playing;
      }
      /**
       * 是否静音状态
       * @returns {Boolean}
       */


      isMute() {
        return this.player.audio.isMute();
      }
      /**
       *
       */


      destroy() {
        this.player.destroy();
        this.player = null;
      }

    }

    window.Jessibuca = Jessibuca;

    return Jessibuca;

}));
//# sourceMappingURL=jessibuca.js.map
