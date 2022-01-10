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
      videoBuffer: 1000,
      //1000ms == 1 second
      isResize: true,
      isFullResize: false,
      //
      isFlv: false,
      debug: false,
      loadingTimeout: 10,
      // loading timeout
      heartTimeout: 10,
      // heart timeout
      timeout: 10,
      // second
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
        record: false
      },
      hasControl: false,
      loadingText: '',
      background: '',
      decoder: 'decoder.js',
      url: '',
      //
      rotate: 0,
      // text: '',
      forceNoOffscreen: true,
      // 默认是不采用
      hiddenAutoPause: false,
      protocol: PLAYER_PLAY_PROTOCOL.fetch,
      demuxType: DEMUX_TYPE.flv,
      //
      useWCS: false,
      //
      useMSE: false,
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
      fullscreen: 'fullscreen$2',
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
      delayTimeout: 'delayTimeout',
      loadingTimeout: 'loadingTimeout',
      stats: 'stats',
      performance: "performance",
      record: 'record',
      recording: 'recording',
      recordingTimestamp: 'recordingTimestamp',
      recordStart: 'recordStart',
      recordEnd: 'recordEnd',
      recordCreateError: 'recordCreateError',
      buffer: 'buffer',
      videoFrame: 'videoFrame',
      start: 'start',
      metadata: 'metadata',
      resize: 'resize',
      streamEnd: 'streamEnd',
      streamSuccess: 'streamSuccess',
      streamMessage: 'streamMessage',
      streamError: 'streamError',
      volumechange: 'volumechange',
      destroy: 'destroy',
      mseSourceOpen: 'mseSourceOpen',
      mseSourceClose: 'mseSourceClose',
      mseSourceBufferError: 'mseSourceBufferError',
      mseSourceBufferBusy: 'mseSourceBufferBusy',
      videoWaiting: 'videoWaiting',
      videoTimeUpdate: 'videoTimeUpdate',
      videoSyncAudio: 'videoSyncAudio'
    };
    const JESSIBUCA_EVENTS = {
      load: EVENTS.load,
      timeUpdate: EVENTS.timeUpdate,
      videoInfo: EVENTS.videoInfo,
      audioInfo: EVENTS.audioInfo,
      error: EVENTS.error,
      kBps: EVENTS.kBps,
      log: EVENTS.log,
      start: EVENTS.start,
      timeout: EVENTS.timeout,
      loadingTimeout: EVENTS.loadingTimeout,
      delayTimeout: EVENTS.delayTimeout,
      fullscreen: 'fullscreen',
      play: EVENTS.play,
      pause: EVENTS.pause,
      mute: EVENTS.mute,
      stats: EVENTS.stats,
      performance: EVENTS.performance,
      recordingTimestamp: EVENTS.recordingTimestamp,
      recordStart: EVENTS.recordStart,
      recordEnd: EVENTS.recordEnd
    };
    const EVENTS_ERROR = {
      playError: 'playIsNotPauseOrUrlIsNull',
      fetchError: "fetchError",
      websocketError: 'websocketError',
      webcodecsH265NotSupport: 'webcodecsH265NotSupport',
      mediaSourceH265NotSupport: 'mediaSourceH265NotSupport',
      wasmDecodeError: 'wasmDecodeError'
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
      7: 'H264(AVC)',
      //
      12: 'H265(HEVC)' //

    };
    const VIDEO_ENC_CODE = {
      h264: 7,
      h265: 12
    };
    const AUDIO_ENC_TYPE = {
      10: 'AAC',
      7: 'ALAW',
      8: 'MULAW'
    };
    const H265_NAL_TYPE = {
      vps: 32,
      sps: 33,
      pps: 34
    };
    const CONTROL_HEIGHT = 38;
    const SCALE_MODE_TYPE = {
      full: 0,
      //  视频画面完全填充canvas区域,画面会被拉伸
      auto: 1,
      // 视频画面做等比缩放后,高或宽对齐canvas区域,画面不被拉伸,但有黑边
      fullAuto: 2 // 视频画面做等比缩放后,完全填充canvas区域,画面不被拉伸,没有黑边,但画面显示不全

    };
    const FILE_SUFFIX$1 = {
      mp4: 'mp4',
      webm: 'webm'
    };
    const CANVAS_RENDER_TYPE = {
      webcodecs: 'webcodecs',
      webgl: 'webgl',
      offscreen: 'offscreen'
    };
    const ENCODED_VIDEO_TYPE = {
      key: 'key',
      delta: 'delta'
    };
    const MP4_CODECS = {
      avc: 'video/mp4; codecs="avc1.64002A"',
      hev: 'video/mp4; codecs="hev1.1.6.L123.b0"'
    };
    const MEDIA_SOURCE_STATE = {
      ended: 'ended',
      open: 'open',
      closed: 'closed'
    }; // frag duration
    const AUDIO_SYNC_VIDEO_DIFF = 200;

    class Debug {
      constructor(master) {
        this.log = function (name) {
          if (master._opt.debug) {
            for (var _len = arguments.length, args = new Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
              args[_key - 1] = arguments[_key];
            }

            console.log(`Jessibuca: [${name}]`, ...args);
          }
        };

        this.warn = function (name) {
          if (master._opt.debug) {
            for (var _len2 = arguments.length, args = new Array(_len2 > 1 ? _len2 - 1 : 0), _key2 = 1; _key2 < _len2; _key2++) {
              args[_key2 - 1] = arguments[_key2];
            }

            console.warn(`Jessibuca: [${name}]`, ...args);
          }
        };

        this.error = function (name) {
          if (master._opt.debug) {
            for (var _len3 = arguments.length, args = new Array(_len3 > 1 ? _len3 - 1 : 0), _key3 = 1; _key3 < _len3; _key3++) {
              args[_key3 - 1] = arguments[_key3];
            }

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

      proxy(target, name, callback) {
        let option = arguments.length > 3 && arguments[3] !== undefined ? arguments[3] : {};

        if (!target) {
          return;
        }

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

    var commonjsGlobal = typeof globalThis !== 'undefined' ? globalThis : typeof window !== 'undefined' ? window : typeof global !== 'undefined' ? global : typeof self !== 'undefined' ? self : {};

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
    function dataURLToFile() {
      let dataURL = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : '';
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
    function setStyle(element, key, value) {
      if (!element) {
        return;
      }

      if (typeof key === 'object') {
        Object.keys(key).forEach(item => {
          setStyle(element, item, key[item]);
        });
      }

      element.style[key] = value;
      return element;
    }
    function getStyle(element, key) {
      let numberType = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : true;

      if (!element) {
        return 0;
      }

      const value = getComputedStyle(element, null).getPropertyValue(key);
      return numberType ? parseFloat(value) : value;
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
    function downloadRecord(blob, name, suffix) {
      const url = window.URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = (name || now()) + '.' + (suffix || FILE_SUFFIX.webm);
      a.click();
      window.URL.revokeObjectURL(url);
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
    function bpsSize(value) {
      if (null == value || value === '') {
        return "0 KB/S";
      }

      let size = parseFloat(value);
      size = size.toFixed(2);
      return size + 'KB/S';
    }
    function fpsStatus(fps) {
      let result = 0;

      if (fps >= 24) {
        result = 2;
      } else if (fps >= 15) {
        result = 1;
      }

      return result;
    }
    function createEmptyImageBitmap(width, height) {
      const $canvasElement = document.createElement("canvas");
      $canvasElement.width = width;
      $canvasElement.height = height;
      return createImageBitmap($canvasElement, 0, 0, width, height);
    }
    function supportMSE() {
      return window.MediaSource && window.MediaSource.isTypeSupported(MP4_CODECS.avc);
    }
    function isEmpty(value) {
      return value === null || value === undefined;
    }
    function isNotEmpty(value) {
      return !isEmpty(value);
    }

    var events$1 = (player => {
      try {
        const screenfullChange = () => {
          player.emit(JESSIBUCA_EVENTS.fullscreen, player.fullscreen); // 如果不是fullscreen,则触发下 resize 方法

          if (!player.fullscreen) {
            player.resize();
          } else {
            if (player._opt.useMSE) {
              player.resize();
            }
          }
        };

        screenfull.on('change', screenfullChange);
        player.events.destroys.push(() => {
          screenfull.off('change', screenfullChange);
        });
      } catch (error) {//
      } //


      player.on(EVENTS.decoderWorkerInit, () => {
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
            screenfull.exit().then(() => {}).catch(() => {
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
        const ignoreList = [EVENTS.timeUpdate];
        Object.keys(EVENTS).forEach(key => {
          player.on(EVENTS[key], value => {
            if (ignoreList.includes(key)) {
              return;
            }

            player.debug.log('player events', EVENTS[key], value);
          });
        });
        Object.keys(EVENTS_ERROR).forEach(key => {
          player.on(EVENTS_ERROR[key], value => {
            player.debug.log('player event error', EVENTS_ERROR[key], value);
          });
        });
      }
    });

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

        function listener() {
          self.off(name, listener);

          for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
            args[_key] = arguments[_key];
          }

          fn.apply(ctx, args);
        }

        listener._ = fn;
        return this.on(name, listener, ctx);
      }

      emit(name) {
        const evtArr = ((this.e || (this.e = {}))[name] || []).slice();

        for (var _len2 = arguments.length, data = new Array(_len2 > 1 ? _len2 - 1 : 0), _key2 = 1; _key2 < _len2; _key2++) {
          data[_key2 - 1] = arguments[_key2];
        }

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

      gl.useProgram(program); // initBuffers

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

    class CommonLoader$1 extends Emitter {
      constructor() {
        super();
        this.init = false;
      } //


      updateVideoInfo(data) {
        if (data.encTypeCode) {
          this.videoInfo.encType = VIDEO_ENC_TYPE[data.encTypeCode];
        }

        if (data.width) {
          this.videoInfo.width = data.width;
        }

        if (data.height) {
          this.videoInfo.height = data.height;
        } // video 基本信息


        if (this.videoInfo.encType && this.videoInfo.height && this.videoInfo.width && !this.init) {
          this.player.emit(EVENTS.videoInfo, this.videoInfo);
          this.init = true;
        }
      }

    }

    class CanvasVideoLoader extends CommonLoader$1 {
      constructor(player) {
        super();
        this.player = player;
        const $canvasElement = document.createElement("canvas");
        $canvasElement.style.position = "absolute";
        $canvasElement.style.top = 0;
        $canvasElement.style.left = 0;
        this.$videoElement = $canvasElement;
        player.$container.appendChild(this.$videoElement);
        this.context2D = null;
        this.contextGl = null;
        this.contextGlRender = null;
        this.contextGlDestroy = null;
        this.bitmaprenderer = null;
        this.renderType = null;
        this.videoInfo = {
          width: '',
          height: '',
          encType: ''
        }; //

        this._initCanvasRender();

        this.player.debug.log('CanvasVideo', 'init');
      }

      _initContextGl() {
        this.contextGl = createContextGL(this.$videoElement);
        const webgl = createWebGL(this.contextGl);
        this.contextGlRender = webgl.render;
        this.contextGlDestroy = webgl.destroy;
      }

      _initContext2D() {
        this.context2D = this.$videoElement.getContext('2d');
      } // 渲染类型


      _initCanvasRender() {
        if (this.player._opt.useWCS && !this._supportOffscreen()) {
          this.renderType = CANVAS_RENDER_TYPE.webcodecs;

          this._initContext2D();
        } else if (this._supportOffscreen()) {
          this.renderType = CANVAS_RENDER_TYPE.offscreen;

          this._bindOffscreen();
        } else {
          this.renderType = CANVAS_RENDER_TYPE.webgl;

          this._initContextGl();
        }
      }

      _supportOffscreen() {
        return supportOffscreen(this.$videoElement) && this.player._opt.useOffscreen;
      } //


      _bindOffscreen() {
        this.bitmaprenderer = this.$videoElement.getContext('bitmaprenderer');
      }

      initCanvasViewSize() {
        this.$videoElement.width = this.videoInfo.width;
        this.$videoElement.height = this.videoInfo.height;
        this.resize();
      } //


      render(msg) {
        this.player.videoTimestamp = msg.ts;

        switch (this.renderType) {
          case CANVAS_RENDER_TYPE.offscreen:
            this.bitmaprenderer.transferFromImageBitmap(msg.buffer);
            break;

          case CANVAS_RENDER_TYPE.webgl:
            this.contextGlRender(this.$videoElement.width, this.$videoElement.height, msg.output[0], msg.output[1], msg.output[2]);
            break;

          case CANVAS_RENDER_TYPE.webcodecs:
            this.context2D.drawImage(msg.videoFrame, 0, 0, this.$videoElement.width, this.$videoElement.height);
            break;
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
        switch (this.renderType) {
          case CANVAS_RENDER_TYPE.offscreen:
            createEmptyImageBitmap(this.$videoElement.width, this.$videoElement.height).then(imageBitMap => {
              this.bitmaprenderer.transferFromImageBitmap(imageBitMap);
            });
            break;

          case CANVAS_RENDER_TYPE.webgl:
            this.contextGl.clear(this.contextGl.COLOR_BUFFER_BIT);
            break;

          case CANVAS_RENDER_TYPE.webcodecs:
            this.context2D.clearRect(0, 0, this.$videoElement.width, this.$videoElement.height);
            break;
        }
      }

      resize() {
        this.player.debug.log('canvasVideo', 'resize');
        const option = this.player._opt;
        const width = this.player.width;
        let height = this.player.height;

        if (option.hasControl) {
          height -= CONTROL_HEIGHT;
        }

        let resizeWidth = this.$videoElement.width;
        let resizeHeight = this.$videoElement.height;
        const rotate = option.rotate;
        let left = (width - resizeWidth) / 2;
        let top = (height - resizeHeight) / 2;

        if (rotate === 270 || rotate === 90) {
          resizeWidth = this.$videoElement.height;
          resizeHeight = this.$videoElement.width;
        }

        const wScale = width / resizeWidth;
        const hScale = height / resizeHeight;
        let scale = wScale > hScale ? hScale : wScale; //

        if (!option.isResize) {
          if (wScale !== hScale) {
            scale = wScale + ',' + hScale;
          }
        } //


        if (option.isFullResize) {
          scale = wScale > hScale ? wScale : hScale;
        }

        let transform = "scale(" + scale + ")";

        if (rotate) {
          transform += ' rotate(' + rotate + 'deg)';
        }

        this.$videoElement.style.transform = transform;
        this.$videoElement.style.left = left + "px";
        this.$videoElement.style.top = top + "px";
      }

      destroy() {
        if (this.contextGl) {
          this.contextGl = null;
        }

        if (this.context2D) {
          this.context2D = null;
        }

        if (this.contextGlRender) {
          this.contextGlDestroy && this.contextGlDestroy();
          this.contextGlDestroy = null;
          this.contextGlRender = null;
        }

        if (this.bitmaprenderer) {
          this.bitmaprenderer = null;
        }

        this.renderType = null;
        this.videoInfo = {
          width: '',
          height: '',
          encType: '',
          encTypeCode: ''
        };
        this.player.$container.removeChild(this.$videoElement);
        this.init = false;
        this.off();
        this.player.debug.log(`CanvasVideoLoader`, 'destroy');
      }

    }

    class VideoLoader extends CommonLoader$1 {
      constructor(player) {
        super();
        this.player = player;
        const $videoElement = document.createElement('video');
        $videoElement.muted = true;
        $videoElement.style.position = "absolute";
        $videoElement.style.top = 0;
        $videoElement.style.left = 0;
        player.$container.appendChild($videoElement);
        this.$videoElement = $videoElement;
        this.videoInfo = {
          width: '',
          height: '',
          encType: ''
        };
        this.resize();
        const {
          proxy
        } = this.player.events;
        proxy(this.$videoElement, 'canplay', () => {
          this.player.debug.log('Video', 'canplay');
        });
        proxy(this.$videoElement, 'waiting', () => {
          this.player.emit(EVENTS.videoWaiting);
        });
        proxy(this.$videoElement, 'timeupdate', event => {// this.player.emit(EVENTS.videoTimeUpdate, event.timeStamp);
        });
        this.player.debug.log('Video', 'init');
      }

      play() {
        // this.$videoElement.autoplay = true;
        this.$videoElement.play();
      }

      clearView() {}

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

        const $video = this.$videoElement;
        let canvas = document.createElement('canvas');
        canvas.width = $video.videoWidth;
        canvas.height = $video.videoHeight;
        const context = canvas.getContext('2d');
        context.drawImage($video, 0, 0, canvas.width, canvas.height);
        const dataURL = canvas.toDataURL(SCREENSHOT_TYPE[format] || SCREENSHOT_TYPE.png, encoderOptions);
        const file = dataURLToFile(dataURL);

        if (type === SCREENSHOT_TYPE.base64) {
          return dataURL;
        } else if (type === SCREENSHOT_TYPE.blob) {
          return file;
        } else if (type === SCREENSHOT_TYPE.download) {
          downloadImg(file, filename);
        }
      }

      initCanvasViewSize() {
        this.resize();
      }

      resize() {
        this.$videoElement.width = this.player.width;
        this.$videoElement.height = this.player._opt.hasControl ? this.player.height - CONTROL_HEIGHT : this.player.height;
        const option = this.player._opt;
        let objectFill = 'contain';
        const rotate = option.rotate; // 默认是true
        // 视频画面做等比缩放后,高或宽对齐canvas区域,画面不被拉伸,但有黑边
        // 视频画面完全填充canvas区域,画面会被拉伸

        if (!option.isResize) {
          objectFill = 'fill';
        } // 视频画面做等比缩放后,完全填充canvas区域,画面不被拉伸,没有黑边,但画面显示不全


        if (option.isFullResize) {
          objectFill = 'none';
        }

        this.$videoElement.style.objectFit = objectFill;
        this.$videoElement.style.transform = 'rotate(' + rotate + 'deg)';
      }

      destroy() {
        this.player.$container.removeChild(this.$videoElement);
        this.init = false;
        this.off();
        this.player.debug.log('Video', 'destroy');
      }

    }

    class Video {
      constructor(player) {
        const Loader = Video.getLoaderFactory(player._opt);
        return new Loader(player);
      }

      static getLoaderFactory(opt) {
        if (opt.useMSE) {
          return VideoLoader;
        } else {
          return CanvasVideoLoader;
        }
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
        this.playing = false; //

        this.audioSyncVideoOption = {
          diff: null
        };
        this.audioInfo = {
          encType: '',
          channels: '',
          sampleRate: ''
        };
        this.init = false; // update

        this.on(EVENTS.videoSyncAudio, options => {
          this.player.debug.log('AudioContext', `videoSyncAudio , audioTimestamp: ${options.audioTimestamp},videoTimestamp: ${options.videoTimestamp},diff:${options.diff}`);
          this.audioSyncVideoOption = options;
        });
        this.player.debug.log('AudioContext', 'init');
      }

      updateAudioInfo(data) {
        if (data.encTypeCode) {
          this.audioInfo.encType = AUDIO_ENC_TYPE[data.encTypeCode];
        }

        if (data.channels) {
          this.audioInfo.channels = data.channels;
        }

        if (data.sampleRate) {
          this.audioInfo.sampleRate = data.sampleRate;
        } // audio 基本信息


        if (this.audioInfo.sampleRate && this.audioInfo.channels && this.audioInfo.encType && !this.init) {
          this.player.emit(EVENTS.audioInfo, this.audioInfo);
          this.init = true;
        }
      } //


      get isPlaying() {
        return this.playing;
      }

      get isMute() {
        return this.gainNode.gain.value === 0 || this.isStateSuspended();
      }

      get volume() {
        return this.gainNode.gain.value;
      }

      get bufferSize() {
        return this.bufferList.length;
      }

      initScriptNode() {
        this.playing = true;

        if (this.hasInitScriptNode) {
          return;
        }

        const channels = this.audioInfo.channels;
        const scriptNode = this.audioContext.createScriptProcessor(1024, 0, channels);

        scriptNode.onaudioprocess = audioProcessingEvent => {
          const outputBuffer = audioProcessingEvent.outputBuffer;

          if (this.bufferList.length && this.playing) {
            // just for wasm
            if (!this.player._opt.useWCS && !this.player._opt.useMSE) {
              // audio > video
              // wait
              if (this.audioSyncVideoOption.diff > AUDIO_SYNC_VIDEO_DIFF) {
                this.player.debug.warn('AudioContext', `audioSyncVideoOption more than diff :${this.audioSyncVideoOption.diff}`); // wait

                return;
              } // audio < video
              // throw away then chase video
              else if (this.audioSyncVideoOption.diff < -AUDIO_SYNC_VIDEO_DIFF) {
                this.player.debug.warn('AudioContext', `audioSyncVideoOption less than diff :${this.audioSyncVideoOption.diff}`); //

                let bufferItem = this.bufferList.shift(); //

                while (bufferItem.ts - this.player.videoTimestamp < -AUDIO_SYNC_VIDEO_DIFF && this.bufferList.length > 0) {
                  // this.player.debug.warn('AudioContext', `audioSyncVideoOption less than inner ts is:${bufferItem.ts}, videoTimestamp is ${this.player.videoTimestamp},diff:${bufferItem.ts - this.player.videoTimestamp}`)
                  bufferItem = this.bufferList.shift();
                }

                if (this.bufferList.length === 0) {
                  return;
                }
              }
            }

            if (this.bufferList.length === 0) {
              return;
            }

            const bufferItem = this.bufferList.shift(); // update audio time stamp

            if (bufferItem && bufferItem.ts) {
              this.player.audioTimestamp = bufferItem.ts;
            }

            for (let channel = 0; channel < channels; channel++) {
              const b = bufferItem.buffer[channel];
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
          if (!this.isMute) {
            this.player.emit(EVENTS.mute, flag);
          }

          this.setVolume(0);
          this.audioEnabled(false);
          this.clear();
        } else {
          if (this.isMute) {
            this.player.emit(EVENTS.mute, flag);
          }

          this.setVolume(0.5);
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
        this.bufferList = [];
      }

      play(buffer, ts) {
        this.bufferList.push({
          buffer,
          ts
        });

        if (this.bufferList.length > 20) {
          this.player.debug.warn('AudioContext', `bufferList is large: ${this.bufferList.length}`); // out of memory

          if (this.bufferList.length > 50) {
            this.bufferList.shift();
          }
        } // this.player.debug.log('AudioContext', `bufferList is ${this.bufferList.length}`)

      }

      pause() {
        this.audioSyncVideoOption = {
          diff: null
        };
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
        this.init = false;

        if (this.scriptNode) {
          this.scriptNode.onaudioprocess = noop;
          this.scriptNode = null;
        }

        this.audioBufferSourceNode = null;
        this.mediaStreamAudioDestinationNode = null;
        this.hasInitScriptNode = false;
        this.audioSyncVideoOption = {
          diff: null
        };
        this.off();
        this.player.debug.log('AudioContext', 'destroy');
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
          player.emit(EVENTS.kBps, (rate / 1024).toFixed(2));
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
            reader.read().then(_ref => {
              let {
                done,
                value
              } = _ref;

              if (done) {
                demux.close();
              } else {
                this.streamRate && this.streamRate(value.byteLength);
                demux.dispatch(value);
                fetchNext();
              }
            }).catch(e => {
              demux.close(); // 这边会报用户 aborted a request 错误。

              this.emit(EVENTS_ERROR.fetchError, e);
              this.player.emit(EVENTS.error, EVENTS_ERROR.fetchError);
              this.abort();
            });
          };

          fetchNext();
        }).catch(e => {
          this.abort();
          this.emit(EVENTS_ERROR.fetchError, e);
          this.player.emit(EVENTS.error, EVENTS_ERROR.fetchError);
        });
      }

      abort() {
        if (this.abortController) {
          this.abortController.abort();
          this.abortController = null;
        }
      }

      destroy() {
        this.abort();
        this.off();
        this.streamRate = null;
        this.player.debug.log('FetchStream', 'destroy');
      }

    }

    class WebsocketLoader extends Emitter {
      constructor(player) {
        super();
        this.player = player;
        this.socket = null;
        this.socketStatus = WEBSOCKET_STATUS.notConnect;
        this.wsUrl = null; //

        this.streamRate = calculationRate(rate => {
          player.emit(EVENTS.kBps, (rate / 1024).toFixed(2));
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
          this.streamRate && this.streamRate(event.data.byteLength);

          this._handleMessage(event.data);
        });
        proxy(this.socket, 'close', () => {
          debug.log('websocketLoader', 'socket close');
          this.emit(EVENTS.streamEnd);
          this.socketStatus = WEBSOCKET_STATUS.close;
        });
        proxy(this.socket, 'error', error => {
          debug.log('websocketLoader', 'socket error');
          this.emit(EVENTS_ERROR.websocketError, error);
          this.player.emit(EVENTS.error, EVENTS_ERROR.websocketError);
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
        this.streamRate = null;
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

    var RecordRTC_1 = createCommonjsModule(function (module) {

    // Last time updated: 2021-03-09 3:20:22 AM UTC

    // ________________
    // RecordRTC v5.6.2

    // Open-Sourced: https://github.com/muaz-khan/RecordRTC

    // --------------------------------------------------
    // Muaz Khan     - www.MuazKhan.com
    // MIT License   - www.WebRTC-Experiment.com/licence
    // --------------------------------------------------

    // ____________
    // RecordRTC.js

    /**
     * {@link https://github.com/muaz-khan/RecordRTC|RecordRTC} is a WebRTC JavaScript library for audio/video as well as screen activity recording. It supports Chrome, Firefox, Opera, Android, and Microsoft Edge. Platforms: Linux, Mac and Windows. 
     * @summary Record audio, video or screen inside the browser.
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @typedef RecordRTC
     * @class
     * @example
     * var recorder = RecordRTC(mediaStream or [arrayOfMediaStream], {
     *     type: 'video', // audio or video or gif or canvas
     *     recorderType: MediaStreamRecorder || CanvasRecorder || StereoAudioRecorder || Etc
     * });
     * recorder.startRecording();
     * @see For further information:
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     * @param {MediaStream} mediaStream - Single media-stream object, array of media-streams, html-canvas-element, etc.
     * @param {object} config - {type:"video", recorderType: MediaStreamRecorder, disableLogs: true, numberOfAudioChannels: 1, bufferSize: 0, sampleRate: 0, desiredSampRate: 16000, video: HTMLVideoElement, etc.}
     */

    function RecordRTC(mediaStream, config) {
        if (!mediaStream) {
            throw 'First parameter is required.';
        }

        config = config || {
            type: 'video'
        };

        config = new RecordRTCConfiguration(mediaStream, config);

        // a reference to user's recordRTC object
        var self = this;

        function startRecording(config2) {
            if (!config.disableLogs) {
                console.log('RecordRTC version: ', self.version);
            }

            if (!!config2) {
                // allow users to set options using startRecording method
                // config2 is similar to main "config" object (second parameter over RecordRTC constructor)
                config = new RecordRTCConfiguration(mediaStream, config2);
            }

            if (!config.disableLogs) {
                console.log('started recording ' + config.type + ' stream.');
            }

            if (mediaRecorder) {
                mediaRecorder.clearRecordedData();
                mediaRecorder.record();

                setState('recording');

                if (self.recordingDuration) {
                    handleRecordingDuration();
                }
                return self;
            }

            initRecorder(function() {
                if (self.recordingDuration) {
                    handleRecordingDuration();
                }
            });

            return self;
        }

        function initRecorder(initCallback) {
            if (initCallback) {
                config.initCallback = function() {
                    initCallback();
                    initCallback = config.initCallback = null; // recorder.initRecorder should be call-backed once.
                };
            }

            var Recorder = new GetRecorderType(mediaStream, config);

            mediaRecorder = new Recorder(mediaStream, config);
            mediaRecorder.record();

            setState('recording');

            if (!config.disableLogs) {
                console.log('Initialized recorderType:', mediaRecorder.constructor.name, 'for output-type:', config.type);
            }
        }

        function stopRecording(callback) {
            callback = callback || function() {};

            if (!mediaRecorder) {
                warningLog();
                return;
            }

            if (self.state === 'paused') {
                self.resumeRecording();

                setTimeout(function() {
                    stopRecording(callback);
                }, 1);
                return;
            }

            if (self.state !== 'recording' && !config.disableLogs) {
                console.warn('Recording state should be: "recording", however current state is: ', self.state);
            }

            if (!config.disableLogs) {
                console.log('Stopped recording ' + config.type + ' stream.');
            }

            if (config.type !== 'gif') {
                mediaRecorder.stop(_callback);
            } else {
                mediaRecorder.stop();
                _callback();
            }

            setState('stopped');

            function _callback(__blob) {
                if (!mediaRecorder) {
                    if (typeof callback.call === 'function') {
                        callback.call(self, '');
                    } else {
                        callback('');
                    }
                    return;
                }

                Object.keys(mediaRecorder).forEach(function(key) {
                    if (typeof mediaRecorder[key] === 'function') {
                        return;
                    }

                    self[key] = mediaRecorder[key];
                });

                var blob = mediaRecorder.blob;

                if (!blob) {
                    if (__blob) {
                        mediaRecorder.blob = blob = __blob;
                    } else {
                        throw 'Recording failed.';
                    }
                }

                if (blob && !config.disableLogs) {
                    console.log(blob.type, '->', bytesToSize(blob.size));
                }

                if (callback) {
                    var url;

                    try {
                        url = URL.createObjectURL(blob);
                    } catch (e) {}

                    if (typeof callback.call === 'function') {
                        callback.call(self, url);
                    } else {
                        callback(url);
                    }
                }

                if (!config.autoWriteToDisk) {
                    return;
                }

                getDataURL(function(dataURL) {
                    var parameter = {};
                    parameter[config.type + 'Blob'] = dataURL;
                    DiskStorage.Store(parameter);
                });
            }
        }

        function pauseRecording() {
            if (!mediaRecorder) {
                warningLog();
                return;
            }

            if (self.state !== 'recording') {
                if (!config.disableLogs) {
                    console.warn('Unable to pause the recording. Recording state: ', self.state);
                }
                return;
            }

            setState('paused');

            mediaRecorder.pause();

            if (!config.disableLogs) {
                console.log('Paused recording.');
            }
        }

        function resumeRecording() {
            if (!mediaRecorder) {
                warningLog();
                return;
            }

            if (self.state !== 'paused') {
                if (!config.disableLogs) {
                    console.warn('Unable to resume the recording. Recording state: ', self.state);
                }
                return;
            }

            setState('recording');

            // not all libs have this method yet
            mediaRecorder.resume();

            if (!config.disableLogs) {
                console.log('Resumed recording.');
            }
        }

        function readFile(_blob) {
            postMessage(new FileReaderSync().readAsDataURL(_blob));
        }

        function getDataURL(callback, _mediaRecorder) {
            if (!callback) {
                throw 'Pass a callback function over getDataURL.';
            }

            var blob = _mediaRecorder ? _mediaRecorder.blob : (mediaRecorder || {}).blob;

            if (!blob) {
                if (!config.disableLogs) {
                    console.warn('Blob encoder did not finish its job yet.');
                }

                setTimeout(function() {
                    getDataURL(callback, _mediaRecorder);
                }, 1000);
                return;
            }

            if (typeof Worker !== 'undefined' && !navigator.mozGetUserMedia) {
                var webWorker = processInWebWorker(readFile);

                webWorker.onmessage = function(event) {
                    callback(event.data);
                };

                webWorker.postMessage(blob);
            } else {
                var reader = new FileReader();
                reader.readAsDataURL(blob);
                reader.onload = function(event) {
                    callback(event.target.result);
                };
            }

            function processInWebWorker(_function) {
                try {
                    var blob = URL.createObjectURL(new Blob([_function.toString(),
                        'this.onmessage =  function (eee) {' + _function.name + '(eee.data);}'
                    ], {
                        type: 'application/javascript'
                    }));

                    var worker = new Worker(blob);
                    URL.revokeObjectURL(blob);
                    return worker;
                } catch (e) {}
            }
        }

        function handleRecordingDuration(counter) {
            counter = counter || 0;

            if (self.state === 'paused') {
                setTimeout(function() {
                    handleRecordingDuration(counter);
                }, 1000);
                return;
            }

            if (self.state === 'stopped') {
                return;
            }

            if (counter >= self.recordingDuration) {
                stopRecording(self.onRecordingStopped);
                return;
            }

            counter += 1000; // 1-second

            setTimeout(function() {
                handleRecordingDuration(counter);
            }, 1000);
        }

        function setState(state) {
            if (!self) {
                return;
            }

            self.state = state;

            if (typeof self.onStateChanged.call === 'function') {
                self.onStateChanged.call(self, state);
            } else {
                self.onStateChanged(state);
            }
        }

        var WARNING = 'It seems that recorder is destroyed or "startRecording" is not invoked for ' + config.type + ' recorder.';

        function warningLog() {
            if (config.disableLogs === true) {
                return;
            }

            console.warn(WARNING);
        }

        var mediaRecorder;

        var returnObject = {
            /**
             * This method starts the recording.
             * @method
             * @memberof RecordRTC
             * @instance
             * @example
             * var recorder = RecordRTC(mediaStream, {
             *     type: 'video'
             * });
             * recorder.startRecording();
             */
            startRecording: startRecording,

            /**
             * This method stops the recording. It is strongly recommended to get "blob" or "URI" inside the callback to make sure all recorders finished their job.
             * @param {function} callback - Callback to get the recorded blob.
             * @method
             * @memberof RecordRTC
             * @instance
             * @example
             * recorder.stopRecording(function() {
             *     // use either "this" or "recorder" object; both are identical
             *     video.src = this.toURL();
             *     var blob = this.getBlob();
             * });
             */
            stopRecording: stopRecording,

            /**
             * This method pauses the recording. You can resume recording using "resumeRecording" method.
             * @method
             * @memberof RecordRTC
             * @instance
             * @todo Firefox is unable to pause the recording. Fix it.
             * @example
             * recorder.pauseRecording();  // pause the recording
             * recorder.resumeRecording(); // resume again
             */
            pauseRecording: pauseRecording,

            /**
             * This method resumes the recording.
             * @method
             * @memberof RecordRTC
             * @instance
             * @example
             * recorder.pauseRecording();  // first of all, pause the recording
             * recorder.resumeRecording(); // now resume it
             */
            resumeRecording: resumeRecording,

            /**
             * This method initializes the recording.
             * @method
             * @memberof RecordRTC
             * @instance
             * @todo This method should be deprecated.
             * @example
             * recorder.initRecorder();
             */
            initRecorder: initRecorder,

            /**
             * Ask RecordRTC to auto-stop the recording after 5 minutes.
             * @method
             * @memberof RecordRTC
             * @instance
             * @example
             * var fiveMinutes = 5 * 1000 * 60;
             * recorder.setRecordingDuration(fiveMinutes, function() {
             *    var blob = this.getBlob();
             *    video.src = this.toURL();
             * });
             * 
             * // or otherwise
             * recorder.setRecordingDuration(fiveMinutes).onRecordingStopped(function() {
             *    var blob = this.getBlob();
             *    video.src = this.toURL();
             * });
             */
            setRecordingDuration: function(recordingDuration, callback) {
                if (typeof recordingDuration === 'undefined') {
                    throw 'recordingDuration is required.';
                }

                if (typeof recordingDuration !== 'number') {
                    throw 'recordingDuration must be a number.';
                }

                self.recordingDuration = recordingDuration;
                self.onRecordingStopped = callback || function() {};

                return {
                    onRecordingStopped: function(callback) {
                        self.onRecordingStopped = callback;
                    }
                };
            },

            /**
             * This method can be used to clear/reset all the recorded data.
             * @method
             * @memberof RecordRTC
             * @instance
             * @todo Figure out the difference between "reset" and "clearRecordedData" methods.
             * @example
             * recorder.clearRecordedData();
             */
            clearRecordedData: function() {
                if (!mediaRecorder) {
                    warningLog();
                    return;
                }

                mediaRecorder.clearRecordedData();

                if (!config.disableLogs) {
                    console.log('Cleared old recorded data.');
                }
            },

            /**
             * Get the recorded blob. Use this method inside the "stopRecording" callback.
             * @method
             * @memberof RecordRTC
             * @instance
             * @example
             * recorder.stopRecording(function() {
             *     var blob = this.getBlob();
             *
             *     var file = new File([blob], 'filename.webm', {
             *         type: 'video/webm'
             *     });
             *
             *     var formData = new FormData();
             *     formData.append('file', file); // upload "File" object rather than a "Blob"
             *     uploadToServer(formData);
             * });
             * @returns {Blob} Returns recorded data as "Blob" object.
             */
            getBlob: function() {
                if (!mediaRecorder) {
                    warningLog();
                    return;
                }

                return mediaRecorder.blob;
            },

            /**
             * Get data-URI instead of Blob.
             * @param {function} callback - Callback to get the Data-URI.
             * @method
             * @memberof RecordRTC
             * @instance
             * @example
             * recorder.stopRecording(function() {
             *     recorder.getDataURL(function(dataURI) {
             *         video.src = dataURI;
             *     });
             * });
             */
            getDataURL: getDataURL,

            /**
             * Get virtual/temporary URL. Usage of this URL is limited to current tab.
             * @method
             * @memberof RecordRTC
             * @instance
             * @example
             * recorder.stopRecording(function() {
             *     video.src = this.toURL();
             * });
             * @returns {String} Returns a virtual/temporary URL for the recorded "Blob".
             */
            toURL: function() {
                if (!mediaRecorder) {
                    warningLog();
                    return;
                }

                return URL.createObjectURL(mediaRecorder.blob);
            },

            /**
             * Get internal recording object (i.e. internal module) e.g. MutliStreamRecorder, MediaStreamRecorder, StereoAudioRecorder or WhammyRecorder etc.
             * @method
             * @memberof RecordRTC
             * @instance
             * @example
             * var internalRecorder = recorder.getInternalRecorder();
             * if(internalRecorder instanceof MultiStreamRecorder) {
             *     internalRecorder.addStreams([newAudioStream]);
             *     internalRecorder.resetVideoStreams([screenStream]);
             * }
             * @returns {Object} Returns internal recording object.
             */
            getInternalRecorder: function() {
                return mediaRecorder;
            },

            /**
             * Invoke save-as dialog to save the recorded blob into your disk.
             * @param {string} fileName - Set your own file name.
             * @method
             * @memberof RecordRTC
             * @instance
             * @example
             * recorder.stopRecording(function() {
             *     this.save('file-name');
             *
             *     // or manually:
             *     invokeSaveAsDialog(this.getBlob(), 'filename.webm');
             * });
             */
            save: function(fileName) {
                if (!mediaRecorder) {
                    warningLog();
                    return;
                }

                invokeSaveAsDialog(mediaRecorder.blob, fileName);
            },

            /**
             * This method gets a blob from indexed-DB storage.
             * @param {function} callback - Callback to get the recorded blob.
             * @method
             * @memberof RecordRTC
             * @instance
             * @example
             * recorder.getFromDisk(function(dataURL) {
             *     video.src = dataURL;
             * });
             */
            getFromDisk: function(callback) {
                if (!mediaRecorder) {
                    warningLog();
                    return;
                }

                RecordRTC.getFromDisk(config.type, callback);
            },

            /**
             * This method appends an array of webp images to the recorded video-blob. It takes an "array" object.
             * @type {Array.<Array>}
             * @param {Array} arrayOfWebPImages - Array of webp images.
             * @method
             * @memberof RecordRTC
             * @instance
             * @todo This method should be deprecated.
             * @example
             * var arrayOfWebPImages = [];
             * arrayOfWebPImages.push({
             *     duration: index,
             *     image: 'data:image/webp;base64,...'
             * });
             * recorder.setAdvertisementArray(arrayOfWebPImages);
             */
            setAdvertisementArray: function(arrayOfWebPImages) {
                config.advertisement = [];

                var length = arrayOfWebPImages.length;
                for (var i = 0; i < length; i++) {
                    config.advertisement.push({
                        duration: i,
                        image: arrayOfWebPImages[i]
                    });
                }
            },

            /**
             * It is equivalent to <code class="str">"recorder.getBlob()"</code> method. Usage of "getBlob" is recommended, though.
             * @property {Blob} blob - Recorded Blob can be accessed using this property.
             * @memberof RecordRTC
             * @instance
             * @readonly
             * @example
             * recorder.stopRecording(function() {
             *     var blob = this.blob;
             *
             *     // below one is recommended
             *     var blob = this.getBlob();
             * });
             */
            blob: null,

            /**
             * This works only with {recorderType:StereoAudioRecorder}. Use this property on "stopRecording" to verify the encoder's sample-rates.
             * @property {number} bufferSize - Buffer-size used to encode the WAV container
             * @memberof RecordRTC
             * @instance
             * @readonly
             * @example
             * recorder.stopRecording(function() {
             *     alert('Recorder used this buffer-size: ' + this.bufferSize);
             * });
             */
            bufferSize: 0,

            /**
             * This works only with {recorderType:StereoAudioRecorder}. Use this property on "stopRecording" to verify the encoder's sample-rates.
             * @property {number} sampleRate - Sample-rates used to encode the WAV container
             * @memberof RecordRTC
             * @instance
             * @readonly
             * @example
             * recorder.stopRecording(function() {
             *     alert('Recorder used these sample-rates: ' + this.sampleRate);
             * });
             */
            sampleRate: 0,

            /**
             * {recorderType:StereoAudioRecorder} returns ArrayBuffer object.
             * @property {ArrayBuffer} buffer - Audio ArrayBuffer, supported only in Chrome.
             * @memberof RecordRTC
             * @instance
             * @readonly
             * @example
             * recorder.stopRecording(function() {
             *     var arrayBuffer = this.buffer;
             *     alert(arrayBuffer.byteLength);
             * });
             */
            buffer: null,

            /**
             * This method resets the recorder. So that you can reuse single recorder instance many times.
             * @method
             * @memberof RecordRTC
             * @instance
             * @example
             * recorder.reset();
             * recorder.startRecording();
             */
            reset: function() {
                if (self.state === 'recording' && !config.disableLogs) {
                    console.warn('Stop an active recorder.');
                }

                if (mediaRecorder && typeof mediaRecorder.clearRecordedData === 'function') {
                    mediaRecorder.clearRecordedData();
                }
                mediaRecorder = null;
                setState('inactive');
                self.blob = null;
            },

            /**
             * This method is called whenever recorder's state changes. Use this as an "event".
             * @property {String} state - A recorder's state can be: recording, paused, stopped or inactive.
             * @method
             * @memberof RecordRTC
             * @instance
             * @example
             * recorder.onStateChanged = function(state) {
             *     console.log('Recorder state: ', state);
             * };
             */
            onStateChanged: function(state) {
                if (!config.disableLogs) {
                    console.log('Recorder state changed:', state);
                }
            },

            /**
             * A recorder can have inactive, recording, paused or stopped states.
             * @property {String} state - A recorder's state can be: recording, paused, stopped or inactive.
             * @memberof RecordRTC
             * @static
             * @readonly
             * @example
             * // this looper function will keep you updated about the recorder's states.
             * (function looper() {
             *     document.querySelector('h1').innerHTML = 'Recorder\'s state is: ' + recorder.state;
             *     if(recorder.state === 'stopped') return; // ignore+stop
             *     setTimeout(looper, 1000); // update after every 3-seconds
             * })();
             * recorder.startRecording();
             */
            state: 'inactive',

            /**
             * Get recorder's readonly state.
             * @method
             * @memberof RecordRTC
             * @example
             * var state = recorder.getState();
             * @returns {String} Returns recording state.
             */
            getState: function() {
                return self.state;
            },

            /**
             * Destroy RecordRTC instance. Clear all recorders and objects.
             * @method
             * @memberof RecordRTC
             * @example
             * recorder.destroy();
             */
            destroy: function() {
                var disableLogsCache = config.disableLogs;

                config = {
                    disableLogs: true
                };
                self.reset();
                setState('destroyed');
                returnObject = self = null;

                if (Storage.AudioContextConstructor) {
                    Storage.AudioContextConstructor.close();
                    Storage.AudioContextConstructor = null;
                }

                config.disableLogs = disableLogsCache;

                if (!config.disableLogs) {
                    console.log('RecordRTC is destroyed.');
                }
            },

            /**
             * RecordRTC version number
             * @property {String} version - Release version number.
             * @memberof RecordRTC
             * @static
             * @readonly
             * @example
             * alert(recorder.version);
             */
            version: '5.6.2'
        };

        if (!this) {
            self = returnObject;
            return returnObject;
        }

        // if someone wants to use RecordRTC with the "new" keyword.
        for (var prop in returnObject) {
            this[prop] = returnObject[prop];
        }

        self = this;

        return returnObject;
    }

    RecordRTC.version = '5.6.2';

    {
        module.exports = RecordRTC;
    }

    RecordRTC.getFromDisk = function(type, callback) {
        if (!callback) {
            throw 'callback is mandatory.';
        }

        console.log('Getting recorded ' + (type === 'all' ? 'blobs' : type + ' blob ') + ' from disk!');
        DiskStorage.Fetch(function(dataURL, _type) {
            if (type !== 'all' && _type === type + 'Blob' && callback) {
                callback(dataURL);
            }

            if (type === 'all' && callback) {
                callback(dataURL, _type.replace('Blob', ''));
            }
        });
    };

    /**
     * This method can be used to store recorded blobs into IndexedDB storage.
     * @param {object} options - {audio: Blob, video: Blob, gif: Blob}
     * @method
     * @memberof RecordRTC
     * @example
     * RecordRTC.writeToDisk({
     *     audio: audioBlob,
     *     video: videoBlob,
     *     gif  : gifBlob
     * });
     */
    RecordRTC.writeToDisk = function(options) {
        console.log('Writing recorded blob(s) to disk!');
        options = options || {};
        if (options.audio && options.video && options.gif) {
            options.audio.getDataURL(function(audioDataURL) {
                options.video.getDataURL(function(videoDataURL) {
                    options.gif.getDataURL(function(gifDataURL) {
                        DiskStorage.Store({
                            audioBlob: audioDataURL,
                            videoBlob: videoDataURL,
                            gifBlob: gifDataURL
                        });
                    });
                });
            });
        } else if (options.audio && options.video) {
            options.audio.getDataURL(function(audioDataURL) {
                options.video.getDataURL(function(videoDataURL) {
                    DiskStorage.Store({
                        audioBlob: audioDataURL,
                        videoBlob: videoDataURL
                    });
                });
            });
        } else if (options.audio && options.gif) {
            options.audio.getDataURL(function(audioDataURL) {
                options.gif.getDataURL(function(gifDataURL) {
                    DiskStorage.Store({
                        audioBlob: audioDataURL,
                        gifBlob: gifDataURL
                    });
                });
            });
        } else if (options.video && options.gif) {
            options.video.getDataURL(function(videoDataURL) {
                options.gif.getDataURL(function(gifDataURL) {
                    DiskStorage.Store({
                        videoBlob: videoDataURL,
                        gifBlob: gifDataURL
                    });
                });
            });
        } else if (options.audio) {
            options.audio.getDataURL(function(audioDataURL) {
                DiskStorage.Store({
                    audioBlob: audioDataURL
                });
            });
        } else if (options.video) {
            options.video.getDataURL(function(videoDataURL) {
                DiskStorage.Store({
                    videoBlob: videoDataURL
                });
            });
        } else if (options.gif) {
            options.gif.getDataURL(function(gifDataURL) {
                DiskStorage.Store({
                    gifBlob: gifDataURL
                });
            });
        }
    };

    // __________________________
    // RecordRTC-Configuration.js

    /**
     * {@link RecordRTCConfiguration} is an inner/private helper for {@link RecordRTC}.
     * @summary It configures the 2nd parameter passed over {@link RecordRTC} and returns a valid "config" object.
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @typedef RecordRTCConfiguration
     * @class
     * @example
     * var options = RecordRTCConfiguration(mediaStream, options);
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     * @param {MediaStream} mediaStream - MediaStream object fetched using getUserMedia API or generated using captureStreamUntilEnded or WebAudio API.
     * @param {object} config - {type:"video", disableLogs: true, numberOfAudioChannels: 1, bufferSize: 0, sampleRate: 0, video: HTMLVideoElement, getNativeBlob:true, etc.}
     */

    function RecordRTCConfiguration(mediaStream, config) {
        if (!config.recorderType && !config.type) {
            if (!!config.audio && !!config.video) {
                config.type = 'video';
            } else if (!!config.audio && !config.video) {
                config.type = 'audio';
            }
        }

        if (config.recorderType && !config.type) {
            if (config.recorderType === WhammyRecorder || config.recorderType === CanvasRecorder || (typeof WebAssemblyRecorder !== 'undefined' && config.recorderType === WebAssemblyRecorder)) {
                config.type = 'video';
            } else if (config.recorderType === GifRecorder) {
                config.type = 'gif';
            } else if (config.recorderType === StereoAudioRecorder) {
                config.type = 'audio';
            } else if (config.recorderType === MediaStreamRecorder) {
                if (getTracks(mediaStream, 'audio').length && getTracks(mediaStream, 'video').length) {
                    config.type = 'video';
                } else if (!getTracks(mediaStream, 'audio').length && getTracks(mediaStream, 'video').length) {
                    config.type = 'video';
                } else if (getTracks(mediaStream, 'audio').length && !getTracks(mediaStream, 'video').length) {
                    config.type = 'audio';
                } else ;
            }
        }

        if (typeof MediaStreamRecorder !== 'undefined' && typeof MediaRecorder !== 'undefined' && 'requestData' in MediaRecorder.prototype) {
            if (!config.mimeType) {
                config.mimeType = 'video/webm';
            }

            if (!config.type) {
                config.type = config.mimeType.split('/')[0];
            }

            if (!config.bitsPerSecond) ;
        }

        // consider default type=audio
        if (!config.type) {
            if (config.mimeType) {
                config.type = config.mimeType.split('/')[0];
            }
            if (!config.type) {
                config.type = 'audio';
            }
        }

        return config;
    }

    // __________________
    // GetRecorderType.js

    /**
     * {@link GetRecorderType} is an inner/private helper for {@link RecordRTC}.
     * @summary It returns best recorder-type available for your browser.
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @typedef GetRecorderType
     * @class
     * @example
     * var RecorderType = GetRecorderType(options);
     * var recorder = new RecorderType(options);
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     * @param {MediaStream} mediaStream - MediaStream object fetched using getUserMedia API or generated using captureStreamUntilEnded or WebAudio API.
     * @param {object} config - {type:"video", disableLogs: true, numberOfAudioChannels: 1, bufferSize: 0, sampleRate: 0, video: HTMLVideoElement, etc.}
     */

    function GetRecorderType(mediaStream, config) {
        var recorder;

        // StereoAudioRecorder can work with all three: Edge, Firefox and Chrome
        // todo: detect if it is Edge, then auto use: StereoAudioRecorder
        if (isChrome || isEdge || isOpera) {
            // Media Stream Recording API has not been implemented in chrome yet;
            // That's why using WebAudio API to record stereo audio in WAV format
            recorder = StereoAudioRecorder;
        }

        if (typeof MediaRecorder !== 'undefined' && 'requestData' in MediaRecorder.prototype && !isChrome) {
            recorder = MediaStreamRecorder;
        }

        // video recorder (in WebM format)
        if (config.type === 'video' && (isChrome || isOpera)) {
            recorder = WhammyRecorder;

            if (typeof WebAssemblyRecorder !== 'undefined' && typeof ReadableStream !== 'undefined') {
                recorder = WebAssemblyRecorder;
            }
        }

        // video recorder (in Gif format)
        if (config.type === 'gif') {
            recorder = GifRecorder;
        }

        // html2canvas recording!
        if (config.type === 'canvas') {
            recorder = CanvasRecorder;
        }

        if (isMediaRecorderCompatible() && recorder !== CanvasRecorder && recorder !== GifRecorder && typeof MediaRecorder !== 'undefined' && 'requestData' in MediaRecorder.prototype) {
            if (getTracks(mediaStream, 'video').length || getTracks(mediaStream, 'audio').length) {
                // audio-only recording
                if (config.type === 'audio') {
                    if (typeof MediaRecorder.isTypeSupported === 'function' && MediaRecorder.isTypeSupported('audio/webm')) {
                        recorder = MediaStreamRecorder;
                    }
                    // else recorder = StereoAudioRecorder;
                } else {
                    // video or screen tracks
                    if (typeof MediaRecorder.isTypeSupported === 'function' && MediaRecorder.isTypeSupported('video/webm')) {
                        recorder = MediaStreamRecorder;
                    }
                }
            }
        }

        if (mediaStream instanceof Array && mediaStream.length) {
            recorder = MultiStreamRecorder;
        }

        if (config.recorderType) {
            recorder = config.recorderType;
        }

        if (!config.disableLogs && !!recorder && !!recorder.name) {
            console.log('Using recorderType:', recorder.name || recorder.constructor.name);
        }

        if (!recorder && isSafari) {
            recorder = MediaStreamRecorder;
        }

        return recorder;
    }

    // _____________
    // MRecordRTC.js

    /**
     * MRecordRTC runs on top of {@link RecordRTC} to bring multiple recordings in a single place, by providing simple API.
     * @summary MRecordRTC stands for "Multiple-RecordRTC".
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @typedef MRecordRTC
     * @class
     * @example
     * var recorder = new MRecordRTC();
     * recorder.addStream(MediaStream);
     * recorder.mediaType = {
     *     audio: true, // or StereoAudioRecorder or MediaStreamRecorder
     *     video: true, // or WhammyRecorder or MediaStreamRecorder or WebAssemblyRecorder or CanvasRecorder
     *     gif: true    // or GifRecorder
     * };
     * // mimeType is optional and should be set only in advance cases.
     * recorder.mimeType = {
     *     audio: 'audio/wav',
     *     video: 'video/webm',
     *     gif:   'image/gif'
     * };
     * recorder.startRecording();
     * @see For further information:
     * @see {@link https://github.com/muaz-khan/RecordRTC/tree/master/MRecordRTC|MRecordRTC Source Code}
     * @param {MediaStream} mediaStream - MediaStream object fetched using getUserMedia API or generated using captureStreamUntilEnded or WebAudio API.
     * @requires {@link RecordRTC}
     */

    function MRecordRTC(mediaStream) {

        /**
         * This method attaches MediaStream object to {@link MRecordRTC}.
         * @param {MediaStream} mediaStream - A MediaStream object, either fetched using getUserMedia API, or generated using captureStreamUntilEnded or WebAudio API.
         * @method
         * @memberof MRecordRTC
         * @example
         * recorder.addStream(MediaStream);
         */
        this.addStream = function(_mediaStream) {
            if (_mediaStream) {
                mediaStream = _mediaStream;
            }
        };

        /**
         * This property can be used to set the recording type e.g. audio, or video, or gif, or canvas.
         * @property {object} mediaType - {audio: true, video: true, gif: true}
         * @memberof MRecordRTC
         * @example
         * var recorder = new MRecordRTC();
         * recorder.mediaType = {
         *     audio: true, // TRUE or StereoAudioRecorder or MediaStreamRecorder
         *     video: true, // TRUE or WhammyRecorder or MediaStreamRecorder or WebAssemblyRecorder or CanvasRecorder
         *     gif  : true  // TRUE or GifRecorder
         * };
         */
        this.mediaType = {
            audio: true,
            video: true
        };

        /**
         * This method starts recording.
         * @method
         * @memberof MRecordRTC
         * @example
         * recorder.startRecording();
         */
        this.startRecording = function() {
            var mediaType = this.mediaType;
            var recorderType;
            var mimeType = this.mimeType || {
                audio: null,
                video: null,
                gif: null
            };

            if (typeof mediaType.audio !== 'function' && isMediaRecorderCompatible() && !getTracks(mediaStream, 'audio').length) {
                mediaType.audio = false;
            }

            if (typeof mediaType.video !== 'function' && isMediaRecorderCompatible() && !getTracks(mediaStream, 'video').length) {
                mediaType.video = false;
            }

            if (typeof mediaType.gif !== 'function' && isMediaRecorderCompatible() && !getTracks(mediaStream, 'video').length) {
                mediaType.gif = false;
            }

            if (!mediaType.audio && !mediaType.video && !mediaType.gif) {
                throw 'MediaStream must have either audio or video tracks.';
            }

            if (!!mediaType.audio) {
                recorderType = null;
                if (typeof mediaType.audio === 'function') {
                    recorderType = mediaType.audio;
                }

                this.audioRecorder = new RecordRTC(mediaStream, {
                    type: 'audio',
                    bufferSize: this.bufferSize,
                    sampleRate: this.sampleRate,
                    numberOfAudioChannels: this.numberOfAudioChannels || 2,
                    disableLogs: this.disableLogs,
                    recorderType: recorderType,
                    mimeType: mimeType.audio,
                    timeSlice: this.timeSlice,
                    onTimeStamp: this.onTimeStamp
                });

                if (!mediaType.video) {
                    this.audioRecorder.startRecording();
                }
            }

            if (!!mediaType.video) {
                recorderType = null;
                if (typeof mediaType.video === 'function') {
                    recorderType = mediaType.video;
                }

                var newStream = mediaStream;

                if (isMediaRecorderCompatible() && !!mediaType.audio && typeof mediaType.audio === 'function') {
                    var videoTrack = getTracks(mediaStream, 'video')[0];

                    if (isFirefox) {
                        newStream = new MediaStream();
                        newStream.addTrack(videoTrack);

                        if (recorderType && recorderType === WhammyRecorder) {
                            // Firefox does NOT supports webp-encoding yet
                            // But Firefox do supports WebAssemblyRecorder
                            recorderType = MediaStreamRecorder;
                        }
                    } else {
                        newStream = new MediaStream();
                        newStream.addTrack(videoTrack);
                    }
                }

                this.videoRecorder = new RecordRTC(newStream, {
                    type: 'video',
                    video: this.video,
                    canvas: this.canvas,
                    frameInterval: this.frameInterval || 10,
                    disableLogs: this.disableLogs,
                    recorderType: recorderType,
                    mimeType: mimeType.video,
                    timeSlice: this.timeSlice,
                    onTimeStamp: this.onTimeStamp,
                    workerPath: this.workerPath,
                    webAssemblyPath: this.webAssemblyPath,
                    frameRate: this.frameRate, // used by WebAssemblyRecorder; values: usually 30; accepts any.
                    bitrate: this.bitrate // used by WebAssemblyRecorder; values: 0 to 1000+
                });

                if (!mediaType.audio) {
                    this.videoRecorder.startRecording();
                }
            }

            if (!!mediaType.audio && !!mediaType.video) {
                var self = this;

                var isSingleRecorder = isMediaRecorderCompatible() === true;

                if (mediaType.audio instanceof StereoAudioRecorder && !!mediaType.video) {
                    isSingleRecorder = false;
                } else if (mediaType.audio !== true && mediaType.video !== true && mediaType.audio !== mediaType.video) {
                    isSingleRecorder = false;
                }

                if (isSingleRecorder === true) {
                    self.audioRecorder = null;
                    self.videoRecorder.startRecording();
                } else {
                    self.videoRecorder.initRecorder(function() {
                        self.audioRecorder.initRecorder(function() {
                            // Both recorders are ready to record things accurately
                            self.videoRecorder.startRecording();
                            self.audioRecorder.startRecording();
                        });
                    });
                }
            }

            if (!!mediaType.gif) {
                recorderType = null;
                if (typeof mediaType.gif === 'function') {
                    recorderType = mediaType.gif;
                }
                this.gifRecorder = new RecordRTC(mediaStream, {
                    type: 'gif',
                    frameRate: this.frameRate || 200,
                    quality: this.quality || 10,
                    disableLogs: this.disableLogs,
                    recorderType: recorderType,
                    mimeType: mimeType.gif
                });
                this.gifRecorder.startRecording();
            }
        };

        /**
         * This method stops recording.
         * @param {function} callback - Callback function is invoked when all encoders finished their jobs.
         * @method
         * @memberof MRecordRTC
         * @example
         * recorder.stopRecording(function(recording){
         *     var audioBlob = recording.audio;
         *     var videoBlob = recording.video;
         *     var gifBlob   = recording.gif;
         * });
         */
        this.stopRecording = function(callback) {
            callback = callback || function() {};

            if (this.audioRecorder) {
                this.audioRecorder.stopRecording(function(blobURL) {
                    callback(blobURL, 'audio');
                });
            }

            if (this.videoRecorder) {
                this.videoRecorder.stopRecording(function(blobURL) {
                    callback(blobURL, 'video');
                });
            }

            if (this.gifRecorder) {
                this.gifRecorder.stopRecording(function(blobURL) {
                    callback(blobURL, 'gif');
                });
            }
        };

        /**
         * This method pauses recording.
         * @method
         * @memberof MRecordRTC
         * @example
         * recorder.pauseRecording();
         */
        this.pauseRecording = function() {
            if (this.audioRecorder) {
                this.audioRecorder.pauseRecording();
            }

            if (this.videoRecorder) {
                this.videoRecorder.pauseRecording();
            }

            if (this.gifRecorder) {
                this.gifRecorder.pauseRecording();
            }
        };

        /**
         * This method resumes recording.
         * @method
         * @memberof MRecordRTC
         * @example
         * recorder.resumeRecording();
         */
        this.resumeRecording = function() {
            if (this.audioRecorder) {
                this.audioRecorder.resumeRecording();
            }

            if (this.videoRecorder) {
                this.videoRecorder.resumeRecording();
            }

            if (this.gifRecorder) {
                this.gifRecorder.resumeRecording();
            }
        };

        /**
         * This method can be used to manually get all recorded blobs.
         * @param {function} callback - All recorded blobs are passed back to the "callback" function.
         * @method
         * @memberof MRecordRTC
         * @example
         * recorder.getBlob(function(recording){
         *     var audioBlob = recording.audio;
         *     var videoBlob = recording.video;
         *     var gifBlob   = recording.gif;
         * });
         * // or
         * var audioBlob = recorder.getBlob().audio;
         * var videoBlob = recorder.getBlob().video;
         */
        this.getBlob = function(callback) {
            var output = {};

            if (this.audioRecorder) {
                output.audio = this.audioRecorder.getBlob();
            }

            if (this.videoRecorder) {
                output.video = this.videoRecorder.getBlob();
            }

            if (this.gifRecorder) {
                output.gif = this.gifRecorder.getBlob();
            }

            if (callback) {
                callback(output);
            }

            return output;
        };

        /**
         * Destroy all recorder instances.
         * @method
         * @memberof MRecordRTC
         * @example
         * recorder.destroy();
         */
        this.destroy = function() {
            if (this.audioRecorder) {
                this.audioRecorder.destroy();
                this.audioRecorder = null;
            }

            if (this.videoRecorder) {
                this.videoRecorder.destroy();
                this.videoRecorder = null;
            }

            if (this.gifRecorder) {
                this.gifRecorder.destroy();
                this.gifRecorder = null;
            }
        };

        /**
         * This method can be used to manually get all recorded blobs' DataURLs.
         * @param {function} callback - All recorded blobs' DataURLs are passed back to the "callback" function.
         * @method
         * @memberof MRecordRTC
         * @example
         * recorder.getDataURL(function(recording){
         *     var audioDataURL = recording.audio;
         *     var videoDataURL = recording.video;
         *     var gifDataURL   = recording.gif;
         * });
         */
        this.getDataURL = function(callback) {
            this.getBlob(function(blob) {
                if (blob.audio && blob.video) {
                    getDataURL(blob.audio, function(_audioDataURL) {
                        getDataURL(blob.video, function(_videoDataURL) {
                            callback({
                                audio: _audioDataURL,
                                video: _videoDataURL
                            });
                        });
                    });
                } else if (blob.audio) {
                    getDataURL(blob.audio, function(_audioDataURL) {
                        callback({
                            audio: _audioDataURL
                        });
                    });
                } else if (blob.video) {
                    getDataURL(blob.video, function(_videoDataURL) {
                        callback({
                            video: _videoDataURL
                        });
                    });
                }
            });

            function getDataURL(blob, callback00) {
                if (typeof Worker !== 'undefined') {
                    var webWorker = processInWebWorker(function readFile(_blob) {
                        postMessage(new FileReaderSync().readAsDataURL(_blob));
                    });

                    webWorker.onmessage = function(event) {
                        callback00(event.data);
                    };

                    webWorker.postMessage(blob);
                } else {
                    var reader = new FileReader();
                    reader.readAsDataURL(blob);
                    reader.onload = function(event) {
                        callback00(event.target.result);
                    };
                }
            }

            function processInWebWorker(_function) {
                var blob = URL.createObjectURL(new Blob([_function.toString(),
                    'this.onmessage =  function (eee) {' + _function.name + '(eee.data);}'
                ], {
                    type: 'application/javascript'
                }));

                var worker = new Worker(blob);
                var url;
                if (typeof URL !== 'undefined') {
                    url = URL;
                } else if (typeof webkitURL !== 'undefined') {
                    url = webkitURL;
                } else {
                    throw 'Neither URL nor webkitURL detected.';
                }
                url.revokeObjectURL(blob);
                return worker;
            }
        };

        /**
         * This method can be used to ask {@link MRecordRTC} to write all recorded blobs into IndexedDB storage.
         * @method
         * @memberof MRecordRTC
         * @example
         * recorder.writeToDisk();
         */
        this.writeToDisk = function() {
            RecordRTC.writeToDisk({
                audio: this.audioRecorder,
                video: this.videoRecorder,
                gif: this.gifRecorder
            });
        };

        /**
         * This method can be used to invoke a save-as dialog for all recorded blobs.
         * @param {object} args - {audio: 'audio-name', video: 'video-name', gif: 'gif-name'}
         * @method
         * @memberof MRecordRTC
         * @example
         * recorder.save({
         *     audio: 'audio-file-name',
         *     video: 'video-file-name',
         *     gif  : 'gif-file-name'
         * });
         */
        this.save = function(args) {
            args = args || {
                audio: true,
                video: true,
                gif: true
            };

            if (!!args.audio && this.audioRecorder) {
                this.audioRecorder.save(typeof args.audio === 'string' ? args.audio : '');
            }

            if (!!args.video && this.videoRecorder) {
                this.videoRecorder.save(typeof args.video === 'string' ? args.video : '');
            }
            if (!!args.gif && this.gifRecorder) {
                this.gifRecorder.save(typeof args.gif === 'string' ? args.gif : '');
            }
        };
    }

    /**
     * This method can be used to get all recorded blobs from IndexedDB storage.
     * @param {string} type - 'all' or 'audio' or 'video' or 'gif'
     * @param {function} callback - Callback function to get all stored blobs.
     * @method
     * @memberof MRecordRTC
     * @example
     * MRecordRTC.getFromDisk('all', function(dataURL, type){
     *     if(type === 'audio') { }
     *     if(type === 'video') { }
     *     if(type === 'gif')   { }
     * });
     */
    MRecordRTC.getFromDisk = RecordRTC.getFromDisk;

    /**
     * This method can be used to store recorded blobs into IndexedDB storage.
     * @param {object} options - {audio: Blob, video: Blob, gif: Blob}
     * @method
     * @memberof MRecordRTC
     * @example
     * MRecordRTC.writeToDisk({
     *     audio: audioBlob,
     *     video: videoBlob,
     *     gif  : gifBlob
     * });
     */
    MRecordRTC.writeToDisk = RecordRTC.writeToDisk;

    if (typeof RecordRTC !== 'undefined') {
        RecordRTC.MRecordRTC = MRecordRTC;
    }

    var browserFakeUserAgent = 'Fake/5.0 (FakeOS) AppleWebKit/123 (KHTML, like Gecko) Fake/12.3.4567.89 Fake/123.45';

    (function(that) {
        if (!that) {
            return;
        }

        if (typeof window !== 'undefined') {
            return;
        }

        if (typeof commonjsGlobal === 'undefined') {
            return;
        }

        commonjsGlobal.navigator = {
            userAgent: browserFakeUserAgent,
            getUserMedia: function() {}
        };

        if (!commonjsGlobal.console) {
            commonjsGlobal.console = {};
        }

        if (typeof commonjsGlobal.console.log === 'undefined' || typeof commonjsGlobal.console.error === 'undefined') {
            commonjsGlobal.console.error = commonjsGlobal.console.log = commonjsGlobal.console.log || function() {
                console.log(arguments);
            };
        }

        if (typeof document === 'undefined') {
            /*global document:true */
            that.document = {
                documentElement: {
                    appendChild: function() {
                        return '';
                    }
                }
            };

            document.createElement = document.captureStream = document.mozCaptureStream = function() {
                var obj = {
                    getContext: function() {
                        return obj;
                    },
                    play: function() {},
                    pause: function() {},
                    drawImage: function() {},
                    toDataURL: function() {
                        return '';
                    },
                    style: {}
                };
                return obj;
            };

            that.HTMLVideoElement = function() {};
        }

        if (typeof location === 'undefined') {
            /*global location:true */
            that.location = {
                protocol: 'file:',
                href: '',
                hash: ''
            };
        }

        if (typeof screen === 'undefined') {
            /*global screen:true */
            that.screen = {
                width: 0,
                height: 0
            };
        }

        if (typeof URL === 'undefined') {
            /*global screen:true */
            that.URL = {
                createObjectURL: function() {
                    return '';
                },
                revokeObjectURL: function() {
                    return '';
                }
            };
        }

        /*global window:true */
        that.window = commonjsGlobal;
    })(typeof commonjsGlobal !== 'undefined' ? commonjsGlobal : null);

    // _____________________________
    // Cross-Browser-Declarations.js

    // animation-frame used in WebM recording

    /*jshint -W079 */
    var requestAnimationFrame = window.requestAnimationFrame;
    if (typeof requestAnimationFrame === 'undefined') {
        if (typeof webkitRequestAnimationFrame !== 'undefined') {
            /*global requestAnimationFrame:true */
            requestAnimationFrame = webkitRequestAnimationFrame;
        } else if (typeof mozRequestAnimationFrame !== 'undefined') {
            /*global requestAnimationFrame:true */
            requestAnimationFrame = mozRequestAnimationFrame;
        } else if (typeof msRequestAnimationFrame !== 'undefined') {
            /*global requestAnimationFrame:true */
            requestAnimationFrame = msRequestAnimationFrame;
        } else if (typeof requestAnimationFrame === 'undefined') {
            // via: https://gist.github.com/paulirish/1579671
            var lastTime = 0;

            /*global requestAnimationFrame:true */
            requestAnimationFrame = function(callback, element) {
                var currTime = new Date().getTime();
                var timeToCall = Math.max(0, 16 - (currTime - lastTime));
                var id = setTimeout(function() {
                    callback(currTime + timeToCall);
                }, timeToCall);
                lastTime = currTime + timeToCall;
                return id;
            };
        }
    }

    /*jshint -W079 */
    var cancelAnimationFrame = window.cancelAnimationFrame;
    if (typeof cancelAnimationFrame === 'undefined') {
        if (typeof webkitCancelAnimationFrame !== 'undefined') {
            /*global cancelAnimationFrame:true */
            cancelAnimationFrame = webkitCancelAnimationFrame;
        } else if (typeof mozCancelAnimationFrame !== 'undefined') {
            /*global cancelAnimationFrame:true */
            cancelAnimationFrame = mozCancelAnimationFrame;
        } else if (typeof msCancelAnimationFrame !== 'undefined') {
            /*global cancelAnimationFrame:true */
            cancelAnimationFrame = msCancelAnimationFrame;
        } else if (typeof cancelAnimationFrame === 'undefined') {
            /*global cancelAnimationFrame:true */
            cancelAnimationFrame = function(id) {
                clearTimeout(id);
            };
        }
    }

    // WebAudio API representer
    var AudioContext = window.AudioContext;

    if (typeof AudioContext === 'undefined') {
        if (typeof webkitAudioContext !== 'undefined') {
            /*global AudioContext:true */
            AudioContext = webkitAudioContext;
        }

        if (typeof mozAudioContext !== 'undefined') {
            /*global AudioContext:true */
            AudioContext = mozAudioContext;
        }
    }

    /*jshint -W079 */
    var URL = window.URL;

    if (typeof URL === 'undefined' && typeof webkitURL !== 'undefined') {
        /*global URL:true */
        URL = webkitURL;
    }

    if (typeof navigator !== 'undefined' && typeof navigator.getUserMedia === 'undefined') { // maybe window.navigator?
        if (typeof navigator.webkitGetUserMedia !== 'undefined') {
            navigator.getUserMedia = navigator.webkitGetUserMedia;
        }

        if (typeof navigator.mozGetUserMedia !== 'undefined') {
            navigator.getUserMedia = navigator.mozGetUserMedia;
        }
    }

    var isEdge = navigator.userAgent.indexOf('Edge') !== -1 && (!!navigator.msSaveBlob || !!navigator.msSaveOrOpenBlob);
    var isOpera = !!window.opera || navigator.userAgent.indexOf('OPR/') !== -1;
    var isFirefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1 && ('netscape' in window) && / rv:/.test(navigator.userAgent);
    var isChrome = (!isOpera && !isEdge && !!navigator.webkitGetUserMedia) || isElectron() || navigator.userAgent.toLowerCase().indexOf('chrome/') !== -1;

    var isSafari = /^((?!chrome|android).)*safari/i.test(navigator.userAgent);

    if (isSafari && !isChrome && navigator.userAgent.indexOf('CriOS') !== -1) {
        isSafari = false;
        isChrome = true;
    }

    var MediaStream = window.MediaStream;

    if (typeof MediaStream === 'undefined' && typeof webkitMediaStream !== 'undefined') {
        MediaStream = webkitMediaStream;
    }

    /*global MediaStream:true */
    if (typeof MediaStream !== 'undefined') {
        // override "stop" method for all browsers
        if (typeof MediaStream.prototype.stop === 'undefined') {
            MediaStream.prototype.stop = function() {
                this.getTracks().forEach(function(track) {
                    track.stop();
                });
            };
        }
    }

    // below function via: http://goo.gl/B3ae8c
    /**
     * Return human-readable file size.
     * @param {number} bytes - Pass bytes and get formatted string.
     * @returns {string} - formatted string
     * @example
     * bytesToSize(1024*1024*5) === '5 GB'
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     */
    function bytesToSize(bytes) {
        var k = 1000;
        var sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
        if (bytes === 0) {
            return '0 Bytes';
        }
        var i = parseInt(Math.floor(Math.log(bytes) / Math.log(k)), 10);
        return (bytes / Math.pow(k, i)).toPrecision(3) + ' ' + sizes[i];
    }

    /**
     * @param {Blob} file - File or Blob object. This parameter is required.
     * @param {string} fileName - Optional file name e.g. "Recorded-Video.webm"
     * @example
     * invokeSaveAsDialog(blob or file, [optional] fileName);
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     */
    function invokeSaveAsDialog(file, fileName) {
        if (!file) {
            throw 'Blob object is required.';
        }

        if (!file.type) {
            try {
                file.type = 'video/webm';
            } catch (e) {}
        }

        var fileExtension = (file.type || 'video/webm').split('/')[1];
        if (fileExtension.indexOf(';') !== -1) {
            // extended mimetype, e.g. 'video/webm;codecs=vp8,opus'
            fileExtension = fileExtension.split(';')[0];
        }
        if (fileName && fileName.indexOf('.') !== -1) {
            var splitted = fileName.split('.');
            fileName = splitted[0];
            fileExtension = splitted[1];
        }

        var fileFullName = (fileName || (Math.round(Math.random() * 9999999999) + 888888888)) + '.' + fileExtension;

        if (typeof navigator.msSaveOrOpenBlob !== 'undefined') {
            return navigator.msSaveOrOpenBlob(file, fileFullName);
        } else if (typeof navigator.msSaveBlob !== 'undefined') {
            return navigator.msSaveBlob(file, fileFullName);
        }

        var hyperlink = document.createElement('a');
        hyperlink.href = URL.createObjectURL(file);
        hyperlink.download = fileFullName;

        hyperlink.style = 'display:none;opacity:0;color:transparent;';
        (document.body || document.documentElement).appendChild(hyperlink);

        if (typeof hyperlink.click === 'function') {
            hyperlink.click();
        } else {
            hyperlink.target = '_blank';
            hyperlink.dispatchEvent(new MouseEvent('click', {
                view: window,
                bubbles: true,
                cancelable: true
            }));
        }

        URL.revokeObjectURL(hyperlink.href);
    }

    /**
     * from: https://github.com/cheton/is-electron/blob/master/index.js
     **/
    function isElectron() {
        // Renderer process
        if (typeof window !== 'undefined' && typeof window.process === 'object' && window.process.type === 'renderer') {
            return true;
        }

        // Main process
        if (typeof process !== 'undefined' && typeof process.versions === 'object' && !!process.versions.electron) {
            return true;
        }

        // Detect the user agent when the `nodeIntegration` option is set to true
        if (typeof navigator === 'object' && typeof navigator.userAgent === 'string' && navigator.userAgent.indexOf('Electron') >= 0) {
            return true;
        }

        return false;
    }

    function getTracks(stream, kind) {
        if (!stream || !stream.getTracks) {
            return [];
        }

        return stream.getTracks().filter(function(t) {
            return t.kind === (kind || 'audio');
        });
    }

    function setSrcObject(stream, element) {
        if ('srcObject' in element) {
            element.srcObject = stream;
        } else if ('mozSrcObject' in element) {
            element.mozSrcObject = stream;
        } else {
            element.srcObject = stream;
        }
    }

    /**
     * @param {Blob} file - File or Blob object.
     * @param {function} callback - Callback function.
     * @example
     * getSeekableBlob(blob or file, callback);
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     */
    function getSeekableBlob(inputBlob, callback) {
        // EBML.js copyrights goes to: https://github.com/legokichi/ts-ebml
        if (typeof EBML === 'undefined') {
            throw new Error('Please link: https://www.webrtc-experiment.com/EBML.js');
        }

        var reader = new EBML.Reader();
        var decoder = new EBML.Decoder();
        var tools = EBML.tools;

        var fileReader = new FileReader();
        fileReader.onload = function(e) {
            var ebmlElms = decoder.decode(this.result);
            ebmlElms.forEach(function(element) {
                reader.read(element);
            });
            reader.stop();
            var refinedMetadataBuf = tools.makeMetadataSeekable(reader.metadatas, reader.duration, reader.cues);
            var body = this.result.slice(reader.metadataSize);
            var newBlob = new Blob([refinedMetadataBuf, body], {
                type: 'video/webm'
            });

            callback(newBlob);
        };
        fileReader.readAsArrayBuffer(inputBlob);
    }

    if (typeof RecordRTC !== 'undefined') {
        RecordRTC.invokeSaveAsDialog = invokeSaveAsDialog;
        RecordRTC.getTracks = getTracks;
        RecordRTC.getSeekableBlob = getSeekableBlob;
        RecordRTC.bytesToSize = bytesToSize;
        RecordRTC.isElectron = isElectron;
    }

    // __________ (used to handle stuff like http://goo.gl/xmE5eg) issue #129
    // Storage.js

    /**
     * Storage is a standalone object used by {@link RecordRTC} to store reusable objects e.g. "new AudioContext".
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @example
     * Storage.AudioContext === webkitAudioContext
     * @property {webkitAudioContext} AudioContext - Keeps a reference to AudioContext object.
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     */

    var Storage = {};

    if (typeof AudioContext !== 'undefined') {
        Storage.AudioContext = AudioContext;
    } else if (typeof webkitAudioContext !== 'undefined') {
        Storage.AudioContext = webkitAudioContext;
    }

    if (typeof RecordRTC !== 'undefined') {
        RecordRTC.Storage = Storage;
    }

    function isMediaRecorderCompatible() {
        if (isFirefox || isSafari || isEdge) {
            return true;
        }
        var nAgt = navigator.userAgent;
        var fullVersion = '' + parseFloat(navigator.appVersion);
        var majorVersion = parseInt(navigator.appVersion, 10);
        var verOffset, ix;

        if (isChrome || isOpera) {
            verOffset = nAgt.indexOf('Chrome');
            fullVersion = nAgt.substring(verOffset + 7);
        }

        // trim the fullVersion string at semicolon/space if present
        if ((ix = fullVersion.indexOf(';')) !== -1) {
            fullVersion = fullVersion.substring(0, ix);
        }

        if ((ix = fullVersion.indexOf(' ')) !== -1) {
            fullVersion = fullVersion.substring(0, ix);
        }

        majorVersion = parseInt('' + fullVersion, 10);

        if (isNaN(majorVersion)) {
            fullVersion = '' + parseFloat(navigator.appVersion);
            majorVersion = parseInt(navigator.appVersion, 10);
        }

        return majorVersion >= 49;
    }

    // ______________________
    // MediaStreamRecorder.js

    /**
     * MediaStreamRecorder is an abstraction layer for {@link https://w3c.github.io/mediacapture-record/MediaRecorder.html|MediaRecorder API}. It is used by {@link RecordRTC} to record MediaStream(s) in both Chrome and Firefox.
     * @summary Runs top over {@link https://w3c.github.io/mediacapture-record/MediaRecorder.html|MediaRecorder API}.
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://github.com/muaz-khan|Muaz Khan}
     * @typedef MediaStreamRecorder
     * @class
     * @example
     * var config = {
     *     mimeType: 'video/webm', // vp8, vp9, h264, mkv, opus/vorbis
     *     audioBitsPerSecond : 256 * 8 * 1024,
     *     videoBitsPerSecond : 256 * 8 * 1024,
     *     bitsPerSecond: 256 * 8 * 1024,  // if this is provided, skip above two
     *     checkForInactiveTracks: true,
     *     timeSlice: 1000, // concatenate intervals based blobs
     *     ondataavailable: function() {} // get intervals based blobs
     * }
     * var recorder = new MediaStreamRecorder(mediaStream, config);
     * recorder.record();
     * recorder.stop(function(blob) {
     *     video.src = URL.createObjectURL(blob);
     *
     *     // or
     *     var blob = recorder.blob;
     * });
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     * @param {MediaStream} mediaStream - MediaStream object fetched using getUserMedia API or generated using captureStreamUntilEnded or WebAudio API.
     * @param {object} config - {disableLogs:true, initCallback: function, mimeType: "video/webm", timeSlice: 1000}
     * @throws Will throw an error if first argument "MediaStream" is missing. Also throws error if "MediaRecorder API" are not supported by the browser.
     */

    function MediaStreamRecorder(mediaStream, config) {
        var self = this;

        if (typeof mediaStream === 'undefined') {
            throw 'First argument "MediaStream" is required.';
        }

        if (typeof MediaRecorder === 'undefined') {
            throw 'Your browser does not support the Media Recorder API. Please try other modules e.g. WhammyRecorder or StereoAudioRecorder.';
        }

        config = config || {
            // bitsPerSecond: 256 * 8 * 1024,
            mimeType: 'video/webm'
        };

        if (config.type === 'audio') {
            if (getTracks(mediaStream, 'video').length && getTracks(mediaStream, 'audio').length) {
                var stream;
                if (!!navigator.mozGetUserMedia) {
                    stream = new MediaStream();
                    stream.addTrack(getTracks(mediaStream, 'audio')[0]);
                } else {
                    // webkitMediaStream
                    stream = new MediaStream(getTracks(mediaStream, 'audio'));
                }
                mediaStream = stream;
            }

            if (!config.mimeType || config.mimeType.toString().toLowerCase().indexOf('audio') === -1) {
                config.mimeType = isChrome ? 'audio/webm' : 'audio/ogg';
            }

            if (config.mimeType && config.mimeType.toString().toLowerCase() !== 'audio/ogg' && !!navigator.mozGetUserMedia) {
                // forcing better codecs on Firefox (via #166)
                config.mimeType = 'audio/ogg';
            }
        }

        var arrayOfBlobs = [];

        /**
         * This method returns array of blobs. Use only with "timeSlice". Its useful to preview recording anytime, without using the "stop" method.
         * @method
         * @memberof MediaStreamRecorder
         * @example
         * var arrayOfBlobs = recorder.getArrayOfBlobs();
         * @returns {Array} Returns array of recorded blobs.
         */
        this.getArrayOfBlobs = function() {
            return arrayOfBlobs;
        };

        /**
         * This method records MediaStream.
         * @method
         * @memberof MediaStreamRecorder
         * @example
         * recorder.record();
         */
        this.record = function() {
            // set defaults
            self.blob = null;
            self.clearRecordedData();
            self.timestamps = [];
            allStates = [];
            arrayOfBlobs = [];

            var recorderHints = config;

            if (!config.disableLogs) {
                console.log('Passing following config over MediaRecorder API.', recorderHints);
            }

            if (mediaRecorder) {
                // mandatory to make sure Firefox doesn't fails to record streams 3-4 times without reloading the page.
                mediaRecorder = null;
            }

            if (isChrome && !isMediaRecorderCompatible()) {
                // to support video-only recording on stable
                recorderHints = 'video/vp8';
            }

            if (typeof MediaRecorder.isTypeSupported === 'function' && recorderHints.mimeType) {
                if (!MediaRecorder.isTypeSupported(recorderHints.mimeType)) {
                    if (!config.disableLogs) {
                        console.warn('MediaRecorder API seems unable to record mimeType:', recorderHints.mimeType);
                    }

                    recorderHints.mimeType = config.type === 'audio' ? 'audio/webm' : 'video/webm';
                }
            }

            // using MediaRecorder API here
            try {
                mediaRecorder = new MediaRecorder(mediaStream, recorderHints);

                // reset
                config.mimeType = recorderHints.mimeType;
            } catch (e) {
                // chrome-based fallback
                mediaRecorder = new MediaRecorder(mediaStream);
            }

            // old hack?
            if (recorderHints.mimeType && !MediaRecorder.isTypeSupported && 'canRecordMimeType' in mediaRecorder && mediaRecorder.canRecordMimeType(recorderHints.mimeType) === false) {
                if (!config.disableLogs) {
                    console.warn('MediaRecorder API seems unable to record mimeType:', recorderHints.mimeType);
                }
            }

            // Dispatching OnDataAvailable Handler
            mediaRecorder.ondataavailable = function(e) {
                if (e.data) {
                    allStates.push('ondataavailable: ' + bytesToSize(e.data.size));
                }

                if (typeof config.timeSlice === 'number') {
                    if (e.data && e.data.size) {
                        arrayOfBlobs.push(e.data);
                        updateTimeStamp();

                        if (typeof config.ondataavailable === 'function') {
                            // intervals based blobs
                            var blob = config.getNativeBlob ? e.data : new Blob([e.data], {
                                type: getMimeType(recorderHints)
                            });
                            config.ondataavailable(blob);
                        }
                    }
                    return;
                }

                if (!e.data || !e.data.size || e.data.size < 100 || self.blob) {
                    // make sure that stopRecording always getting fired
                    // even if there is invalid data
                    if (self.recordingCallback) {
                        self.recordingCallback(new Blob([], {
                            type: getMimeType(recorderHints)
                        }));
                        self.recordingCallback = null;
                    }
                    return;
                }

                self.blob = config.getNativeBlob ? e.data : new Blob([e.data], {
                    type: getMimeType(recorderHints)
                });

                if (self.recordingCallback) {
                    self.recordingCallback(self.blob);
                    self.recordingCallback = null;
                }
            };

            mediaRecorder.onstart = function() {
                allStates.push('started');
            };

            mediaRecorder.onpause = function() {
                allStates.push('paused');
            };

            mediaRecorder.onresume = function() {
                allStates.push('resumed');
            };

            mediaRecorder.onstop = function() {
                allStates.push('stopped');
            };

            mediaRecorder.onerror = function(error) {
                if (!error) {
                    return;
                }

                if (!error.name) {
                    error.name = 'UnknownError';
                }

                allStates.push('error: ' + error);

                if (!config.disableLogs) {
                    // via: https://w3c.github.io/mediacapture-record/MediaRecorder.html#exception-summary
                    if (error.name.toString().toLowerCase().indexOf('invalidstate') !== -1) {
                        console.error('The MediaRecorder is not in a state in which the proposed operation is allowed to be executed.', error);
                    } else if (error.name.toString().toLowerCase().indexOf('notsupported') !== -1) {
                        console.error('MIME type (', recorderHints.mimeType, ') is not supported.', error);
                    } else if (error.name.toString().toLowerCase().indexOf('security') !== -1) {
                        console.error('MediaRecorder security error', error);
                    }

                    // older code below
                    else if (error.name === 'OutOfMemory') {
                        console.error('The UA has exhaused the available memory. User agents SHOULD provide as much additional information as possible in the message attribute.', error);
                    } else if (error.name === 'IllegalStreamModification') {
                        console.error('A modification to the stream has occurred that makes it impossible to continue recording. An example would be the addition of a Track while recording is occurring. User agents SHOULD provide as much additional information as possible in the message attribute.', error);
                    } else if (error.name === 'OtherRecordingError') {
                        console.error('Used for an fatal error other than those listed above. User agents SHOULD provide as much additional information as possible in the message attribute.', error);
                    } else if (error.name === 'GenericError') {
                        console.error('The UA cannot provide the codec or recording option that has been requested.', error);
                    } else {
                        console.error('MediaRecorder Error', error);
                    }
                }

                (function(looper) {
                    if (!self.manuallyStopped && mediaRecorder && mediaRecorder.state === 'inactive') {
                        delete config.timeslice;

                        // 10 minutes, enough?
                        mediaRecorder.start(10 * 60 * 1000);
                        return;
                    }

                    setTimeout(looper, 1000);
                })();

                if (mediaRecorder.state !== 'inactive' && mediaRecorder.state !== 'stopped') {
                    mediaRecorder.stop();
                }
            };

            if (typeof config.timeSlice === 'number') {
                updateTimeStamp();
                mediaRecorder.start(config.timeSlice);
            } else {
                // default is 60 minutes; enough?
                // use config => {timeSlice: 1000} otherwise

                mediaRecorder.start(3.6e+6);
            }

            if (config.initCallback) {
                config.initCallback(); // old code
            }
        };

        /**
         * @property {Array} timestamps - Array of time stamps
         * @memberof MediaStreamRecorder
         * @example
         * console.log(recorder.timestamps);
         */
        this.timestamps = [];

        function updateTimeStamp() {
            self.timestamps.push(new Date().getTime());

            if (typeof config.onTimeStamp === 'function') {
                config.onTimeStamp(self.timestamps[self.timestamps.length - 1], self.timestamps);
            }
        }

        function getMimeType(secondObject) {
            if (mediaRecorder && mediaRecorder.mimeType) {
                return mediaRecorder.mimeType;
            }

            return secondObject.mimeType || 'video/webm';
        }

        /**
         * This method stops recording MediaStream.
         * @param {function} callback - Callback function, that is used to pass recorded blob back to the callee.
         * @method
         * @memberof MediaStreamRecorder
         * @example
         * recorder.stop(function(blob) {
         *     video.src = URL.createObjectURL(blob);
         * });
         */
        this.stop = function(callback) {
            callback = callback || function() {};

            self.manuallyStopped = true; // used inside the mediaRecorder.onerror

            if (!mediaRecorder) {
                return;
            }

            this.recordingCallback = callback;

            if (mediaRecorder.state === 'recording') {
                mediaRecorder.stop();
            }

            if (typeof config.timeSlice === 'number') {
                setTimeout(function() {
                    self.blob = new Blob(arrayOfBlobs, {
                        type: getMimeType(config)
                    });

                    self.recordingCallback(self.blob);
                }, 100);
            }
        };

        /**
         * This method pauses the recording process.
         * @method
         * @memberof MediaStreamRecorder
         * @example
         * recorder.pause();
         */
        this.pause = function() {
            if (!mediaRecorder) {
                return;
            }

            if (mediaRecorder.state === 'recording') {
                mediaRecorder.pause();
            }
        };

        /**
         * This method resumes the recording process.
         * @method
         * @memberof MediaStreamRecorder
         * @example
         * recorder.resume();
         */
        this.resume = function() {
            if (!mediaRecorder) {
                return;
            }

            if (mediaRecorder.state === 'paused') {
                mediaRecorder.resume();
            }
        };

        /**
         * This method resets currently recorded data.
         * @method
         * @memberof MediaStreamRecorder
         * @example
         * recorder.clearRecordedData();
         */
        this.clearRecordedData = function() {
            if (mediaRecorder && mediaRecorder.state === 'recording') {
                self.stop(clearRecordedDataCB);
            }

            clearRecordedDataCB();
        };

        function clearRecordedDataCB() {
            arrayOfBlobs = [];
            mediaRecorder = null;
            self.timestamps = [];
        }

        // Reference to "MediaRecorder" object
        var mediaRecorder;

        /**
         * Access to native MediaRecorder API
         * @method
         * @memberof MediaStreamRecorder
         * @instance
         * @example
         * var internal = recorder.getInternalRecorder();
         * internal.ondataavailable = function() {}; // override
         * internal.stream, internal.onpause, internal.onstop, etc.
         * @returns {Object} Returns internal recording object.
         */
        this.getInternalRecorder = function() {
            return mediaRecorder;
        };

        function isMediaStreamActive() {
            if ('active' in mediaStream) {
                if (!mediaStream.active) {
                    return false;
                }
            } else if ('ended' in mediaStream) { // old hack
                if (mediaStream.ended) {
                    return false;
                }
            }
            return true;
        }

        /**
         * @property {Blob} blob - Recorded data as "Blob" object.
         * @memberof MediaStreamRecorder
         * @example
         * recorder.stop(function() {
         *     var blob = recorder.blob;
         * });
         */
        this.blob = null;


        /**
         * Get MediaRecorder readonly state.
         * @method
         * @memberof MediaStreamRecorder
         * @example
         * var state = recorder.getState();
         * @returns {String} Returns recording state.
         */
        this.getState = function() {
            if (!mediaRecorder) {
                return 'inactive';
            }

            return mediaRecorder.state || 'inactive';
        };

        // list of all recording states
        var allStates = [];

        /**
         * Get MediaRecorder all recording states.
         * @method
         * @memberof MediaStreamRecorder
         * @example
         * var state = recorder.getAllStates();
         * @returns {Array} Returns all recording states
         */
        this.getAllStates = function() {
            return allStates;
        };

        // if any Track within the MediaStream is muted or not enabled at any time, 
        // the browser will only record black frames 
        // or silence since that is the content produced by the Track
        // so we need to stopRecording as soon as any single track ends.
        if (typeof config.checkForInactiveTracks === 'undefined') {
            config.checkForInactiveTracks = false; // disable to minimize CPU usage
        }

        var self = this;

        // this method checks if media stream is stopped
        // or if any track is ended.
        (function looper() {
            if (!mediaRecorder || config.checkForInactiveTracks === false) {
                return;
            }

            if (isMediaStreamActive() === false) {
                if (!config.disableLogs) {
                    console.log('MediaStream seems stopped.');
                }
                self.stop();
                return;
            }

            setTimeout(looper, 1000); // check every second
        })();

        // for debugging
        this.name = 'MediaStreamRecorder';
        this.toString = function() {
            return this.name;
        };
    }

    if (typeof RecordRTC !== 'undefined') {
        RecordRTC.MediaStreamRecorder = MediaStreamRecorder;
    }

    // source code from: http://typedarray.org/wp-content/projects/WebAudioRecorder/script.js
    // https://github.com/mattdiamond/Recorderjs#license-mit
    // ______________________
    // StereoAudioRecorder.js

    /**
     * StereoAudioRecorder is a standalone class used by {@link RecordRTC} to bring "stereo" audio-recording in chrome.
     * @summary JavaScript standalone object for stereo audio recording.
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @typedef StereoAudioRecorder
     * @class
     * @example
     * var recorder = new StereoAudioRecorder(MediaStream, {
     *     sampleRate: 44100,
     *     bufferSize: 4096
     * });
     * recorder.record();
     * recorder.stop(function(blob) {
     *     video.src = URL.createObjectURL(blob);
     * });
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     * @param {MediaStream} mediaStream - MediaStream object fetched using getUserMedia API or generated using captureStreamUntilEnded or WebAudio API.
     * @param {object} config - {sampleRate: 44100, bufferSize: 4096, numberOfAudioChannels: 1, etc.}
     */

    function StereoAudioRecorder(mediaStream, config) {
        if (!getTracks(mediaStream, 'audio').length) {
            throw 'Your stream has no audio tracks.';
        }

        config = config || {};

        var self = this;

        // variables
        var leftchannel = [];
        var rightchannel = [];
        var recording = false;
        var recordingLength = 0;
        var jsAudioNode;

        var numberOfAudioChannels = 2;

        /**
         * Set sample rates such as 8K or 16K. Reference: http://stackoverflow.com/a/28977136/552182
         * @property {number} desiredSampRate - Desired Bits per sample * 1000
         * @memberof StereoAudioRecorder
         * @instance
         * @example
         * var recorder = StereoAudioRecorder(mediaStream, {
         *   desiredSampRate: 16 * 1000 // bits-per-sample * 1000
         * });
         */
        var desiredSampRate = config.desiredSampRate;

        // backward compatibility
        if (config.leftChannel === true) {
            numberOfAudioChannels = 1;
        }

        if (config.numberOfAudioChannels === 1) {
            numberOfAudioChannels = 1;
        }

        if (!numberOfAudioChannels || numberOfAudioChannels < 1) {
            numberOfAudioChannels = 2;
        }

        if (!config.disableLogs) {
            console.log('StereoAudioRecorder is set to record number of channels: ' + numberOfAudioChannels);
        }

        // if any Track within the MediaStream is muted or not enabled at any time, 
        // the browser will only record black frames 
        // or silence since that is the content produced by the Track
        // so we need to stopRecording as soon as any single track ends.
        if (typeof config.checkForInactiveTracks === 'undefined') {
            config.checkForInactiveTracks = true;
        }

        function isMediaStreamActive() {
            if (config.checkForInactiveTracks === false) {
                // always return "true"
                return true;
            }

            if ('active' in mediaStream) {
                if (!mediaStream.active) {
                    return false;
                }
            } else if ('ended' in mediaStream) { // old hack
                if (mediaStream.ended) {
                    return false;
                }
            }
            return true;
        }

        /**
         * This method records MediaStream.
         * @method
         * @memberof StereoAudioRecorder
         * @example
         * recorder.record();
         */
        this.record = function() {
            if (isMediaStreamActive() === false) {
                throw 'Please make sure MediaStream is active.';
            }

            resetVariables();

            isAudioProcessStarted = isPaused = false;
            recording = true;

            if (typeof config.timeSlice !== 'undefined') {
                looper();
            }
        };

        function mergeLeftRightBuffers(config, callback) {
            function mergeAudioBuffers(config, cb) {
                var numberOfAudioChannels = config.numberOfAudioChannels;

                // todo: "slice(0)" --- is it causes loop? Should be removed?
                var leftBuffers = config.leftBuffers.slice(0);
                var rightBuffers = config.rightBuffers.slice(0);
                var sampleRate = config.sampleRate;
                var internalInterleavedLength = config.internalInterleavedLength;
                var desiredSampRate = config.desiredSampRate;

                if (numberOfAudioChannels === 2) {
                    leftBuffers = mergeBuffers(leftBuffers, internalInterleavedLength);
                    rightBuffers = mergeBuffers(rightBuffers, internalInterleavedLength);

                    if (desiredSampRate) {
                        leftBuffers = interpolateArray(leftBuffers, desiredSampRate, sampleRate);
                        rightBuffers = interpolateArray(rightBuffers, desiredSampRate, sampleRate);
                    }
                }

                if (numberOfAudioChannels === 1) {
                    leftBuffers = mergeBuffers(leftBuffers, internalInterleavedLength);

                    if (desiredSampRate) {
                        leftBuffers = interpolateArray(leftBuffers, desiredSampRate, sampleRate);
                    }
                }

                // set sample rate as desired sample rate
                if (desiredSampRate) {
                    sampleRate = desiredSampRate;
                }

                // for changing the sampling rate, reference:
                // http://stackoverflow.com/a/28977136/552182
                function interpolateArray(data, newSampleRate, oldSampleRate) {
                    var fitCount = Math.round(data.length * (newSampleRate / oldSampleRate));
                    var newData = [];
                    var springFactor = Number((data.length - 1) / (fitCount - 1));
                    newData[0] = data[0];
                    for (var i = 1; i < fitCount - 1; i++) {
                        var tmp = i * springFactor;
                        var before = Number(Math.floor(tmp)).toFixed();
                        var after = Number(Math.ceil(tmp)).toFixed();
                        var atPoint = tmp - before;
                        newData[i] = linearInterpolate(data[before], data[after], atPoint);
                    }
                    newData[fitCount - 1] = data[data.length - 1];
                    return newData;
                }

                function linearInterpolate(before, after, atPoint) {
                    return before + (after - before) * atPoint;
                }

                function mergeBuffers(channelBuffer, rLength) {
                    var result = new Float64Array(rLength);
                    var offset = 0;
                    var lng = channelBuffer.length;

                    for (var i = 0; i < lng; i++) {
                        var buffer = channelBuffer[i];
                        result.set(buffer, offset);
                        offset += buffer.length;
                    }

                    return result;
                }

                function interleave(leftChannel, rightChannel) {
                    var length = leftChannel.length + rightChannel.length;

                    var result = new Float64Array(length);

                    var inputIndex = 0;

                    for (var index = 0; index < length;) {
                        result[index++] = leftChannel[inputIndex];
                        result[index++] = rightChannel[inputIndex];
                        inputIndex++;
                    }
                    return result;
                }

                function writeUTFBytes(view, offset, string) {
                    var lng = string.length;
                    for (var i = 0; i < lng; i++) {
                        view.setUint8(offset + i, string.charCodeAt(i));
                    }
                }

                // interleave both channels together
                var interleaved;

                if (numberOfAudioChannels === 2) {
                    interleaved = interleave(leftBuffers, rightBuffers);
                }

                if (numberOfAudioChannels === 1) {
                    interleaved = leftBuffers;
                }

                var interleavedLength = interleaved.length;

                // create wav file
                var resultingBufferLength = 44 + interleavedLength * 2;

                var buffer = new ArrayBuffer(resultingBufferLength);

                var view = new DataView(buffer);

                // RIFF chunk descriptor/identifier 
                writeUTFBytes(view, 0, 'RIFF');

                // RIFF chunk length
                // changed "44" to "36" via #401
                view.setUint32(4, 36 + interleavedLength * 2, true);

                // RIFF type 
                writeUTFBytes(view, 8, 'WAVE');

                // format chunk identifier 
                // FMT sub-chunk
                writeUTFBytes(view, 12, 'fmt ');

                // format chunk length 
                view.setUint32(16, 16, true);

                // sample format (raw)
                view.setUint16(20, 1, true);

                // stereo (2 channels)
                view.setUint16(22, numberOfAudioChannels, true);

                // sample rate 
                view.setUint32(24, sampleRate, true);

                // byte rate (sample rate * block align)
                view.setUint32(28, sampleRate * numberOfAudioChannels * 2, true);

                // block align (channel count * bytes per sample) 
                view.setUint16(32, numberOfAudioChannels * 2, true);

                // bits per sample 
                view.setUint16(34, 16, true);

                // data sub-chunk
                // data chunk identifier 
                writeUTFBytes(view, 36, 'data');

                // data chunk length 
                view.setUint32(40, interleavedLength * 2, true);

                // write the PCM samples
                var lng = interleavedLength;
                var index = 44;
                var volume = 1;
                for (var i = 0; i < lng; i++) {
                    view.setInt16(index, interleaved[i] * (0x7FFF * volume), true);
                    index += 2;
                }

                if (cb) {
                    return cb({
                        buffer: buffer,
                        view: view
                    });
                }

                postMessage({
                    buffer: buffer,
                    view: view
                });
            }

            if (config.noWorker) {
                mergeAudioBuffers(config, function(data) {
                    callback(data.buffer, data.view);
                });
                return;
            }


            var webWorker = processInWebWorker(mergeAudioBuffers);

            webWorker.onmessage = function(event) {
                callback(event.data.buffer, event.data.view);

                // release memory
                URL.revokeObjectURL(webWorker.workerURL);

                // kill webworker (or Chrome will kill your page after ~25 calls)
                webWorker.terminate();
            };

            webWorker.postMessage(config);
        }

        function processInWebWorker(_function) {
            var workerURL = URL.createObjectURL(new Blob([_function.toString(),
                ';this.onmessage =  function (eee) {' + _function.name + '(eee.data);}'
            ], {
                type: 'application/javascript'
            }));

            var worker = new Worker(workerURL);
            worker.workerURL = workerURL;
            return worker;
        }

        /**
         * This method stops recording MediaStream.
         * @param {function} callback - Callback function, that is used to pass recorded blob back to the callee.
         * @method
         * @memberof StereoAudioRecorder
         * @example
         * recorder.stop(function(blob) {
         *     video.src = URL.createObjectURL(blob);
         * });
         */
        this.stop = function(callback) {
            callback = callback || function() {};

            // stop recording
            recording = false;

            mergeLeftRightBuffers({
                desiredSampRate: desiredSampRate,
                sampleRate: sampleRate,
                numberOfAudioChannels: numberOfAudioChannels,
                internalInterleavedLength: recordingLength,
                leftBuffers: leftchannel,
                rightBuffers: numberOfAudioChannels === 1 ? [] : rightchannel,
                noWorker: config.noWorker
            }, function(buffer, view) {
                /**
                 * @property {Blob} blob - The recorded blob object.
                 * @memberof StereoAudioRecorder
                 * @example
                 * recorder.stop(function(){
                 *     var blob = recorder.blob;
                 * });
                 */
                self.blob = new Blob([view], {
                    type: 'audio/wav'
                });

                /**
                 * @property {ArrayBuffer} buffer - The recorded buffer object.
                 * @memberof StereoAudioRecorder
                 * @example
                 * recorder.stop(function(){
                 *     var buffer = recorder.buffer;
                 * });
                 */
                self.buffer = new ArrayBuffer(view.buffer.byteLength);

                /**
                 * @property {DataView} view - The recorded data-view object.
                 * @memberof StereoAudioRecorder
                 * @example
                 * recorder.stop(function(){
                 *     var view = recorder.view;
                 * });
                 */
                self.view = view;

                self.sampleRate = desiredSampRate || sampleRate;
                self.bufferSize = bufferSize;

                // recorded audio length
                self.length = recordingLength;

                isAudioProcessStarted = false;

                if (callback) {
                    callback(self.blob);
                }
            });
        };

        if (typeof RecordRTC.Storage === 'undefined') {
            RecordRTC.Storage = {
                AudioContextConstructor: null,
                AudioContext: window.AudioContext || window.webkitAudioContext
            };
        }

        if (!RecordRTC.Storage.AudioContextConstructor || RecordRTC.Storage.AudioContextConstructor.state === 'closed') {
            RecordRTC.Storage.AudioContextConstructor = new RecordRTC.Storage.AudioContext();
        }

        var context = RecordRTC.Storage.AudioContextConstructor;

        // creates an audio node from the microphone incoming stream
        var audioInput = context.createMediaStreamSource(mediaStream);

        var legalBufferValues = [0, 256, 512, 1024, 2048, 4096, 8192, 16384];

        /**
         * From the spec: This value controls how frequently the audioprocess event is
         * dispatched and how many sample-frames need to be processed each call.
         * Lower values for buffer size will result in a lower (better) latency.
         * Higher values will be necessary to avoid audio breakup and glitches
         * The size of the buffer (in sample-frames) which needs to
         * be processed each time onprocessaudio is called.
         * Legal values are (256, 512, 1024, 2048, 4096, 8192, 16384).
         * @property {number} bufferSize - Buffer-size for how frequently the audioprocess event is dispatched.
         * @memberof StereoAudioRecorder
         * @example
         * recorder = new StereoAudioRecorder(mediaStream, {
         *     bufferSize: 4096
         * });
         */

        // "0" means, let chrome decide the most accurate buffer-size for current platform.
        var bufferSize = typeof config.bufferSize === 'undefined' ? 4096 : config.bufferSize;

        if (legalBufferValues.indexOf(bufferSize) === -1) {
            if (!config.disableLogs) {
                console.log('Legal values for buffer-size are ' + JSON.stringify(legalBufferValues, null, '\t'));
            }
        }

        if (context.createJavaScriptNode) {
            jsAudioNode = context.createJavaScriptNode(bufferSize, numberOfAudioChannels, numberOfAudioChannels);
        } else if (context.createScriptProcessor) {
            jsAudioNode = context.createScriptProcessor(bufferSize, numberOfAudioChannels, numberOfAudioChannels);
        } else {
            throw 'WebAudio API has no support on this browser.';
        }

        // connect the stream to the script processor
        audioInput.connect(jsAudioNode);

        if (!config.bufferSize) {
            bufferSize = jsAudioNode.bufferSize; // device buffer-size
        }

        /**
         * The sample rate (in sample-frames per second) at which the
         * AudioContext handles audio. It is assumed that all AudioNodes
         * in the context run at this rate. In making this assumption,
         * sample-rate converters or "varispeed" processors are not supported
         * in real-time processing.
         * The sampleRate parameter describes the sample-rate of the
         * linear PCM audio data in the buffer in sample-frames per second.
         * An implementation must support sample-rates in at least
         * the range 22050 to 96000.
         * @property {number} sampleRate - Buffer-size for how frequently the audioprocess event is dispatched.
         * @memberof StereoAudioRecorder
         * @example
         * recorder = new StereoAudioRecorder(mediaStream, {
         *     sampleRate: 44100
         * });
         */
        var sampleRate = typeof config.sampleRate !== 'undefined' ? config.sampleRate : context.sampleRate || 44100;

        if (sampleRate < 22050 || sampleRate > 96000) {
            // Ref: http://stackoverflow.com/a/26303918/552182
            if (!config.disableLogs) {
                console.log('sample-rate must be under range 22050 and 96000.');
            }
        }

        if (!config.disableLogs) {
            if (config.desiredSampRate) {
                console.log('Desired sample-rate: ' + config.desiredSampRate);
            }
        }

        var isPaused = false;
        /**
         * This method pauses the recording process.
         * @method
         * @memberof StereoAudioRecorder
         * @example
         * recorder.pause();
         */
        this.pause = function() {
            isPaused = true;
        };

        /**
         * This method resumes the recording process.
         * @method
         * @memberof StereoAudioRecorder
         * @example
         * recorder.resume();
         */
        this.resume = function() {
            if (isMediaStreamActive() === false) {
                throw 'Please make sure MediaStream is active.';
            }

            if (!recording) {
                if (!config.disableLogs) {
                    console.log('Seems recording has been restarted.');
                }
                this.record();
                return;
            }

            isPaused = false;
        };

        /**
         * This method resets currently recorded data.
         * @method
         * @memberof StereoAudioRecorder
         * @example
         * recorder.clearRecordedData();
         */
        this.clearRecordedData = function() {
            config.checkForInactiveTracks = false;

            if (recording) {
                this.stop(clearRecordedDataCB);
            }

            clearRecordedDataCB();
        };

        function resetVariables() {
            leftchannel = [];
            rightchannel = [];
            recordingLength = 0;
            isAudioProcessStarted = false;
            recording = false;
            isPaused = false;
            context = null;

            self.leftchannel = leftchannel;
            self.rightchannel = rightchannel;
            self.numberOfAudioChannels = numberOfAudioChannels;
            self.desiredSampRate = desiredSampRate;
            self.sampleRate = sampleRate;
            self.recordingLength = recordingLength;

            intervalsBasedBuffers = {
                left: [],
                right: [],
                recordingLength: 0
            };
        }

        function clearRecordedDataCB() {
            if (jsAudioNode) {
                jsAudioNode.onaudioprocess = null;
                jsAudioNode.disconnect();
                jsAudioNode = null;
            }

            if (audioInput) {
                audioInput.disconnect();
                audioInput = null;
            }

            resetVariables();
        }

        // for debugging
        this.name = 'StereoAudioRecorder';
        this.toString = function() {
            return this.name;
        };

        var isAudioProcessStarted = false;

        function onAudioProcessDataAvailable(e) {
            if (isPaused) {
                return;
            }

            if (isMediaStreamActive() === false) {
                if (!config.disableLogs) {
                    console.log('MediaStream seems stopped.');
                }
                jsAudioNode.disconnect();
                recording = false;
            }

            if (!recording) {
                if (audioInput) {
                    audioInput.disconnect();
                    audioInput = null;
                }
                return;
            }

            /**
             * This method is called on "onaudioprocess" event's first invocation.
             * @method {function} onAudioProcessStarted
             * @memberof StereoAudioRecorder
             * @example
             * recorder.onAudioProcessStarted: function() { };
             */
            if (!isAudioProcessStarted) {
                isAudioProcessStarted = true;
                if (config.onAudioProcessStarted) {
                    config.onAudioProcessStarted();
                }

                if (config.initCallback) {
                    config.initCallback();
                }
            }

            var left = e.inputBuffer.getChannelData(0);

            // we clone the samples
            var chLeft = new Float32Array(left);
            leftchannel.push(chLeft);

            if (numberOfAudioChannels === 2) {
                var right = e.inputBuffer.getChannelData(1);
                var chRight = new Float32Array(right);
                rightchannel.push(chRight);
            }

            recordingLength += bufferSize;

            // export raw PCM
            self.recordingLength = recordingLength;

            if (typeof config.timeSlice !== 'undefined') {
                intervalsBasedBuffers.recordingLength += bufferSize;
                intervalsBasedBuffers.left.push(chLeft);

                if (numberOfAudioChannels === 2) {
                    intervalsBasedBuffers.right.push(chRight);
                }
            }
        }

        jsAudioNode.onaudioprocess = onAudioProcessDataAvailable;

        // to prevent self audio to be connected with speakers
        if (context.createMediaStreamDestination) {
            jsAudioNode.connect(context.createMediaStreamDestination());
        } else {
            jsAudioNode.connect(context.destination);
        }

        // export raw PCM
        this.leftchannel = leftchannel;
        this.rightchannel = rightchannel;
        this.numberOfAudioChannels = numberOfAudioChannels;
        this.desiredSampRate = desiredSampRate;
        this.sampleRate = sampleRate;
        self.recordingLength = recordingLength;

        // helper for intervals based blobs
        var intervalsBasedBuffers = {
            left: [],
            right: [],
            recordingLength: 0
        };

        // this looper is used to support intervals based blobs (via timeSlice+ondataavailable)
        function looper() {
            if (!recording || typeof config.ondataavailable !== 'function' || typeof config.timeSlice === 'undefined') {
                return;
            }

            if (intervalsBasedBuffers.left.length) {
                mergeLeftRightBuffers({
                    desiredSampRate: desiredSampRate,
                    sampleRate: sampleRate,
                    numberOfAudioChannels: numberOfAudioChannels,
                    internalInterleavedLength: intervalsBasedBuffers.recordingLength,
                    leftBuffers: intervalsBasedBuffers.left,
                    rightBuffers: numberOfAudioChannels === 1 ? [] : intervalsBasedBuffers.right
                }, function(buffer, view) {
                    var blob = new Blob([view], {
                        type: 'audio/wav'
                    });
                    config.ondataavailable(blob);

                    setTimeout(looper, config.timeSlice);
                });

                intervalsBasedBuffers = {
                    left: [],
                    right: [],
                    recordingLength: 0
                };
            } else {
                setTimeout(looper, config.timeSlice);
            }
        }
    }

    if (typeof RecordRTC !== 'undefined') {
        RecordRTC.StereoAudioRecorder = StereoAudioRecorder;
    }

    // _________________
    // CanvasRecorder.js

    /**
     * CanvasRecorder is a standalone class used by {@link RecordRTC} to bring HTML5-Canvas recording into video WebM. It uses HTML2Canvas library and runs top over {@link Whammy}.
     * @summary HTML2Canvas recording into video WebM.
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @typedef CanvasRecorder
     * @class
     * @example
     * var recorder = new CanvasRecorder(htmlElement, { disableLogs: true, useWhammyRecorder: true });
     * recorder.record();
     * recorder.stop(function(blob) {
     *     video.src = URL.createObjectURL(blob);
     * });
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     * @param {HTMLElement} htmlElement - querySelector/getElementById/getElementsByTagName[0]/etc.
     * @param {object} config - {disableLogs:true, initCallback: function}
     */

    function CanvasRecorder(htmlElement, config) {
        if (typeof html2canvas === 'undefined') {
            throw 'Please link: https://www.webrtc-experiment.com/screenshot.js';
        }

        config = config || {};
        if (!config.frameInterval) {
            config.frameInterval = 10;
        }

        // via DetectRTC.js
        var isCanvasSupportsStreamCapturing = false;
        ['captureStream', 'mozCaptureStream', 'webkitCaptureStream'].forEach(function(item) {
            if (item in document.createElement('canvas')) {
                isCanvasSupportsStreamCapturing = true;
            }
        });

        var _isChrome = (!!window.webkitRTCPeerConnection || !!window.webkitGetUserMedia) && !!window.chrome;

        var chromeVersion = 50;
        var matchArray = navigator.userAgent.match(/Chrom(e|ium)\/([0-9]+)\./);
        if (_isChrome && matchArray && matchArray[2]) {
            chromeVersion = parseInt(matchArray[2], 10);
        }

        if (_isChrome && chromeVersion < 52) {
            isCanvasSupportsStreamCapturing = false;
        }

        if (config.useWhammyRecorder) {
            isCanvasSupportsStreamCapturing = false;
        }

        var globalCanvas, mediaStreamRecorder;

        if (isCanvasSupportsStreamCapturing) {
            if (!config.disableLogs) {
                console.log('Your browser supports both MediRecorder API and canvas.captureStream!');
            }

            if (htmlElement instanceof HTMLCanvasElement) {
                globalCanvas = htmlElement;
            } else if (htmlElement instanceof CanvasRenderingContext2D) {
                globalCanvas = htmlElement.canvas;
            } else {
                throw 'Please pass either HTMLCanvasElement or CanvasRenderingContext2D.';
            }
        } else if (!!navigator.mozGetUserMedia) {
            if (!config.disableLogs) {
                console.error('Canvas recording is NOT supported in Firefox.');
            }
        }

        var isRecording;

        /**
         * This method records Canvas.
         * @method
         * @memberof CanvasRecorder
         * @example
         * recorder.record();
         */
        this.record = function() {
            isRecording = true;

            if (isCanvasSupportsStreamCapturing && !config.useWhammyRecorder) {
                // CanvasCaptureMediaStream
                var canvasMediaStream;
                if ('captureStream' in globalCanvas) {
                    canvasMediaStream = globalCanvas.captureStream(25); // 25 FPS
                } else if ('mozCaptureStream' in globalCanvas) {
                    canvasMediaStream = globalCanvas.mozCaptureStream(25);
                } else if ('webkitCaptureStream' in globalCanvas) {
                    canvasMediaStream = globalCanvas.webkitCaptureStream(25);
                }

                try {
                    var mdStream = new MediaStream();
                    mdStream.addTrack(getTracks(canvasMediaStream, 'video')[0]);
                    canvasMediaStream = mdStream;
                } catch (e) {}

                if (!canvasMediaStream) {
                    throw 'captureStream API are NOT available.';
                }

                // Note: Jan 18, 2016 status is that, 
                // Firefox MediaRecorder API can't record CanvasCaptureMediaStream object.
                mediaStreamRecorder = new MediaStreamRecorder(canvasMediaStream, {
                    mimeType: config.mimeType || 'video/webm'
                });
                mediaStreamRecorder.record();
            } else {
                whammy.frames = [];
                lastTime = new Date().getTime();
                drawCanvasFrame();
            }

            if (config.initCallback) {
                config.initCallback();
            }
        };

        this.getWebPImages = function(callback) {
            if (htmlElement.nodeName.toLowerCase() !== 'canvas') {
                callback();
                return;
            }

            var framesLength = whammy.frames.length;
            whammy.frames.forEach(function(frame, idx) {
                var framesRemaining = framesLength - idx;
                if (!config.disableLogs) {
                    console.log(framesRemaining + '/' + framesLength + ' frames remaining');
                }

                if (config.onEncodingCallback) {
                    config.onEncodingCallback(framesRemaining, framesLength);
                }

                var webp = frame.image.toDataURL('image/webp', 1);
                whammy.frames[idx].image = webp;
            });

            if (!config.disableLogs) {
                console.log('Generating WebM');
            }

            callback();
        };

        /**
         * This method stops recording Canvas.
         * @param {function} callback - Callback function, that is used to pass recorded blob back to the callee.
         * @method
         * @memberof CanvasRecorder
         * @example
         * recorder.stop(function(blob) {
         *     video.src = URL.createObjectURL(blob);
         * });
         */
        this.stop = function(callback) {
            isRecording = false;

            var that = this;

            if (isCanvasSupportsStreamCapturing && mediaStreamRecorder) {
                mediaStreamRecorder.stop(callback);
                return;
            }

            this.getWebPImages(function() {
                /**
                 * @property {Blob} blob - Recorded frames in video/webm blob.
                 * @memberof CanvasRecorder
                 * @example
                 * recorder.stop(function() {
                 *     var blob = recorder.blob;
                 * });
                 */
                whammy.compile(function(blob) {
                    if (!config.disableLogs) {
                        console.log('Recording finished!');
                    }

                    that.blob = blob;

                    if (that.blob.forEach) {
                        that.blob = new Blob([], {
                            type: 'video/webm'
                        });
                    }

                    if (callback) {
                        callback(that.blob);
                    }

                    whammy.frames = [];
                });
            });
        };

        var isPausedRecording = false;

        /**
         * This method pauses the recording process.
         * @method
         * @memberof CanvasRecorder
         * @example
         * recorder.pause();
         */
        this.pause = function() {
            isPausedRecording = true;

            if (mediaStreamRecorder instanceof MediaStreamRecorder) {
                mediaStreamRecorder.pause();
                return;
            }
        };

        /**
         * This method resumes the recording process.
         * @method
         * @memberof CanvasRecorder
         * @example
         * recorder.resume();
         */
        this.resume = function() {
            isPausedRecording = false;

            if (mediaStreamRecorder instanceof MediaStreamRecorder) {
                mediaStreamRecorder.resume();
                return;
            }

            if (!isRecording) {
                this.record();
            }
        };

        /**
         * This method resets currently recorded data.
         * @method
         * @memberof CanvasRecorder
         * @example
         * recorder.clearRecordedData();
         */
        this.clearRecordedData = function() {
            if (isRecording) {
                this.stop(clearRecordedDataCB);
            }
            clearRecordedDataCB();
        };

        function clearRecordedDataCB() {
            whammy.frames = [];
            isRecording = false;
            isPausedRecording = false;
        }

        // for debugging
        this.name = 'CanvasRecorder';
        this.toString = function() {
            return this.name;
        };

        function cloneCanvas() {
            //create a new canvas
            var newCanvas = document.createElement('canvas');
            var context = newCanvas.getContext('2d');

            //set dimensions
            newCanvas.width = htmlElement.width;
            newCanvas.height = htmlElement.height;

            //apply the old canvas to the new one
            context.drawImage(htmlElement, 0, 0);

            //return the new canvas
            return newCanvas;
        }

        function drawCanvasFrame() {
            if (isPausedRecording) {
                lastTime = new Date().getTime();
                return setTimeout(drawCanvasFrame, 500);
            }

            if (htmlElement.nodeName.toLowerCase() === 'canvas') {
                var duration = new Date().getTime() - lastTime;
                // via #206, by Jack i.e. @Seymourr
                lastTime = new Date().getTime();

                whammy.frames.push({
                    image: cloneCanvas(),
                    duration: duration
                });

                if (isRecording) {
                    setTimeout(drawCanvasFrame, config.frameInterval);
                }
                return;
            }

            html2canvas(htmlElement, {
                grabMouse: typeof config.showMousePointer === 'undefined' || config.showMousePointer,
                onrendered: function(canvas) {
                    var duration = new Date().getTime() - lastTime;
                    if (!duration) {
                        return setTimeout(drawCanvasFrame, config.frameInterval);
                    }

                    // via #206, by Jack i.e. @Seymourr
                    lastTime = new Date().getTime();

                    whammy.frames.push({
                        image: canvas.toDataURL('image/webp', 1),
                        duration: duration
                    });

                    if (isRecording) {
                        setTimeout(drawCanvasFrame, config.frameInterval);
                    }
                }
            });
        }

        var lastTime = new Date().getTime();

        var whammy = new Whammy.Video(100);
    }

    if (typeof RecordRTC !== 'undefined') {
        RecordRTC.CanvasRecorder = CanvasRecorder;
    }

    // _________________
    // WhammyRecorder.js

    /**
     * WhammyRecorder is a standalone class used by {@link RecordRTC} to bring video recording in Chrome. It runs top over {@link Whammy}.
     * @summary Video recording feature in Chrome.
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @typedef WhammyRecorder
     * @class
     * @example
     * var recorder = new WhammyRecorder(mediaStream);
     * recorder.record();
     * recorder.stop(function(blob) {
     *     video.src = URL.createObjectURL(blob);
     * });
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     * @param {MediaStream} mediaStream - MediaStream object fetched using getUserMedia API or generated using captureStreamUntilEnded or WebAudio API.
     * @param {object} config - {disableLogs: true, initCallback: function, video: HTMLVideoElement, etc.}
     */

    function WhammyRecorder(mediaStream, config) {

        config = config || {};

        if (!config.frameInterval) {
            config.frameInterval = 10;
        }

        if (!config.disableLogs) {
            console.log('Using frames-interval:', config.frameInterval);
        }

        /**
         * This method records video.
         * @method
         * @memberof WhammyRecorder
         * @example
         * recorder.record();
         */
        this.record = function() {
            if (!config.width) {
                config.width = 320;
            }

            if (!config.height) {
                config.height = 240;
            }

            if (!config.video) {
                config.video = {
                    width: config.width,
                    height: config.height
                };
            }

            if (!config.canvas) {
                config.canvas = {
                    width: config.width,
                    height: config.height
                };
            }

            canvas.width = config.canvas.width || 320;
            canvas.height = config.canvas.height || 240;

            context = canvas.getContext('2d');

            // setting defaults
            if (config.video && config.video instanceof HTMLVideoElement) {
                video = config.video.cloneNode();

                if (config.initCallback) {
                    config.initCallback();
                }
            } else {
                video = document.createElement('video');

                setSrcObject(mediaStream, video);

                video.onloadedmetadata = function() { // "onloadedmetadata" may NOT work in FF?
                    if (config.initCallback) {
                        config.initCallback();
                    }
                };

                video.width = config.video.width;
                video.height = config.video.height;
            }

            video.muted = true;
            video.play();

            lastTime = new Date().getTime();
            whammy = new Whammy.Video();

            if (!config.disableLogs) {
                console.log('canvas resolutions', canvas.width, '*', canvas.height);
                console.log('video width/height', video.width || canvas.width, '*', video.height || canvas.height);
            }

            drawFrames(config.frameInterval);
        };

        /**
         * Draw and push frames to Whammy
         * @param {integer} frameInterval - set minimum interval (in milliseconds) between each time we push a frame to Whammy
         */
        function drawFrames(frameInterval) {
            frameInterval = typeof frameInterval !== 'undefined' ? frameInterval : 10;

            var duration = new Date().getTime() - lastTime;
            if (!duration) {
                return setTimeout(drawFrames, frameInterval, frameInterval);
            }

            if (isPausedRecording) {
                lastTime = new Date().getTime();
                return setTimeout(drawFrames, 100);
            }

            // via #206, by Jack i.e. @Seymourr
            lastTime = new Date().getTime();

            if (video.paused) {
                // via: https://github.com/muaz-khan/WebRTC-Experiment/pull/316
                // Tweak for Android Chrome
                video.play();
            }

            context.drawImage(video, 0, 0, canvas.width, canvas.height);
            whammy.frames.push({
                duration: duration,
                image: canvas.toDataURL('image/webp')
            });

            if (!isStopDrawing) {
                setTimeout(drawFrames, frameInterval, frameInterval);
            }
        }

        function asyncLoop(o) {
            var i = -1,
                length = o.length;

            (function loop() {
                i++;
                if (i === length) {
                    o.callback();
                    return;
                }

                // "setTimeout" added by Jim McLeod
                setTimeout(function() {
                    o.functionToLoop(loop, i);
                }, 1);
            })();
        }


        /**
         * remove black frames from the beginning to the specified frame
         * @param {Array} _frames - array of frames to be checked
         * @param {number} _framesToCheck - number of frame until check will be executed (-1 - will drop all frames until frame not matched will be found)
         * @param {number} _pixTolerance - 0 - very strict (only black pixel color) ; 1 - all
         * @param {number} _frameTolerance - 0 - very strict (only black frame color) ; 1 - all
         * @returns {Array} - array of frames
         */
        // pull#293 by @volodalexey
        function dropBlackFrames(_frames, _framesToCheck, _pixTolerance, _frameTolerance, callback) {
            var localCanvas = document.createElement('canvas');
            localCanvas.width = canvas.width;
            localCanvas.height = canvas.height;
            var context2d = localCanvas.getContext('2d');
            var resultFrames = [];

            var checkUntilNotBlack = _framesToCheck === -1;
            var endCheckFrame = (_framesToCheck && _framesToCheck > 0 && _framesToCheck <= _frames.length) ?
                _framesToCheck : _frames.length;
            var sampleColor = {
                r: 0,
                g: 0,
                b: 0
            };
            var maxColorDifference = Math.sqrt(
                Math.pow(255, 2) +
                Math.pow(255, 2) +
                Math.pow(255, 2)
            );
            var pixTolerance = _pixTolerance && _pixTolerance >= 0 && _pixTolerance <= 1 ? _pixTolerance : 0;
            var frameTolerance = _frameTolerance && _frameTolerance >= 0 && _frameTolerance <= 1 ? _frameTolerance : 0;
            var doNotCheckNext = false;

            asyncLoop({
                length: endCheckFrame,
                functionToLoop: function(loop, f) {
                    var matchPixCount, endPixCheck, maxPixCount;

                    var finishImage = function() {
                        if (!doNotCheckNext && maxPixCount - matchPixCount <= maxPixCount * frameTolerance) ; else {
                            // console.log('frame is passed : ' + f);
                            if (checkUntilNotBlack) {
                                doNotCheckNext = true;
                            }
                            resultFrames.push(_frames[f]);
                        }
                        loop();
                    };

                    if (!doNotCheckNext) {
                        var image = new Image();
                        image.onload = function() {
                            context2d.drawImage(image, 0, 0, canvas.width, canvas.height);
                            var imageData = context2d.getImageData(0, 0, canvas.width, canvas.height);
                            matchPixCount = 0;
                            endPixCheck = imageData.data.length;
                            maxPixCount = imageData.data.length / 4;

                            for (var pix = 0; pix < endPixCheck; pix += 4) {
                                var currentColor = {
                                    r: imageData.data[pix],
                                    g: imageData.data[pix + 1],
                                    b: imageData.data[pix + 2]
                                };
                                var colorDifference = Math.sqrt(
                                    Math.pow(currentColor.r - sampleColor.r, 2) +
                                    Math.pow(currentColor.g - sampleColor.g, 2) +
                                    Math.pow(currentColor.b - sampleColor.b, 2)
                                );
                                // difference in color it is difference in color vectors (r1,g1,b1) <=> (r2,g2,b2)
                                if (colorDifference <= maxColorDifference * pixTolerance) {
                                    matchPixCount++;
                                }
                            }
                            finishImage();
                        };
                        image.src = _frames[f].image;
                    } else {
                        finishImage();
                    }
                },
                callback: function() {
                    resultFrames = resultFrames.concat(_frames.slice(endCheckFrame));

                    if (resultFrames.length <= 0) {
                        // at least one last frame should be available for next manipulation
                        // if total duration of all frames will be < 1000 than ffmpeg doesn't work well...
                        resultFrames.push(_frames[_frames.length - 1]);
                    }
                    callback(resultFrames);
                }
            });
        }

        var isStopDrawing = false;

        /**
         * This method stops recording video.
         * @param {function} callback - Callback function, that is used to pass recorded blob back to the callee.
         * @method
         * @memberof WhammyRecorder
         * @example
         * recorder.stop(function(blob) {
         *     video.src = URL.createObjectURL(blob);
         * });
         */
        this.stop = function(callback) {
            callback = callback || function() {};

            isStopDrawing = true;

            var _this = this;
            // analyse of all frames takes some time!
            setTimeout(function() {
                // e.g. dropBlackFrames(frames, 10, 1, 1) - will cut all 10 frames
                // e.g. dropBlackFrames(frames, 10, 0.5, 0.5) - will analyse 10 frames
                // e.g. dropBlackFrames(frames, 10) === dropBlackFrames(frames, 10, 0, 0) - will analyse 10 frames with strict black color
                dropBlackFrames(whammy.frames, -1, null, null, function(frames) {
                    whammy.frames = frames;

                    // to display advertisement images!
                    if (config.advertisement && config.advertisement.length) {
                        whammy.frames = config.advertisement.concat(whammy.frames);
                    }

                    /**
                     * @property {Blob} blob - Recorded frames in video/webm blob.
                     * @memberof WhammyRecorder
                     * @example
                     * recorder.stop(function() {
                     *     var blob = recorder.blob;
                     * });
                     */
                    whammy.compile(function(blob) {
                        _this.blob = blob;

                        if (_this.blob.forEach) {
                            _this.blob = new Blob([], {
                                type: 'video/webm'
                            });
                        }

                        if (callback) {
                            callback(_this.blob);
                        }
                    });
                });
            }, 10);
        };

        var isPausedRecording = false;

        /**
         * This method pauses the recording process.
         * @method
         * @memberof WhammyRecorder
         * @example
         * recorder.pause();
         */
        this.pause = function() {
            isPausedRecording = true;
        };

        /**
         * This method resumes the recording process.
         * @method
         * @memberof WhammyRecorder
         * @example
         * recorder.resume();
         */
        this.resume = function() {
            isPausedRecording = false;

            if (isStopDrawing) {
                this.record();
            }
        };

        /**
         * This method resets currently recorded data.
         * @method
         * @memberof WhammyRecorder
         * @example
         * recorder.clearRecordedData();
         */
        this.clearRecordedData = function() {
            if (!isStopDrawing) {
                this.stop(clearRecordedDataCB);
            }
            clearRecordedDataCB();
        };

        function clearRecordedDataCB() {
            whammy.frames = [];
            isStopDrawing = true;
            isPausedRecording = false;
        }

        // for debugging
        this.name = 'WhammyRecorder';
        this.toString = function() {
            return this.name;
        };

        var canvas = document.createElement('canvas');
        var context = canvas.getContext('2d');

        var video;
        var lastTime;
        var whammy;
    }

    if (typeof RecordRTC !== 'undefined') {
        RecordRTC.WhammyRecorder = WhammyRecorder;
    }

    // https://github.com/antimatter15/whammy/blob/master/LICENSE
    // _________
    // Whammy.js

    // todo: Firefox now supports webp for webm containers!
    // their MediaRecorder implementation works well!
    // should we provide an option to record via Whammy.js or MediaRecorder API is a better solution?

    /**
     * Whammy is a standalone class used by {@link RecordRTC} to bring video recording in Chrome. It is written by {@link https://github.com/antimatter15|antimatter15}
     * @summary A real time javascript webm encoder based on a canvas hack.
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @typedef Whammy
     * @class
     * @example
     * var recorder = new Whammy().Video(15);
     * recorder.add(context || canvas || dataURL);
     * var output = recorder.compile();
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     */

    var Whammy = (function() {
        // a more abstract-ish API

        function WhammyVideo(duration) {
            this.frames = [];
            this.duration = duration || 1;
            this.quality = 0.8;
        }

        /**
         * Pass Canvas or Context or image/webp(string) to {@link Whammy} encoder.
         * @method
         * @memberof Whammy
         * @example
         * recorder = new Whammy().Video(0.8, 100);
         * recorder.add(canvas || context || 'image/webp');
         * @param {string} frame - Canvas || Context || image/webp
         * @param {number} duration - Stick a duration (in milliseconds)
         */
        WhammyVideo.prototype.add = function(frame, duration) {
            if ('canvas' in frame) { //CanvasRenderingContext2D
                frame = frame.canvas;
            }

            if ('toDataURL' in frame) {
                frame = frame.toDataURL('image/webp', this.quality);
            }

            if (!(/^data:image\/webp;base64,/ig).test(frame)) {
                throw 'Input must be formatted properly as a base64 encoded DataURI of type image/webp';
            }
            this.frames.push({
                image: frame,
                duration: duration || this.duration
            });
        };

        function processInWebWorker(_function) {
            var blob = URL.createObjectURL(new Blob([_function.toString(),
                'this.onmessage =  function (eee) {' + _function.name + '(eee.data);}'
            ], {
                type: 'application/javascript'
            }));

            var worker = new Worker(blob);
            URL.revokeObjectURL(blob);
            return worker;
        }

        function whammyInWebWorker(frames) {
            function ArrayToWebM(frames) {
                var info = checkFrames(frames);
                if (!info) {
                    return [];
                }

                var clusterMaxDuration = 30000;

                var EBML = [{
                    'id': 0x1a45dfa3, // EBML
                    'data': [{
                        'data': 1,
                        'id': 0x4286 // EBMLVersion
                    }, {
                        'data': 1,
                        'id': 0x42f7 // EBMLReadVersion
                    }, {
                        'data': 4,
                        'id': 0x42f2 // EBMLMaxIDLength
                    }, {
                        'data': 8,
                        'id': 0x42f3 // EBMLMaxSizeLength
                    }, {
                        'data': 'webm',
                        'id': 0x4282 // DocType
                    }, {
                        'data': 2,
                        'id': 0x4287 // DocTypeVersion
                    }, {
                        'data': 2,
                        'id': 0x4285 // DocTypeReadVersion
                    }]
                }, {
                    'id': 0x18538067, // Segment
                    'data': [{
                        'id': 0x1549a966, // Info
                        'data': [{
                            'data': 1e6, //do things in millisecs (num of nanosecs for duration scale)
                            'id': 0x2ad7b1 // TimecodeScale
                        }, {
                            'data': 'whammy',
                            'id': 0x4d80 // MuxingApp
                        }, {
                            'data': 'whammy',
                            'id': 0x5741 // WritingApp
                        }, {
                            'data': doubleToString(info.duration),
                            'id': 0x4489 // Duration
                        }]
                    }, {
                        'id': 0x1654ae6b, // Tracks
                        'data': [{
                            'id': 0xae, // TrackEntry
                            'data': [{
                                'data': 1,
                                'id': 0xd7 // TrackNumber
                            }, {
                                'data': 1,
                                'id': 0x73c5 // TrackUID
                            }, {
                                'data': 0,
                                'id': 0x9c // FlagLacing
                            }, {
                                'data': 'und',
                                'id': 0x22b59c // Language
                            }, {
                                'data': 'V_VP8',
                                'id': 0x86 // CodecID
                            }, {
                                'data': 'VP8',
                                'id': 0x258688 // CodecName
                            }, {
                                'data': 1,
                                'id': 0x83 // TrackType
                            }, {
                                'id': 0xe0, // Video
                                'data': [{
                                    'data': info.width,
                                    'id': 0xb0 // PixelWidth
                                }, {
                                    'data': info.height,
                                    'id': 0xba // PixelHeight
                                }]
                            }]
                        }]
                    }]
                }];

                //Generate clusters (max duration)
                var frameNumber = 0;
                var clusterTimecode = 0;
                while (frameNumber < frames.length) {

                    var clusterFrames = [];
                    var clusterDuration = 0;
                    do {
                        clusterFrames.push(frames[frameNumber]);
                        clusterDuration += frames[frameNumber].duration;
                        frameNumber++;
                    } while (frameNumber < frames.length && clusterDuration < clusterMaxDuration);

                    var clusterCounter = 0;
                    var cluster = {
                        'id': 0x1f43b675, // Cluster
                        'data': getClusterData(clusterTimecode, clusterCounter, clusterFrames)
                    }; //Add cluster to segment
                    EBML[1].data.push(cluster);
                    clusterTimecode += clusterDuration;
                }

                return generateEBML(EBML);
            }

            function getClusterData(clusterTimecode, clusterCounter, clusterFrames) {
                return [{
                    'data': clusterTimecode,
                    'id': 0xe7 // Timecode
                }].concat(clusterFrames.map(function(webp) {
                    var block = makeSimpleBlock({
                        discardable: 0,
                        frame: webp.data.slice(4),
                        invisible: 0,
                        keyframe: 1,
                        lacing: 0,
                        trackNum: 1,
                        timecode: Math.round(clusterCounter)
                    });
                    clusterCounter += webp.duration;
                    return {
                        data: block,
                        id: 0xa3
                    };
                }));
            }

            // sums the lengths of all the frames and gets the duration

            function checkFrames(frames) {
                if (!frames[0]) {
                    postMessage({
                        error: 'Something went wrong. Maybe WebP format is not supported in the current browser.'
                    });
                    return;
                }

                var width = frames[0].width,
                    height = frames[0].height,
                    duration = frames[0].duration;

                for (var i = 1; i < frames.length; i++) {
                    duration += frames[i].duration;
                }
                return {
                    duration: duration,
                    width: width,
                    height: height
                };
            }

            function numToBuffer(num) {
                var parts = [];
                while (num > 0) {
                    parts.push(num & 0xff);
                    num = num >> 8;
                }
                return new Uint8Array(parts.reverse());
            }

            function strToBuffer(str) {
                return new Uint8Array(str.split('').map(function(e) {
                    return e.charCodeAt(0);
                }));
            }

            function bitsToBuffer(bits) {
                var data = [];
                var pad = (bits.length % 8) ? (new Array(1 + 8 - (bits.length % 8))).join('0') : '';
                bits = pad + bits;
                for (var i = 0; i < bits.length; i += 8) {
                    data.push(parseInt(bits.substr(i, 8), 2));
                }
                return new Uint8Array(data);
            }

            function generateEBML(json) {
                var ebml = [];
                for (var i = 0; i < json.length; i++) {
                    var data = json[i].data;

                    if (typeof data === 'object') {
                        data = generateEBML(data);
                    }

                    if (typeof data === 'number') {
                        data = bitsToBuffer(data.toString(2));
                    }

                    if (typeof data === 'string') {
                        data = strToBuffer(data);
                    }

                    var len = data.size || data.byteLength || data.length;
                    var zeroes = Math.ceil(Math.ceil(Math.log(len) / Math.log(2)) / 8);
                    var sizeToString = len.toString(2);
                    var padded = (new Array((zeroes * 7 + 7 + 1) - sizeToString.length)).join('0') + sizeToString;
                    var size = (new Array(zeroes)).join('0') + '1' + padded;

                    ebml.push(numToBuffer(json[i].id));
                    ebml.push(bitsToBuffer(size));
                    ebml.push(data);
                }

                return new Blob(ebml, {
                    type: 'video/webm'
                });
            }

            function makeSimpleBlock(data) {
                var flags = 0;

                if (data.keyframe) {
                    flags |= 128;
                }

                if (data.invisible) {
                    flags |= 8;
                }

                if (data.lacing) {
                    flags |= (data.lacing << 1);
                }

                if (data.discardable) {
                    flags |= 1;
                }

                if (data.trackNum > 127) {
                    throw 'TrackNumber > 127 not supported';
                }

                var out = [data.trackNum | 0x80, data.timecode >> 8, data.timecode & 0xff, flags].map(function(e) {
                    return String.fromCharCode(e);
                }).join('') + data.frame;

                return out;
            }

            function parseWebP(riff) {
                var VP8 = riff.RIFF[0].WEBP[0];

                var frameStart = VP8.indexOf('\x9d\x01\x2a'); // A VP8 keyframe starts with the 0x9d012a header
                for (var i = 0, c = []; i < 4; i++) {
                    c[i] = VP8.charCodeAt(frameStart + 3 + i);
                }

                var width, height, tmp;

                //the code below is literally copied verbatim from the bitstream spec
                tmp = (c[1] << 8) | c[0];
                width = tmp & 0x3FFF;
                tmp = (c[3] << 8) | c[2];
                height = tmp & 0x3FFF;
                return {
                    width: width,
                    height: height,
                    data: VP8,
                    riff: riff
                };
            }

            function getStrLength(string, offset) {
                return parseInt(string.substr(offset + 4, 4).split('').map(function(i) {
                    var unpadded = i.charCodeAt(0).toString(2);
                    return (new Array(8 - unpadded.length + 1)).join('0') + unpadded;
                }).join(''), 2);
            }

            function parseRIFF(string) {
                var offset = 0;
                var chunks = {};

                while (offset < string.length) {
                    var id = string.substr(offset, 4);
                    var len = getStrLength(string, offset);
                    var data = string.substr(offset + 4 + 4, len);
                    offset += 4 + 4 + len;
                    chunks[id] = chunks[id] || [];

                    if (id === 'RIFF' || id === 'LIST') {
                        chunks[id].push(parseRIFF(data));
                    } else {
                        chunks[id].push(data);
                    }
                }
                return chunks;
            }

            function doubleToString(num) {
                return [].slice.call(
                    new Uint8Array((new Float64Array([num])).buffer), 0).map(function(e) {
                    return String.fromCharCode(e);
                }).reverse().join('');
            }

            var webm = new ArrayToWebM(frames.map(function(frame) {
                var webp = parseWebP(parseRIFF(atob(frame.image.slice(23))));
                webp.duration = frame.duration;
                return webp;
            }));

            postMessage(webm);
        }

        /**
         * Encodes frames in WebM container. It uses WebWorkinvoke to invoke 'ArrayToWebM' method.
         * @param {function} callback - Callback function, that is used to pass recorded blob back to the callee.
         * @method
         * @memberof Whammy
         * @example
         * recorder = new Whammy().Video(0.8, 100);
         * recorder.compile(function(blob) {
         *    // blob.size - blob.type
         * });
         */
        WhammyVideo.prototype.compile = function(callback) {
            var webWorker = processInWebWorker(whammyInWebWorker);

            webWorker.onmessage = function(event) {
                if (event.data.error) {
                    console.error(event.data.error);
                    return;
                }
                callback(event.data);
            };

            webWorker.postMessage(this.frames);
        };

        return {
            /**
             * A more abstract-ish API.
             * @method
             * @memberof Whammy
             * @example
             * recorder = new Whammy().Video(0.8, 100);
             * @param {?number} speed - 0.8
             * @param {?number} quality - 100
             */
            Video: WhammyVideo
        };
    })();

    if (typeof RecordRTC !== 'undefined') {
        RecordRTC.Whammy = Whammy;
    }

    // ______________ (indexed-db)
    // DiskStorage.js

    /**
     * DiskStorage is a standalone object used by {@link RecordRTC} to store recorded blobs in IndexedDB storage.
     * @summary Writing blobs into IndexedDB.
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @example
     * DiskStorage.Store({
     *     audioBlob: yourAudioBlob,
     *     videoBlob: yourVideoBlob,
     *     gifBlob  : yourGifBlob
     * });
     * DiskStorage.Fetch(function(dataURL, type) {
     *     if(type === 'audioBlob') { }
     *     if(type === 'videoBlob') { }
     *     if(type === 'gifBlob')   { }
     * });
     * // DiskStorage.dataStoreName = 'recordRTC';
     * // DiskStorage.onError = function(error) { };
     * @property {function} init - This method must be called once to initialize IndexedDB ObjectStore. Though, it is auto-used internally.
     * @property {function} Fetch - This method fetches stored blobs from IndexedDB.
     * @property {function} Store - This method stores blobs in IndexedDB.
     * @property {function} onError - This function is invoked for any known/unknown error.
     * @property {string} dataStoreName - Name of the ObjectStore created in IndexedDB storage.
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     */


    var DiskStorage = {
        /**
         * This method must be called once to initialize IndexedDB ObjectStore. Though, it is auto-used internally.
         * @method
         * @memberof DiskStorage
         * @internal
         * @example
         * DiskStorage.init();
         */
        init: function() {
            var self = this;

            if (typeof indexedDB === 'undefined' || typeof indexedDB.open === 'undefined') {
                console.error('IndexedDB API are not available in this browser.');
                return;
            }

            var dbVersion = 1;
            var dbName = this.dbName || location.href.replace(/\/|:|#|%|\.|\[|\]/g, ''),
                db;
            var request = indexedDB.open(dbName, dbVersion);

            function createObjectStore(dataBase) {
                dataBase.createObjectStore(self.dataStoreName);
            }

            function putInDB() {
                var transaction = db.transaction([self.dataStoreName], 'readwrite');

                if (self.videoBlob) {
                    transaction.objectStore(self.dataStoreName).put(self.videoBlob, 'videoBlob');
                }

                if (self.gifBlob) {
                    transaction.objectStore(self.dataStoreName).put(self.gifBlob, 'gifBlob');
                }

                if (self.audioBlob) {
                    transaction.objectStore(self.dataStoreName).put(self.audioBlob, 'audioBlob');
                }

                function getFromStore(portionName) {
                    transaction.objectStore(self.dataStoreName).get(portionName).onsuccess = function(event) {
                        if (self.callback) {
                            self.callback(event.target.result, portionName);
                        }
                    };
                }

                getFromStore('audioBlob');
                getFromStore('videoBlob');
                getFromStore('gifBlob');
            }

            request.onerror = self.onError;

            request.onsuccess = function() {
                db = request.result;
                db.onerror = self.onError;

                if (db.setVersion) {
                    if (db.version !== dbVersion) {
                        var setVersion = db.setVersion(dbVersion);
                        setVersion.onsuccess = function() {
                            createObjectStore(db);
                            putInDB();
                        };
                    } else {
                        putInDB();
                    }
                } else {
                    putInDB();
                }
            };
            request.onupgradeneeded = function(event) {
                createObjectStore(event.target.result);
            };
        },
        /**
         * This method fetches stored blobs from IndexedDB.
         * @method
         * @memberof DiskStorage
         * @internal
         * @example
         * DiskStorage.Fetch(function(dataURL, type) {
         *     if(type === 'audioBlob') { }
         *     if(type === 'videoBlob') { }
         *     if(type === 'gifBlob')   { }
         * });
         */
        Fetch: function(callback) {
            this.callback = callback;
            this.init();

            return this;
        },
        /**
         * This method stores blobs in IndexedDB.
         * @method
         * @memberof DiskStorage
         * @internal
         * @example
         * DiskStorage.Store({
         *     audioBlob: yourAudioBlob,
         *     videoBlob: yourVideoBlob,
         *     gifBlob  : yourGifBlob
         * });
         */
        Store: function(config) {
            this.audioBlob = config.audioBlob;
            this.videoBlob = config.videoBlob;
            this.gifBlob = config.gifBlob;

            this.init();

            return this;
        },
        /**
         * This function is invoked for any known/unknown error.
         * @method
         * @memberof DiskStorage
         * @internal
         * @example
         * DiskStorage.onError = function(error){
         *     alerot( JSON.stringify(error) );
         * };
         */
        onError: function(error) {
            console.error(JSON.stringify(error, null, '\t'));
        },

        /**
         * @property {string} dataStoreName - Name of the ObjectStore created in IndexedDB storage.
         * @memberof DiskStorage
         * @internal
         * @example
         * DiskStorage.dataStoreName = 'recordRTC';
         */
        dataStoreName: 'recordRTC',
        dbName: null
    };

    if (typeof RecordRTC !== 'undefined') {
        RecordRTC.DiskStorage = DiskStorage;
    }

    // ______________
    // GifRecorder.js

    /**
     * GifRecorder is standalone calss used by {@link RecordRTC} to record video or canvas into animated gif.
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @typedef GifRecorder
     * @class
     * @example
     * var recorder = new GifRecorder(mediaStream || canvas || context, { onGifPreview: function, onGifRecordingStarted: function, width: 1280, height: 720, frameRate: 200, quality: 10 });
     * recorder.record();
     * recorder.stop(function(blob) {
     *     img.src = URL.createObjectURL(blob);
     * });
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     * @param {MediaStream} mediaStream - MediaStream object or HTMLCanvasElement or CanvasRenderingContext2D.
     * @param {object} config - {disableLogs:true, initCallback: function, width: 320, height: 240, frameRate: 200, quality: 10}
     */

    function GifRecorder(mediaStream, config) {
        if (typeof GIFEncoder === 'undefined') {
            var script = document.createElement('script');
            script.src = 'https://www.webrtc-experiment.com/gif-recorder.js';
            (document.body || document.documentElement).appendChild(script);
        }

        config = config || {};

        var isHTMLObject = mediaStream instanceof CanvasRenderingContext2D || mediaStream instanceof HTMLCanvasElement;

        /**
         * This method records MediaStream.
         * @method
         * @memberof GifRecorder
         * @example
         * recorder.record();
         */
        this.record = function() {
            if (typeof GIFEncoder === 'undefined') {
                setTimeout(self.record, 1000);
                return;
            }

            if (!isLoadedMetaData) {
                setTimeout(self.record, 1000);
                return;
            }

            if (!isHTMLObject) {
                if (!config.width) {
                    config.width = video.offsetWidth || 320;
                }

                if (!config.height) {
                    config.height = video.offsetHeight || 240;
                }

                if (!config.video) {
                    config.video = {
                        width: config.width,
                        height: config.height
                    };
                }

                if (!config.canvas) {
                    config.canvas = {
                        width: config.width,
                        height: config.height
                    };
                }

                canvas.width = config.canvas.width || 320;
                canvas.height = config.canvas.height || 240;

                video.width = config.video.width || 320;
                video.height = config.video.height || 240;
            }

            // external library to record as GIF images
            gifEncoder = new GIFEncoder();

            // void setRepeat(int iter) 
            // Sets the number of times the set of GIF frames should be played. 
            // Default is 1; 0 means play indefinitely.
            gifEncoder.setRepeat(0);

            // void setFrameRate(Number fps) 
            // Sets frame rate in frames per second. 
            // Equivalent to setDelay(1000/fps).
            // Using "setDelay" instead of "setFrameRate"
            gifEncoder.setDelay(config.frameRate || 200);

            // void setQuality(int quality) 
            // Sets quality of color quantization (conversion of images to the 
            // maximum 256 colors allowed by the GIF specification). 
            // Lower values (minimum = 1) produce better colors, 
            // but slow processing significantly. 10 is the default, 
            // and produces good color mapping at reasonable speeds. 
            // Values greater than 20 do not yield significant improvements in speed.
            gifEncoder.setQuality(config.quality || 10);

            // Boolean start() 
            // This writes the GIF Header and returns false if it fails.
            gifEncoder.start();

            if (typeof config.onGifRecordingStarted === 'function') {
                config.onGifRecordingStarted();
            }

            function drawVideoFrame(time) {
                if (self.clearedRecordedData === true) {
                    return;
                }

                if (isPausedRecording) {
                    return setTimeout(function() {
                        drawVideoFrame(time);
                    }, 100);
                }

                lastAnimationFrame = requestAnimationFrame(drawVideoFrame);

                if (typeof lastFrameTime === undefined) {
                    lastFrameTime = time;
                }

                // ~10 fps
                if (time - lastFrameTime < 90) {
                    return;
                }

                if (!isHTMLObject && video.paused) {
                    // via: https://github.com/muaz-khan/WebRTC-Experiment/pull/316
                    // Tweak for Android Chrome
                    video.play();
                }

                if (!isHTMLObject) {
                    context.drawImage(video, 0, 0, canvas.width, canvas.height);
                }

                if (config.onGifPreview) {
                    config.onGifPreview(canvas.toDataURL('image/png'));
                }

                gifEncoder.addFrame(context);
                lastFrameTime = time;
            }

            lastAnimationFrame = requestAnimationFrame(drawVideoFrame);

            if (config.initCallback) {
                config.initCallback();
            }
        };

        /**
         * This method stops recording MediaStream.
         * @param {function} callback - Callback function, that is used to pass recorded blob back to the callee.
         * @method
         * @memberof GifRecorder
         * @example
         * recorder.stop(function(blob) {
         *     img.src = URL.createObjectURL(blob);
         * });
         */
        this.stop = function(callback) {
            callback = callback || function() {};

            if (lastAnimationFrame) {
                cancelAnimationFrame(lastAnimationFrame);
            }

            /**
             * @property {Blob} blob - The recorded blob object.
             * @memberof GifRecorder
             * @example
             * recorder.stop(function(){
             *     var blob = recorder.blob;
             * });
             */
            this.blob = new Blob([new Uint8Array(gifEncoder.stream().bin)], {
                type: 'image/gif'
            });

            callback(this.blob);

            // bug: find a way to clear old recorded blobs
            gifEncoder.stream().bin = [];
        };

        var isPausedRecording = false;

        /**
         * This method pauses the recording process.
         * @method
         * @memberof GifRecorder
         * @example
         * recorder.pause();
         */
        this.pause = function() {
            isPausedRecording = true;
        };

        /**
         * This method resumes the recording process.
         * @method
         * @memberof GifRecorder
         * @example
         * recorder.resume();
         */
        this.resume = function() {
            isPausedRecording = false;
        };

        /**
         * This method resets currently recorded data.
         * @method
         * @memberof GifRecorder
         * @example
         * recorder.clearRecordedData();
         */
        this.clearRecordedData = function() {
            self.clearedRecordedData = true;
            clearRecordedDataCB();
        };

        function clearRecordedDataCB() {
            if (gifEncoder) {
                gifEncoder.stream().bin = [];
            }
        }

        // for debugging
        this.name = 'GifRecorder';
        this.toString = function() {
            return this.name;
        };

        var canvas = document.createElement('canvas');
        var context = canvas.getContext('2d');

        if (isHTMLObject) {
            if (mediaStream instanceof CanvasRenderingContext2D) {
                context = mediaStream;
                canvas = context.canvas;
            } else if (mediaStream instanceof HTMLCanvasElement) {
                context = mediaStream.getContext('2d');
                canvas = mediaStream;
            }
        }

        var isLoadedMetaData = true;

        if (!isHTMLObject) {
            var video = document.createElement('video');
            video.muted = true;
            video.autoplay = true;
            video.playsInline = true;

            isLoadedMetaData = false;
            video.onloadedmetadata = function() {
                isLoadedMetaData = true;
            };

            setSrcObject(mediaStream, video);

            video.play();
        }

        var lastAnimationFrame = null;
        var lastFrameTime;

        var gifEncoder;

        var self = this;
    }

    if (typeof RecordRTC !== 'undefined') {
        RecordRTC.GifRecorder = GifRecorder;
    }

    // Last time updated: 2019-06-21 4:09:42 AM UTC

    // ________________________
    // MultiStreamsMixer v1.2.2

    // Open-Sourced: https://github.com/muaz-khan/MultiStreamsMixer

    // --------------------------------------------------
    // Muaz Khan     - www.MuazKhan.com
    // MIT License   - www.WebRTC-Experiment.com/licence
    // --------------------------------------------------

    function MultiStreamsMixer(arrayOfMediaStreams, elementClass) {

        var browserFakeUserAgent = 'Fake/5.0 (FakeOS) AppleWebKit/123 (KHTML, like Gecko) Fake/12.3.4567.89 Fake/123.45';

        (function(that) {
            if (typeof RecordRTC !== 'undefined') {
                return;
            }

            if (!that) {
                return;
            }

            if (typeof window !== 'undefined') {
                return;
            }

            if (typeof commonjsGlobal === 'undefined') {
                return;
            }

            commonjsGlobal.navigator = {
                userAgent: browserFakeUserAgent,
                getUserMedia: function() {}
            };

            if (!commonjsGlobal.console) {
                commonjsGlobal.console = {};
            }

            if (typeof commonjsGlobal.console.log === 'undefined' || typeof commonjsGlobal.console.error === 'undefined') {
                commonjsGlobal.console.error = commonjsGlobal.console.log = commonjsGlobal.console.log || function() {
                    console.log(arguments);
                };
            }

            if (typeof document === 'undefined') {
                /*global document:true */
                that.document = {
                    documentElement: {
                        appendChild: function() {
                            return '';
                        }
                    }
                };

                document.createElement = document.captureStream = document.mozCaptureStream = function() {
                    var obj = {
                        getContext: function() {
                            return obj;
                        },
                        play: function() {},
                        pause: function() {},
                        drawImage: function() {},
                        toDataURL: function() {
                            return '';
                        },
                        style: {}
                    };
                    return obj;
                };

                that.HTMLVideoElement = function() {};
            }

            if (typeof location === 'undefined') {
                /*global location:true */
                that.location = {
                    protocol: 'file:',
                    href: '',
                    hash: ''
                };
            }

            if (typeof screen === 'undefined') {
                /*global screen:true */
                that.screen = {
                    width: 0,
                    height: 0
                };
            }

            if (typeof URL === 'undefined') {
                /*global screen:true */
                that.URL = {
                    createObjectURL: function() {
                        return '';
                    },
                    revokeObjectURL: function() {
                        return '';
                    }
                };
            }

            /*global window:true */
            that.window = commonjsGlobal;
        })(typeof commonjsGlobal !== 'undefined' ? commonjsGlobal : null);

        // requires: chrome://flags/#enable-experimental-web-platform-features

        elementClass = elementClass || 'multi-streams-mixer';

        var videos = [];
        var isStopDrawingFrames = false;

        var canvas = document.createElement('canvas');
        var context = canvas.getContext('2d');
        canvas.style.opacity = 0;
        canvas.style.position = 'absolute';
        canvas.style.zIndex = -1;
        canvas.style.top = '-1000em';
        canvas.style.left = '-1000em';
        canvas.className = elementClass;
        (document.body || document.documentElement).appendChild(canvas);

        this.disableLogs = false;
        this.frameInterval = 10;

        this.width = 360;
        this.height = 240;

        // use gain node to prevent echo
        this.useGainNode = true;

        var self = this;

        // _____________________________
        // Cross-Browser-Declarations.js

        // WebAudio API representer
        var AudioContext = window.AudioContext;

        if (typeof AudioContext === 'undefined') {
            if (typeof webkitAudioContext !== 'undefined') {
                /*global AudioContext:true */
                AudioContext = webkitAudioContext;
            }

            if (typeof mozAudioContext !== 'undefined') {
                /*global AudioContext:true */
                AudioContext = mozAudioContext;
            }
        }

        /*jshint -W079 */
        var URL = window.URL;

        if (typeof URL === 'undefined' && typeof webkitURL !== 'undefined') {
            /*global URL:true */
            URL = webkitURL;
        }

        if (typeof navigator !== 'undefined' && typeof navigator.getUserMedia === 'undefined') { // maybe window.navigator?
            if (typeof navigator.webkitGetUserMedia !== 'undefined') {
                navigator.getUserMedia = navigator.webkitGetUserMedia;
            }

            if (typeof navigator.mozGetUserMedia !== 'undefined') {
                navigator.getUserMedia = navigator.mozGetUserMedia;
            }
        }

        var MediaStream = window.MediaStream;

        if (typeof MediaStream === 'undefined' && typeof webkitMediaStream !== 'undefined') {
            MediaStream = webkitMediaStream;
        }

        /*global MediaStream:true */
        if (typeof MediaStream !== 'undefined') {
            // override "stop" method for all browsers
            if (typeof MediaStream.prototype.stop === 'undefined') {
                MediaStream.prototype.stop = function() {
                    this.getTracks().forEach(function(track) {
                        track.stop();
                    });
                };
            }
        }

        var Storage = {};

        if (typeof AudioContext !== 'undefined') {
            Storage.AudioContext = AudioContext;
        } else if (typeof webkitAudioContext !== 'undefined') {
            Storage.AudioContext = webkitAudioContext;
        }

        function setSrcObject(stream, element) {
            if ('srcObject' in element) {
                element.srcObject = stream;
            } else if ('mozSrcObject' in element) {
                element.mozSrcObject = stream;
            } else {
                element.srcObject = stream;
            }
        }

        this.startDrawingFrames = function() {
            drawVideosToCanvas();
        };

        function drawVideosToCanvas() {
            if (isStopDrawingFrames) {
                return;
            }

            var videosLength = videos.length;

            var fullcanvas = false;
            var remaining = [];
            videos.forEach(function(video) {
                if (!video.stream) {
                    video.stream = {};
                }

                if (video.stream.fullcanvas) {
                    fullcanvas = video;
                } else {
                    // todo: video.stream.active or video.stream.live to fix blank frames issues?
                    remaining.push(video);
                }
            });

            if (fullcanvas) {
                canvas.width = fullcanvas.stream.width;
                canvas.height = fullcanvas.stream.height;
            } else if (remaining.length) {
                canvas.width = videosLength > 1 ? remaining[0].width * 2 : remaining[0].width;

                var height = 1;
                if (videosLength === 3 || videosLength === 4) {
                    height = 2;
                }
                if (videosLength === 5 || videosLength === 6) {
                    height = 3;
                }
                if (videosLength === 7 || videosLength === 8) {
                    height = 4;
                }
                if (videosLength === 9 || videosLength === 10) {
                    height = 5;
                }
                canvas.height = remaining[0].height * height;
            } else {
                canvas.width = self.width || 360;
                canvas.height = self.height || 240;
            }

            if (fullcanvas && fullcanvas instanceof HTMLVideoElement) {
                drawImage(fullcanvas);
            }

            remaining.forEach(function(video, idx) {
                drawImage(video, idx);
            });

            setTimeout(drawVideosToCanvas, self.frameInterval);
        }

        function drawImage(video, idx) {
            if (isStopDrawingFrames) {
                return;
            }

            var x = 0;
            var y = 0;
            var width = video.width;
            var height = video.height;

            if (idx === 1) {
                x = video.width;
            }

            if (idx === 2) {
                y = video.height;
            }

            if (idx === 3) {
                x = video.width;
                y = video.height;
            }

            if (idx === 4) {
                y = video.height * 2;
            }

            if (idx === 5) {
                x = video.width;
                y = video.height * 2;
            }

            if (idx === 6) {
                y = video.height * 3;
            }

            if (idx === 7) {
                x = video.width;
                y = video.height * 3;
            }

            if (typeof video.stream.left !== 'undefined') {
                x = video.stream.left;
            }

            if (typeof video.stream.top !== 'undefined') {
                y = video.stream.top;
            }

            if (typeof video.stream.width !== 'undefined') {
                width = video.stream.width;
            }

            if (typeof video.stream.height !== 'undefined') {
                height = video.stream.height;
            }

            context.drawImage(video, x, y, width, height);

            if (typeof video.stream.onRender === 'function') {
                video.stream.onRender(context, x, y, width, height, idx);
            }
        }

        function getMixedStream() {
            isStopDrawingFrames = false;
            var mixedVideoStream = getMixedVideoStream();

            var mixedAudioStream = getMixedAudioStream();
            if (mixedAudioStream) {
                mixedAudioStream.getTracks().filter(function(t) {
                    return t.kind === 'audio';
                }).forEach(function(track) {
                    mixedVideoStream.addTrack(track);
                });
            }
            arrayOfMediaStreams.forEach(function(stream) {
                if (stream.fullcanvas) ;
            });

            // mixedVideoStream.prototype.appendStreams = appendStreams;
            // mixedVideoStream.prototype.resetVideoStreams = resetVideoStreams;
            // mixedVideoStream.prototype.clearRecordedData = clearRecordedData;

            return mixedVideoStream;
        }

        function getMixedVideoStream() {
            resetVideoStreams();

            var capturedStream;

            if ('captureStream' in canvas) {
                capturedStream = canvas.captureStream();
            } else if ('mozCaptureStream' in canvas) {
                capturedStream = canvas.mozCaptureStream();
            } else if (!self.disableLogs) {
                console.error('Upgrade to latest Chrome or otherwise enable this flag: chrome://flags/#enable-experimental-web-platform-features');
            }

            var videoStream = new MediaStream();

            capturedStream.getTracks().filter(function(t) {
                return t.kind === 'video';
            }).forEach(function(track) {
                videoStream.addTrack(track);
            });

            canvas.stream = videoStream;

            return videoStream;
        }

        function getMixedAudioStream() {
            // via: @pehrsons
            if (!Storage.AudioContextConstructor) {
                Storage.AudioContextConstructor = new Storage.AudioContext();
            }

            self.audioContext = Storage.AudioContextConstructor;

            self.audioSources = [];

            if (self.useGainNode === true) {
                self.gainNode = self.audioContext.createGain();
                self.gainNode.connect(self.audioContext.destination);
                self.gainNode.gain.value = 0; // don't hear self
            }

            var audioTracksLength = 0;
            arrayOfMediaStreams.forEach(function(stream) {
                if (!stream.getTracks().filter(function(t) {
                        return t.kind === 'audio';
                    }).length) {
                    return;
                }

                audioTracksLength++;

                var audioSource = self.audioContext.createMediaStreamSource(stream);

                if (self.useGainNode === true) {
                    audioSource.connect(self.gainNode);
                }

                self.audioSources.push(audioSource);
            });

            if (!audioTracksLength) {
                // because "self.audioContext" is not initialized
                // that's why we've to ignore rest of the code
                return;
            }

            self.audioDestination = self.audioContext.createMediaStreamDestination();
            self.audioSources.forEach(function(audioSource) {
                audioSource.connect(self.audioDestination);
            });
            return self.audioDestination.stream;
        }

        function getVideo(stream) {
            var video = document.createElement('video');

            setSrcObject(stream, video);

            video.className = elementClass;

            video.muted = true;
            video.volume = 0;

            video.width = stream.width || self.width || 360;
            video.height = stream.height || self.height || 240;

            video.play();

            return video;
        }

        this.appendStreams = function(streams) {
            if (!streams) {
                throw 'First parameter is required.';
            }

            if (!(streams instanceof Array)) {
                streams = [streams];
            }

            streams.forEach(function(stream) {
                var newStream = new MediaStream();

                if (stream.getTracks().filter(function(t) {
                        return t.kind === 'video';
                    }).length) {
                    var video = getVideo(stream);
                    video.stream = stream;
                    videos.push(video);

                    newStream.addTrack(stream.getTracks().filter(function(t) {
                        return t.kind === 'video';
                    })[0]);
                }

                if (stream.getTracks().filter(function(t) {
                        return t.kind === 'audio';
                    }).length) {
                    var audioSource = self.audioContext.createMediaStreamSource(stream);
                    self.audioDestination = self.audioContext.createMediaStreamDestination();
                    audioSource.connect(self.audioDestination);

                    newStream.addTrack(self.audioDestination.stream.getTracks().filter(function(t) {
                        return t.kind === 'audio';
                    })[0]);
                }

                arrayOfMediaStreams.push(newStream);
            });
        };

        this.releaseStreams = function() {
            videos = [];
            isStopDrawingFrames = true;

            if (self.gainNode) {
                self.gainNode.disconnect();
                self.gainNode = null;
            }

            if (self.audioSources.length) {
                self.audioSources.forEach(function(source) {
                    source.disconnect();
                });
                self.audioSources = [];
            }

            if (self.audioDestination) {
                self.audioDestination.disconnect();
                self.audioDestination = null;
            }

            if (self.audioContext) {
                self.audioContext.close();
            }

            self.audioContext = null;

            context.clearRect(0, 0, canvas.width, canvas.height);

            if (canvas.stream) {
                canvas.stream.stop();
                canvas.stream = null;
            }
        };

        this.resetVideoStreams = function(streams) {
            if (streams && !(streams instanceof Array)) {
                streams = [streams];
            }

            resetVideoStreams(streams);
        };

        function resetVideoStreams(streams) {
            videos = [];
            streams = streams || arrayOfMediaStreams;

            // via: @adrian-ber
            streams.forEach(function(stream) {
                if (!stream.getTracks().filter(function(t) {
                        return t.kind === 'video';
                    }).length) {
                    return;
                }

                var video = getVideo(stream);
                video.stream = stream;
                videos.push(video);
            });
        }

        // for debugging
        this.name = 'MultiStreamsMixer';
        this.toString = function() {
            return this.name;
        };

        this.getMixedStream = getMixedStream;

    }

    if (typeof RecordRTC === 'undefined') {
        {
            module.exports = MultiStreamsMixer;
        }
    }

    // ______________________
    // MultiStreamRecorder.js

    /*
     * Video conference recording, using captureStream API along with WebAudio and Canvas2D API.
     */

    /**
     * MultiStreamRecorder can record multiple videos in single container.
     * @summary Multi-videos recorder.
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @typedef MultiStreamRecorder
     * @class
     * @example
     * var options = {
     *     mimeType: 'video/webm'
     * }
     * var recorder = new MultiStreamRecorder(ArrayOfMediaStreams, options);
     * recorder.record();
     * recorder.stop(function(blob) {
     *     video.src = URL.createObjectURL(blob);
     *
     *     // or
     *     var blob = recorder.blob;
     * });
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     * @param {MediaStreams} mediaStreams - Array of MediaStreams.
     * @param {object} config - {disableLogs:true, frameInterval: 1, mimeType: "video/webm"}
     */

    function MultiStreamRecorder(arrayOfMediaStreams, options) {
        arrayOfMediaStreams = arrayOfMediaStreams || [];
        var self = this;

        var mixer;
        var mediaRecorder;

        options = options || {
            elementClass: 'multi-streams-mixer',
            mimeType: 'video/webm',
            video: {
                width: 360,
                height: 240
            }
        };

        if (!options.frameInterval) {
            options.frameInterval = 10;
        }

        if (!options.video) {
            options.video = {};
        }

        if (!options.video.width) {
            options.video.width = 360;
        }

        if (!options.video.height) {
            options.video.height = 240;
        }

        /**
         * This method records all MediaStreams.
         * @method
         * @memberof MultiStreamRecorder
         * @example
         * recorder.record();
         */
        this.record = function() {
            // github/muaz-khan/MultiStreamsMixer
            mixer = new MultiStreamsMixer(arrayOfMediaStreams, options.elementClass || 'multi-streams-mixer');

            if (getAllVideoTracks().length) {
                mixer.frameInterval = options.frameInterval || 10;
                mixer.width = options.video.width || 360;
                mixer.height = options.video.height || 240;
                mixer.startDrawingFrames();
            }

            if (options.previewStream && typeof options.previewStream === 'function') {
                options.previewStream(mixer.getMixedStream());
            }

            // record using MediaRecorder API
            mediaRecorder = new MediaStreamRecorder(mixer.getMixedStream(), options);
            mediaRecorder.record();
        };

        function getAllVideoTracks() {
            var tracks = [];
            arrayOfMediaStreams.forEach(function(stream) {
                getTracks(stream, 'video').forEach(function(track) {
                    tracks.push(track);
                });
            });
            return tracks;
        }

        /**
         * This method stops recording MediaStream.
         * @param {function} callback - Callback function, that is used to pass recorded blob back to the callee.
         * @method
         * @memberof MultiStreamRecorder
         * @example
         * recorder.stop(function(blob) {
         *     video.src = URL.createObjectURL(blob);
         * });
         */
        this.stop = function(callback) {
            if (!mediaRecorder) {
                return;
            }

            mediaRecorder.stop(function(blob) {
                self.blob = blob;

                callback(blob);

                self.clearRecordedData();
            });
        };

        /**
         * This method pauses the recording process.
         * @method
         * @memberof MultiStreamRecorder
         * @example
         * recorder.pause();
         */
        this.pause = function() {
            if (mediaRecorder) {
                mediaRecorder.pause();
            }
        };

        /**
         * This method resumes the recording process.
         * @method
         * @memberof MultiStreamRecorder
         * @example
         * recorder.resume();
         */
        this.resume = function() {
            if (mediaRecorder) {
                mediaRecorder.resume();
            }
        };

        /**
         * This method resets currently recorded data.
         * @method
         * @memberof MultiStreamRecorder
         * @example
         * recorder.clearRecordedData();
         */
        this.clearRecordedData = function() {
            if (mediaRecorder) {
                mediaRecorder.clearRecordedData();
                mediaRecorder = null;
            }

            if (mixer) {
                mixer.releaseStreams();
                mixer = null;
            }
        };

        /**
         * Add extra media-streams to existing recordings.
         * @method
         * @memberof MultiStreamRecorder
         * @param {MediaStreams} mediaStreams - Array of MediaStreams
         * @example
         * recorder.addStreams([newAudioStream, newVideoStream]);
         */
        this.addStreams = function(streams) {
            if (!streams) {
                throw 'First parameter is required.';
            }

            if (!(streams instanceof Array)) {
                streams = [streams];
            }

            arrayOfMediaStreams.concat(streams);

            if (!mediaRecorder || !mixer) {
                return;
            }

            mixer.appendStreams(streams);

            if (options.previewStream && typeof options.previewStream === 'function') {
                options.previewStream(mixer.getMixedStream());
            }
        };

        /**
         * Reset videos during live recording. Replace old videos e.g. replace cameras with full-screen.
         * @method
         * @memberof MultiStreamRecorder
         * @param {MediaStreams} mediaStreams - Array of MediaStreams
         * @example
         * recorder.resetVideoStreams([newVideo1, newVideo2]);
         */
        this.resetVideoStreams = function(streams) {
            if (!mixer) {
                return;
            }

            if (streams && !(streams instanceof Array)) {
                streams = [streams];
            }

            mixer.resetVideoStreams(streams);
        };

        /**
         * Returns MultiStreamsMixer
         * @method
         * @memberof MultiStreamRecorder
         * @example
         * let mixer = recorder.getMixer();
         * mixer.appendStreams([newStream]);
         */
        this.getMixer = function() {
            return mixer;
        };

        // for debugging
        this.name = 'MultiStreamRecorder';
        this.toString = function() {
            return this.name;
        };
    }

    if (typeof RecordRTC !== 'undefined') {
        RecordRTC.MultiStreamRecorder = MultiStreamRecorder;
    }

    // _____________________
    // RecordRTC.promises.js

    /**
     * RecordRTCPromisesHandler adds promises support in {@link RecordRTC}. Try a {@link https://github.com/muaz-khan/RecordRTC/blob/master/simple-demos/RecordRTCPromisesHandler.html|demo here}
     * @summary Promises for {@link RecordRTC}
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @typedef RecordRTCPromisesHandler
     * @class
     * @example
     * var recorder = new RecordRTCPromisesHandler(mediaStream, options);
     * recorder.startRecording()
     *         .then(successCB)
     *         .catch(errorCB);
     * // Note: You can access all RecordRTC API using "recorder.recordRTC" e.g. 
     * recorder.recordRTC.onStateChanged = function(state) {};
     * recorder.recordRTC.setRecordingDuration(5000);
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     * @param {MediaStream} mediaStream - Single media-stream object, array of media-streams, html-canvas-element, etc.
     * @param {object} config - {type:"video", recorderType: MediaStreamRecorder, disableLogs: true, numberOfAudioChannels: 1, bufferSize: 0, sampleRate: 0, video: HTMLVideoElement, etc.}
     * @throws Will throw an error if "new" keyword is not used to initiate "RecordRTCPromisesHandler". Also throws error if first argument "MediaStream" is missing.
     * @requires {@link RecordRTC}
     */

    function RecordRTCPromisesHandler(mediaStream, options) {
        if (!this) {
            throw 'Use "new RecordRTCPromisesHandler()"';
        }

        if (typeof mediaStream === 'undefined') {
            throw 'First argument "MediaStream" is required.';
        }

        var self = this;

        /**
         * @property {Blob} blob - Access/reach the native {@link RecordRTC} object.
         * @memberof RecordRTCPromisesHandler
         * @example
         * let internal = recorder.recordRTC.getInternalRecorder();
         * alert(internal instanceof MediaStreamRecorder);
         * recorder.recordRTC.onStateChanged = function(state) {};
         */
        self.recordRTC = new RecordRTC(mediaStream, options);

        /**
         * This method records MediaStream.
         * @method
         * @memberof RecordRTCPromisesHandler
         * @example
         * recorder.startRecording()
         *         .then(successCB)
         *         .catch(errorCB);
         */
        this.startRecording = function() {
            return new Promise(function(resolve, reject) {
                try {
                    self.recordRTC.startRecording();
                    resolve();
                } catch (e) {
                    reject(e);
                }
            });
        };

        /**
         * This method stops the recording.
         * @method
         * @memberof RecordRTCPromisesHandler
         * @example
         * recorder.stopRecording().then(function() {
         *     var blob = recorder.getBlob();
         * }).catch(errorCB);
         */
        this.stopRecording = function() {
            return new Promise(function(resolve, reject) {
                try {
                    self.recordRTC.stopRecording(function(url) {
                        self.blob = self.recordRTC.getBlob();

                        if (!self.blob || !self.blob.size) {
                            reject('Empty blob.', self.blob);
                            return;
                        }

                        resolve(url);
                    });
                } catch (e) {
                    reject(e);
                }
            });
        };

        /**
         * This method pauses the recording. You can resume recording using "resumeRecording" method.
         * @method
         * @memberof RecordRTCPromisesHandler
         * @example
         * recorder.pauseRecording()
         *         .then(successCB)
         *         .catch(errorCB);
         */
        this.pauseRecording = function() {
            return new Promise(function(resolve, reject) {
                try {
                    self.recordRTC.pauseRecording();
                    resolve();
                } catch (e) {
                    reject(e);
                }
            });
        };

        /**
         * This method resumes the recording.
         * @method
         * @memberof RecordRTCPromisesHandler
         * @example
         * recorder.resumeRecording()
         *         .then(successCB)
         *         .catch(errorCB);
         */
        this.resumeRecording = function() {
            return new Promise(function(resolve, reject) {
                try {
                    self.recordRTC.resumeRecording();
                    resolve();
                } catch (e) {
                    reject(e);
                }
            });
        };

        /**
         * This method returns data-url for the recorded blob.
         * @method
         * @memberof RecordRTCPromisesHandler
         * @example
         * recorder.stopRecording().then(function() {
         *     recorder.getDataURL().then(function(dataURL) {
         *         window.open(dataURL);
         *     }).catch(errorCB);;
         * }).catch(errorCB);
         */
        this.getDataURL = function(callback) {
            return new Promise(function(resolve, reject) {
                try {
                    self.recordRTC.getDataURL(function(dataURL) {
                        resolve(dataURL);
                    });
                } catch (e) {
                    reject(e);
                }
            });
        };

        /**
         * This method returns the recorded blob.
         * @method
         * @memberof RecordRTCPromisesHandler
         * @example
         * recorder.stopRecording().then(function() {
         *     recorder.getBlob().then(function(blob) {})
         * }).catch(errorCB);
         */
        this.getBlob = function() {
            return new Promise(function(resolve, reject) {
                try {
                    resolve(self.recordRTC.getBlob());
                } catch (e) {
                    reject(e);
                }
            });
        };

        /**
         * This method returns the internal recording object.
         * @method
         * @memberof RecordRTCPromisesHandler
         * @example
         * let internalRecorder = await recorder.getInternalRecorder();
         * if(internalRecorder instanceof MultiStreamRecorder) {
         *     internalRecorder.addStreams([newAudioStream]);
         *     internalRecorder.resetVideoStreams([screenStream]);
         * }
         * @returns {Object} 
         */
        this.getInternalRecorder = function() {
            return new Promise(function(resolve, reject) {
                try {
                    resolve(self.recordRTC.getInternalRecorder());
                } catch (e) {
                    reject(e);
                }
            });
        };

        /**
         * This method resets the recorder. So that you can reuse single recorder instance many times.
         * @method
         * @memberof RecordRTCPromisesHandler
         * @example
         * await recorder.reset();
         * recorder.startRecording(); // record again
         */
        this.reset = function() {
            return new Promise(function(resolve, reject) {
                try {
                    resolve(self.recordRTC.reset());
                } catch (e) {
                    reject(e);
                }
            });
        };

        /**
         * Destroy RecordRTC instance. Clear all recorders and objects.
         * @method
         * @memberof RecordRTCPromisesHandler
         * @example
         * recorder.destroy().then(successCB).catch(errorCB);
         */
        this.destroy = function() {
            return new Promise(function(resolve, reject) {
                try {
                    resolve(self.recordRTC.destroy());
                } catch (e) {
                    reject(e);
                }
            });
        };

        /**
         * Get recorder's readonly state.
         * @method
         * @memberof RecordRTCPromisesHandler
         * @example
         * let state = await recorder.getState();
         * // or
         * recorder.getState().then(state => { console.log(state); })
         * @returns {String} Returns recording state.
         */
        this.getState = function() {
            return new Promise(function(resolve, reject) {
                try {
                    resolve(self.recordRTC.getState());
                } catch (e) {
                    reject(e);
                }
            });
        };

        /**
         * @property {Blob} blob - Recorded data as "Blob" object.
         * @memberof RecordRTCPromisesHandler
         * @example
         * await recorder.stopRecording();
         * let blob = recorder.getBlob(); // or "recorder.recordRTC.blob"
         * invokeSaveAsDialog(blob);
         */
        this.blob = null;

        /**
         * RecordRTC version number
         * @property {String} version - Release version number.
         * @memberof RecordRTCPromisesHandler
         * @static
         * @readonly
         * @example
         * alert(recorder.version);
         */
        this.version = '5.6.2';
    }

    if (typeof RecordRTC !== 'undefined') {
        RecordRTC.RecordRTCPromisesHandler = RecordRTCPromisesHandler;
    }

    // ______________________
    // WebAssemblyRecorder.js

    /**
     * WebAssemblyRecorder lets you create webm videos in JavaScript via WebAssembly. The library consumes raw RGBA32 buffers (4 bytes per pixel) and turns them into a webm video with the given framerate and quality. This makes it compatible out-of-the-box with ImageData from a CANVAS. With realtime mode you can also use webm-wasm for streaming webm videos.
     * @summary Video recording feature in Chrome, Firefox and maybe Edge.
     * @license {@link https://github.com/muaz-khan/RecordRTC/blob/master/LICENSE|MIT}
     * @author {@link https://MuazKhan.com|Muaz Khan}
     * @typedef WebAssemblyRecorder
     * @class
     * @example
     * var recorder = new WebAssemblyRecorder(mediaStream);
     * recorder.record();
     * recorder.stop(function(blob) {
     *     video.src = URL.createObjectURL(blob);
     * });
     * @see {@link https://github.com/muaz-khan/RecordRTC|RecordRTC Source Code}
     * @param {MediaStream} mediaStream - MediaStream object fetched using getUserMedia API or generated using captureStreamUntilEnded or WebAudio API.
     * @param {object} config - {webAssemblyPath:'webm-wasm.wasm',workerPath: 'webm-worker.js', frameRate: 30, width: 1920, height: 1080, bitrate: 1024, realtime: true}
     */
    function WebAssemblyRecorder(stream, config) {
        // based on: github.com/GoogleChromeLabs/webm-wasm

        if (typeof ReadableStream === 'undefined' || typeof WritableStream === 'undefined') {
            // because it fixes readable/writable streams issues
            console.error('Following polyfill is strongly recommended: https://unpkg.com/@mattiasbuelens/web-streams-polyfill/dist/polyfill.min.js');
        }

        config = config || {};

        config.width = config.width || 640;
        config.height = config.height || 480;
        config.frameRate = config.frameRate || 30;
        config.bitrate = config.bitrate || 1200;
        config.realtime = config.realtime || true;

        var finished;

        function cameraStream() {
            return new ReadableStream({
                start: function(controller) {
                    var cvs = document.createElement('canvas');
                    var video = document.createElement('video');
                    var first = true;
                    video.srcObject = stream;
                    video.muted = true;
                    video.height = config.height;
                    video.width = config.width;
                    video.volume = 0;
                    video.onplaying = function() {
                        cvs.width = config.width;
                        cvs.height = config.height;
                        var ctx = cvs.getContext('2d');
                        var frameTimeout = 1000 / config.frameRate;
                        var cameraTimer = setInterval(function f() {
                            if (finished) {
                                clearInterval(cameraTimer);
                                controller.close();
                            }

                            if (first) {
                                first = false;
                                if (config.onVideoProcessStarted) {
                                    config.onVideoProcessStarted();
                                }
                            }

                            ctx.drawImage(video, 0, 0);
                            if (controller._controlledReadableStream.state !== 'closed') {
                                try {
                                    controller.enqueue(
                                        ctx.getImageData(0, 0, config.width, config.height)
                                    );
                                } catch (e) {}
                            }
                        }, frameTimeout);
                    };
                    video.play();
                }
            });
        }

        var worker;

        function startRecording(stream, buffer) {
            if (!config.workerPath && !buffer) {
                finished = false;

                // is it safe to use @latest ?

                fetch(
                    'https://unpkg.com/webm-wasm@latest/dist/webm-worker.js'
                ).then(function(r) {
                    r.arrayBuffer().then(function(buffer) {
                        startRecording(stream, buffer);
                    });
                });
                return;
            }

            if (!config.workerPath && buffer instanceof ArrayBuffer) {
                var blob = new Blob([buffer], {
                    type: 'text/javascript'
                });
                config.workerPath = URL.createObjectURL(blob);
            }

            if (!config.workerPath) {
                console.error('workerPath parameter is missing.');
            }

            worker = new Worker(config.workerPath);

            worker.postMessage(config.webAssemblyPath || 'https://unpkg.com/webm-wasm@latest/dist/webm-wasm.wasm');
            worker.addEventListener('message', function(event) {
                if (event.data === 'READY') {
                    worker.postMessage({
                        width: config.width,
                        height: config.height,
                        bitrate: config.bitrate || 1200,
                        timebaseDen: config.frameRate || 30,
                        realtime: config.realtime
                    });

                    cameraStream().pipeTo(new WritableStream({
                        write: function(image) {
                            if (finished) {
                                console.error('Got image, but recorder is finished!');
                                return;
                            }

                            worker.postMessage(image.data.buffer, [image.data.buffer]);
                        }
                    }));
                } else if (!!event.data) {
                    if (!isPaused) {
                        arrayOfBuffers.push(event.data);
                    }
                }
            });
        }

        /**
         * This method records video.
         * @method
         * @memberof WebAssemblyRecorder
         * @example
         * recorder.record();
         */
        this.record = function() {
            arrayOfBuffers = [];
            isPaused = false;
            this.blob = null;
            startRecording(stream);

            if (typeof config.initCallback === 'function') {
                config.initCallback();
            }
        };

        var isPaused;

        /**
         * This method pauses the recording process.
         * @method
         * @memberof WebAssemblyRecorder
         * @example
         * recorder.pause();
         */
        this.pause = function() {
            isPaused = true;
        };

        /**
         * This method resumes the recording process.
         * @method
         * @memberof WebAssemblyRecorder
         * @example
         * recorder.resume();
         */
        this.resume = function() {
            isPaused = false;
        };

        function terminate(callback) {
            if (!worker) {
                if (callback) {
                    callback();
                }

                return;
            }

            // Wait for null event data to indicate that the encoding is complete
            worker.addEventListener('message', function(event) {
                if (event.data === null) {
                    worker.terminate();
                    worker = null;

                    if (callback) {
                        callback();
                    }
                }
            });

            worker.postMessage(null);
        }

        var arrayOfBuffers = [];

        /**
         * This method stops recording video.
         * @param {function} callback - Callback function, that is used to pass recorded blob back to the callee.
         * @method
         * @memberof WebAssemblyRecorder
         * @example
         * recorder.stop(function(blob) {
         *     video.src = URL.createObjectURL(blob);
         * });
         */
        this.stop = function(callback) {
            finished = true;

            var recorder = this;

            terminate(function() {
                recorder.blob = new Blob(arrayOfBuffers, {
                    type: 'video/webm'
                });

                callback(recorder.blob);
            });
        };

        // for debugging
        this.name = 'WebAssemblyRecorder';
        this.toString = function() {
            return this.name;
        };

        /**
         * This method resets currently recorded data.
         * @method
         * @memberof WebAssemblyRecorder
         * @example
         * recorder.clearRecordedData();
         */
        this.clearRecordedData = function() {
            arrayOfBuffers = [];
            isPaused = false;
            this.blob = null;

            // todo: if recording-ON then STOP it first
        };

        /**
         * @property {Blob} blob - The recorded blob object.
         * @memberof WebAssemblyRecorder
         * @example
         * recorder.stop(function(){
         *     var blob = recorder.blob;
         * });
         */
        this.blob = null;
    }

    if (typeof RecordRTC !== 'undefined') {
        RecordRTC.WebAssemblyRecorder = WebAssemblyRecorder;
    }
    });

    class RecordRTCLoader extends Emitter {
      constructor(player) {
        super();
        this.player = player;
        this.fileName = '';
        this.fileType = FILE_SUFFIX$1.webm;
        this.isRecording = false;
        this.recordingTimestamp = 0;
        this.recordingInterval = null;
        player.debug.log('Recorder', 'init');
      }

      setFileName(fileName, fileType) {
        this.fileName = fileName;

        if (FILE_SUFFIX$1.mp4 === fileType || FILE_SUFFIX$1.webm === fileType) {
          this.fileType = fileType;
        }
      }

      get recording() {
        return this.isRecording;
      }

      get recordTime() {
        return this.recordingTimestamp;
      }

      startRecord() {
        const debug = this.player.debug;
        const options = {
          type: 'video',
          mimeType: 'video/webm;codecs=h264',
          onTimeStamp: timestamp => {
            debug.log('Recorder', 'record timestamp :' + timestamp);
          },
          disableLogs: !this.player._opt.debug
        };

        try {
          const stream = this.player.video.$videoElement.captureStream(25);
          const audioStream = this.player.audio.mediaStreamAudioDestinationNode.stream;
          stream.addTrack(audioStream.getAudioTracks()[0]);
          this.recorder = RecordRTC_1(stream, options);
        } catch (e) {
          debug.error('Recorder', e);
          this.emit(EVENTS.recordCreateError);
        }

        if (this.recorder) {
          this.isRecording = true;
          this.emit(EVENTS.recording, true);
          this.recorder.startRecording();
          debug.log('Recorder', 'start recording');
          this.player.emit(EVENTS.recordStart);
          this.recordingInterval = window.setInterval(() => {
            this.recordingTimestamp += 1;
            this.player.emit(EVENTS.recordingTimestamp, this.recordingTimestamp);
          }, 1000);
        }
      }

      stopRecordAndSave() {
        if (!this.recorder || !this.isRecording) {
          return;
        }

        this.recorder.stopRecording(() => {
          this.player.debug.log('Recorder', 'stop recording');
          this.player.emit(EVENTS.recordEnd);
          downloadRecord(this.recorder.getBlob(), this.fileName, this.fileType);

          this._reset();

          this.emit(EVENTS.recording, false);
        });
      }

      _reset() {
        this.isRecording = false;
        this.recordingTimestamp = 0;

        if (this.recorder) {
          this.recorder.destroy();
          this.recorder = null;
        }

        this.fileName = null;

        if (this.recordingInterval) {
          clearInterval(this.recordingInterval);
        }

        this.recordingInterval = null;
      }

      destroy() {
        this._reset();

        this.player.debug.log('Recorder', 'destroy');
        this.player = null;
      }

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

        this._initDecoderWorker();

        player.debug.log('decoderWorker', 'init');
      }

      _initDecoderWorker() {
        const {
          debug,
          events: {
            proxy
          }
        } = this.player;

        this.decoderWorker.onmessage = event => {
          const msg = event.data;

          switch (msg.cmd) {
            case WORKER_CMD_TYPE.init:
              debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.init);

              if (!this.player.loaded) {
                this.player.emit(EVENTS.load);
              }

              this.player.emit(EVENTS.decoderWorkerInit);

              this._initWork();

              break;

            case WORKER_CMD_TYPE.videoCode:
              debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.videoCode, msg.code);
              this.player.video.updateVideoInfo({
                encTypeCode: msg.code
              });
              break;

            case WORKER_CMD_TYPE.audioCode:
              debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.audioCode, msg.code);
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
              break;

            case WORKER_CMD_TYPE.initAudio:
              debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.initAudio, `channels:${msg.channels},sampleRate:${msg.sampleRate}`);
              this.player.audio.updateAudioInfo(msg);
              this.player.audio.initScriptNode(msg);
              break;

            case WORKER_CMD_TYPE.render:
              // debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.render, `msg ts:${msg.ts}`);
              this.player.handleRender();
              this.player.video.render(msg);
              this.player.emit(EVENTS.timeUpdate, msg.ts);
              this.player.updateStats({
                fps: true,
                ts: msg.ts,
                buf: msg.delay
              });
              break;

            case WORKER_CMD_TYPE.playAudio:
              // debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.playAudio, `msg ts:${msg.ts}`);
              // 只有在 playing 的时候。
              if (this.player.playing) {
                this.player.audio.play(msg.buffer, msg.ts);
              }

              break;

            default:
              this.player[msg.cmd] && this.player[msg.cmd](msg);
          }
        };
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
        }; // this.player.debug.log('decoderWorker', 'decodeVideo', options);

        this.decoderWorker.postMessage({
          cmd: WORKER_SEND_TYPE.decode,
          buffer: arrayBuffer,
          options
        }, [arrayBuffer.buffer]);
      }

      decodeAudio(arrayBuffer, ts) {
        if (this.player._opt.useWCS && !this.player._opt.useOffscreen) {
          this._decodeAudioNoDelay(arrayBuffer, ts);
        } else if (this.player._opt.useMSE) {
          this._decodeAudioNoDelay(arrayBuffer, ts);
        } else {
          this._decodeAudio(arrayBuffer, ts);
        }
      } //


      _decodeAudio(arrayBuffer, ts) {
        const options = {
          type: MEDIA_TYPE.audio,
          ts: Math.max(ts, 0)
        }; // this.player.debug.log('decoderWorker', 'decodeAudio',options);

        this.decoderWorker.postMessage({
          cmd: WORKER_SEND_TYPE.decode,
          buffer: arrayBuffer,
          options
        }, [arrayBuffer.buffer]);
      }

      _decodeAudioNoDelay(arrayBuffer, ts) {
        this.decoderWorker.postMessage({
          cmd: WORKER_SEND_TYPE.audioDecode,
          buffer: arrayBuffer,
          ts: Math.max(ts, 0)
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

    class CommonLoader extends Emitter {
      constructor(player) {
        super();
        this.player = player;
        this.stopId = null;
        this.firstTimestamp = null;
        this.startTimestamp = null;
        this.delay = -1;
        this.bufferList = [];
        this.dropping = false;
        this.initInterval();
      }

      getDelay(timestamp) {
        if (!timestamp) {
          return -1;
        }

        if (!this.firstTimestamp) {
          this.firstTimestamp = timestamp;
          this.startTimestamp = Date.now();
          this.delay = -1;
        } else {
          if (timestamp) {
            this.delay = Date.now() - this.startTimestamp - (timestamp - this.firstTimestamp);
          }
        }

        return this.delay;
      } //


      initInterval() {
        const videoBuffer = this.player._opt.videoBuffer;
        this.player.debug.log('common dumex', `init Interval`);

        let _loop = () => {
          let data;

          if (this.bufferList.length) {
            if (this.dropping) {
              data = this.bufferList.shift();

              while (!data.isIFrame && this.bufferList.length) {
                data = this.bufferList.shift();
              }

              if (data.isIFrame) {
                this.dropping = false;

                this._doDecoderDecode(data);
              }
            } else {
              data = this.bufferList[0];

              if (this.getDelay(data.ts) === -1) {
                this.bufferList.shift();

                this._doDecoderDecode(data);
              } else if (this.delay > videoBuffer + 1000) {
                this.dropping = true;
              } else {
                while (this.bufferList.length) {
                  data = this.bufferList[0];

                  if (this.getDelay(data.ts) > videoBuffer) {
                    // drop frame
                    this.bufferList.shift();

                    this._doDecoderDecode(data);
                  } else {
                    // this.player.debug.log('common dumex', `delay is ${this.delay}`);
                    break;
                  }
                }
              }
            }
          }
        };

        _loop();

        this.stopId = setInterval(_loop, 10);
      }

      _doDecode(payload, type, ts, isIFrame) {
        const player = this.player;
        const {
          decoderWorker
        } = player;
        let options = {
          ts: ts,
          type: type,
          isIFrame: false
        };

        if (player._opt.useWCS && !player._opt.useOffscreen) {
          if (type === MEDIA_TYPE.video) {
            options.isIFrame = isIFrame;
          }

          this.pushBuffer(payload, options);
        } else if (player._opt.useMSE) {
          if (type === MEDIA_TYPE.video) {
            options.isIFrame = isIFrame;
          }

          this.pushBuffer(payload, options);
        } else {
          //
          if (type === MEDIA_TYPE.video) {
            decoderWorker.decodeVideo(payload, ts, isIFrame);
          } else if (type === MEDIA_TYPE.audio) {
            decoderWorker.decodeAudio(payload, ts);
          }
        }
      }

      _doDecoderDecode(data) {
        const player = this.player;
        const {
          decoderWorker,
          webcodecsDecoder,
          mseDecoder
        } = player;

        if (data.type === MEDIA_TYPE.audio) {
          decoderWorker.decodeAudio(data.payload, data.ts);
        } else if (data.type === MEDIA_TYPE.video) {
          if (player._opt.useWCS && !player._opt.useOffscreen) {
            webcodecsDecoder.decodeVideo(data.payload, data.ts, data.isIFrame);
          } else if (player._opt.useMSE) {
            mseDecoder.decodeVideo(data.payload, data.ts, data.isIFrame);
          }
        }
      }

      pushBuffer(payload, options) {
        // 音频
        if (options.type === MEDIA_TYPE.audio) {
          this.bufferList.push({
            ts: options.ts,
            payload: payload,
            type: MEDIA_TYPE.audio
          });
        } else if (options.type === MEDIA_TYPE.video) {
          this.bufferList.push({
            ts: options.ts,
            payload: payload,
            type: MEDIA_TYPE.video,
            isIFrame: options.isIFrame
          });
        }
      }

      close() {}

      destroy() {
        if (this.stopId) {
          clearInterval(this.stopId);
          this.stopId = null;
        }

        this.firstTimestamp = null;
        this.startTimestamp = null;
        this.delay = -1;
        this.bufferList = [];
        this.dropping = false;
        this.off();
      }

    }

    class FlvLoader extends CommonLoader {
      constructor(player) {
        super(player);
        this.input = this._inputFlv();
        this.flvDemux = this.dispatchFlvData(this.input);
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
                player.updateStats({
                  abps: payload.byteLength
                });

                if (payload.byteLength > 0) {
                  this._doDecode(payload, MEDIA_TYPE.audio, ts);
                }
              }

              break;

            case FLV_MEDIA_TYPE.video:
              if (player._opt.hasVideo) {
                player.updateStats({
                  vbps: payload.byteLength
                });
                const isIFrame = payload[0] >> 4 === 1;

                if (payload.byteLength > 0) {
                  this._doDecode(payload, MEDIA_TYPE.video, ts, isIFrame);
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

      close() {
        this.input && this.input.return(null);
      }

      destroy() {
        super.destroy();
        this.input = null;
        this.flvDemux = null;
        this.player.debug.log('FlvDemux', 'destroy');
      }

    }

    class M7sLoader extends CommonLoader {
      constructor(player) {
        super(player);
        player.debug.log('M7sDemux', 'init');
      }

      dispatch(data) {
        const player = this.player;
        const dv = new DataView(data);
        const type = dv.getUint8(0);
        const ts = dv.getUint32(1, false);

        switch (type) {
          case MEDIA_TYPE.audio:
            if (player._opt.hasAudio) {
              const payload = new Uint8Array(data, 5);
              player.updateStats({
                abps: payload.byteLength
              });

              if (payload.byteLength > 0) {
                this._doDecode(payload, type, ts);
              }
            }

            break;

          case MEDIA_TYPE.video:
            if (player._opt.hasVideo) {
              if (dv.byteLength > 5) {
                const payload = new Uint8Array(data, 5);
                const isIframe = dv.getUint8(5) >> 4 === 1;
                player.updateStats({
                  vbps: payload.byteLength
                });

                if (payload.byteLength > 0) {
                  this._doDecode(payload, type, ts, isIframe);
                }
              }
            }

            break;
        }
      }

      destroy() {
        super.destroy();
        this.player.debug.log('M7sDemux', 'destroy');
      }

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
        player.debug.log('Webcodecs', 'init');
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

        this.player.handleRender();
        this.player.video.render({
          videoFrame
        });
        this.player.updateStats({
          fps: true,
          ts: 0,
          buf: this.player.demux.delay
        }); // release resource

        setTimeout(function () {
          if (videoFrame.close) {
            videoFrame.close();
          } else {
            videoFrame.destroy();
          }
        }, 100);
      }

      handleError(error) {
        this.player.debug.log('Webcodecs', 'VideoDecoder handleError', error);
      }

      decodeVideo(payload, ts, isIframe) {
        // this.player.debug.log('Webcodecs decoder', 'decodeVideo', ts, isIframe);
        if (!this.hasInit) {
          if (isIframe && payload[1] === 0) {
            const videoCodec = payload[0] & 0x0F;
            this.player.video.updateVideoInfo({
              encTypeCode: videoCodec
            }); // 如果解码出来的是

            if (videoCodec === VIDEO_ENC_CODE.h265) {
              this.emit(EVENTS_ERROR.webcodecsH265NotSupport);
              return;
            }

            const config = formatVideoDecoderConfigure(payload.slice(5));
            this.decoder.configure(config);
            this.hasInit = true;
          }
        } else {
          const chunk = new EncodedVideoChunk({
            data: payload.slice(5),
            timestamp: ts,
            type: isIframe ? ENCODED_VIDEO_TYPE.key : ENCODED_VIDEO_TYPE.delta
          });
          this.decoder.decode(chunk);
        }
      }

      destroy() {
        this.decoder.close();
        this.decoder = null;
        this.hasInit = false;
        this.isInitInfo = false;
        this.off();
        this.player.debug.log('Webcodecs', 'destroy');
        this.player = null;
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
    ${iconsMap[key] ? `<span class="icon-title-tips"><span class="icon-title">${iconsMap[key]}</span></span>` : ''}
`;
      return icons;
    }, {});

    var template = ((player, control) => {
      player.$container.classList.add('jessibuca-controls-show');
      const options = player._opt;
      const operateBtns = options.operateBtns;
      player.$container.insertAdjacentHTML('beforeend', `
            ${options.background ? `<div class="jessibuca-poster" style="background-image: url(${options.background})"></div>` : ''}
            <div class="jessibuca-loading">
                ${icons.loading}
                ${options.loadingText ? `<div class="jessibuca-loading-text">${options.loadingText}</div>` : ''}
            </div>
            ${options.hasControl && operateBtns.play ? `<div class="jessibuca-play-big"></div>` : ''}
            ${options.hasControl ? `
                <div class="jessibuca-controls">
                    <div class="jessibuca-controls-bottom">
                        <div class="jessibuca-controls-left">
                            ${options.showBandwidth ? `<div class="jessibuca-controls-item jessibuca-speed"></div>` : ''}
                        </div>
                        <div class="jessibuca-controls-right">
                             ${operateBtns.audio ? `
                                 <div class="jessibuca-controls-item jessibuca-volume">
                                     ${icons.audio}
                                     ${icons.mute}
                                     <div class="jessibuca-volume-panel-wrap">
                                          <div class="jessibuca-volume-panel">
                                                 <div class="jessibuca-volume-panel-handle"></div>
                                          </div>
                                          <div class="jessibuca-volume-panel-text"></div>
                                     </div>
                                 </div>
                             ` : ''}
                             ${operateBtns.play ? `<div class="jessibuca-controls-item jessibuca-play">${icons.play}</div><div class="jessibuca-controls-item jessibuca-pause">${icons.pause}</div>` : ''}
                             ${operateBtns.screenshot ? `<div class="jessibuca-controls-item jessibuca-screenshot">${icons.screenshot}</div>` : ''}
                             ${operateBtns.record ? ` <div class="jessibuca-controls-item jessibuca-record">${icons.record}</div><div class="jessibuca-controls-item jessibuca-record-stop">${icons.recordStop}</div>` : ''}
                             ${operateBtns.fullscreen ? `<div class="jessibuca-controls-item jessibuca-fullscreen">${icons.fullscreen}</div><div class="jessibuca-controls-item jessibuca-fullscreen-exit">${icons.fullscreenExit}</div>` : ''}
                        </div>
                    </div>
                </div>
            ` : ''}

        `);
      Object.defineProperty(control, '$poster', {
        value: player.$container.querySelector('.jessibuca-poster')
      });
      Object.defineProperty(control, '$loading', {
        value: player.$container.querySelector('.jessibuca-loading')
      });
      Object.defineProperty(control, '$play', {
        value: player.$container.querySelector('.jessibuca-play')
      });
      Object.defineProperty(control, '$playBig', {
        value: player.$container.querySelector('.jessibuca-play-big')
      });
      Object.defineProperty(control, '$pause', {
        value: player.$container.querySelector('.jessibuca-pause')
      });
      Object.defineProperty(control, '$controls', {
        value: player.$container.querySelector('.jessibuca-controls')
      });
      Object.defineProperty(control, '$fullscreen', {
        value: player.$container.querySelector('.jessibuca-fullscreen')
      });
      Object.defineProperty(control, '$fullscreen', {
        value: player.$container.querySelector('.jessibuca-fullscreen')
      });
      Object.defineProperty(control, '$volume', {
        value: player.$container.querySelector('.jessibuca-volume')
      });
      Object.defineProperty(control, '$volumePanelWrap', {
        value: player.$container.querySelector('.jessibuca-volume-panel-wrap')
      });
      Object.defineProperty(control, '$volumePanelText', {
        value: player.$container.querySelector('.jessibuca-volume-panel-text')
      });
      Object.defineProperty(control, '$volumePanel', {
        value: player.$container.querySelector('.jessibuca-volume-panel')
      });
      Object.defineProperty(control, '$volumeHandle', {
        value: player.$container.querySelector('.jessibuca-volume-panel-handle')
      });
      Object.defineProperty(control, '$volumeOn', {
        value: player.$container.querySelector('.jessibuca-icon-audio')
      });
      Object.defineProperty(control, '$volumeOff', {
        value: player.$container.querySelector('.jessibuca-icon-mute')
      });
      Object.defineProperty(control, '$fullscreen', {
        value: player.$container.querySelector('.jessibuca-fullscreen')
      });
      Object.defineProperty(control, '$fullscreenExit', {
        value: player.$container.querySelector('.jessibuca-fullscreen-exit')
      });
      Object.defineProperty(control, '$record', {
        value: player.$container.querySelector('.jessibuca-record')
      });
      Object.defineProperty(control, '$recordStop', {
        value: player.$container.querySelector('.jessibuca-record-stop')
      });
      Object.defineProperty(control, '$screenshot', {
        value: player.$container.querySelector('.jessibuca-screenshot')
      });
      Object.defineProperty(control, '$speed', {
        value: player.$container.querySelector('.jessibuca-speed')
      });
    });

    var observer$1 = ((player, control) => {
      const {
        events: {
          proxy
        }
      } = player;
      const object = document.createElement('object');
      object.setAttribute('aria-hidden', 'true');
      object.setAttribute('tabindex', -1);
      object.type = 'text/html';
      object.data = 'about:blank';
      setStyle(object, {
        display: 'block',
        position: 'absolute',
        top: '0',
        left: '0',
        height: '100%',
        width: '100%',
        overflow: 'hidden',
        pointerEvents: 'none',
        zIndex: '-1'
      });
      let playerWidth = player.width;
      let playerHeight = player.height;
      proxy(object, 'load', () => {
        proxy(object.contentDocument.defaultView, 'resize', () => {
          if (player.width !== playerWidth || player.height !== playerHeight) {
            playerWidth = player.width;
            playerHeight = player.height;
            player.emit(EVENTS.resize);
          }
        });
      });
      player.$container.appendChild(object);
      player.on(EVENTS.destroy, () => {
        player.$container.removeChild(object);
      });

      function setVolumeHandle(percentage) {
        if (percentage === 0) {
          setStyle(control.$volumeOn, 'display', 'none');
          setStyle(control.$volumeOff, 'display', 'flex');
          setStyle(control.$volumeHandle, 'top', `${48}px`);
        } else {
          if (control.$volumeHandle && control.$volumePanel) {
            const panelHeight = getStyle(control.$volumePanel, 'height') || 60;
            const handleHeight = getStyle(control.$volumeHandle, 'height');
            const top = panelHeight - (panelHeight - handleHeight) * percentage - handleHeight;
            setStyle(control.$volumeHandle, 'top', `${top}px`);
            setStyle(control.$volumeOn, 'display', 'flex');
            setStyle(control.$volumeOff, 'display', 'none');
          }
        }

        control.$volumePanelText && (control.$volumePanelText.innerHTML = parseInt(percentage * 100));
      }

      player.on(EVENTS.volumechange, () => {
        setVolumeHandle(player.volume);
      });
      player.on(EVENTS.loading, flag => {
        setStyle(control.$loading, 'display', flag ? 'flex' : 'none');
        setStyle(control.$poster, 'display', 'none');

        if (flag) {
          setStyle(control.$playBig, 'display', 'none');
        }
      });

      try {
        const screenfullChange = () => {
          setStyle(control.$fullscreenExit, 'display', player.fullscreen ? 'flex' : 'none');
          setStyle(control.$fullscreen, 'display', player.fullscreen ? 'none' : 'flex'); // control.autoSize();
        };

        screenfull.on('change', screenfullChange);
        player.events.destroys.push(() => {
          screenfull.off('change', screenfullChange);
        });
      } catch (error) {//
      }

      player.on(EVENTS.recording, () => {
        setStyle(control.$record, 'display', player.recording ? 'none' : 'flex');
        setStyle(control.$recordStop, 'display', player.recording ? 'flex' : 'none');
      }); //

      player.on(EVENTS.recordingTimestamp, timestamp => {// console.log(timestamp);
      });
      player.on(EVENTS.playing, flag => {
        setStyle(control.$play, 'display', flag ? 'none' : 'flex');
        setStyle(control.$playBig, 'display', flag ? 'none' : 'block');
        setStyle(control.$pause, 'display', flag ? 'flex' : 'none');
        setStyle(control.$screenshot, 'display', flag ? 'flex' : 'none');
        setStyle(control.$record, 'display', flag ? 'flex' : 'none');
        setStyle(control.$fullscreen, 'display', flag ? 'flex' : 'none'); // 不在播放

        if (!flag) {
          control.$speed && (control.$speed.innerHTML = bpsSize(''));
        }
      });
      player.on(EVENTS.kBps, rate => {
        const bps = bpsSize(rate);
        control.$speed && (control.$speed.innerHTML = bps);
      });
    });

    var property = ((player, control) => {
      Object.defineProperty(control, 'controlsRect', {
        get: () => {
          return control.$controls.getBoundingClientRect();
        }
      });
    });

    var events = ((player, control) => {
      const {
        events: {
          proxy
        }
      } = player;

      function volumeChangeFromEvent(event) {
        const {
          bottom: panelBottom,
          height: panelHeight
        } = control.$volumePanel.getBoundingClientRect();
        const {
          height: handleHeight
        } = control.$volumeHandle.getBoundingClientRect();
        const percentage = clamp(panelBottom - event.y - handleHeight / 2, 0, panelHeight - handleHeight / 2) / (panelHeight - handleHeight);
        return percentage;
      } //


      proxy(window, ['click', 'contextmenu'], event => {
        if (event.composedPath().indexOf(player.$container) > -1) {
          control.isFocus = true;
        } else {
          control.isFocus = false;
        }
      }); //

      proxy(window, 'orientationchange', () => {
        setTimeout(() => {
          player.resize();
        }, 300);
      });
      proxy(control.$controls, 'click', e => {
        e.stopPropagation();
      });
      proxy(control.$pause, 'click', e => {
        player.pause();
      }); // 监听 play 方法

      proxy(control.$play, 'click', e => {
        player.play();
      }); // 监听 play 方法

      proxy(control.$playBig, 'click', e => {
        player.play();
      });
      proxy(control.$volume, 'mouseover', () => {
        control.$volumePanelWrap.classList.add('jessibuca-volume-panel-wrap-show');
      });
      proxy(control.$volume, 'mouseout', () => {
        control.$volumePanelWrap.classList.remove('jessibuca-volume-panel-wrap-show');
      });
      proxy(control.$volumeOn, 'click', e => {
        e.stopPropagation();
        setStyle(control.$volumeOn, 'display', 'none');
        setStyle(control.$volumeOff, 'display', 'block');
        player.lastVolume = player.volume;
        player.volume = 0;
      });
      proxy(control.$volumeOff, 'click', e => {
        e.stopPropagation();
        setStyle(control.$volumeOn, 'display', 'block');
        setStyle(control.$volumeOff, 'display', 'none');
        player.volume = player.lastVolume || 0.5;
      });
      proxy(control.$screenshot, 'click', e => {
        e.stopPropagation();
        player.video.screenshot();
      });
      proxy(control.$volumePanel, 'click', event => {
        event.stopPropagation();
        player.volume = volumeChangeFromEvent(event);
      });
      proxy(control.$volumeHandle, 'mousedown', () => {
        control.isVolumeDroging = true;
      });
      proxy(control.$volumeHandle, 'mousemove', event => {
        if (control.isVolumeDroging) {
          player.volume = volumeChangeFromEvent(event);
        }
      });
      proxy(document, 'mouseup', () => {
        if (control.isVolumeDroging) {
          control.isVolumeDroging = false;
        }
      });
      proxy(control.$record, 'click', e => {
        e.stopPropagation();
        player.recording = true;
      });
      proxy(control.$recordStop, 'click', e => {
        e.stopPropagation();
        player.recording = false;
      });
      proxy(control.$fullscreen, 'click', e => {
        e.stopPropagation();
        player.fullscreen = true;
      });
      proxy(control.$fullscreenExit, 'click', e => {
        e.stopPropagation();
        player.fullscreen = false;
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

    var css_248z$1 = "@keyframes rotation{0%{-webkit-transform:rotate(0deg)}to{-webkit-transform:rotate(1turn)}}.jessibuca-container .jessibuca-icon{cursor:pointer;width:16px;height:16px}.jessibuca-container .jessibuca-poster{position:absolute;z-index:10;left:0;top:0;right:0;bottom:0;height:100%;width:100%;background-position:50%;background-repeat:no-repeat;background-size:contain;pointer-events:none}.jessibuca-container .jessibuca-play-big{position:absolute;display:none;height:100%;width:100%;background:rgba(0,0,0,.4);background-image:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAACgklEQVRoQ+3ZPYsTQRjA8eeZZCFlWttAwCIkZOaZJt8hlvkeHrlccuAFT6wEG0FQOeQQLCIWih6chQgKgkkKIyqKCVYip54IWmiQkTmyYhFvd3Zn3yDb7szu/7cv7GaDkPEFM94PK0DSZ9DzDAyHw7uI2HRDlVJX5/N5r9FoHCYdr/fvCRiNRmpJ6AEidoUQ15NG+AH8BgD2n9AHANAmohdJQfwAfgGA4xF4bjabnW21Whob62ILoKNfAsAGEd2PU2ATcNSNiDf0/cE5/xAHxDpgEf0NADaJ6HLUiKgAbvcjpdSGlPJZVJCoAUfdSqkLxWLxTLlc/mkbEgtgET1TSnWklLdtIuIEuN23crlcp16vv7cBSQKgu38AwBYRXQyLSArg3hsjRDxNRE+CQhIF/BN9qVAobFYqle+mkLQAdLd+8K0T0U0TRJoAbvc9fVkJId75gaQRoLv1C2STiPTb7rFLWgE6+g0RncwyYEJEtawCvjDGmpzzp5kD6NfxfD7frtVqB17xen2a7oG3ALBm+oMoFQBEPD+dTvtBfpImDXjIGFvjnD/3c7ksG5MU4HDxWeZa0HB3XhKAXcdxOn5vUi9gnIDXSqm2lHLPK8pkfVyAbSLqm4T5HRs1YB8RO0KIid8g03FRAT4rpbpSyh3TINPxUQB2GGM9zvkn05gg420CJovLZT9ISNA5tgB9ItoOGhFmnh/AcZ/X9xhj65zzV2Eiwsz1A1j2B8dHAOgS0W6YnduY6wkYj8d3lFKn/j66Ea84jtOrVqtfbQSE3YYnYDAY5Eql0hYAnNDv6kKIx2F3anO+J8DmzqLY1goQxVE12ebqDJgcrSjGrs5AFEfVZJt/AF0m+jHzUTtnAAAAAElFTkSuQmCC\");background-repeat:no-repeat;background-position:50%;cursor:pointer;background-size:48px 48px}.jessibuca-container .jessibuca-play-big:hover{background-image:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAACEElEQVRoQ+2ZXStEQRjH/3/yIXwDdz7J+i7kvdisXCk3SiFJW27kglBcSFFKbqwQSa4krykuKB09Naf2Yndn5jgzc06d53Znd36/mWfeniVyHsw5PwqB0DOonYEoijYBlOpAFwCMkHwLDS/9mwhEDUCfAAyTXA4tYSLwC6CtCegegH6S56FETAR+AHRoACcBTJAUWa+RloBAXwAYIrnt0yBNgZi7qtbHgw8RFwLC/QFglOScawlXAjH3gUqrE1cirgVi7mkAYyS/0xbxJSDcdwAGSa6nKeFTIOZeUyL3aYiEEBDuLwDjJGf+KxFKIOY+BdBL8iipSGiBmHtWbbuftiJZERBuOfgGSK7aSGRJIObeUml1ayKSRQHhlgtkiaTcdltGVgUE+ppkV54FaiS78yrwqlLoOI8Cch2XV548W7WRpTVwA6DP9kGUFYEpAOUkT9LQAvtq1M+0udKkQSgBqSlJWWYxKXj8vRACK+o6bbRIdYI+Ba7U7rKjg7L53JdAhWTZBsy0rWuBXZUuNVMg23auBF7UIl2yBbJt70JAoKV6/WwLk6R9mgKSJlJ1kLTxFmkJyCla8UZd15GJQKvyumyJ8gy8DAEvfZoINPqD41EtUjmUgoaJwAaAnjrKebVI34OSq85NBNqlCAWgE0CV5GEWwI3vQlmCbcSinYFCwPEIFDPgeIC1P1/MgHaIHDf4Aydx2TF7wnKeAAAAAElFTkSuQmCC\")}.jessibuca-container .jessibuca-loading{display:none;flex-direction:column;justify-content:center;align-items:center;position:absolute;z-index:20;left:0;top:0;right:0;bottom:0;width:100%;height:100%;pointer-events:none}.jessibuca-container .jessibuca-loading-text{line-height:20px;font-size:13px;color:#fff;margin-top:10px}.jessibuca-container .jessibuca-controls{background-color:#161616;display:flex;flex-direction:column;justify-content:flex-end;position:absolute;z-index:40;left:0;right:0;bottom:0;height:38px;padding-left:13px;padding-right:13px;font-size:14px;color:#fff;opacity:0;visibility:hidden;transition:all .2s ease-in-out;-webkit-user-select:none;user-select:none}.jessibuca-container .jessibuca-controls .jessibuca-controls-item{position:relative;display:flex;justify-content:center;padding:0 8px}.jessibuca-container .jessibuca-controls .jessibuca-controls-item:hover .icon-title-tips{visibility:visible;opacity:1}.jessibuca-container .jessibuca-controls .jessibuca-fullscreen,.jessibuca-container .jessibuca-controls .jessibuca-fullscreen-exit,.jessibuca-container .jessibuca-controls .jessibuca-icon-audio,.jessibuca-container .jessibuca-controls .jessibuca-microphone-close,.jessibuca-container .jessibuca-controls .jessibuca-pause,.jessibuca-container .jessibuca-controls .jessibuca-play,.jessibuca-container .jessibuca-controls .jessibuca-record,.jessibuca-container .jessibuca-controls .jessibuca-record-stop,.jessibuca-container .jessibuca-controls .jessibuca-screenshot{display:none}.jessibuca-container .jessibuca-controls .jessibuca-icon-audio,.jessibuca-container .jessibuca-controls .jessibuca-icon-mute{z-index:1}.jessibuca-container .jessibuca-controls .jessibuca-controls-bottom{display:flex;justify-content:space-between;height:100%}.jessibuca-container .jessibuca-controls .jessibuca-controls-bottom .jessibuca-controls-left,.jessibuca-container .jessibuca-controls .jessibuca-controls-bottom .jessibuca-controls-right{display:flex;align-items:center}.jessibuca-container.jessibuca-controls-show .jessibuca-controls{opacity:1;visibility:visible}.jessibuca-container.jessibuca-hide-cursor *{cursor:none!important}.jessibuca-container.jessibuca-fullscreen-web{position:fixed;z-index:9999;left:0;top:0;right:0;bottom:0;width:100%!important;height:100%!important;background:#000}.jessibuca-container .jessibuca-icon-loading{width:50px;height:50px;background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%;animation:rotation 1s linear infinite}.jessibuca-container .jessibuca-icon-screenshot{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAE5UlEQVRoQ+1YW2sdVRT+1s7JxbsoVkEUrIIX0ouz15zYNA+N1RdtQfCltlUfvLbqL/BCwZ8grbHtizQqPojgBSr0JkiMmT2nxgapqBURtPVCq7HxJCeZJVPmxDlzZubMmXOSEsnAvOy917fXt9e39tp7E5b4R0vcfywTuNgRbBgBx3HuJqLVzPzmYjprjHkcwAlmLqXNm4XAISLaSESPaq2HF4OE67rbRGRYRA7btn1fbgLGmKsA/Azg0gBkGzO/vZAkHMd5hIiqc5wHcCMz/5k0Z2oExsfHV1QqldPAf8lORNu11m8tBAljzFYAYWxRSl1vWdZvuQj4RsYYF4AVBlgIOVVlE55HRIxt23ZuCfmGjuOsJ6LPoiAistW27XfaEYmIbOYhPc9bXywWR1oiEJDYQkR1zrYjEjGyqfqbKd8a7kJVtLgQ+30i8pht2wfyRKIdmJkJBPkQTbILfudJ7CTZNBvVpggEcgpvc/ML38zESbLJsxBNE/A9biX0rdjGyTQXgbxyapdsarb0PMlXtWnGoXbKpm0Essqp3bJpK4E0OXmed3+hUBDP8w5FI91M0rdcyLLILElOCbaZilSWeXMncRx4klTCY1spfG3dhZJWx3GcDUR0EEB3ZMw0ET2gtT6SZWWzjmlrBIJCl0hAKfWgZVmHszqXZVxbCSxpCS2JJA6umIhe8ZKKVLPbaBJ+S9toqVRa53nedgAbAKwIwH4FcAzAa0R0l4i8F7PPz189k6RFRA+LyNcAXojDV0oNW5b1eW4Cxpg9AHZkSaaa6hhzb065uDSCH2LmRB8Sk9gY4293g43Qo/1pV80m8yQMfZSZ781cB1zXHRKRZ2IMpgD8A+DamL4ZItqitX4/jbQx5iEA7wLoihn3V/ACckWMJN/QWj9b1x5tGBsbW6uUOh5pPy0iL3Z2dn6ilJqanp5ep5TaJSLhF4NppdRNaU8gPmapVLrO87yfIoXuWyJ6uVKp+HmFjo6OQSJ6FcBtYT+UUmstyxqvkWuUgDFmP4AnQu2/e563qlgs+u9DNZ8xZhRAX7VRRPbath0XuXk7Y8xeAE+FgL6fnJzsHRwcLIfBR0ZGLunq6poAsDLUvp+Zw7b1r9PGmJMAbg8Z7WDmoThZuK67WkS+DD18fcPMdzSQUBR/EzN/nIC/SUQ+DPXV4dclsTHmHAD/SfHCNzc3t7Kvr++HJKeMMacA3BL0nyuXyzcPDAxMxo0fHR29slAo/Ajg6qD/fE9Pzw29vb1/x42fmJi4vFwu+5G/LOg/y8zXNJLQ2dAES5JANMQ7mfn1jBI6ycx3NiMhItqstf4oAX+ziHwQ6qvDj5NQNIn/ALCKmX+JSeIvABRD7fuY+ekGBPYBeDI05tTMzExvf3+/vz2Hk91/ET8RSeI6/DoCpVJpjed5fmKGvzMAXpqdnT3oed5Ud3d3v4jsAqBr9Ei0Rmv9VRqBBPzvROQVETnq2xJRdRu9tRF+bCVOKWT+Kvl/TSIFk6SW/LAjKfjV5K8rZABi8dOOEv7FI7Z8x6zwEWbemLbyMfJr5qiSiJ96oclymBOR3bZtP9+M89WxxpjdAHY2sN3DzM8ljWl4I3Nd9x7/OE1ENcdpETnmH3e11n41zv0l4J8RkU+J6AAz+xtF4teQQG7PFslwmcAiLfSyhC72Qv9/I/Avns2OT7QJskoAAAAASUVORK5CYII=\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-screenshot:hover{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAED0lEQVRoQ+2ZycsdRRTFf2ejqHFAMQqiYBTUoElUHLNx3GgCgpuYRF2o0UT9CxwQ/BMkMSbZSKLiQgQHUDCJgjiAxiEiESdEcJbEedgcKaj3UV+/6q7u/jovPPkK3qbr1ql76p5bt6qemPKmKfefeQKHOoLFCNg+H1gi6fFJOmv7VmCvpD1N87Yh8ApwNXCzpB2TIGF7DRDm2inpmt4EbB8LfAMcGUHWSHryYJKwfRMwmuMP4BRJv9TN2RgB2wuB72BWsq+V9MTBIGF7NZBiGzhJ0o+9CIRBtt8FLqgADC6nRDbpVO9Iuqi3hCKB5cDrGZDVkp4aIhIV2aSQyyW9MScCkcQqIOfsnCORkc3I31b5VtyFRmg1IQ7dt0ja3icSQ2C2JhAjUU2ykd+dE7tBNp2i2olAJJFuc+nCt564QTadF6IzgUhiVGiqyinKaQjZpJP2ItBXTkPJZhACXeU0pGwGI9BWTkPLZlACBTldG4o5EA6E1dY66edcyNrs8Q36zg1vVaTazNs7iXPgDVJJzYs7VRvHRzaDEohyugJ4CTi84sg/wHWSdnVxsGQ7aQLXS9pZcqpL/6AEplpCU5HE8YpJ9YrXUKQ6baN1+HPaRm1fBqwFQnKGK2ZoPwCvAo8Ai4FnMpPMHMwapHUj8DFwbw3+Dklv9iZgexOwvktSRduxU2VDlErwmyXV+lCbxLbDdndlCT3TX3vV7JgnKfRuSVflfMkSsL0ZuDMz4E/gL+CETN+/wCpJzzaRtn0D8DRwWMbu1/gCcnSm7zFJd1W/jxGwvQx4r2IYnlbuA14GAomQFw8B6YtBKFSnNj2BxEJ3IvB1pdB9CjwQ8yqYhcg/DJxZ8WOZpA/SbzkC24DbEqOfgPMkBRKzmu23gEuSj1sk5SI3Y2J7C3BHMuZz4FxJf6fgto8APgIWJd+3SUrHjr9O294HnJUMWi8pSGqs2V4CvJ88fH0i6eyChKr4KyS9WIO/Ang+6RvDz0XgABCeFEdtkaQv65yy/QVweuwPY0+T9FuNQ8cAXwHHxf7wdHiypN9r7BfEl8GjYv9+SceXJLQ/mSDYTh2Baog3SHq0pYT2STqno4RWSnqhBn8l8FzSN4bfJol/jkn8bXUS228DFyfft0paVyCwFbg9sQkSDEkctueZZju8iO+tJPEYfo7A0piYKd73wP3xnB+20cvjNnphxdmlkj4sEMjhfwY8COyOY0fb6Bkl/K6FLKxS+M1KpDhJY8mvrG5doRwlf66QZfGbjhLh4pEt35kV3iUp/IvTunU8qtTil/7gaHOY2yjpntaez9b5RmBDYewmSXfX2RRvZLYvbThOh+NuqMa9Ww1+yLnXgO2SwkZR24oEens2oYHzBCa00PMSOtQL/f+NwH+Hg8hAnbrYgQAAAABJRU5ErkJggg==\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-play{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAACgklEQVRoQ+3ZPYsTQRjA8eeZZCFlWttAwCIkZOaZJt8hlvkeHrlccuAFT6wEG0FQOeQQLCIWih6chQgKgkkKIyqKCVYip54IWmiQkTmyYhFvd3Zn3yDb7szu/7cv7GaDkPEFM94PK0DSZ9DzDAyHw7uI2HRDlVJX5/N5r9FoHCYdr/fvCRiNRmpJ6AEidoUQ15NG+AH8BgD2n9AHANAmohdJQfwAfgGA4xF4bjabnW21Whob62ILoKNfAsAGEd2PU2ATcNSNiDf0/cE5/xAHxDpgEf0NADaJ6HLUiKgAbvcjpdSGlPJZVJCoAUfdSqkLxWLxTLlc/mkbEgtgET1TSnWklLdtIuIEuN23crlcp16vv7cBSQKgu38AwBYRXQyLSArg3hsjRDxNRE+CQhIF/BN9qVAobFYqle+mkLQAdLd+8K0T0U0TRJoAbvc9fVkJId75gaQRoLv1C2STiPTb7rFLWgE6+g0RncwyYEJEtawCvjDGmpzzp5kD6NfxfD7frtVqB17xen2a7oG3ALBm+oMoFQBEPD+dTvtBfpImDXjIGFvjnD/3c7ksG5MU4HDxWeZa0HB3XhKAXcdxOn5vUi9gnIDXSqm2lHLPK8pkfVyAbSLqm4T5HRs1YB8RO0KIid8g03FRAT4rpbpSyh3TINPxUQB2GGM9zvkn05gg420CJovLZT9ISNA5tgB9ItoOGhFmnh/AcZ/X9xhj65zzV2Eiwsz1A1j2B8dHAOgS0W6YnduY6wkYj8d3lFKn/j66Ea84jtOrVqtfbQSE3YYnYDAY5Eql0hYAnNDv6kKIx2F3anO+J8DmzqLY1goQxVE12ebqDJgcrSjGrs5AFEfVZJt/AF0m+jHzUTtnAAAAAElFTkSuQmCC\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-play:hover{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAACEElEQVRoQ+2ZXStEQRjH/3/yIXwDdz7J+i7kvdisXCk3SiFJW27kglBcSFFKbqwQSa4krykuKB09Naf2Yndn5jgzc06d53Znd36/mWfeniVyHsw5PwqB0DOonYEoijYBlOpAFwCMkHwLDS/9mwhEDUCfAAyTXA4tYSLwC6CtCegegH6S56FETAR+AHRoACcBTJAUWa+RloBAXwAYIrnt0yBNgZi7qtbHgw8RFwLC/QFglOScawlXAjH3gUqrE1cirgVi7mkAYyS/0xbxJSDcdwAGSa6nKeFTIOZeUyL3aYiEEBDuLwDjJGf+KxFKIOY+BdBL8iipSGiBmHtWbbuftiJZERBuOfgGSK7aSGRJIObeUml1ayKSRQHhlgtkiaTcdltGVgUE+ppkV54FaiS78yrwqlLoOI8Cch2XV548W7WRpTVwA6DP9kGUFYEpAOUkT9LQAvtq1M+0udKkQSgBqSlJWWYxKXj8vRACK+o6bbRIdYI+Ba7U7rKjg7L53JdAhWTZBsy0rWuBXZUuNVMg23auBF7UIl2yBbJt70JAoKV6/WwLk6R9mgKSJlJ1kLTxFmkJyCla8UZd15GJQKvyumyJ8gy8DAEvfZoINPqD41EtUjmUgoaJwAaAnjrKebVI34OSq85NBNqlCAWgE0CV5GEWwI3vQlmCbcSinYFCwPEIFDPgeIC1P1/MgHaIHDf4Aydx2TF7wnKeAAAAAElFTkSuQmCC\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-pause{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAABA0lEQVRoQ+1YwQqCUBAcfWXXsLr2AXWTPXno8yVB8AP6Aa3oHI+kCDqYaawJljSe133uzO44bx0M/HEG/v1gAd9mkAyQgY4I/F8LJUlyrQFtD2AtIkcNoFEU+Z7n7QD4DfFHEVlocrVmgAUAIAOl3mILPcDgEFcUhyrUKMGUUcroc3NQRimj9XJBGaWMvvPydKN0o6/9QTdKN6rZANxj6EbpRulGuZnjYqs8BbyR8Ub2Izeys+u6yyAIDpo/ehzHM2NMDsA0xFsRmWhyfTIDWSXxCEBmrd2EYXjSHJqm6bQoii2AOYBL5Z0xgFxEVppcrQvQJO0zhgX0iXbdWWSADHRE4AZQ731AhEUeNwAAAABJRU5ErkJggg==\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-pause:hover{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAA7klEQVRoQ+2YSwrCQBBEX6HiVvxsPYDewfN7By/gD9ciQkvERQwJdBSiYs0mEDo96aruombEjy/9+P/jAj7NoBkwA28i8H8tFBFRA9oeWEo6ZgCNiDGwAYpn3TpKmmVytWbABQBmoNRbbqEHGB7iiuJYhRol2DJqGX1uDsuoZdRmLuNZSzGWUcuoZdRHSp/IylNgK2ErYSthK3FHwLcSvpXIjoLt9Jfa6TMwl3TIMBkRE2AH9BriL5KGmVyvWIltJXEfKN6tJJ0ym0bECFgDU+Ba+WZQFCdpkcnVuoBM0i5jXECXaNftZQbMwJsI3AAPN3dAQflHegAAAABJRU5ErkJggg==\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-record{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAC+UlEQVRoQ+1ZS2sTURT+zlDJYE3XSq219QHVuEjnJDT+Bff9Abqw2voAEfGxqygUqWhVFHGl/yMLu9BwByxk5SNI66ML6U7axjhHbmhgWiftncxoOiV3FcI53z3f/e65594zhIQPSnj86BBot4IdBToKRFyBnbeFlFIScVEiuYvIWC6Xe2YK8pcC7SYA4CMzH4mDQBXAqilQBDsLQLfPf9FxnF4i8kwwmypARI+Wl5dvmIBEsUmlUkNE9NaHsVCpVAZGR0d/m+A2JSAid3K53E0TkCg2pVKpz7KseR/GfKVSGYxMAMA0M1+JEpyJb6lUOm5ZVnkrAsVisaunp+esiByr1Wp3R0ZGvmifzZK4XQQWHMc52MgBpdQuAOcAXABwuB400ZTjONdaIjA7O5u2bVsnWU1EujzP+5nP5xdMVjvIJkCBD8x8VCm1G8AYgAkAAxt8Z5j5YmgCSqlTAJ4D2OcD/AXgATNfbYVEAIFPIvKKiE4D6GuCea8xX6gtpJT6DmBvECgRFRzHeROWRAABE4iWCbwHEFhkPM/L5vP5dyaz+23+KwHXdR3P854S0YG1ILSCuthNMfNM2OC1/RYENLY+ygcBnPfht6ZAA6BYLNr6dyqVokKhsGpaNQ2TWJstreXaE2aed133sojcj41AKyvdzCdAgSXLsk4MDw9/a/i4rntbRPxFNZoC/5jAV2be759DKTUJ4FZSFFi0bbs/k8noy2R9dAjEuWU2YgXkQOK3kD6BMsysi2Z9JC2Jdcw/ALzwPO+xvmcl7Rj177JVEbkO4BARjSflFDJJuW1dBxJPoCIiL4noDIB1BS0pW6j+oJmbm+uuVqvjRKQfLr0bZHnIzJf0f6HeAybahrUJqAPruhLlcnnPysqKfpXp11n/Gv62zoHAroS+AafT6QkiGrIsazKbzX7eVIHEt1US39gCkOzWYthkjNE+tuZujDGZQ8XRXn8N4KT5lLFZ6uaYPt+nwyDuvC80YdhvB9uOAu1WoaNAR4GIK/AHvdr+QAexB7EAAAAASUVORK5CYII=\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-record:hover{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAACfUlEQVRoQ+2ZSYsUQRCFvycK4nJXXEbHBdwO4kn/gv9CD467ICIutxEFkREdFUU86T/xojcPntyQcT2INw+uISFVkD1Wd2dWlU7nUHlqisiX+fJFZGREi8yHMt8/HYG5VrBToFOg4QnMPxcyM2t4KE2nT0i6EwvylwIjQOCFpE1tEPgGfI0FamC3AFgazP8IrJL0KwZzkAI3gLMxIA1ttgCPA4w3wHpJP2NwBxG4KOlcDEgTGzNbA8wEGP57vA0CU5JONtlczFwz2wY8HUbAzBYCB4CtwCVJb33OIAXmioC70LoyBsxsEXAQOApsLIhelnS6FgEzW+5BBvwA/FS+SPJFa40KBZ5L2mxmS4AJ4IjHxCzwaUnHkgmY2V7gLrAyAPwOXJN0qg6DCgIvgQfAPsDjo2pcKddLciEz+wCs6AO6W9KjVBIVBGIgahN4BvRLMjslPYlZPbT53wR2AbeBtcUmXEFPdh5U06mbd/shBBzbr/Jx4FCAX0+BEsDMFocEYrNmFcE+BD4XsXZL0oyZnQCutkagzkn3m1NBwDe/Q9L74MAuFEqUn5op8I8JvJO0elacTALnc1HAH3Njkvwx+WeYWUegTa/pwaqIgexdyIN4uyRPmqULZRXEvulPwD3gpr+zcrtGQxfzRHYG2AAczuUWiom3kc4D2RN4BdwH9gM9CS0XFyoLGu9UuN974eIFVDiuSzruH5LqgRhtU20q8kBPV8LMlhVVmVdnYwX+SMdAZVeieAF7eeltmElJr4cpkH1bJfvGVvatxdR4bMu+teZuWxtKxWncXn8I7EldtQV7vz79fp9KwZp//9CksB8F206BuVahU6BToOEJ/Ab7+KdABdTt8AAAAABJRU5ErkJggg==\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-recordStop{background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-recordStop:hover{background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-fullscreen{background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-fullscreen:hover{background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-fullscreenExit{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAADd0lEQVRoQ+2Zz2sdVRTHv+fJBDW6anDVXen6wZszYxYBiYgtFGst3VSDunKjpS0GpUlqfjVpsVVs6aaL0or4YxMVFCJZ2ZLdPUP+gq5bQnTxtNAkfTnlhnnlkmQy9yV9780rudt77tzv5/y4v4bQ4Y06XD/2ANodwec/AiJygJnvtdvTWfPnRkBEJAiCN8rl8kMfiPn5+Ve7u7v3rays0Orq6lJfX99/PuN2auMDoAD+BvA2M6/mTWSMOUtE48D6AjHGzN/kjdlNvy+AnWOOmQ/lTSYiEwDOWzsimgrDcCRvzG76GwGw8/zJzO9sN6GInAMwbW1UdSSKoqndCMwb6wNwGsB39Q+p6h/M/C4R2dTa1AoHYBWKyCkA1+pqiWi2Wq0e7e/vf7yRoJAAKcQggMtuJKIoOtoxACnE0/xOi/SXMAxPuhCFjUBdpIjYVWXSEf0TM3/g9BeriDMKdSPEz8z8vrU1xgwT0YXCrEJZy1iSJKOqOub0/8jMA0mSfKKqNwoPkHp7ioiGHIhRIvpHVa93BEBa2JcAfOlALAHo6RgAKzRJkk9V1S6xL7kpV4idOM31taxaIKJHqmpPnMMA9hcOQES2PDJkAT1XAAC+ZebPfWB3auNzmLObVsNRUNUXVHUujuM7OxXnMy4XwOcj29mIyOuq+lapVGrYCelKpkEQ3CyXy4tbzdN0AGPMxr2iYZ+sra3FcRybtgCIiK2BKw2rdgaUSqWoUqlIkQAepFDdAF7cBq5ERI9rtdr1OI7tmE2t6SmUEYFHAEaexYW/1QC2EF+ru5GIvg7D0D2GNJxprQY4o6qv1I/b6SpzOYqiLxpWng5oOQAzXxWRWwA+dkRfYOb1p5hGW6sBJpn5KytSRG4D+KguWFXHoyhy7xdeLC0F2ChSRL4H8OFuINoKYIUbY34gogHH3eeZef1K6tPaDpCm068A3nMEDzHzxY4BUNWSiPxORO6z5aDPPlGICNQ9bYyZIaLjjudzIQoFkKbTbwCO+UI0HcB9J/LdeY0xs0R02IGYYObRrWqiFQCfEZEtSHsfmGZm+4qxbbM/hQD8BeBNa0hEM2EYnmgLgP3lFARBT1dXly4vL//b29tbzQNIU+llAHeJaLFSqRzJes5vegR8xGbZLCwsHKzVav8z8/0sm0ID+MDvAfh4qZk2exFopnd9vv0ELrXBQO7fD10AAAAASUVORK5CYII=\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-fullscreenExit:hover{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAC/ElEQVRoQ+2Zy49NQRCHvx+ReK6IlZ34E7CUiCAR4xEbTLCyQRATYswwb2IQZDYWgojHZpCQECts+ResiQwLj0RClNSkb9Lu3HtPz7mZc8+V6eXt6tP1VVV3VdcVbT7U5vozC9BqD/7/HjCzlZLet9rS9fbP9ICZvQPWSfqRAmFmS4ClMHm+JiR9S1mXVyYFwIBXwEZJv7I2MrPjQH8A6JN0OWtNM/OpAL7HS0mbsjYzswGgN8gNS+rJWtPM/HQAfJ9nkrY22tDMTgMjQaZH0nAzCmatTQE4ClyNPvQU2CbJQ2vKKB2Aa2hmR4DrkbbPgQ5Jv6sJSgkQILqA0dgTkjraBiBAxPHtPz2UtDuGKK0HKkqamd8qg5HS9yXtjebLdYjrHNRqiAeS9gQvnQGGSnML1bvGzOwc0BfN35PUaWYHgRulBwjW9ju+O4JwqM/AWFsABIgLwKkIYgJY1jYAAeJQuGIXVIVcKTKxh8WfBin9J+AVpx/eFWUEqFkyNACKp0rhgWYArkg6kQibSyylmPOklQdibijBX+fSLHFRJkDid+qKmdlaYENOI0zeEcBNSZ9qbVIEQHWuyGOTNZLetgrAz8ClPFpHa1ZL8rf5lFGEB2oBfAxQi4D5DeDmAP7mGJPka0oD4LnDr9imH/xFe8AP4vLIjBclxWXItCOtaIBjwOKo3HaFRyWdnLbmYUHhAJKumdkt4ECk9JCkSitmWixFAwxKOjt5uZvdBvZH2vZLit8XSSBFA/yjpJndAfY1A9FSgOCJu0BnBNErqfIkzfRCywECxCNgR6Rtt6TzmdqHBmyKXG4ZM4sTWc04NzNPWE+AuG3ZlZInSuGBinXMbBzYGVkrE6JUACGcHgPbUyGKAIj7REmZ18y897o5ghiQ5E/bltRChwE/kF7Xj0jyLkbDYWbzgBfA+iA4LmlXqwD8LydvszjAF0lfswBCKC0E3gBeP22p186f8RBKUbaejJmtAr5L+lBPptQAKfCzAClWmkmZWQ/MpHVTvv0X9iFAQGQyevIAAAAASUVORK5CYII=\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-audio{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAACrUlEQVRoQ+2ZPYgTURCAZzbBXJnCeL2Cnb87b9MEtPBUrrMQFAtrtT5/ClGs9LBWWz0RtbBUFCF4oJDsbO68wsLA2YqQSmLlvpEHu7IuMdlLcus+yUKKhJfZ+ebnvZl5CJY/aLn+MAP41x7M1QPMfFtr/crzvHfTAs8FoNPp1LTWzwHgqIg0lFLvrQHwfX8BER8DwC6jNCIecF13wwoA3/dvIuKNpLJa60Oe560XGoCZd4rICiKeTCtaeABmPg4AJmRqg6xcaABmvg4At4aFRyEBhoVM4UMoCplHADCfJTEL5YEsIVNID5iQAYCHALCYxeq5b6PMfF5EBAAEESthGK7W6/XPRpFWq7W3VCqtZg2ZcT3g+/6i4zjzIlLSWn/yPO/DIGMNLCWY2Sj/+xGRK0qpZfNDEASnROTFVi0fr8+aA8z8Ld6KEfGt67oLYwMAwEUium8EREn7OgeAjwCwPyo/nrque3YSgAtE9GDaAM1mc65arc4Zuf1+P2w0Gt9jJZl5DQAORt+fENG5wgEw8zUAMB/zbBBRwyqAIAjuiMjlSOlNItpjFUCqWl0josMzgChR/9hGAWBbknjmAdPhDdqa0gfZzAMJKyVP4v8hhJYRcSni+0JEu63ahZj5anyQici6UuqIVQDdbrfS6/UqRulyufyTiH5sF8AlIro37VpoWEHIzGZ2tM+sEZFnSqkzk9RCS0R01wjIsZz+mug53hDRia0AnI4bGgDYISItz/M2jYC8Gpp2u30MEWuO4zha665Sqp0ZYFStX/iWchRAItFGzoHSsrJ2ZFl1mHg6bfVYJeGJv85CC++BpIJZ5kSFC6G0ha0e7mYJqcJ7IOkRay84UhD2XjHFIFZf8iW9YcYoYRi+tO6aNeupOs66iU/icV46zf/MAKZpzXFk/QL+JG1PUPhRiQAAAABJRU5ErkJggg==\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-audio:hover{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAACSElEQVRoQ+2Zu4sUQRCHf5+C+gf4yBXMfMYHGvjCzEBQDIzV+HwEohipGKupD0QNDE8UEwUFTe68wEDhTMVUMFJ+0tArzbjs9u3Ojt0wBR0M9MzUV1XdXVWNKhcq1189wP/2YKcesH1d0nPgdVvgnQDY3iTpqaT9kuaAt9UA2D4o6aGkzVHpXcByFQC2r0q60lB2D7BUNIDtjZIeSDoyRNGyAWwfiiET4n6YlAtg+7Kka2PCozyAMSHT5CkLIIbMfUlbMhdmOQCZIVOeB2LI3JN0NNPq6bTZe8D2aUmOY72kN8DnoIXt7eF5FSEzkQdsB+OEsFwr6RPwbpixhqYStoPyqVwAbkaAY5KeTWD5wStZHrD9XdJgK34FhBP9H8kFOAvciQBhn3/RAcBHSTvjfx4DJ6cBOAPcbRvA9gZJYQT5DfwYKGl7UdLu+PwIOFUiwCVJYQRZBuZqA7gh6XxUegXYVhtAmq0uAnt7gLhQm9vorBZx74Hcc6D3QLKH/z2JGyVnlYs4pCfzEe4rsLW2XehicpAtAftqAwiZbhhBfgE/ZwVwDrjddi40KiG0HXpHO+KcJ8CJaXKheeBWBOgqnf6W1BwvgcOrATieFDTrJL0HViJAVwXNgVgPrJH0BfiQDTDKtREiNK7KLSnHASQLLacP1PxcVkWWq8PU3emq2yqJJ0b1Qsv2QKpdZp+orBBqmrfq5m5mSJXtgUZI1XnB0YCo94opCal6L/ka3ghtlIXqrllzT9VJ5k19Ek/y0zbf6QHatOYk3/oDujC8QMWgjf4AAAAASUVORK5CYII=\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-mute{background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-mute:hover{background:url(\"data:image/png;base64,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\") no-repeat 50%;background-size:100% 100%}.jessibuca-container .jessibuca-icon-text{font-size:14px;width:30px}.jessibuca-container .jessibuca-speed{font-size:14px;color:#fff}.jessibuca-container .jessibuca-quality-menu-list{position:absolute;left:50%;bottom:100%;visibility:hidden;opacity:0;transform:translateX(-50%);transition:visibility .3s,opacity .3s;background-color:rgba(0,0,0,.5);border-radius:4px}.jessibuca-container .jessibuca-quality-menu-list.jessibuca-quality-menu-shown{visibility:visible;opacity:1}.jessibuca-container .icon-title-tips{pointer-events:none;position:absolute;left:50%;bottom:100%;visibility:hidden;opacity:0;transform:translateX(-50%);transition:visibility .3s ease 0s,opacity .3s ease 0s;background-color:rgba(0,0,0,.5);border-radius:4px}.jessibuca-container .icon-title{display:inline-block;padding:5px 10px;font-size:12px;white-space:nowrap;color:#fff}.jessibuca-container .jessibuca-quality-menu{padding:8px 0}.jessibuca-container .jessibuca-quality-menu-item{display:block;height:25px;margin:0;padding:0 10px;cursor:pointer;font-size:14px;text-align:center;width:50px;color:hsla(0,0%,100%,.5);transition:color .3s,background-color .3s}.jessibuca-container .jessibuca-quality-menu-item:hover{background-color:hsla(0,0%,100%,.2)}.jessibuca-container .jessibuca-quality-menu-item:focus{outline:none}.jessibuca-container .jessibuca-quality-menu-item.jessibuca-quality-menu-item-active{color:#2298fc}.jessibuca-container .jessibuca-volume-panel-wrap{position:absolute;left:50%;bottom:100%;visibility:hidden;opacity:0;transform:translateX(-50%) translateY(22%);transition:visibility .3s,opacity .3s;background-color:rgba(0,0,0,.5);border-radius:4px;height:120px;width:50px;overflow:hidden}.jessibuca-container .jessibuca-volume-panel-wrap.jessibuca-volume-panel-wrap-show{visibility:visible;opacity:1}.jessibuca-container .jessibuca-volume-panel{cursor:pointer;position:absolute;top:21px;height:60px;width:50px;overflow:hidden}.jessibuca-container .jessibuca-volume-panel-text{position:absolute;left:0;top:0;width:50px;height:20px;line-height:20px;text-align:center;color:#fff;font-size:12px}.jessibuca-container .jessibuca-volume-panel-handle{position:absolute;top:48px;left:50%;width:12px;height:12px;border-radius:12px;margin-left:-6px;background:#fff}.jessibuca-container .jessibuca-volume-panel-handle:before{bottom:-54px;background:#fff}.jessibuca-container .jessibuca-volume-panel-handle:after{bottom:6px;background:hsla(0,0%,100%,.2)}.jessibuca-container .jessibuca-volume-panel-handle:after,.jessibuca-container .jessibuca-volume-panel-handle:before{content:\"\";position:absolute;display:block;left:50%;width:3px;margin-left:-1px;height:60px}\n/*# sourceMappingURL=data:application/json;base64,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bEIsTUFBTyxDQUNQLEtBQU0sQ0FDTixVQUFXLENBQ1gsV0FBWSxDQUNaLGdCQUFpQixDQUNqQixpQkFBa0IsQ0FDbEIsVUFBVyxDQUNYLGNBQWlCLENBRW5CLG9EQUNFLGlCQUFrQixDQUNsQixRQUFTLENBQ1QsUUFBUyxDQUNULFVBQVcsQ0FDWCxXQUFZLENBQ1osa0JBQW1CLENBQ25CLGdCQUFpQixDQUNqQixlQUFrQixDQUNsQiwyREFDRSxZQUFhLENBQ2IsZUFBa0IsQ0FDcEIsMERBQ0UsVUFBVyxDQUNYLDZCQUFzQyxDQUN4QyxxSEFDRSxVQUFXLENBQ1gsaUJBQWtCLENBQ2xCLGFBQWMsQ0FDZCxRQUFTLENBQ1QsU0FBVSxDQUNWLGdCQUFpQixDQUNqQixXQUFjIiwiZmlsZSI6InN0eWxlLnNjc3MiLCJzb3VyY2VzQ29udGVudCI6WyJAa2V5ZnJhbWVzIHJvdGF0aW9uIHtcbiAgZnJvbSB7XG4gICAgLXdlYmtpdC10cmFuc2Zvcm06IHJvdGF0ZSgwZGVnKTsgfVxuICB0byB7XG4gICAgLXdlYmtpdC10cmFuc2Zvcm06IHJvdGF0ZSgzNjBkZWcpOyB9IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1pY29uIHtcbiAgY3Vyc29yOiBwb2ludGVyO1xuICB3aWR0aDogMTZweDtcbiAgaGVpZ2h0OiAxNnB4OyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtcG9zdGVyIHtcbiAgcG9zaXRpb246IGFic29sdXRlO1xuICB6LWluZGV4OiAxMDtcbiAgbGVmdDogMDtcbiAgdG9wOiAwO1xuICByaWdodDogMDtcbiAgYm90dG9tOiAwO1xuICBoZWlnaHQ6IDEwMCU7XG4gIHdpZHRoOiAxMDAlO1xuICBiYWNrZ3JvdW5kLXBvc2l0aW9uOiBjZW50ZXIgY2VudGVyO1xuICBiYWNrZ3JvdW5kLXJlcGVhdDogbm8tcmVwZWF0O1xuICBiYWNrZ3JvdW5kLXNpemU6IGNvbnRhaW47XG4gIHBvaW50ZXItZXZlbnRzOiBub25lOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtcGxheS1iaWcge1xuICBwb3NpdGlvbjogYWJzb2x1dGU7XG4gIGRpc3BsYXk6IG5vbmU7XG4gIGhlaWdodDogMTAwJTtcbiAgd2lkdGg6IDEwMCU7XG4gIGJhY2tncm91bmQ6IHJnYmEoMCwgMCwgMCwgMC40KTtcbiAgYmFja2dyb3VuZC1pbWFnZTogdXJsKFwiLi4vYXNzZXRzL3BsYXkucG5nXCIpO1xuICBiYWNrZ3JvdW5kLXJlcGVhdDogbm8tcmVwZWF0O1xuICBiYWNrZ3JvdW5kLXBvc2l0aW9uOiBjZW50ZXI7XG4gIGN1cnNvcjogcG9pbnRlcjtcbiAgYmFja2dyb3VuZC1zaXplOiA0OHB4IDQ4cHg7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1wbGF5LWJpZzpob3ZlciB7XG4gICAgYmFja2dyb3VuZC1pbWFnZTogdXJsKFwiLi4vYXNzZXRzL3BsYXktaG92ZXIucG5nXCIpOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtbG9hZGluZyB7XG4gIGRpc3BsYXk6IG5vbmU7XG4gIGZsZXgtZGlyZWN0aW9uOiBjb2x1bW47XG4gIGp1c3RpZnktY29udGVudDogY2VudGVyO1xuICBhbGlnbi1pdGVtczogY2VudGVyO1xuICBwb3NpdGlvbjogYWJzb2x1dGU7XG4gIHotaW5kZXg6IDIwO1xuICBsZWZ0OiAwO1xuICB0b3A6IDA7XG4gIHJpZ2h0OiAwO1xuICBib3R0b206IDA7XG4gIHdpZHRoOiAxMDAlO1xuICBoZWlnaHQ6IDEwMCU7XG4gIHBvaW50ZXItZXZlbnRzOiBub25lOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtbG9hZGluZy10ZXh0IHtcbiAgbGluZS1oZWlnaHQ6IDIwcHg7XG4gIGZvbnQtc2l6ZTogMTNweDtcbiAgY29sb3I6ICNmZmY7XG4gIG1hcmdpbi10b3A6IDEwcHg7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1jb250cm9scyB7XG4gIGJhY2tncm91bmQtY29sb3I6ICMxNjE2MTY7XG4gIGRpc3BsYXk6IGZsZXg7XG4gIGZsZXgtZGlyZWN0aW9uOiBjb2x1bW47XG4gIGp1c3RpZnktY29udGVudDogZmxleC1lbmQ7XG4gIHBvc2l0aW9uOiBhYnNvbHV0ZTtcbiAgei1pbmRleDogNDA7XG4gIGxlZnQ6IDA7XG4gIHJpZ2h0OiAwO1xuICBib3R0b206IDA7XG4gIGhlaWdodDogMzhweDtcbiAgcGFkZGluZy1sZWZ0OiAxM3B4O1xuICBwYWRkaW5nLXJpZ2h0OiAxM3B4O1xuICBmb250LXNpemU6IDE0cHg7XG4gIGNvbG9yOiAjZmZmO1xuICBvcGFjaXR5OiAwO1xuICB2aXNpYmlsaXR5OiBoaWRkZW47XG4gIHRyYW5zaXRpb246IGFsbCAwLjJzIGVhc2UtaW4tb3V0O1xuICB1c2VyLXNlbGVjdDogbm9uZTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWNvbnRyb2xzIC5qZXNzaWJ1Y2EtY29udHJvbHMtaXRlbSB7XG4gICAgcG9zaXRpb246IHJlbGF0aXZlO1xuICAgIGRpc3BsYXk6IGZsZXg7XG4gICAganVzdGlmeS1jb250ZW50OiBjZW50ZXI7XG4gICAgcGFkZGluZzogMCA4cHg7IH1cbiAgICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWNvbnRyb2xzIC5qZXNzaWJ1Y2EtY29udHJvbHMtaXRlbTpob3ZlciAuaWNvbi10aXRsZS10aXBzIHtcbiAgICAgIHZpc2liaWxpdHk6IHZpc2libGU7XG4gICAgICBvcGFjaXR5OiAxOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtY29udHJvbHMgLmplc3NpYnVjYS1taWNyb3Bob25lLWNsb3NlIHtcbiAgICBkaXNwbGF5OiBub25lOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtY29udHJvbHMgLmplc3NpYnVjYS1pY29uLWF1ZGlvIHtcbiAgICBkaXNwbGF5OiBub25lOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtY29udHJvbHMgLmplc3NpYnVjYS1wbGF5IHtcbiAgICBkaXNwbGF5OiBub25lOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtY29udHJvbHMgLmplc3NpYnVjYS1wYXVzZSB7XG4gICAgZGlzcGxheTogbm9uZTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWNvbnRyb2xzIC5qZXNzaWJ1Y2EtZnVsbHNjcmVlbi1leGl0IHtcbiAgICBkaXNwbGF5OiBub25lOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtY29udHJvbHMgLmplc3NpYnVjYS1zY3JlZW5zaG90IHtcbiAgICBkaXNwbGF5OiBub25lOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtY29udHJvbHMgLmplc3NpYnVjYS1yZWNvcmQge1xuICAgIGRpc3BsYXk6IG5vbmU7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1jb250cm9scyAuamVzc2lidWNhLWZ1bGxzY3JlZW4ge1xuICAgIGRpc3BsYXk6IG5vbmU7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1jb250cm9scyAuamVzc2lidWNhLXJlY29yZC1zdG9wIHtcbiAgICBkaXNwbGF5OiBub25lOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtY29udHJvbHMgLmplc3NpYnVjYS1pY29uLWF1ZGlvLCAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWNvbnRyb2xzIC5qZXNzaWJ1Y2EtaWNvbi1tdXRlIHtcbiAgICB6LWluZGV4OiAxOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtY29udHJvbHMgLmplc3NpYnVjYS1jb250cm9scy1ib3R0b20ge1xuICAgIGRpc3BsYXk6IGZsZXg7XG4gICAganVzdGlmeS1jb250ZW50OiBzcGFjZS1iZXR3ZWVuO1xuICAgIGhlaWdodDogMTAwJTsgfVxuICAgIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtY29udHJvbHMgLmplc3NpYnVjYS1jb250cm9scy1ib3R0b20gLmplc3NpYnVjYS1jb250cm9scy1sZWZ0IHtcbiAgICAgIGRpc3BsYXk6IGZsZXg7XG4gICAgICBhbGlnbi1pdGVtczogY2VudGVyOyB9XG4gICAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1jb250cm9scyAuamVzc2lidWNhLWNvbnRyb2xzLWJvdHRvbSAuamVzc2lidWNhLWNvbnRyb2xzLXJpZ2h0IHtcbiAgICAgIGRpc3BsYXk6IGZsZXg7XG4gICAgICBhbGlnbi1pdGVtczogY2VudGVyOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyLmplc3NpYnVjYS1jb250cm9scy1zaG93IC5qZXNzaWJ1Y2EtY29udHJvbHMge1xuICBvcGFjaXR5OiAxO1xuICB2aXNpYmlsaXR5OiB2aXNpYmxlOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyLmplc3NpYnVjYS1oaWRlLWN1cnNvciAqIHtcbiAgY3Vyc29yOiBub25lICFpbXBvcnRhbnQ7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIuamVzc2lidWNhLWZ1bGxzY3JlZW4td2ViIHtcbiAgcG9zaXRpb246IGZpeGVkO1xuICB6LWluZGV4OiA5OTk5O1xuICBsZWZ0OiAwO1xuICB0b3A6IDA7XG4gIHJpZ2h0OiAwO1xuICBib3R0b206IDA7XG4gIHdpZHRoOiAxMDAlICFpbXBvcnRhbnQ7XG4gIGhlaWdodDogMTAwJSAhaW1wb3J0YW50O1xuICBiYWNrZ3JvdW5kOiAjMDAwOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtaWNvbi1sb2FkaW5nIHtcbiAgd2lkdGg6IDUwcHg7XG4gIGhlaWdodDogNTBweDtcbiAgYmFja2dyb3VuZDogdXJsKFwiLi4vYXNzZXRzL2xvYWRpbmcucG5nXCIpIG5vLXJlcGVhdCBjZW50ZXI7XG4gIGJhY2tncm91bmQtc2l6ZTogMTAwJSAxMDAlO1xuICBhbmltYXRpb246IHJvdGF0aW9uIDFzIGxpbmVhciBpbmZpbml0ZTsgfVxuXG4uamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWljb24tc2NyZWVuc2hvdCB7XG4gIGJhY2tncm91bmQ6IHVybChcIi4uL2Fzc2V0cy9zY3JlZW5zaG90LnBuZ1wiKSBuby1yZXBlYXQgY2VudGVyO1xuICBiYWNrZ3JvdW5kLXNpemU6IDEwMCUgMTAwJTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWljb24tc2NyZWVuc2hvdDpob3ZlciB7XG4gICAgYmFja2dyb3VuZDogdXJsKFwiLi4vYXNzZXRzL3NjcmVlbnNob3QtaG92ZXIucG5nXCIpIG5vLXJlcGVhdCBjZW50ZXI7XG4gICAgYmFja2dyb3VuZC1zaXplOiAxMDAlIDEwMCU7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1pY29uLXBsYXkge1xuICBiYWNrZ3JvdW5kOiB1cmwoXCIuLi9hc3NldHMvcGxheS5wbmdcIikgbm8tcmVwZWF0IGNlbnRlcjtcbiAgYmFja2dyb3VuZC1zaXplOiAxMDAlIDEwMCU7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1pY29uLXBsYXk6aG92ZXIge1xuICAgIGJhY2tncm91bmQ6IHVybChcIi4uL2Fzc2V0cy9wbGF5LWhvdmVyLnBuZ1wiKSBuby1yZXBlYXQgY2VudGVyO1xuICAgIGJhY2tncm91bmQtc2l6ZTogMTAwJSAxMDAlOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtaWNvbi1wYXVzZSB7XG4gIGJhY2tncm91bmQ6IHVybChcIi4uL2Fzc2V0cy9wYXVzZS5wbmdcIikgbm8tcmVwZWF0IGNlbnRlcjtcbiAgYmFja2dyb3VuZC1zaXplOiAxMDAlIDEwMCU7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1pY29uLXBhdXNlOmhvdmVyIHtcbiAgICBiYWNrZ3JvdW5kOiB1cmwoXCIuLi9hc3NldHMvcGF1c2UtaG92ZXIucG5nXCIpIG5vLXJlcGVhdCBjZW50ZXI7XG4gICAgYmFja2dyb3VuZC1zaXplOiAxMDAlIDEwMCU7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1pY29uLXJlY29yZCB7XG4gIGJhY2tncm91bmQ6IHVybChcIi4uL2Fzc2V0cy9yZWNvcmQucG5nXCIpIG5vLXJlcGVhdCBjZW50ZXI7XG4gIGJhY2tncm91bmQtc2l6ZTogMTAwJSAxMDAlOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtaWNvbi1yZWNvcmQ6aG92ZXIge1xuICAgIGJhY2tncm91bmQ6IHVybChcIi4uL2Fzc2V0cy9yZWNvcmQtaG92ZXIucG5nXCIpIG5vLXJlcGVhdCBjZW50ZXI7XG4gICAgYmFja2dyb3VuZC1zaXplOiAxMDAlIDEwMCU7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1pY29uLXJlY29yZFN0b3Age1xuICBiYWNrZ3JvdW5kOiB1cmwoXCIuLi9hc3NldHMvcmVjb3JkLXN0b3AucG5nXCIpIG5vLXJlcGVhdCBjZW50ZXI7XG4gIGJhY2tncm91bmQtc2l6ZTogMTAwJSAxMDAlOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtaWNvbi1yZWNvcmRTdG9wOmhvdmVyIHtcbiAgICBiYWNrZ3JvdW5kOiB1cmwoXCIuLi9hc3NldHMvcmVjb3JkLXN0b3AtaG92ZXIucG5nXCIpIG5vLXJlcGVhdCBjZW50ZXI7XG4gICAgYmFja2dyb3VuZC1zaXplOiAxMDAlIDEwMCU7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1pY29uLWZ1bGxzY3JlZW4ge1xuICBiYWNrZ3JvdW5kOiB1cmwoXCIuLi9hc3NldHMvZnVsbHNjcmVlbi5wbmdcIikgbm8tcmVwZWF0IGNlbnRlcjtcbiAgYmFja2dyb3VuZC1zaXplOiAxMDAlIDEwMCU7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1pY29uLWZ1bGxzY3JlZW46aG92ZXIge1xuICAgIGJhY2tncm91bmQ6IHVybChcIi4uL2Fzc2V0cy9mdWxsc2NyZWVuLWhvdmVyLnBuZ1wiKSBuby1yZXBlYXQgY2VudGVyO1xuICAgIGJhY2tncm91bmQtc2l6ZTogMTAwJSAxMDAlOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtaWNvbi1mdWxsc2NyZWVuRXhpdCB7XG4gIGJhY2tncm91bmQ6IHVybChcIi4uL2Fzc2V0cy9leGl0LWZ1bGxzY3JlZW4ucG5nXCIpIG5vLXJlcGVhdCBjZW50ZXI7XG4gIGJhY2tncm91bmQtc2l6ZTogMTAwJSAxMDAlOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtaWNvbi1mdWxsc2NyZWVuRXhpdDpob3ZlciB7XG4gICAgYmFja2dyb3VuZDogdXJsKFwiLi4vYXNzZXRzL2V4aXQtZnVsbHNjcmVlbi1ob3Zlci5wbmdcIikgbm8tcmVwZWF0IGNlbnRlcjtcbiAgICBiYWNrZ3JvdW5kLXNpemU6IDEwMCUgMTAwJTsgfVxuXG4uamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWljb24tYXVkaW8ge1xuICBiYWNrZ3JvdW5kOiB1cmwoXCIuLi9hc3NldHMvYXVkaW8ucG5nXCIpIG5vLXJlcGVhdCBjZW50ZXI7XG4gIGJhY2tncm91bmQtc2l6ZTogMTAwJSAxMDAlOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtaWNvbi1hdWRpbzpob3ZlciB7XG4gICAgYmFja2dyb3VuZDogdXJsKFwiLi4vYXNzZXRzL2F1ZGlvLWhvdmVyLnBuZ1wiKSBuby1yZXBlYXQgY2VudGVyO1xuICAgIGJhY2tncm91bmQtc2l6ZTogMTAwJSAxMDAlOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtaWNvbi1tdXRlIHtcbiAgYmFja2dyb3VuZDogdXJsKFwiLi4vYXNzZXRzL211dGUucG5nXCIpIG5vLXJlcGVhdCBjZW50ZXI7XG4gIGJhY2tncm91bmQtc2l6ZTogMTAwJSAxMDAlOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtaWNvbi1tdXRlOmhvdmVyIHtcbiAgICBiYWNrZ3JvdW5kOiB1cmwoXCIuLi9hc3NldHMvbXV0ZS1ob3Zlci5wbmdcIikgbm8tcmVwZWF0IGNlbnRlcjtcbiAgICBiYWNrZ3JvdW5kLXNpemU6IDEwMCUgMTAwJTsgfVxuXG4uamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLWljb24tdGV4dCB7XG4gIGZvbnQtc2l6ZTogMTRweDtcbiAgd2lkdGg6IDMwcHg7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1zcGVlZCB7XG4gIGZvbnQtc2l6ZTogMTRweDtcbiAgY29sb3I6ICNmZmY7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS1xdWFsaXR5LW1lbnUtbGlzdCB7XG4gIHBvc2l0aW9uOiBhYnNvbHV0ZTtcbiAgbGVmdDogNTAlO1xuICBib3R0b206IDEwMCU7XG4gIHZpc2liaWxpdHk6IGhpZGRlbjtcbiAgb3BhY2l0eTogMDtcbiAgdHJhbnNmb3JtOiB0cmFuc2xhdGVYKC01MCUpO1xuICB0cmFuc2l0aW9uOiB2aXNpYmlsaXR5IDMwMG1zLCBvcGFjaXR5IDMwMG1zO1xuICBiYWNrZ3JvdW5kLWNvbG9yOiByZ2JhKDAsIDAsIDAsIDAuNSk7XG4gIGJvcmRlci1yYWRpdXM6IDRweDsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXF1YWxpdHktbWVudS1saXN0Lmplc3NpYnVjYS1xdWFsaXR5LW1lbnUtc2hvd24ge1xuICAgIHZpc2liaWxpdHk6IHZpc2libGU7XG4gICAgb3BhY2l0eTogMTsgfVxuXG4uamVzc2lidWNhLWNvbnRhaW5lciAuaWNvbi10aXRsZS10aXBzIHtcbiAgcG9pbnRlci1ldmVudHM6IG5vbmU7XG4gIHBvc2l0aW9uOiBhYnNvbHV0ZTtcbiAgbGVmdDogNTAlO1xuICBib3R0b206IDEwMCU7XG4gIHZpc2liaWxpdHk6IGhpZGRlbjtcbiAgb3BhY2l0eTogMDtcbiAgdHJhbnNmb3JtOiB0cmFuc2xhdGVYKC01MCUpO1xuICB0cmFuc2l0aW9uOiB2aXNpYmlsaXR5IDMwMG1zIGVhc2UgMHMsIG9wYWNpdHkgMzAwbXMgZWFzZSAwcztcbiAgYmFja2dyb3VuZC1jb2xvcjogcmdiYSgwLCAwLCAwLCAwLjUpO1xuICBib3JkZXItcmFkaXVzOiA0cHg7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmljb24tdGl0bGUge1xuICBkaXNwbGF5OiBpbmxpbmUtYmxvY2s7XG4gIHBhZGRpbmc6IDVweCAxMHB4O1xuICBmb250LXNpemU6IDEycHg7XG4gIHdoaXRlLXNwYWNlOiBub3dyYXA7XG4gIGNvbG9yOiB3aGl0ZTsgfVxuXG4uamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXF1YWxpdHktbWVudSB7XG4gIHBhZGRpbmc6IDhweCAwOyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtcXVhbGl0eS1tZW51LWl0ZW0ge1xuICBkaXNwbGF5OiBibG9jaztcbiAgaGVpZ2h0OiAyNXB4O1xuICBtYXJnaW46IDA7XG4gIHBhZGRpbmc6IDAgMTBweDtcbiAgY3Vyc29yOiBwb2ludGVyO1xuICBmb250LXNpemU6IDE0cHg7XG4gIHRleHQtYWxpZ246IGNlbnRlcjtcbiAgd2lkdGg6IDUwcHg7XG4gIGNvbG9yOiByZ2JhKDI1NSwgMjU1LCAyNTUsIDAuNSk7XG4gIHRyYW5zaXRpb246IGNvbG9yIDMwMG1zLCBiYWNrZ3JvdW5kLWNvbG9yIDMwMG1zOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtcXVhbGl0eS1tZW51LWl0ZW06aG92ZXIge1xuICAgIGJhY2tncm91bmQtY29sb3I6IHJnYmEoMjU1LCAyNTUsIDI1NSwgMC4yKTsgfVxuICAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXF1YWxpdHktbWVudS1pdGVtOmZvY3VzIHtcbiAgICBvdXRsaW5lOiBub25lOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2EtcXVhbGl0eS1tZW51LWl0ZW0uamVzc2lidWNhLXF1YWxpdHktbWVudS1pdGVtLWFjdGl2ZSB7XG4gICAgY29sb3I6ICMyMjk4RkM7IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS12b2x1bWUtcGFuZWwtd3JhcCB7XG4gIHBvc2l0aW9uOiBhYnNvbHV0ZTtcbiAgbGVmdDogNTAlO1xuICBib3R0b206IDEwMCU7XG4gIHZpc2liaWxpdHk6IGhpZGRlbjtcbiAgb3BhY2l0eTogMDtcbiAgdHJhbnNmb3JtOiB0cmFuc2xhdGVYKC01MCUpIHRyYW5zbGF0ZVkoMjIlKTtcbiAgdHJhbnNpdGlvbjogdmlzaWJpbGl0eSAzMDBtcywgb3BhY2l0eSAzMDBtcztcbiAgYmFja2dyb3VuZC1jb2xvcjogcmdiYSgwLCAwLCAwLCAwLjUpO1xuICBib3JkZXItcmFkaXVzOiA0cHg7XG4gIGhlaWdodDogMTIwcHg7XG4gIHdpZHRoOiA1MHB4O1xuICBvdmVyZmxvdzogaGlkZGVuOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2Etdm9sdW1lLXBhbmVsLXdyYXAuamVzc2lidWNhLXZvbHVtZS1wYW5lbC13cmFwLXNob3cge1xuICAgIHZpc2liaWxpdHk6IHZpc2libGU7XG4gICAgb3BhY2l0eTogMTsgfVxuXG4uamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXZvbHVtZS1wYW5lbCB7XG4gIGN1cnNvcjogcG9pbnRlcjtcbiAgcG9zaXRpb246IGFic29sdXRlO1xuICB0b3A6IDIxcHg7XG4gIGhlaWdodDogNjBweDtcbiAgd2lkdGg6IDUwcHg7XG4gIG92ZXJmbG93OiBoaWRkZW47IH1cblxuLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS12b2x1bWUtcGFuZWwtdGV4dCB7XG4gIHBvc2l0aW9uOiBhYnNvbHV0ZTtcbiAgbGVmdDogMDtcbiAgdG9wOiAwO1xuICB3aWR0aDogNTBweDtcbiAgaGVpZ2h0OiAyMHB4O1xuICBsaW5lLWhlaWdodDogMjBweDtcbiAgdGV4dC1hbGlnbjogY2VudGVyO1xuICBjb2xvcjogI2ZmZjtcbiAgZm9udC1zaXplOiAxMnB4OyB9XG5cbi5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2Etdm9sdW1lLXBhbmVsLWhhbmRsZSB7XG4gIHBvc2l0aW9uOiBhYnNvbHV0ZTtcbiAgdG9wOiA0OHB4O1xuICBsZWZ0OiA1MCU7XG4gIHdpZHRoOiAxMnB4O1xuICBoZWlnaHQ6IDEycHg7XG4gIGJvcmRlci1yYWRpdXM6IDEycHg7XG4gIG1hcmdpbi1sZWZ0OiAtNnB4O1xuICBiYWNrZ3JvdW5kOiAjZmZmOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2Etdm9sdW1lLXBhbmVsLWhhbmRsZTo6YmVmb3JlIHtcbiAgICBib3R0b206IC01NHB4O1xuICAgIGJhY2tncm91bmQ6ICNmZmY7IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIgLmplc3NpYnVjYS12b2x1bWUtcGFuZWwtaGFuZGxlOjphZnRlciB7XG4gICAgYm90dG9tOiA2cHg7XG4gICAgYmFja2dyb3VuZDogcmdiYSgyNTUsIDI1NSwgMjU1LCAwLjIpOyB9XG4gIC5qZXNzaWJ1Y2EtY29udGFpbmVyIC5qZXNzaWJ1Y2Etdm9sdW1lLXBhbmVsLWhhbmRsZTo6YmVmb3JlLCAuamVzc2lidWNhLWNvbnRhaW5lciAuamVzc2lidWNhLXZvbHVtZS1wYW5lbC1oYW5kbGU6OmFmdGVyIHtcbiAgICBjb250ZW50OiAnJztcbiAgICBwb3NpdGlvbjogYWJzb2x1dGU7XG4gICAgZGlzcGxheTogYmxvY2s7XG4gICAgbGVmdDogNTAlO1xuICAgIHdpZHRoOiAzcHg7XG4gICAgbWFyZ2luLWxlZnQ6IC0xcHg7XG4gICAgaGVpZ2h0OiA2MHB4OyB9XG4iXX0= */";
    styleInject(css_248z$1);

    class Control {
      constructor(player) {
        this.player = player;
        template(player, this);
        observer$1(player, this);
        property(player, this);
        events(player, this);
        this.player.debug.log('Control', 'init');
      }

      autoSize() {
        const player = this.player;
        player.$container.style.padding = '0 0';
        const playerWidth = player.width;
        const playerHeight = player.height;
        const playerRatio = playerWidth / playerHeight;
        const canvasWidth = player.audio.$videoElement.width;
        const canvasHeight = player.audio.$videoElement.height;
        const canvasRatio = canvasWidth / canvasHeight;

        if (playerRatio > canvasRatio) {
          const padding = (playerWidth - playerHeight * canvasRatio) / 2;
          player.$container.style.padding = `0 ${padding}px`;
        } else {
          const padding = (playerHeight - playerWidth / canvasRatio) / 2;
          player.$container.style.padding = `${padding}px 0`;
        }
      }

      destroy() {
        this.player.debug.log('control', 'destroy');

        if (this.$poster) {
          this.player.$container.removeChild(this.$poster);
        }

        this.player.$container.removeChild(this.$loading);

        if (this.$controls) {
          this.player.$container.removeChild(this.$controls);
        }

        this.player = null;
      }

    }

    var css_248z = ".jessibuca-container{position:relative;width:100%;height:100%;overflow:hidden}.jessibuca-container.jessibuca-fullscreen-web{position:fixed;z-index:9999;left:0;top:0;right:0;bottom:0;width:100%!important;height:100%!important;background:#000}\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInN0eWxlLnNjc3MiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUEscUJBQ0UsaUJBQWtCLENBQ2xCLFVBQVcsQ0FDWCxXQUFZLENBQ1osZUFBa0IsQ0FDbEIsOENBQ0UsY0FBZSxDQUNmLFlBQWEsQ0FDYixNQUFPLENBQ1AsS0FBTSxDQUNOLE9BQVEsQ0FDUixRQUFTLENBQ1Qsb0JBQXNCLENBQ3RCLHFCQUF1QixDQUN2QixlQUFrQiIsImZpbGUiOiJzdHlsZS5zY3NzIiwic291cmNlc0NvbnRlbnQiOlsiLmplc3NpYnVjYS1jb250YWluZXIge1xuICBwb3NpdGlvbjogcmVsYXRpdmU7XG4gIHdpZHRoOiAxMDAlO1xuICBoZWlnaHQ6IDEwMCU7XG4gIG92ZXJmbG93OiBoaWRkZW47IH1cbiAgLmplc3NpYnVjYS1jb250YWluZXIuamVzc2lidWNhLWZ1bGxzY3JlZW4td2ViIHtcbiAgICBwb3NpdGlvbjogZml4ZWQ7XG4gICAgei1pbmRleDogOTk5OTtcbiAgICBsZWZ0OiAwO1xuICAgIHRvcDogMDtcbiAgICByaWdodDogMDtcbiAgICBib3R0b206IDA7XG4gICAgd2lkdGg6IDEwMCUgIWltcG9ydGFudDtcbiAgICBoZWlnaHQ6IDEwMCUgIWltcG9ydGFudDtcbiAgICBiYWNrZ3JvdW5kOiAjMDAwOyB9XG4iXX0= */";
    styleInject(css_248z);

    var observer = (player => {
      const {
        _opt,
        debug,
        events: {
          proxy
        }
      } = player;

      if (_opt.supportDblclickFullscreen) {
        proxy(player.$container, 'dblclick', () => {
          player.fullscreen = !player.fullscreen;
        });
      } //


      proxy(document, 'visibilitychange', () => {
        if (_opt.hiddenAutoPause) {
          debug.log('visibilitychange', document.visibilityState, player._isPlayingBeforePageHidden);

          if ("visible" === document.visibilityState) {
            if (player._isPlayingBeforePageHidden) {
              player.play();
            }
          } else {
            player._isPlayingBeforePageHidden = player.playing; // hidden

            if (player.playing) {
              player.pause();
            }
          }
        }
      });
      proxy(window, 'fullscreenchange', () => {
        //
        if (player.keepScreenOn !== null && "visible" === document.visibilityState) {
          player.enableWakeLock();
        }
      });
    });

    class MP4$1 {
      static init() {
        MP4$1.types = {
          avc1: [],
          avcC: [],
          hvc1: [],
          hvcC: [],
          btrt: [],
          dinf: [],
          dref: [],
          esds: [],
          ftyp: [],
          hdlr: [],
          mdat: [],
          mdhd: [],
          mdia: [],
          mfhd: [],
          minf: [],
          moof: [],
          moov: [],
          mp4a: [],
          mvex: [],
          mvhd: [],
          sdtp: [],
          stbl: [],
          stco: [],
          stsc: [],
          stsd: [],
          stsz: [],
          stts: [],
          tfdt: [],
          tfhd: [],
          traf: [],
          trak: [],
          trun: [],
          trex: [],
          tkhd: [],
          vmhd: [],
          smhd: []
        };

        for (let name in MP4$1.types) {
          if (MP4$1.types.hasOwnProperty(name)) {
            MP4$1.types[name] = [name.charCodeAt(0), name.charCodeAt(1), name.charCodeAt(2), name.charCodeAt(3)];
          }
        }

        let constants = MP4$1.constants = {};
        constants.FTYP = new Uint8Array([0x69, 0x73, 0x6F, 0x6D, // major_brand: isom
        0x0, 0x0, 0x0, 0x1, // minor_version: 0x01
        0x69, 0x73, 0x6F, 0x6D, // isom
        0x61, 0x76, 0x63, 0x31 // avc1
        ]);
        constants.STSD_PREFIX = new Uint8Array([0x00, 0x00, 0x00, 0x00, // version(0) + flags
        0x00, 0x00, 0x00, 0x01 // entry_count
        ]);
        constants.STTS = new Uint8Array([0x00, 0x00, 0x00, 0x00, // version(0) + flags
        0x00, 0x00, 0x00, 0x00 // entry_count
        ]);
        constants.STSC = constants.STCO = constants.STTS;
        constants.STSZ = new Uint8Array([0x00, 0x00, 0x00, 0x00, // version(0) + flags
        0x00, 0x00, 0x00, 0x00, // sample_size
        0x00, 0x00, 0x00, 0x00 // sample_count
        ]);
        constants.HDLR_VIDEO = new Uint8Array([0x00, 0x00, 0x00, 0x00, // version(0) + flags
        0x00, 0x00, 0x00, 0x00, // pre_defined
        0x76, 0x69, 0x64, 0x65, // handler_type: 'vide'
        0x00, 0x00, 0x00, 0x00, // reserved: 3 * 4 bytes
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x56, 0x69, 0x64, 0x65, 0x6F, 0x48, 0x61, 0x6E, 0x64, 0x6C, 0x65, 0x72, 0x00 // name: VideoHandler
        ]);
        constants.HDLR_AUDIO = new Uint8Array([0x00, 0x00, 0x00, 0x00, // version(0) + flags
        0x00, 0x00, 0x00, 0x00, // pre_defined
        0x73, 0x6F, 0x75, 0x6E, // handler_type: 'soun'
        0x00, 0x00, 0x00, 0x00, // reserved: 3 * 4 bytes
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x6F, 0x75, 0x6E, 0x64, 0x48, 0x61, 0x6E, 0x64, 0x6C, 0x65, 0x72, 0x00 // name: SoundHandler
        ]);
        constants.DREF = new Uint8Array([0x00, 0x00, 0x00, 0x00, // version(0) + flags
        0x00, 0x00, 0x00, 0x01, // entry_count
        0x00, 0x00, 0x00, 0x0C, // entry_size
        0x75, 0x72, 0x6C, 0x20, // type 'url '
        0x00, 0x00, 0x00, 0x01 // version(0) + flags
        ]); // Sound media header

        constants.SMHD = new Uint8Array([0x00, 0x00, 0x00, 0x00, // version(0) + flags
        0x00, 0x00, 0x00, 0x00 // balance(2) + reserved(2)
        ]); // video media header

        constants.VMHD = new Uint8Array([0x00, 0x00, 0x00, 0x01, // version(0) + flags
        0x00, 0x00, // graphicsmode: 2 bytes
        0x00, 0x00, 0x00, 0x00, // opcolor: 3 * 2 bytes
        0x00, 0x00]);
      } // Generate a box


      static box(type) {
        let size = 8;
        let result = null;
        let datas = Array.prototype.slice.call(arguments, 1);
        let arrayCount = datas.length;

        for (let i = 0; i < arrayCount; i++) {
          size += datas[i].byteLength;
        }

        result = new Uint8Array(size);
        result[0] = size >>> 24 & 0xFF; // size

        result[1] = size >>> 16 & 0xFF;
        result[2] = size >>> 8 & 0xFF;
        result[3] = size & 0xFF;
        result.set(type, 4); // type

        let offset = 8;

        for (let i = 0; i < arrayCount; i++) {
          // data body
          result.set(datas[i], offset);
          offset += datas[i].byteLength;
        }

        return result;
      } // emit ftyp & moov


      static generateInitSegment(meta) {
        let ftyp = MP4$1.box(MP4$1.types.ftyp, MP4$1.constants.FTYP);
        let moov = MP4$1.moov(meta);
        let result = new Uint8Array(ftyp.byteLength + moov.byteLength);
        result.set(ftyp, 0);
        result.set(moov, ftyp.byteLength);
        return result;
      } // Movie metadata box


      static moov(meta) {
        let mvhd = MP4$1.mvhd(meta.timescale, meta.duration);
        let trak = MP4$1.trak(meta);
        let mvex = MP4$1.mvex(meta);
        return MP4$1.box(MP4$1.types.moov, mvhd, trak, mvex);
      } // Movie header box


      static mvhd(timescale, duration) {
        return MP4$1.box(MP4$1.types.mvhd, new Uint8Array([0x00, 0x00, 0x00, 0x00, // version(0) + flags
        0x00, 0x00, 0x00, 0x00, // creation_time
        0x00, 0x00, 0x00, 0x00, // modification_time
        timescale >>> 24 & 0xFF, // timescale: 4 bytes
        timescale >>> 16 & 0xFF, timescale >>> 8 & 0xFF, timescale & 0xFF, duration >>> 24 & 0xFF, // duration: 4 bytes
        duration >>> 16 & 0xFF, duration >>> 8 & 0xFF, duration & 0xFF, 0x00, 0x01, 0x00, 0x00, // Preferred rate: 1.0
        0x01, 0x00, 0x00, 0x00, // PreferredVolume(1.0, 2bytes) + reserved(2bytes)
        0x00, 0x00, 0x00, 0x00, // reserved: 4 + 4 bytes
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, // ----begin composition matrix----
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, // ----end composition matrix----
        0x00, 0x00, 0x00, 0x00, // ----begin pre_defined 6 * 4 bytes----
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ----end pre_defined 6 * 4 bytes----
        0xFF, 0xFF, 0xFF, 0xFF // next_track_ID
        ]));
      } // Track box


      static trak(meta) {
        return MP4$1.box(MP4$1.types.trak, MP4$1.tkhd(meta), MP4$1.mdia(meta));
      } // Track header box


      static tkhd(meta) {
        let trackId = meta.id,
            duration = meta.duration;
        let width = meta.presentWidth,
            height = meta.presentHeight;
        return MP4$1.box(MP4$1.types.tkhd, new Uint8Array([0x00, 0x00, 0x00, 0x07, // version(0) + flags
        0x00, 0x00, 0x00, 0x00, // creation_time
        0x00, 0x00, 0x00, 0x00, // modification_time
        trackId >>> 24 & 0xFF, // track_ID: 4 bytes
        trackId >>> 16 & 0xFF, trackId >>> 8 & 0xFF, trackId & 0xFF, 0x00, 0x00, 0x00, 0x00, // reserved: 4 bytes
        duration >>> 24 & 0xFF, // duration: 4 bytes
        duration >>> 16 & 0xFF, duration >>> 8 & 0xFF, duration & 0xFF, 0x00, 0x00, 0x00, 0x00, // reserved: 2 * 4 bytes
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // layer(2bytes) + alternate_group(2bytes)
        0x00, 0x00, 0x00, 0x00, // volume(2bytes) + reserved(2bytes)
        0x00, 0x01, 0x00, 0x00, // ----begin composition matrix----
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, // ----end composition matrix----
        width >>> 8 & 0xFF, // width and height
        width & 0xFF, 0x00, 0x00, height >>> 8 & 0xFF, height & 0xFF, 0x00, 0x00]));
      }

      static mdia(meta) {
        return MP4$1.box(MP4$1.types.mdia, MP4$1.mdhd(meta), MP4$1.hdlr(meta), MP4$1.minf(meta));
      } // Media header box


      static mdhd(meta) {
        let timescale = meta.timescale;
        let duration = meta.duration;
        return MP4$1.box(MP4$1.types.mdhd, new Uint8Array([0x00, 0x00, 0x00, 0x00, // version(0) + flags
        0x00, 0x00, 0x00, 0x00, // creation_time
        0x00, 0x00, 0x00, 0x00, // modification_time
        timescale >>> 24 & 0xFF, // timescale: 4 bytes
        timescale >>> 16 & 0xFF, timescale >>> 8 & 0xFF, timescale & 0xFF, duration >>> 24 & 0xFF, // duration: 4 bytes
        duration >>> 16 & 0xFF, duration >>> 8 & 0xFF, duration & 0xFF, 0x55, 0xC4, // language: und (undetermined)
        0x00, 0x00 // pre_defined = 0
        ]));
      } // Media handler reference box


      static hdlr(meta) {
        let data = null;

        if (meta.type === 'audio') {
          data = MP4$1.constants.HDLR_AUDIO;
        } else {
          data = MP4$1.constants.HDLR_VIDEO;
        }

        return MP4$1.box(MP4$1.types.hdlr, data);
      } // Media infomation box


      static minf(meta) {
        let xmhd = null;

        if (meta.type === 'audio') {
          xmhd = MP4$1.box(MP4$1.types.smhd, MP4$1.constants.SMHD);
        } else {
          xmhd = MP4$1.box(MP4$1.types.vmhd, MP4$1.constants.VMHD);
        }

        return MP4$1.box(MP4$1.types.minf, xmhd, MP4$1.dinf(), MP4$1.stbl(meta));
      } // Data infomation box


      static dinf() {
        let result = MP4$1.box(MP4$1.types.dinf, MP4$1.box(MP4$1.types.dref, MP4$1.constants.DREF));
        return result;
      } // Sample table box


      static stbl(meta) {
        let result = MP4$1.box(MP4$1.types.stbl, // type: stbl
        MP4$1.stsd(meta), // Sample Description Table
        MP4$1.box(MP4$1.types.stts, MP4$1.constants.STTS), // Time-To-Sample
        MP4$1.box(MP4$1.types.stsc, MP4$1.constants.STSC), // Sample-To-Chunk
        MP4$1.box(MP4$1.types.stsz, MP4$1.constants.STSZ), // Sample size
        MP4$1.box(MP4$1.types.stco, MP4$1.constants.STCO) // Chunk offset
        );
        return result;
      }

      static stsdOld(meta) {
        return meta.type === "audio" ? MP4$1.box(MP4$1.types.stsd, MP4$1.constants.STSD_PREFIX, MP4$1.mp4a(meta)) : meta.videoType === 'avc' ? MP4$1.box(MP4$1.types.stsd, MP4$1.constants.STSD_PREFIX, MP4$1.avc1(meta)) : MP4$1.box(MP4$1.types.stsd, MP4$1.constants.STSD_PREFIX, MP4$1.hvc1(meta));
      } // Sample description box


      static stsd(meta) {
        if (meta.type === 'audio') {
          // else: aac -> mp4a
          return MP4$1.box(MP4$1.types.stsd, MP4$1.constants.STSD_PREFIX, MP4$1.mp4a(meta));
        } else {
          if (meta.videoType === 'avc') {
            //
            return MP4$1.box(MP4$1.types.stsd, MP4$1.constants.STSD_PREFIX, MP4$1.avc1(meta));
          } else {
            //
            return MP4$1.box(MP4$1.types.stsd, MP4$1.constants.STSD_PREFIX, MP4$1.hvc1(meta));
          }
        }
      }

      static mp4a(meta) {
        let channelCount = meta.channelCount;
        let sampleRate = meta.audioSampleRate;
        let data = new Uint8Array([0x00, 0x00, 0x00, 0x00, // reserved(4)
        0x00, 0x00, 0x00, 0x01, // reserved(2) + data_reference_index(2)
        0x00, 0x00, 0x00, 0x00, // reserved: 2 * 4 bytes
        0x00, 0x00, 0x00, 0x00, 0x00, channelCount, // channelCount(2)
        0x00, 0x10, // sampleSize(2)
        0x00, 0x00, 0x00, 0x00, // reserved(4)
        sampleRate >>> 8 & 0xFF, // Audio sample rate
        sampleRate & 0xFF, 0x00, 0x00]);
        return MP4$1.box(MP4$1.types.mp4a, data, MP4$1.esds(meta));
      }

      static esds(meta) {
        let config = meta.config || [];
        let configSize = config.length;
        let data = new Uint8Array([0x00, 0x00, 0x00, 0x00, // version 0 + flags
        0x03, // descriptor_type
        0x17 + configSize, // length3
        0x00, 0x01, // es_id
        0x00, // stream_priority
        0x04, // descriptor_type
        0x0F + configSize, // length
        0x40, // codec: mpeg4_audio
        0x15, // stream_type: Audio
        0x00, 0x00, 0x00, // buffer_size
        0x00, 0x00, 0x00, 0x00, // maxBitrate
        0x00, 0x00, 0x00, 0x00, // avgBitrate
        0x05 // descriptor_type
        ].concat([configSize]).concat(config).concat([0x06, 0x01, 0x02 // GASpecificConfig
        ]));
        return MP4$1.box(MP4$1.types.esds, data);
      }

      static avc1(meta) {
        let avcc = meta.avcc;
        const width = meta.codecWidth;
        const height = meta.codecHeight;
        let data = new Uint8Array([0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, width >>> 8 & 255, width & 255, height >>> 8 & 255, height & 255, 0, 72, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 255, 255]);
        return MP4$1.box(MP4$1.types.avc1, data, MP4$1.box(MP4$1.types.avcC, avcc));
      }

      static hvc1(meta) {
        let avcc = meta.avcc;
        const width = meta.codecWidth;
        const height = meta.codecHeight;
        let data = new Uint8Array([0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, width >>> 8 & 255, width & 255, height >>> 8 & 255, height & 255, 0, 72, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 255, 255]);
        return MP4$1.box(MP4$1.types.hvc1, data, MP4$1.box(MP4$1.types.hvcC, avcc));
      } // Movie Extends box


      static mvex(meta) {
        return MP4$1.box(MP4$1.types.mvex, MP4$1.trex(meta));
      } // Track Extends box


      static trex(meta) {
        let trackId = meta.id;
        let data = new Uint8Array([0x00, 0x00, 0x00, 0x00, // version(0) + flags
        trackId >>> 24 & 0xFF, // track_ID
        trackId >>> 16 & 0xFF, trackId >>> 8 & 0xFF, trackId & 0xFF, 0x00, 0x00, 0x00, 0x01, // default_sample_description_index
        0x00, 0x00, 0x00, 0x00, // default_sample_duration
        0x00, 0x00, 0x00, 0x00, // default_sample_size
        0x00, 0x01, 0x00, 0x01 // default_sample_flags
        ]);
        return MP4$1.box(MP4$1.types.trex, data);
      } // Movie fragment box


      static moof(track, baseMediaDecodeTime) {
        return MP4$1.box(MP4$1.types.moof, MP4$1.mfhd(track.sequenceNumber), MP4$1.traf(track, baseMediaDecodeTime));
      }

      static mfhd(sequenceNumber) {
        let data = new Uint8Array([0x00, 0x00, 0x00, 0x00, sequenceNumber >>> 24 & 0xFF, // sequence_number: int32
        sequenceNumber >>> 16 & 0xFF, sequenceNumber >>> 8 & 0xFF, sequenceNumber & 0xFF]);
        return MP4$1.box(MP4$1.types.mfhd, data);
      } // Track fragment box


      static traf(track, baseMediaDecodeTime) {
        let trackId = track.id; // Track fragment header box

        let tfhd = MP4$1.box(MP4$1.types.tfhd, new Uint8Array([0x00, 0x00, 0x00, 0x00, // version(0) & flags
        trackId >>> 24 & 0xFF, // track_ID
        trackId >>> 16 & 0xFF, trackId >>> 8 & 0xFF, trackId & 0xFF])); // Track Fragment Decode Time

        let tfdt = MP4$1.box(MP4$1.types.tfdt, new Uint8Array([0x00, 0x00, 0x00, 0x00, // version(0) & flags
        baseMediaDecodeTime >>> 24 & 0xFF, // baseMediaDecodeTime: int32
        baseMediaDecodeTime >>> 16 & 0xFF, baseMediaDecodeTime >>> 8 & 0xFF, baseMediaDecodeTime & 0xFF]));
        let sdtp = MP4$1.sdtp(track);
        let trun = MP4$1.trun(track, sdtp.byteLength + 16 + 16 + 8 + 16 + 8 + 8);
        return MP4$1.box(MP4$1.types.traf, tfhd, tfdt, trun, sdtp);
      } // Sample Dependency Type box


      static sdtpOld(A) {
        let e = new Uint8Array(4 + 1),
            t = A.flags;
        return e[4] = t.isLeading << 6 | t.dependsOn << 4 | t.isDependedOn << 2 | t.hasRedundancy, MP4$1.box(MP4$1.types.sdtp, e);
      }

      static sdtp(track) {
        let data = new Uint8Array(4 + 1);
        let flags = track.flags;
        data[4] = flags.isLeading << 6 | flags.dependsOn << 4 | flags.isDependedOn << 2 | flags.hasRedundancy;
        return MP4$1.box(MP4$1.types.sdtp, data);
      }

      static trun(track, offset) {
        let dataSize = 12 + 16;
        let data = new Uint8Array(dataSize);
        offset += 8 + dataSize;
        data.set([0x00, 0x00, 0x0F, 0x01, // version(0) & flags
        0x00, 0x00, 0x00, 0x01, // sample_count
        offset >>> 24 & 0xFF, // data_offset
        offset >>> 16 & 0xFF, offset >>> 8 & 0xFF, offset & 0xFF], 0);
        let duration = track.duration;
        let size = track.size;
        let flags = track.flags;
        let cts = track.cts;
        data.set([duration >>> 24 & 0xFF, // sample_duration
        duration >>> 16 & 0xFF, duration >>> 8 & 0xFF, duration & 0xFF, size >>> 24 & 0xFF, // sample_size
        size >>> 16 & 0xFF, size >>> 8 & 0xFF, size & 0xFF, flags.isLeading << 2 | flags.dependsOn, // sample_flags
        flags.isDependedOn << 6 | flags.hasRedundancy << 4 | flags.isNonSync, 0x00, 0x00, // sample_degradation_priority
        cts >>> 24 & 0xFF, // sample_composition_time_offset
        cts >>> 16 & 0xFF, cts >>> 8 & 0xFF, cts & 0xFF], 12);
        return MP4$1.box(MP4$1.types.trun, data);
      }

      static mdat(data) {
        return MP4$1.box(MP4$1.types.mdat, data);
      }

    }

    MP4$1.init();

    /*
     * Copyright (C) 2016 Bilibili. All Rights Reserved.
     *
     * @author zheng qian <xqq@xqq.im>
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
     * See the License for the specific language governing permissions and
     * limitations under the License.
     */
    // Exponential-Golomb buffer decoder
    class ExpGolomb {
      constructor(uint8array) {
        this.TAG = 'ExpGolomb';
        this._buffer = uint8array;
        this._buffer_index = 0;
        this._total_bytes = uint8array.byteLength;
        this._total_bits = uint8array.byteLength * 8;
        this._current_word = 0;
        this._current_word_bits_left = 0;
      }

      destroy() {
        this._buffer = null;
      }

      _fillCurrentWord() {
        let buffer_bytes_left = this._total_bytes - this._buffer_index;

        let bytes_read = Math.min(4, buffer_bytes_left);
        let word = new Uint8Array(4);
        word.set(this._buffer.subarray(this._buffer_index, this._buffer_index + bytes_read));
        this._current_word = new DataView(word.buffer).getUint32(0, false);
        this._buffer_index += bytes_read;
        this._current_word_bits_left = bytes_read * 8;
      }

      readBits(bits) {

        if (bits <= this._current_word_bits_left) {
          let result = this._current_word >>> 32 - bits;
          this._current_word <<= bits;
          this._current_word_bits_left -= bits;
          return result;
        }

        let result = this._current_word_bits_left ? this._current_word : 0;
        result = result >>> 32 - this._current_word_bits_left;
        let bits_need_left = bits - this._current_word_bits_left;

        this._fillCurrentWord();

        let bits_read_next = Math.min(bits_need_left, this._current_word_bits_left);
        let result2 = this._current_word >>> 32 - bits_read_next;
        this._current_word <<= bits_read_next;
        this._current_word_bits_left -= bits_read_next;
        result = result << bits_read_next | result2;
        return result;
      }

      readBool() {
        return this.readBits(1) === 1;
      }

      readByte() {
        return this.readBits(8);
      }

      _skipLeadingZero() {
        let zero_count;

        for (zero_count = 0; zero_count < this._current_word_bits_left; zero_count++) {
          if (0 !== (this._current_word & 0x80000000 >>> zero_count)) {
            this._current_word <<= zero_count;
            this._current_word_bits_left -= zero_count;
            return zero_count;
          }
        }

        this._fillCurrentWord();

        return zero_count + this._skipLeadingZero();
      }

      readUEG() {
        // unsigned exponential golomb
        let leading_zeros = this._skipLeadingZero();

        return this.readBits(leading_zeros + 1) - 1;
      }

      readSEG() {
        // signed exponential golomb
        let value = this.readUEG();

        if (value & 0x01) {
          return value + 1 >>> 1;
        } else {
          return -1 * (value >>> 1);
        }
      }

    }

    /*
     * Copyright (C) 2016 Bilibili. All Rights Reserved.
     *
     * @author zheng qian <xqq@xqq.im>
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
     * See the License for the specific language governing permissions and
     * limitations under the License.
     */

    class SPSParser$1 {
      static _ebsp2rbsp(uint8array) {
        let src = uint8array;
        let src_length = src.byteLength;
        let dst = new Uint8Array(src_length);
        let dst_idx = 0;

        for (let i = 0; i < src_length; i++) {
          if (i >= 2) {
            // Unescape: Skip 0x03 after 00 00
            if (src[i] === 0x03 && src[i - 1] === 0x00 && src[i - 2] === 0x00) {
              continue;
            }
          }

          dst[dst_idx] = src[i];
          dst_idx++;
        }

        return new Uint8Array(dst.buffer, 0, dst_idx);
      } // 解析 SPS
      // https://zhuanlan.zhihu.com/p/27896239


      static parseSPS(uint8array) {
        let rbsp = SPSParser$1._ebsp2rbsp(uint8array);

        let gb = new ExpGolomb(rbsp);
        gb.readByte(); // 标识当前H.264码流的profile。
        // 我们知道，H.264中定义了三种常用的档次profile： 基准档次：baseline profile;主要档次：main profile; 扩展档次：extended profile;

        let profile_idc = gb.readByte(); // profile_idc

        gb.readByte(); // constraint_set_flags[5] + reserved_zero[3]
        // 标识当前码流的Level。编码的Level定义了某种条件下的最大视频分辨率、最大视频帧率等参数，码流所遵从的level由level_idc指定。

        let level_idc = gb.readByte(); // level_idc
        // 表示当前的序列参数集的id。通过该id值，图像参数集pps可以引用其代表的sps中的参数。

        gb.readUEG(); // seq_parameter_set_id

        let profile_string = SPSParser$1.getProfileString(profile_idc);
        let level_string = SPSParser$1.getLevelString(level_idc);
        let chroma_format_idc = 1;
        let chroma_format = 420;
        let chroma_format_table = [0, 420, 422, 444];
        let bit_depth = 8; //

        if (profile_idc === 100 || profile_idc === 110 || profile_idc === 122 || profile_idc === 244 || profile_idc === 44 || profile_idc === 83 || profile_idc === 86 || profile_idc === 118 || profile_idc === 128 || profile_idc === 138 || profile_idc === 144) {
          //
          chroma_format_idc = gb.readUEG();

          if (chroma_format_idc === 3) {
            gb.readBits(1); // separate_colour_plane_flag
          }

          if (chroma_format_idc <= 3) {
            chroma_format = chroma_format_table[chroma_format_idc];
          }

          bit_depth = gb.readUEG() + 8; // bit_depth_luma_minus8

          gb.readUEG(); // bit_depth_chroma_minus8

          gb.readBits(1); // qpprime_y_zero_transform_bypass_flag

          if (gb.readBool()) {
            // seq_scaling_matrix_present_flag
            let scaling_list_count = chroma_format_idc !== 3 ? 8 : 12;

            for (let i = 0; i < scaling_list_count; i++) {
              if (gb.readBool()) {
                // seq_scaling_list_present_flag
                if (i < 6) {
                  SPSParser$1._skipScalingList(gb, 16);
                } else {
                  SPSParser$1._skipScalingList(gb, 64);
                }
              }
            }
          }
        } // 用于计算MaxFrameNum的值。计算公式为MaxFrameNum = 2^(log2_max_frame_num_minus4 +


        gb.readUEG(); // log2_max_frame_num_minus4
        // 表示解码picture order count(POC)的方法。POC是另一种计量图像序号的方式，与frame_num有着不同的计算方法。该语法元素的取值为0、1或2。

        let pic_order_cnt_type = gb.readUEG();

        if (pic_order_cnt_type === 0) {
          gb.readUEG(); // log2_max_pic_order_cnt_lsb_minus_4
        } else if (pic_order_cnt_type === 1) {
          gb.readBits(1); // delta_pic_order_always_zero_flag

          gb.readSEG(); // offset_for_non_ref_pic

          gb.readSEG(); // offset_for_top_to_bottom_field

          let num_ref_frames_in_pic_order_cnt_cycle = gb.readUEG();

          for (let i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) {
            gb.readSEG(); // offset_for_ref_frame
          }
        } // 用于表示参考帧的最大数目。


        let ref_frames = gb.readUEG(); // max_num_ref_frames
        // 标识位，说明frame_num中是否允许不连续的值。

        gb.readBits(1); // gaps_in_frame_num_value_allowed_flag
        // 用于计算图像的宽度。单位为宏块个数，因此图像的实际宽度为:

        let pic_width_in_mbs_minus1 = gb.readUEG(); // 使用PicHeightInMapUnits来度量视频中一帧图像的高度。
        // PicHeightInMapUnits并非图像明确的以像素或宏块为单位的高度，而需要考虑该宏块是帧编码或场编码。PicHeightInMapUnits的计算方式为：

        let pic_height_in_map_units_minus1 = gb.readUEG(); // 标识位，说明宏块的编码方式。当该标识位为0时，宏块可能为帧编码或场编码；
        // 该标识位为1时，所有宏块都采用帧编码。根据该标识位取值不同，PicHeightInMapUnits的含义也不同，
        // 为0时表示一场数据按宏块计算的高度，为1时表示一帧数据按宏块计算的高度。

        let frame_mbs_only_flag = gb.readBits(1);

        if (frame_mbs_only_flag === 0) {
          // 标识位，说明是否采用了宏块级的帧场自适应编码。当该标识位为0时，不存在帧编码和场编码之间的切换；当标识位为1时，宏块可能在帧编码和场编码模式之间进行选择。
          gb.readBits(1); // mb_adaptive_frame_field_flag
        } // 标识位，用于B_Skip、B_Direct模式运动矢量的推导计算。


        gb.readBits(1); // direct_8x8_inference_flag

        let frame_crop_left_offset = 0;
        let frame_crop_right_offset = 0;
        let frame_crop_top_offset = 0;
        let frame_crop_bottom_offset = 0;
        let frame_cropping_flag = gb.readBool();

        if (frame_cropping_flag) {
          frame_crop_left_offset = gb.readUEG();
          frame_crop_right_offset = gb.readUEG();
          frame_crop_top_offset = gb.readUEG();
          frame_crop_bottom_offset = gb.readUEG();
        }

        let sar_width = 1,
            sar_height = 1;
        let fps = 0,
            fps_fixed = true,
            fps_num = 0,
            fps_den = 0; // 标识位，说明SPS中是否存在VUI信息。

        let vui_parameters_present_flag = gb.readBool();

        if (vui_parameters_present_flag) {
          if (gb.readBool()) {
            // aspect_ratio_info_present_flag
            let aspect_ratio_idc = gb.readByte();
            let sar_w_table = [1, 12, 10, 16, 40, 24, 20, 32, 80, 18, 15, 64, 160, 4, 3, 2];
            let sar_h_table = [1, 11, 11, 11, 33, 11, 11, 11, 33, 11, 11, 33, 99, 3, 2, 1];

            if (aspect_ratio_idc > 0 && aspect_ratio_idc < 16) {
              sar_width = sar_w_table[aspect_ratio_idc - 1];
              sar_height = sar_h_table[aspect_ratio_idc - 1];
            } else if (aspect_ratio_idc === 255) {
              sar_width = gb.readByte() << 8 | gb.readByte();
              sar_height = gb.readByte() << 8 | gb.readByte();
            }
          }

          if (gb.readBool()) {
            // overscan_info_present_flag
            gb.readBool(); // overscan_appropriate_flag
          }

          if (gb.readBool()) {
            // video_signal_type_present_flag
            gb.readBits(4); // video_format & video_full_range_flag

            if (gb.readBool()) {
              // colour_description_present_flag
              gb.readBits(24); // colour_primaries & transfer_characteristics & matrix_coefficients
            }
          }

          if (gb.readBool()) {
            // chroma_loc_info_present_flag
            gb.readUEG(); // chroma_sample_loc_type_top_field

            gb.readUEG(); // chroma_sample_loc_type_bottom_field
          }

          if (gb.readBool()) {
            // timing_info_present_flag
            let num_units_in_tick = gb.readBits(32);
            let time_scale = gb.readBits(32);
            fps_fixed = gb.readBool(); // fixed_frame_rate_flag

            fps_num = time_scale;
            fps_den = num_units_in_tick * 2;
            fps = fps_num / fps_den;
          }
        }

        let sarScale = 1;

        if (sar_width !== 1 || sar_height !== 1) {
          sarScale = sar_width / sar_height;
        }

        let crop_unit_x = 0,
            crop_unit_y = 0;

        if (chroma_format_idc === 0) {
          crop_unit_x = 1;
          crop_unit_y = 2 - frame_mbs_only_flag;
        } else {
          let sub_wc = chroma_format_idc === 3 ? 1 : 2;
          let sub_hc = chroma_format_idc === 1 ? 2 : 1;
          crop_unit_x = sub_wc;
          crop_unit_y = sub_hc * (2 - frame_mbs_only_flag);
        }

        let codec_width = (pic_width_in_mbs_minus1 + 1) * 16;
        let codec_height = (2 - frame_mbs_only_flag) * ((pic_height_in_map_units_minus1 + 1) * 16);
        codec_width -= (frame_crop_left_offset + frame_crop_right_offset) * crop_unit_x;
        codec_height -= (frame_crop_top_offset + frame_crop_bottom_offset) * crop_unit_y;
        let present_width = Math.ceil(codec_width * sarScale);
        gb.destroy();
        gb = null; // 解析出来的SPS 内容。

        return {
          profile_string: profile_string,
          // baseline, high, high10, ...
          level_string: level_string,
          // 3, 3.1, 4, 4.1, 5, 5.1, ...
          bit_depth: bit_depth,
          // 8bit, 10bit, ...
          ref_frames: ref_frames,
          chroma_format: chroma_format,
          // 4:2:0, 4:2:2, ...
          chroma_format_string: SPSParser$1.getChromaFormatString(chroma_format),
          frame_rate: {
            fixed: fps_fixed,
            fps: fps,
            fps_den: fps_den,
            fps_num: fps_num
          },
          sar_ratio: {
            width: sar_width,
            height: sar_height
          },
          codec_size: {
            width: codec_width,
            height: codec_height
          },
          present_size: {
            width: present_width,
            height: codec_height
          }
        };
      }

      static _skipScalingList(gb, count) {
        let last_scale = 8,
            next_scale = 8;
        let delta_scale = 0;

        for (let i = 0; i < count; i++) {
          if (next_scale !== 0) {
            delta_scale = gb.readSEG();
            next_scale = (last_scale + delta_scale + 256) % 256;
          }

          last_scale = next_scale === 0 ? last_scale : next_scale;
        }
      } // profile_idc = 66 → baseline profile;
      // profile_idc = 77 → main profile;
      // profile_idc = 88 → extended profile;
      // 在新版的标准中，还包括了High、High 10、High 4:2:2、High 4:4:4、High 10 Intra、High
      // 4:2:2 Intra、High 4:4:4 Intra、CAVLC 4:4:4 Intra


      static getProfileString(profile_idc) {
        switch (profile_idc) {
          case 66:
            return 'Baseline';

          case 77:
            return 'Main';

          case 88:
            return 'Extended';

          case 100:
            return 'High';

          case 110:
            return 'High10';

          case 122:
            return 'High422';

          case 244:
            return 'High444';

          default:
            return 'Unknown';
        }
      }

      static getLevelString(level_idc) {
        return (level_idc / 10).toFixed(1);
      }

      static getChromaFormatString(chroma) {
        switch (chroma) {
          case 420:
            return '4:2:0';

          case 422:
            return '4:2:2';

          case 444:
            return '4:4:4';

          default:
            return 'Unknown';
        }
      }

    }

    function parseAVCDecoderConfigurationRecord(arrayBuffer) {
      const meta = {};
      const v = new DataView(arrayBuffer.buffer);
      let version = v.getUint8(0); // configurationVersion

      let avcProfile = v.getUint8(1); // avcProfileIndication

      v.getUint8(2); // profile_compatibil

      v.getUint8(3); // AVCLevelIndication

      if (version !== 1 || avcProfile === 0) {
        // this._onError(DemuxErrors.FORMAT_ERROR, 'Flv: Invalid AVCDecoderConfigurationRecord');
        return;
      }

      const _naluLengthSize = (v.getUint8(4) & 3) + 1; // lengthSizeMinusOne


      if (_naluLengthSize !== 3 && _naluLengthSize !== 4) {
        // holy shit!!!
        // this._onError(DemuxErrors.FORMAT_ERROR, `Flv: Strange NaluLengthSizeMinusOne: ${_naluLengthSize - 1}`);
        return;
      }

      let spsCount = v.getUint8(5) & 31; // numOfSequenceParameterSets

      if (spsCount === 0) {
        // this._onError(DemuxErrors.FORMAT_ERROR, 'Flv: Invalid AVCDecoderConfigurationRecord: No SPS');
        return;
      }

      let offset = 6;

      for (let i = 0; i < spsCount; i++) {
        let len = v.getUint16(offset, false); // sequenceParameterSetLength

        offset += 2;

        if (len === 0) {
          continue;
        } // Notice: Nalu without startcode header (00 00 00 01)


        let sps = new Uint8Array(arrayBuffer.buffer, offset, len);
        offset += len; // flv.js作者选择了自己来解析这个数据结构，也是迫不得已，因为JS环境下没有ffmpeg，解析这个结构主要是为了提取 sps和pps。虽然理论上sps允许有多个，但其实一般就一个。
        // packetTtype 为 1 表示 NALU，NALU= network abstract layer unit，这是H.264的概念，网络抽象层数据单元，其实简单理解就是一帧视频数据。
        // pps的信息没什么用，所以作者只实现了sps的分析器，说明作者下了很大功夫去学习264的标准，其中的Golomb解码还是挺复杂的，能解对不容易，我在PC和手机平台都是用ffmpeg去解析的。
        // SPS里面包括了视频分辨率，帧率，profile level等视频重要信息。

        let config = SPSParser$1.parseSPS(sps);

        if (i !== 0) {
          // ignore other sps's config
          continue;
        }

        meta.codecWidth = config.codec_size.width;
        meta.codecHeight = config.codec_size.height;
        meta.presentWidth = config.present_size.width;
        meta.presentHeight = config.present_size.height;
        meta.profile = config.profile_string;
        meta.level = config.level_string;
        meta.bitDepth = config.bit_depth;
        meta.chromaFormat = config.chroma_format;
        meta.sarRatio = config.sar_ratio;
        meta.frameRate = config.frame_rate;

        if (config.frame_rate.fixed === false || config.frame_rate.fps_num === 0 || config.frame_rate.fps_den === 0) {
          meta.frameRate = {};
        }

        let fps_den = meta.frameRate.fps_den;
        let fps_num = meta.frameRate.fps_num;
        meta.refSampleDuration = meta.timescale * (fps_den / fps_num);
        let codecArray = sps.subarray(1, 4);
        let codecString = 'avc1.';

        for (let j = 0; j < 3; j++) {
          let h = codecArray[j].toString(16);

          if (h.length < 2) {
            h = '0' + h;
          }

          codecString += h;
        } // codec


        meta.codec = codecString;
      }

      let ppsCount = v.getUint8(offset); // numOfPictureParameterSets

      if (ppsCount === 0) {
        // this._onError(DemuxErrors.FORMAT_ERROR, 'Flv: Invalid AVCDecoderConfigurationRecord: No PPS');
        return;
      }

      offset++;

      for (let i = 0; i < ppsCount; i++) {
        let len = v.getUint16(offset, false); // pictureParameterSetLength

        offset += 2;

        if (len === 0) {
          continue;
        }

        new Uint8Array(arrayBuffer.buffer, offset, len); // pps is useless for extracting video information

        offset += len;
      }

      meta.videoType = 'avc'; // meta.avcc = arrayBuffer;

      return meta;
    }

    class SPSParser {
      static parseSPS(uint8array) {}

    }

    function parseHEVCDecoderConfigurationRecord(arrayBuffer) {
      const meta = {};
      meta.videoType = 'hevc';
      let offset = 28 - 5; //

      const vpsTag = arrayBuffer[offset];

      if (vpsTag !== H265_NAL_TYPE.vps) {
        return meta;
      }

      offset += 2;
      offset += 1;
      const vpsLength = arrayBuffer[offset + 1] | arrayBuffer[offset] << 8;
      offset += 2;
      const vpsData = arrayBuffer.slice(offset, offset + vpsLength);
      console.log(Uint8Array.from(vpsData));
      offset += vpsLength;
      const spsTag = arrayBuffer[offset];

      if (spsTag !== H265_NAL_TYPE.sps) {
        return meta;
      }

      offset += 2;
      offset += 1;
      const spsLength = arrayBuffer[offset + 1] | arrayBuffer[offset] << 8;
      offset += 2;
      const spsData = arrayBuffer.slice(offset, offset + spsLength);
      console.log(Uint8Array.from(spsData));
      offset += spsLength;
      const ppsTag = arrayBuffer[offset];

      if (ppsTag !== H265_NAL_TYPE.pps) {
        return meta;
      }

      offset += 2;
      offset += 1;
      const ppsLength = arrayBuffer[offset + 1] | arrayBuffer[offset] << 8;
      offset += 2;
      const ppsData = arrayBuffer.slice(offset, offset + ppsLength);
      console.log(Uint8Array.from(ppsData));
      let sps = Uint8Array.from(spsData);
      let config = SPSParser.parseSPS(sps);
      meta.codecWidth = config.codec_size.width;
      meta.codecHeight = config.codec_size.height;
      meta.presentWidth = config.present_size.width;
      meta.presentHeight = config.present_size.height;
      meta.profile = config.profile_string;
      meta.level = config.level_string;
      meta.bitDepth = config.bit_depth;
      meta.chromaFormat = config.chroma_format;
      meta.sarRatio = config.sar_ratio;
      return meta;
    }

    class MseDecoder extends Emitter {
      constructor(player) {
        super();
        this.player = player;
        this.isAvc = true;
        this.mediaSource = new window.MediaSource();
        this.sourceBuffer = null;
        this.hasInit = false;
        this.isInitInfo = false;
        this.cacheTrack = {};
        this.timeInit = false;
        this.sequenceNumber = 0;
        this.mediaSourceOpen = false;
        this.bufferList = [];
        this.dropping = false;
        this.player.video.$videoElement.src = window.URL.createObjectURL(this.mediaSource);
        const {
          debug,
          events: {
            proxy
          }
        } = player;
        proxy(this.mediaSource, 'sourceopen', () => {
          this.mediaSourceOpen = true;
          this.player.emit(EVENTS.mseSourceOpen);
        });
        proxy(this.mediaSource, 'sourceclose', () => {
          this.player.emit(EVENTS.mseSourceClose);
        });
        player.debug.log('MediaSource', 'init');
      }

      get state() {
        return this.mediaSource.readyState;
      }

      get isStateOpen() {
        return this.state === MEDIA_SOURCE_STATE.open;
      }

      get isStateClosed() {
        return this.state === MEDIA_SOURCE_STATE.closed;
      }

      get isStateEnded() {
        return this.state === MEDIA_SOURCE_STATE.ended;
      }

      get duration() {
        return this.mediaSource.duration;
      }

      set duration(duration) {
        this.mediaSource.duration = duration;
      }

      decodeVideo(payload, ts, isIframe) {
        const player = this.player;

        if (!this.hasInit) {
          if (isIframe && payload[1] === 0) {
            const videoCodec = payload[0] & 0x0F;
            player.video.updateVideoInfo({
              encTypeCode: videoCodec
            }); // 如果解码出来的是

            if (videoCodec === VIDEO_ENC_CODE.h265) {
              this.emit(EVENTS_ERROR.mediaSourceH265NotSupport);
              return;
            }

            this._decodeConfigurationRecord(payload, ts, isIframe, videoCodec);

            this.hasInit = true;
          }
        } else {
          this._decodeVideo(payload, ts, isIframe);
        }
      }

      _doDecode() {
        const bufferItem = this.bufferList.shift();

        if (bufferItem) {
          this._decodeVideo(bufferItem.payload, bufferItem.ts, bufferItem.isIframe);
        }
      }

      _decodeConfigurationRecord(payload, ts, isIframe, videoCodec) {
        let data = payload.slice(5);
        let config = {};

        if (videoCodec === VIDEO_ENC_CODE.h264) {
          config = parseAVCDecoderConfigurationRecord(data);
        } else if (videoCodec === VIDEO_ENC_CODE.h265) {
          config = parseHEVCDecoderConfigurationRecord(data);
        }

        const metaData = {
          id: 1,
          // video tag data
          type: 'video',
          timescale: 1000,
          duration: 0,
          avcc: data,
          codecWidth: config.codecWidth,
          codecHeight: config.codecHeight,
          videoType: config.videoType
        }; // ftyp

        const metaBox = MP4$1.generateInitSegment(metaData);
        this.isAvc = true;
        this.appendBuffer(metaBox.buffer);
        this.sequenceNumber = 0;
        this.cacheTrack = null;
        this.timeInit = false;
      } //


      _decodeVideo(payload, ts, isIframe) {
        const player = this.player;
        let arrayBuffer = payload.slice(5);
        let bytes = arrayBuffer.byteLength;
        let cts = 0;
        let dts = ts;
        const $video = player.video.$videoElement;

        if ($video.buffered.length > 1) {
          this.removeBuffer($video.buffered.start(0), $video.buffered.end(0));
          this.timeInit = false;
        }

        if ($video.drop && dts - this.cacheTrack.dts > 1000) {
          $video.drop = false;
          this.cacheTrack = {};
        } else if (this.cacheTrack && dts > this.cacheTrack.dts) {
          // 需要额外加8个size
          let mdatBytes = 8 + this.cacheTrack.size;
          let mdatbox = new Uint8Array(mdatBytes);
          mdatbox[0] = mdatBytes >>> 24 & 255;
          mdatbox[1] = mdatBytes >>> 16 & 255;
          mdatbox[2] = mdatBytes >>> 8 & 255;
          mdatbox[3] = mdatBytes & 255;
          mdatbox.set(MP4$1.types.mdat, 4);
          mdatbox.set(this.cacheTrack.data, 8);
          this.cacheTrack.duration = dts - this.cacheTrack.dts; // moof

          let moofbox = MP4$1.moof(this.cacheTrack, this.cacheTrack.dts);
          let result = new Uint8Array(moofbox.byteLength + mdatbox.byteLength);
          result.set(moofbox, 0);
          result.set(mdatbox, moofbox.byteLength); // appendBuffer

          this.appendBuffer(result.buffer);
          player.handleRender();
          player.updateStats({
            fps: true,
            ts: ts,
            buf: player.demux.delay
          });
        } else {
          player.debug.log('MediaSource', 'timeInit set false , cacheTrack = {}');
          this.timeInit = false;
          this.cacheTrack = {};
        }

        this.cacheTrack.id = 1;
        this.cacheTrack.sequenceNumber = ++this.sequenceNumber;
        this.cacheTrack.size = bytes;
        this.cacheTrack.dts = dts;
        this.cacheTrack.cts = cts;
        this.cacheTrack.isKeyframe = isIframe;
        this.cacheTrack.data = arrayBuffer; //

        this.cacheTrack.flags = {
          isLeading: 0,
          dependsOn: isIframe ? 2 : 1,
          isDependedOn: isIframe ? 1 : 0,
          hasRedundancy: 0,
          isNonSync: isIframe ? 0 : 1
        }; //

        if (!this.timeInit && $video.buffered.length === 1) {
          player.debug.log('MediaSource', 'timeInit set true');
          this.timeInit = true;
          $video.currentTime = $video.buffered.end(0);
        }

        if (!this.isInitInfo && $video.videoWidth > 0 && $video.videoHeight > 0) {
          player.debug.log('MediaSource', `updateVideoInfo: ${$video.videoWidth},${$video.videoHeight}`);
          player.video.updateVideoInfo({
            width: $video.videoWidth,
            height: $video.videoHeight
          });
          player.video.initCanvasViewSize();
          this.isInitInfo = true;
        }
      }

      appendBuffer(buffer) {
        const {
          debug,
          events: {
            proxy
          }
        } = this.player;

        if (this.sourceBuffer === null) {
          this.sourceBuffer = this.mediaSource.addSourceBuffer(MP4_CODECS.avc);
          proxy(this.sourceBuffer, 'error', error => {
            this.player.emit(EVENTS.mseSourceBufferError, error);
          });
        }

        if (this.sourceBuffer.updating === false && this.isStateOpen) {
          this.sourceBuffer.appendBuffer(buffer);
          return;
        }

        if (this.isStateClosed) {
          this.player.emit(EVENTS.mseSourceBufferError, 'mediaSource is not attached to video or mediaSource is closed');
        } else if (this.isStateEnded) {
          this.player.emit(EVENTS.mseSourceBufferError, 'mediaSource is closed');
        } else {
          if (this.sourceBuffer.updating === true) {
            this.player.emit(EVENTS.mseSourceBufferBusy);
            this.dropSourceBuffer(true);
          }
        }
      }

      stop() {
        if (this.isStateOpen) {
          if (this.sourceBuffer) {
            this.sourceBuffer.abort();
          }
        }

        this.endOfStream();
      }

      dropSourceBuffer(flag) {
        const video = this.player.video;
        const $video = video.$videoElement;
        this.dropping = flag;

        if ($video.buffered.length > 0) {
          if ($video.buffered.end(0) - $video.currentTime > 1) {
            $video.currentTime = $video.buffered.end(0);
          }
        }
      }

      removeBuffer(start, end) {
        if (this.isStateOpen && this.sourceBuffer.updating === false) {
          try {
            this.sourceBuffer.remove(start, end);
          } catch (e) {
            console.error(e);
          }
        }
      }

      endOfStream() {
        if (this.isStateOpen) {
          this.mediaSource.endOfStream();
        }
      }

      destroy() {
        this.stop();
        this.bufferList = [];
        this.mediaSource = null;
        this.mediaSourceOpen = false;
        this.sourceBuffer = null;
        this.hasInit = false;
        this.isInitInfo = false;
        this.sequenceNumber = 0;
        this.cacheTrack = null;
        this.timeInit = false;
        this.off();
        this.player.debug.log('MediaSource', 'destroy');
      }

    }

    // tks: https://github.com/richtr/NoSleep.js
    const WEBM = "data:video/webm;base64,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";
    const MP4 = "data:video/mp4;base64,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"; // Detect iOS browsers < version 10

    const oldIOS = () => typeof navigator !== "undefined" && parseFloat(("" + (/CPU.*OS ([0-9_]{3,4})[0-9_]{0,1}|(CPU like).*AppleWebKit.*Mobile/i.exec(navigator.userAgent) || [0, ""])[1]).replace("undefined", "3_2").replace("_", ".").replace("_", "")) < 10 && !window.MSStream; // Detect native Wake Lock API support


    const nativeWakeLock = () => "wakeLock" in navigator;

    class NoSleep {
      constructor(player) {
        this.player = player;
        this.enabled = false;

        if (nativeWakeLock()) {
          this._wakeLock = null;

          const handleVisibilityChange = () => {
            if (this._wakeLock !== null && document.visibilityState === "visible") {
              this.enable();
            }
          };

          document.addEventListener("visibilitychange", handleVisibilityChange);
          document.addEventListener("fullscreenchange", handleVisibilityChange);
        } else if (oldIOS()) {
          this.noSleepTimer = null;
        } else {
          // Set up no sleep video element
          this.noSleepVideo = document.createElement("video");
          this.noSleepVideo.setAttribute("title", "No Sleep");
          this.noSleepVideo.setAttribute("playsinline", "");

          this._addSourceToVideo(this.noSleepVideo, "webm", WEBM);

          this._addSourceToVideo(this.noSleepVideo, "mp4", MP4);

          this.noSleepVideo.addEventListener("loadedmetadata", () => {
            if (this.noSleepVideo.duration <= 1) {
              // webm source
              this.noSleepVideo.setAttribute("loop", "");
            } else {
              // mp4 source
              this.noSleepVideo.addEventListener("timeupdate", () => {
                if (this.noSleepVideo.currentTime > 0.5) {
                  this.noSleepVideo.currentTime = Math.random();
                }
              });
            }
          });
        }
      }

      _addSourceToVideo(element, type, dataURI) {
        var source = document.createElement("source");
        source.src = dataURI;
        source.type = `video/${type}`;
        element.appendChild(source);
      }

      get isEnabled() {
        return this.enabled;
      }

      enable() {
        const debug = this.player.debug;

        if (nativeWakeLock()) {
          return navigator.wakeLock.request("screen").then(wakeLock => {
            this._wakeLock = wakeLock;
            this.enabled = true;
            debug.log('wakeLock', 'Wake Lock active.');

            this._wakeLock.addEventListener("release", () => {
              // ToDo: Potentially emit an event for the page to observe since
              // Wake Lock releases happen when page visibility changes.
              // (https://web.dev/wakelock/#wake-lock-lifecycle)
              debug.log('wakeLock', 'Wake Lock released.');
            });
          }).catch(err => {
            this.enabled = false;
            debug.error('wakeLock', `${err.name}, ${err.message}`);
            throw err;
          });
        } else if (oldIOS()) {
          this.disable();
          this.noSleepTimer = window.setInterval(() => {
            if (!document.hidden) {
              window.location.href = window.location.href.split("#")[0];
              window.setTimeout(window.stop, 0);
            }
          }, 15000);
          this.enabled = true;
          return Promise.resolve();
        } else {
          let playPromise = this.noSleepVideo.play();
          return playPromise.then(res => {
            this.enabled = true;
            return res;
          }).catch(err => {
            this.enabled = false;
            throw err;
          });
        }
      }

      disable() {
        const debug = this.player.debug;

        if (nativeWakeLock()) {
          if (this._wakeLock) {
            this._wakeLock.release();
          }

          this._wakeLock = null;
        } else if (oldIOS()) {
          if (this.noSleepTimer) {
            debug.warn('wakeLock', 'NoSleep now disabled for older iOS devices.');
            window.clearInterval(this.noSleepTimer);
            this.noSleepTimer = null;
          }
        } else {
          this.noSleepVideo.pause();
        }

        this.enabled = false;
      }

    }

    class Player extends Emitter {
      constructor(container, options) {
        super();
        this.$container = container;
        this._opt = Object.assign({}, DEFAULT_PLAYER_OPTIONS, options);
        this.debug = new Debug(this);

        if (this._opt.useWCS) {
          this._opt.useWCS = supportWCS();
        }

        if (this._opt.useMSE) {
          this._opt.useMSE = supportMSE();
        } // 如果使用mse则强制不允许 webcodecs


        if (this._opt.useMSE) {
          if (this._opt.useWCS) {
            this.debug.log('Player', 'useWCS set true->false');
          }

          if (!this._opt.forceNoOffscreen) {
            this.debug.log('Player', 'forceNoOffscreen set false->true');
          }

          this._opt.useWCS = false;
          this._opt.forceNoOffscreen = true;
        } else if (this._opt.useWCS) ;

        if (!this._opt.forceNoOffscreen) {
          if (!supportOffscreenV2()) {
            this._opt.forceNoOffscreen = true;
            this._opt.useOffscreen = false;
          } else {
            this._opt.useOffscreen = true;
          }
        }

        this._opt.hasControl = this._hasControl(); //

        this._loading = false;
        this._playing = false;
        this._hasLoaded = false; //

        this._checkHeartTimeout = null;
        this._checkLoadingTimeout = null; //

        this._startBpsTime = null;
        this._isPlayingBeforePageHidden = false;
        this._stats = {
          buf: 0,
          // 当前缓冲区时长，单位毫秒,
          fps: 0,
          // 当前视频帧率
          abps: 0,
          // 当前音频码率，单位bit
          vbps: 0,
          // 当前视频码率，单位bit
          ts: 0 // 当前视频帧pts，单位毫秒

        }; //

        this._videoTimestamp = 0;
        this._audioTimestamp = 0;
        property$1(this);
        this.events = new Events(this);
        this.video = new Video(this);
        this.audio = new Audio(this);
        this.recorder = new Recorder(this);
        this.decoderWorker = new DecoderWorker(this);
        this.stream = null;
        this.demux = null;

        if (this._opt.useWCS) {
          this.webcodecsDecoder = new WebcodecsDecoder(this);
        }

        if (this._opt.useMSE) {
          this.mseDecoder = new MseDecoder(this);
        } //


        this.control = new Control(this);
        this.keepScreenOn = new NoSleep(this);
        events$1(this);
        observer(this);

        if (this._opt.useWCS) {
          this.debug.log('Player', 'use WCS');
        }

        if (this._opt.useMSE) {
          this.debug.log('Player', 'use MSE');
        }

        if (this._opt.useOffscreen) {
          this.debug.log('Player', 'use offscreen');
        }

        this.debug.log('Player options', this._opt);
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
        if (value) {
          // 将loading 设置为 false
          this.loading = false;
        }

        if (this.playing !== value) {
          this._playing = value;
          this.emit(EVENTS.playing, value);
          this.emit(EVENTS.volumechange, this.volume);

          if (value) {
            this.emit(EVENTS.play);
          } else {
            this.emit(EVENTS.pause);
          }
        }
      }

      get playing() {
        return this._playing;
      }

      get volume() {
        return this.audio.volume;
      }

      set volume(value) {
        this.audio.setVolume(value);
      }

      set loading(value) {
        if (this.loading !== value) {
          this._loading = value;
          this.emit(EVENTS.loading, this._loading);
        }
      }

      get loading() {
        return this._loading;
      }

      set recording(value) {
        if (this.playing) {
          if (value) {
            this.recorder.startRecord();
          } else {
            this.recorder.stopRecordAndSave();
          }
        }
      }

      get recording() {
        return this.recorder.recording;
      }

      set audioTimestamp(value) {
        if (value === null) {
          return;
        }

        this._audioTimestamp = value;
      } //


      get audioTimestamp() {
        return this._audioTimestamp;
      } //


      set videoTimestamp(value) {
        if (value === null) {
          return;
        }

        this._videoTimestamp = value; // just for wasm

        if (!this._opt.useWCS && !this._opt.useMSE) {
          if (this.audioTimestamp && this.videoTimestamp) {
            this.audio.emit(EVENTS.videoSyncAudio, {
              audioTimestamp: this.audioTimestamp,
              videoTimestamp: this.videoTimestamp,
              diff: this.audioTimestamp - this.videoTimestamp
            });
          }
        }
      } //


      get videoTimestamp() {
        return this._videoTimestamp;
      }
      /**
       *
       * @param options
       */


      updateOption(options) {
        this._opt = Object.assign({}, this._opt, options);
      }
      /**
       *
       * @returns {Promise<unknown>}
       */


      init() {
        return new Promise((resolve, reject) => {
          if (!this.stream) {
            this.stream = new Stream(this);
          }

          if (!this.demux) {
            this.demux = new Demux(this);
          }

          if (this._opt.useWCS) {
            if (!this.webcodecsDecoder) {
              this.webcodecsDecoder = new WebcodecsDecoder(this);
            }
          }

          if (this._opt.useMSE) {
            if (!this.mseDecoder) {
              this.mseDecoder = new MseDecoder(this);
            }
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
      /**
       *
       * @param url
       * @returns {Promise<unknown>}
       */


      play(url) {
        return new Promise((resolve, reject) => {
          if (!url && !this._opt.url) {
            return reject();
          }

          this.loading = true;
          this.playing = false;

          if (!url) {
            url = this._opt.url;
          }

          this._opt.url = url;
          this.clearCheckHeartTimeout();
          this.init().then(() => {
            //
            if (this._opt.isNotMute) {
              this.mute(false);
            }

            if (this.webcodecsDecoder) {
              this.webcodecsDecoder.once(EVENTS_ERROR.webcodecsH265NotSupport, () => {
                this.emit(EVENTS_ERROR.webcodecsH265NotSupport);
                this.emit(EVENTS.error, EVENTS_ERROR.webcodecsH265NotSupport);
              });
            }

            if (this.mseDecoder) {
              this.mseDecoder.once(EVENTS_ERROR.mediaSourceH265NotSupport, () => {
                this.emit(EVENTS_ERROR.mediaSourceH265NotSupport);
                this.emit(EVENTS.error, EVENTS_ERROR.mediaSourceH265NotSupport);
              });
            }

            this.enableWakeLock();
            this.stream.fetchStream(url); //

            this.checkLoadingTimeout(); // fetch error

            this.stream.once(EVENTS_ERROR.fetchError, error => {
              reject(error);
            }); // ws

            this.stream.once(EVENTS_ERROR.websocketError, error => {
              reject(error);
            }); // success

            this.stream.once(EVENTS.streamSuccess, () => {
              resolve(); //

              if (this._opt.useMSE) {
                this.video.play();
              }
            });
          }).catch(e => {
            reject(e);
          });
        });
      }
      /**
       *
       */


      close() {
        return new Promise((resolve, reject) => {
          this._close().then(() => {
            this.video.clearView();
            resolve();
          });
        });
      }

      _close() {
        return new Promise((resolve, reject) => {
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

          if (this.mseDecoder) {
            this.mseDecoder.destroy();
            this.mseDecoder = null;
          }

          this.clearCheckHeartTimeout();
          this.clearCheckLoadingTimeout();
          this.playing = false;
          this.loading = false;
          this.recording = false; // release audio buffer

          this.audio.pause(); // release lock

          this.releaseWakeLock(); // reset stats

          this.resetStats(); //

          this._audioTimestamp = 0;
          this._videoTimestamp = 0; //

          setTimeout(() => {
            resolve();
          }, 0);
        });
      }
      /**
       *
       * @param flag {boolean} 是否清除画面
       * @returns {Promise<unknown>}
       */


      pause(flag) {
        if (flag) {
          return this.close();
        } else {
          return this._close();
        }
      }
      /**
       *
       * @param flag
       */


      mute(flag) {
        this.audio.mute(flag);
      }
      /**
       *
       */


      resize() {
        this.video.resize();
      }
      /**
       *
       * @param fileName
       * @param fileType
       */


      startRecord(fileName, fileType) {
        if (this.recording) {
          return;
        }

        this.recorder.setFileName(fileName, fileType);
        this.recording = true;
      }
      /**
       *
       */


      stopRecordAndSave() {
        if (this.recording) {
          this.recording = false;
        }
      }

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

      checkHeart() {
        this.clearCheckHeartTimeout();
        this.checkHeartTimeout();
      } // 心跳检查，如果渲染间隔暂停了多少时间之后，就会抛出异常


      checkHeartTimeout() {
        this._checkHeartTimeout = setTimeout(() => {
          this.pause(false).then(() => {
            this.emit(EVENTS.timeout, EVENTS.delayTimeout);
            this.emit(EVENTS.delayTimeout);
          });
        }, this._opt.heartTimeout * 1000);
      } //


      clearCheckHeartTimeout() {
        if (this._checkHeartTimeout) {
          clearTimeout(this._checkHeartTimeout);
          this._checkHeartTimeout = null;
        }
      } // loading 等待时间


      checkLoadingTimeout() {
        this._checkLoadingTimeout = setTimeout(() => {
          this.pause(false).then(() => {
            this.emit(EVENTS.timeout, EVENTS.loadingTimeout);
            this.emit(EVENTS.loadingTimeout);
          });
        }, this._opt.loadingTimeout * 1000);
      }

      clearCheckLoadingTimeout() {
        if (this._checkLoadingTimeout) {
          clearTimeout(this._checkLoadingTimeout);
          this._checkLoadingTimeout = null;
        }
      }

      handleRender() {
        if (this.loading) {
          this.emit(EVENTS.start);
          this.loading = false;
          this.clearCheckLoadingTimeout();
        }

        if (!this.playing) {
          this.playing = true;
        }

        this.checkHeart();
      } //


      updateStats(options) {
        options = options || {};

        if (!this._startBpsTime) {
          this._startBpsTime = now();
        }

        if (isNotEmpty(options.ts)) {
          this._stats.ts = options.ts;
        }

        if (isNotEmpty(options.buf)) {
          this._stats.buf = options.buf;
        }

        if (options.fps) {
          this._stats.fps += 1;
        }

        if (options.abps) {
          this._stats.abps += options.abps;
        }

        if (options.vbps) {
          this._stats.vbps += options.vbps;
        }

        const _nowTime = now();

        const timestamp = _nowTime - this._startBpsTime;

        if (timestamp < 1 * 1000) {
          return;
        }

        this.emit(EVENTS.stats, this._stats);
        this.emit(EVENTS.performance, fpsStatus(this._stats.fps));
        this._stats.fps = 0;
        this._stats.abps = 0;
        this._stats.vbps = 0;
        this._startBpsTime = _nowTime;
      }

      resetStats() {
        this._startBpsTime = null;
        this._stats = {
          buf: 0,
          //ms
          fps: 0,
          abps: 0,
          vbps: 0,
          ts: 0
        };
      }

      enableWakeLock() {
        if (this._opt.keepScreenOn) {
          this.keepScreenOn.enable();
        }
      }

      releaseWakeLock() {
        if (this._opt.keepScreenOn) {
          this.keepScreenOn.disable();
        }
      }

      destroy() {
        this._loading = false;
        this._playing = false;
        this._hasLoaded = false;

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
        }

        if (this.webcodecsDecoder) {
          this.webcodecsDecoder.destroy();
          this.webcodecsDecoder = null;
        }

        if (this.mseDecoder) {
          this.mseDecoder.destroy();
          this.mseDecoder = null;
        }

        if (this.demux) {
          this.demux.destroy();
          this.demux = null;
        }

        if (this.events) {
          this.events.destroy();
          this.events = null;
        }

        this.clearCheckHeartTimeout();
        this.clearCheckLoadingTimeout(); //

        this.releaseWakeLock();
        this.keepScreenOn = null; // reset stats

        this.resetStats();
        this._audioTimestamp = 0;
        this._videoTimestamp = 0; // 其他没法解耦的，通过 destroy 方式

        this.emit('destroy'); // 接触所有绑定事件

        this.off();
        this.debug.log('play', 'destroy end');
      }

    }

    var aegis_min = createCommonjsModule(function (module, exports) {
    /**
     *  =====================================================================
     * @tencent/aegis-web-sdk@1.24.48 (c) 2021 Tencent Application Monitor.
     * Author pumpkincai.
     * Last Release Time Wed Dec 29 2021 23:00:35 GMT+0800 (GMT+08:00).
     * Released under the MIT License.
     * Thanks for supporting TAM & Aegis!
     * =====================================================================
     **/
    !function(e,t){module.exports=t();}(commonjsGlobal,function(){var t,n;function e(e){this.name="__st"+(1e9*Math.random()>>>0)+t+"__",null==e||e.forEach(this.add,this),t+=1;}Array.prototype.find||Object.defineProperty(Array.prototype,"find",{configurable:!0,writable:!0,value:function(e){if(null===this)throw new TypeError('"this" is null or not defined');var t=Object(this),n=t.length>>>0;if("function"!=typeof e)throw new TypeError("predicate must be a function");for(var i=arguments[1],o=0;o<n;){var r=t[o];if(e.call(i,r,o,t))return r;o+=1;}}}),String.prototype.startsWith||Object.defineProperty(String.prototype,"startsWith",{value:function(e,t){return this.substring(t=!t||t<0?0:+t,t+e.length)===e}}),window.WeakSet||(t=Date.now()%1e9,e.prototype.add=function(e){var t=this.name;return e[t]||Object.defineProperty(e,t,{value:!0,writable:!0}),this},e.prototype.delete=function(e){return !!e[this.name]&&!(e[this.name]=void 0)},e.prototype.has=function(e){return !!e[this.name]},n=e,Object.defineProperty(window,"WeakSet",{value:function(e){return new n(e)}})),Object.assign||Object.defineProperty(Object,"assign",{enumerable:!1,configurable:!0,writable:!0,value:function(e){if(null==e)throw new TypeError("Cannot convert first argument to object");for(var t=Object(e),n=1;n<arguments.length;n++)if(null!=(i=arguments[n]))for(var i=Object(i),o=Object.keys(Object(i)),r=0,s=o.length;r<s;r++){var a=o[r],u=Object.getOwnPropertyDescriptor(i,a);null!=u&&u.enumerable&&(t[a]=i[a]);}return t}});var i=function(e,t){return (i=Object.setPrototypeOf||{__proto__:[]}instanceof Array&&function(e,t){e.__proto__=t;}||function(e,t){for(var n in t)Object.prototype.hasOwnProperty.call(t,n)&&(e[n]=t[n]);})(e,t)},E=function(){return (E=Object.assign||function(e){for(var t,n=1,i=arguments.length;n<i;n++)for(var o in t=arguments[n])Object.prototype.hasOwnProperty.call(t,o)&&(e[o]=t[o]);return e}).apply(this,arguments)};function u(){for(var e=0,t=0,n=arguments.length;t<n;t++)e+=arguments[t].length;for(var i=Array(e),o=0,t=0;t<n;t++)for(var r=arguments[t],s=0,a=r.length;s<a;s++,o++)i[o]=r[s];return i}function R(e){return (e=Array.isArray(e)?e:[e]).map(function(t,n){return Object.getOwnPropertyNames(t).map(function(e){return o(e)+"["+n+"]="+(void 0===t[e]?"":o(t[e]))}).join("&")}).join("&")+(e.length?"&count="+e.length:"")}var h,r=(s.prototype.indexOf=function(e,t){for(var n=0;n<e.length;n++)if(e[n].callback===t)return n;return -1},s.prototype.on=function(e,t,n){if(void 0===n&&(n=0),this){var i=this.eventsList[e];return (i||(this.eventsList[e]=[],i=this.eventsList[e]),-1!==this.indexOf(i,t))?this:(i.push({name:e,type:n||0,callback:t}),this)}},s.prototype.one=function(e,t){this.on(e,t,1);},s.prototype.remove=function(e,t){if(this){var n=this.eventsList[e];if(!n)return null;if(t)return n.length&&(t=this.indexOf(n,t),n.splice(t,1)),this;try{delete this.eventsList[e];}catch(e){}return null}},s.prototype.clear=function(){this.eventsList={};},s),o=function(t){try{return encodeURIComponent(decodeURIComponent(t))}catch(e){return t}};function s(){var s=this;this.emit=function(e,t){if(s){var n;if(null!=(i=s.eventsList[e])&&i.length)for(var i=i.slice(),o=0;o<i.length;o++){n=i[o];try{var r=n.callback.apply(s,[t]);if(1===n.type&&s.remove(e,n.callback),!1===r)break}catch(e){throw e}}return s}},this.eventsList={};}(se=h=h||{})[se.number=-1]="number",se.string="";function a(e,t){return "number"==typeof e||"string"==typeof e?e:t?h.string:h.number}function g(e,t){return "string"==typeof e?e.split("?")[t?1:0]||"":e}function v(e){return "string"==typeof e&&e.startsWith("//")?"undefined"!=typeof location&&"https:"===location.protocol:/^https/.test(e)}function d(t){return l.some(function(e){return -1!==t.indexOf(e)})}var O,T,c,l=["application/octet-stream","application/xhtml+xml","application/xml","application/pdf","application/pkcs12","application/javascript","application/ecmascript","application/vnd.mspowerpoint","application/ogg","text/html","text/css","text/javascript","image","audio","video"],f=["ret","retcode","code","errcode"],m=function(e,t,n){var i;try{if("function"==typeof(null==t?void 0:t.retCodeHandler)){var o=t.retCodeHandler(e,null==n?void 0:n.url,null==n?void 0:n.ctx)||{};return {code:void 0===(r=o.code)?"unknown":r,isErr:o.isErr}}"string"==typeof e&&(e=JSON.parse(e)),"function"==typeof(null===(i=null==t?void 0:t.ret)||void 0===i?void 0:i.join)&&(f=[].concat(t.ret.map(function(e){return e.toLowerCase()})));var r,s=Object.getOwnPropertyNames(e).filter(function(e){return -1!==f.indexOf(e.toLowerCase())});return s.length?{code:""+(r=Number(e[s[0]])),isErr:0!==r}:{code:"unknown",isErr:!1}}catch(e){return {code:"unknown",isErr:!1}}},S=function(e){if("string"==typeof e)return e;try{return (JSON.stringify(e,(i=[],o=[],function(e,t){if(t instanceof Error)return "Error.message: "+t.message+" \n  Error.stack: "+t.stack;if("object"==typeof t&&null!==t){var n=i.indexOf(t);if(-1!==n)return "[Circular "+o[n]+"]";i.push(t),o.push(e||"root");}return t}),4)||"undefined").replace(/"/gim,"")}catch(e){return "error happen when aegis stringify: \n "+e.message+" \n "+e.stack}var i,o;},y=function(n,i){void 0===i&&(i=3);var o,r,s,a="";return Array.isArray(n)?(a+="[",o=n.length,n.forEach(function(e,t){a+="object"==typeof e&&1<i?y(e,i-1):w(e),a+=t===o-1?"":",";}),a+="]"):n instanceof Object?(a="{",r=Object.keys(n),s=r.length,r.forEach(function(e,t){"object"==typeof n[e]&&1<i?a+='"'+e+'":'+y(n[e],i-1):a+=p(e,n[e]),a+=t===s-1||t<s-1&&void 0===n[r[t+1]]?"":",";}),a+="}"):a+=n,a},p=function(e,t){var n=typeof t,i="";return "string"==n||"object"==n?i+='"'+e+'":"'+t+'"':"function"==typeof t?i+='"'+e+'":"function '+t.name+'"':"symbol"==typeof t?i+='"'+e+'":"symbol"':"number"!=typeof t&&"boolean"!=n||(i+='"'+e+'": '+t),i},w=function(e){var t=typeof e;return ""+("undefined"==t||"symbol"==t||"function"==t?"null":"string"==t||"object"==t?'"'+e+'"':e)},b=(x.prototype.sourceURL=function(){return this.data.responseURL},x.prototype.status=function(){return Number(this.data.status)},x.prototype.headers=function(){var e=this.data.getAllResponseHeaders().split("\n"),n={};return e.forEach(function(e){var t;e&&(e=(t=e.split(": "))[0],t=t[1].trim(),n[e]=t);}),n},x),L=(P.prototype.sourceURL=function(){return this.data.url},P.prototype.status=function(){return Number(this.data.status)},P.prototype.headers=function(){var n={};return this.data.headers.forEach(function(e,t){n[t]=e;}),n},P);function P(e,t){this.type="fetch",this.data=e||{},this.data.response=t;}function x(e){this.type="xhr",this.data=e;}(Be=O=O||{}).INFO_ALL="-1",Be.API_RESPONSE="1",Be.INFO="2",Be.ERROR="4",Be.PROMISE_ERROR="8",Be.AJAX_ERROR="16",Be.SCRIPT_ERROR="32",Be.IMAGE_ERROR="64",Be.CSS_ERROR="128",Be.CONSOLE_ERROR="256",Be.MEDIA_ERROR="512",Be.RET_ERROR="1024",Be.REPORT="2048",Be.PV="4096",Be.EVENT="8192",Be.PAGE_NOT_FOUND_ERROR="16384",(Fe={})[Fe.android=1]="android",Fe[Fe.ios=2]="ios",Fe[Fe.windows=3]="windows",Fe[Fe.macos=4]="macos",Fe[Fe.linux=5]="linux",Fe[Fe.devtools=6]="devtools",Fe[Fe.other=100]="other",(we={})[we.unknown=100]="unknown",we[we.wifi=1]="wifi",we[we.net2g=2]="net2g",we[we.net3g=3]="net3g",we[we.net4g=4]="net4g",we[we.net5g=5]="net5g",we[we.net6g=6]="net6g",(Re=T=T||{}).LOG="log",Re.SPEED="speed",Re.PERFORMANCE="performance",Re.OFFLINE="offline",Re.WHITE_LIST="whiteList",Re.VITALS="vitals",Re.PV="pv",Re.CUSTOM_PV="customPV",Re.EVENT="event",Re.CUSTOM="custom",Re.SDK_ERROR="sdkError",Re.SET_DATA="setData",(Te=c=c||{}).production="production",Te.gray="gray",Te.pre="pre",Te.daily="daily",Te.local="local",Te.test="test",Te.others="others";function C(e){return e.filter(function(n,i){return "static"!==n.type||!e.find(function(e,t){return i!==t&&n.url===e.url&&200===n.status})})}function I(e){e.level===O.INFO_ALL&&(e.level=O.INFO);}function A(o){return function(e,t){o.lifeCycle.emit("modifyRequest",e);var n=o.config.modifyRequest;if("function"==typeof n)try{var i=n(e);"object"==typeof i&&"url"in i&&(e=i);}catch(e){console.error(e);}t(e);}}function N(i){return function(e,t){i.lifeCycle.emit("afterRequest",e);var n=i.config.afterRequest;"function"==typeof n&&!1===n(e)||t(e);}}function k(){}function _(t,n){Object.getOwnPropertyNames(t).forEach(function(e){"function"==typeof t[e]&&"constructor"!==e&&(n?n[e]="sendPipeline"===e?function(){return function(){}}:function(){}:t[e]=function(){});});}function U(t){if(t.payload){var n={};return Object.keys(t).forEach(function(e){"payload"!==e&&(n[e]=t[e]);}),n}return t}function j(){return void 0!==window.performance&&"function"==typeof performance.clearResourceTimings&&"function"==typeof performance.getEntriesByType&&"function"==typeof performance.now}function q(t){if(ie.find(function(e){return e.name===t.name}))throw new Error("name '"+t.name+"' is already in hackXHR option list");var n,i;ie.push(t),!ne&&window.XMLHttpRequest&&(ne=!0,n=window.XMLHttpRequest.prototype.send,i=window.XMLHttpRequest.prototype.open,window.XMLHttpRequest.prototype.open=function(){if(this.aegisMethod=arguments[0],this.aegisUrl=arguments[1],this.aegisXhrStartTime=Date.now(),!this.sendByAegis)for(var e=0;e<ie.length;e++){var t=ie[e];try{"function"==typeof t.open&&t.open(this);}catch(e){}}return i.apply(this,arguments)},window.XMLHttpRequest.prototype.send=function(){if(!this.sendByAegis)for(var e=0;e<ie.length;e++){var t=ie[e];try{"function"==typeof t.send&&t.send(this,arguments[0]);}catch(e){}}return n.apply(this,arguments)});}function F(t){var e=ie.findIndex(function(e){return e.name===t.name});-1!==e&&ie.splice(e,1);}var D,H,M=function(i,o){return function(e,t){var n=o.logCreated;if("function"!=typeof n)return i("beforeWrite",e),t(e);e=e.filter(function(e){return !1!==n(e)});return i("beforeWrite",e),t(e)}},B=function(e,n){var i,o=[],r=e.config;return e.lifeCycle.on("destroy",function(){o.length=0;}),function(e,t){if(o.push(e),n&&o.length>=n)return o=C(o),t(o.splice(0,o.length)),void(i&&clearTimeout(i));i&&clearTimeout(i),i=setTimeout(function(){i=null,0<(o=C(o)).length&&t(o.splice(0,o.length));},r.delay);}},V=function(e,t){return Array.isArray(e)?t(e.map(function(e){return E(E({},e),{msg:"string"==typeof e.msg?e.msg:[].concat(e.msg).map(S).join(" ")})})):t(E(E({},e),{msg:"string"==typeof e.msg?e.msg:S(e.msg)}))},W=function(s){var e,a=!1,u=!1,t=!1,c=[];return s.lifeCycle.on("onConfigChange",function(){e&&clearTimeout(e),e=setTimeout(function(){var e,n;!t&&s.config&&(t=!0,e=s.config.whiteListUrl,(n=void 0===e?"":e)&&s.sendPipeline([function(e,t){t({url:n,type:T.WHITE_LIST,success:function(e){u=!0;try{var t=e.data||JSON.parse(e),n=t.retcode,i=t.result,o=void 0===i?{}:i;if(0===n){if(a=o.is_in_white_list,s.isWhiteList=a,o.shutdown)return void s.destroy();0<=o.rate&&o.rate<=1&&(s.config.random=o.rate,s.isGetSample=!1);}s.isWhiteList&&c.length?$(s)(c.splice(0),function(){}):!s.isWhiteList&&c.length&&(c.length=0);var r=s.config.onWhitelist;"function"==typeof r&&r(a);}catch(e){}},fail:function(e){"403 forbidden"===e&&s.destroy(),u=!0;}});}],T.WHITE_LIST)(null),t=!1);},s.config.uin?50:500);}),s.lifeCycle.on("destroy",function(){c.length=0;}),function(e,t){var n;a||null!==(n=null===(n=s.config)||void 0===n?void 0:n.api)&&void 0!==n&&n.reportRequest?t(e.concat(c.splice(0)).map(function(e){return I(e),e})):(e=e.filter(function(e){return e.level!==O.INFO&&e.level!==O.API_RESPONSE?(I(e),!0):(u||(c.push(e),200<=c.length&&(c.length=200)),!1)})).length&&t(e);}},X=function(i){return setTimeout(function(){var e=i.config.pvUrl,n=void 0===e?"":e;n&&i.sendPipeline([function(e,t){t({url:n,type:T.PV,fail:function(e){"403 forbidden"===e&&i.destroy();}});}],T.PV)(null);},100),function(e,t){t(e);}},G=function(o){var r={};return function(e,t){var n,i;o.speedSample?(i="object"==typeof o.repeat?o.repeat:{repeat:o.repeat},n=+i.speed||+i.repeat||5,Array.isArray(e)?(i=e.filter(function(e){var t=!r[e.url]||r[e.url]<n;return r[e.url]=1+~~r[e.url],t})).length&&t(i):(!r[e.url]||r[e.url]<n)&&(r[e.url]=1+~~r[e.url],t(e))):t(e);}},J=function(i){var o={};return function(e,t){var n="number"==typeof i.repeat?i.repeat:5;if(0===n)return t(e);t(e.filter(function(e){return e.level!==O.ERROR&&e.level!==O.PROMISE_ERROR&&e.level!==O.AJAX_ERROR&&e.level!==O.SCRIPT_ERROR&&e.level!==O.IMAGE_ERROR&&e.level!==O.CSS_ERROR&&e.level!==O.MEDIA_ERROR||(o[e.msg]=o[e.msg]||0,o[e.msg]+=1,!(o[e.msg]>n))}));}},$=function(i){return function(e){return i.sendPipeline([function(e,n){return n({url:i.config.url||"",data:R(e),method:"post",contentType:"application/x-www-form-urlencoded",type:T.LOG,log:e,requestConfig:{timeout:5e3},success:function(){var t=i.config.onReport;"function"==typeof t&&e.forEach(function(e){t(e);}),"function"==typeof n&&n([]);},fail:function(e){"403 forbidden"===e&&i.destroy();}})}],T.LOG)(e)}},K=function(n){if(!n||!n.reduce||!n.length)throw new TypeError("createPipeline need at least one function param");return 1===n.length?function(e,t){n[0](e,t||k);}:n.reduce(function(n,i){return function(e,t){return void 0===t&&(t=k),n(e,function(e){return null==i?void 0:i(e,t)})}})},Q=(Object.defineProperty(ue.prototype,"__version__",{get:function(){return console.warn("__version__ has discard, please use version"),"1.24.48"},enumerable:!1,configurable:!0}),Object.defineProperty(ue.prototype,"LogType",{get:function(){return console.warn("LogType has discard, please use logType"),O},enumerable:!1,configurable:!0}),ue.prototype.init=function(e){this.setConfig(e);for(var t=0;t<ue.installedPlugins.length;t++)try{ue.installedPlugins[t].patch(this);}catch(e){this.sendSDKError(e);}this.lifeCycle.emit("onInited");},ue.prototype.setConfig=function(e){Object.assign(this.config,e);var t=this.config,n=t.id,i=t.uin,o=t.version,r=t.ext1,s=t.ext2,a=t.ext3,e=t.aid,t=t.env,u=void 0===t?"production":t,t=this.bean.id!==n||this.bean.uin!==i||this.bean.aid!==e;return this.bean.id=n||"",this.bean.uin=i||"",this.bean.version=o||"1.24.48",this.bean.aid=e||"",this.bean.env=function(){switch(u){case c.production:case c.gray:case c.pre:case c.daily:case c.local:case c.others:return 1;default:return}}()?u:c.others,r&&(this.bean.ext1=encodeURIComponent(r)),s&&(this.bean.ext2=encodeURIComponent(s)),a&&(this.bean.ext3=encodeURIComponent(a)),t&&this.lifeCycle.emit("onConfigChange",this.config),this.config},ue.use=function(e){-1===ue.installedPlugins.indexOf(e)&&e.aegisPlugin&&ue.installedPlugins.push(e);},ue.unuse=function(e){e=ue.installedPlugins.indexOf(e);-1!==e&&ue.installedPlugins.splice(e,1);},ue.prototype.info=function(){for(var e=[],t=0;t<arguments.length;t++)e[t]=arguments[t];var n={level:O.INFO,msg:e};1===e.length&&e[0].msg&&Object.assign(n,E({},e[0]),{level:O.INFO}),this.normalLogPipeline(n);},ue.prototype.infoAll=function(){for(var e=[],t=0;t<arguments.length;t++)e[t]=arguments[t];var n={level:O.INFO_ALL,msg:e};1===e.length&&e[0].msg&&Object.assign(n,E({},e[0]),{level:O.INFO_ALL}),this.normalLogPipeline(n);},ue.prototype.report=function(){for(var e=[],t=0;t<arguments.length;t++)e[t]=arguments[t];var n={level:O.REPORT,msg:e};1===e.length&&e[0].msg&&Object.assign(n,E({},e[0])),this.normalLogPipeline(n);},ue.prototype.error=function(){for(var e=[],t=0;t<arguments.length;t++)e[t]=arguments[t];var n={level:O.ERROR,msg:e};1===e.length&&e[0].msg&&Object.assign(n,E({},e[0]),{level:O.ERROR}),this.normalLogPipeline(n);},ue.prototype.speedLogPipeline=function(e){throw new Error('You need to override "speedLogPipeline" method')},ue.prototype.reportPv=function(n){var i,o=this;n&&(console.warn("reportPv is deprecated, please use reportEvent"),i=""+Object.getOwnPropertyNames(this.bean).filter(function(e){return "id"!==e}).map(function(e){return e+"="+o.bean[e]}).join("&"),this.sendPipeline([function(e,t){t({url:o.config.url+"/"+n+"?"+i,addBean:!1,type:T.CUSTOM_PV,fail:function(e){"403 forbidden"===e&&o.destroy();}});}],T.CUSTOM_PV)(null));},ue.prototype.reportEvent=function(e){e&&((e="string"==typeof e?{name:e,ext1:this.config.ext1||"",ext2:this.config.ext2||"",ext3:this.config.ext3||""}:e).name?this.eventPipeline(e):console.warn("reportEvent params error"));},ue.prototype.reportTime=function(e,t){if("object"==typeof e)return this.reportT(e);"string"==typeof e?"number"==typeof t?t<0||6e4<t?console.warn("reportTime: duration must between 0 and 60000"):this.submitCustomTime(e,t):console.warn("reportTime: second param must be number"):console.warn("reportTime: first param must be a string");},ue.prototype.reportT=function(e){var t=e.name,n=e.duration,i=e.ext1,o=void 0===i?"":i,r=e.ext2,i=void 0===r?"":r,r=e.ext3,r=void 0===r?"":r,e=e.from;if("string"==typeof t&&"number"==typeof n&&"string"==typeof o&&"string"==typeof i&&"string"==typeof r){if(!(n<0||6e4<n))return this.submitCustomTime(t,n,o,i,r,void 0===e?"":e);console.warn("reportTime: duration must between 0 and 60000");}else console.warn("reportTime: params error");},ue.prototype.time=function(e){"string"==typeof e?this.timeMap[e]?console.warn("Timer "+e+" already exists"):this.timeMap[e]=Date.now():console.warn("time: first param must be a string");},ue.prototype.timeEnd=function(e){"string"==typeof e?this.timeMap[e]?(this.submitCustomTime(e,Date.now()-this.timeMap[e]),delete this.timeMap[e]):console.warn("Timer "+e+" does not exist"):console.warn("timeEnd: first param must be a string");},ue.prototype.submitCustomTime=function(e,t,n,i,o,r){this.customTimePipeline({name:e,duration:t,ext1:n||this.config.ext1,ext2:i||this.config.ext2,ext3:o||this.config.ext3,from:r||void 0});},ue.prototype.extendBean=function(e,t){this.bean[e]=t;},ue.prototype.sendPipeline=function(e,t){var n,r,s,a=this;return K(u([function(e,t){if("number"!=typeof n.config.random&&(console.warn("random must in [0, 1], default is 1."),n.config.random=1),!n.isHidden||!n.isGetSample)if(n.isGetSample)n.isHidden||t(e);else {if(n.isGetSample=!0,Math.random()<n.config.random)return n.isHidden=!1,t(e);n.isHidden=!0;}},(s=t,function(e,t){var n=Array.isArray(e),i=n?e:[e];r.lifeCycle.emit("beforeRequest",e);var o=r.config.beforeRequest;(i="function"==typeof o?i.map(function(t){try{var e=o({logs:t,logType:s});return (null==e?void 0:e.logType)===s&&null!=e&&e.logs?e.logs:!1!==e&&t}catch(e){return t}}).filter(function(e){return !1!==e}):i).length&&(i=function(e,t){if(!Array.isArray(e)||e.length<=1)return e;var n=[],i=[];return !(i="string"==typeof t?[t]:t)||i.length<=0||(i.forEach(function(t){e.forEach(function(e){null!=e&&e[t]&&n.push(t);});}),0<n.length&&(e=e.map(function(e){var t={};return n.forEach(function(e){t[e]="";}),E(E({},t),e)}))),e}(i,["ext1","ext2","ext3"]),t(n?i:i[0]));})],e,[A(r=n=this),function(i,o){a.request(i,function(){for(var e,t=[],n=0;n<arguments.length;n++)t[n]=arguments[n];o({isErr:!1,result:t,logType:null==i?void 0:i.type,logs:null==i?void 0:i.log}),null===(e=null==i?void 0:i.success)||void 0===e||e.call.apply(e,u([i],t));},function(){for(var e,t=[],n=0;n<arguments.length;n++)t[n]=arguments[n];o({isErr:!0,result:t,logType:null==i?void 0:i.type,logs:null==i?void 0:i.log}),null===(e=null==i?void 0:i.fail)||void 0===e||e.call.apply(e,u([i],t));});},N(this)]))},ue.prototype.send=function(e,o,r){var t=this;return K([A(this),function(n,i){t.request(n,function(){for(var e=[],t=0;t<arguments.length;t++)e[t]=arguments[t];i({isErr:!1,result:e,logType:n.type,logs:n.log}),null==o||o.apply(void 0,e);},function(){for(var e=[],t=0;t<arguments.length;t++)e[t]=arguments[t];i({isErr:!0,result:e,logType:n.type,logs:n.log}),null==r||r.apply(void 0,e);});},N(this)])(e)},ue.prototype.request=function(e,t,n){throw new Error('You need to override "request" method')},ue.prototype.sendSDKError=function(e){var n=this;this.sendPipeline([function(e,t){t({url:n.config.url+"?id=1085&msg[0]="+encodeURIComponent(S(e))+"&level[0]=2&from="+n.config.id+"&count=1&version="+n.config.id+"(1.24.48)",addBean:!1,method:"get",type:T.SDK_ERROR,log:e});}],T.SDK_ERROR)(e);},ue.prototype.destroy=function(e){void 0===e&&(e=!1);var t,n,i=ue.instances.indexOf(this);-1!==i&&ue.instances.splice(i,1);for(var o=ue.installedPlugins.length-1;0<=o;o--)try{ue.installedPlugins[o].unpatch(this);}catch(e){this.sendSDKError(e);}if(this.lifeCycle.emit("destroy"),this.lifeCycle.clear(),e)t=this,n=Object.getOwnPropertyDescriptors(t),Object.keys(n).forEach(function(e){n[e].writable&&(t[e]=null);}),Object.setPrototypeOf(this,null);else {for(var r=this;r.constructor!==Object&&_(r,this),r=Object.getPrototypeOf(r););0===ue.instances.length&&(e=Object.getPrototypeOf(this).constructor,_(e),_(ue));}},ue.version="1.24.48",ue.instances=[],ue.logType=O,ue.environment=c,ue.installedPlugins=[],ue),Y=(ae.prototype.patch=function(e){this.canUse(e)&&this.exist(e)&&(this.instances.push(e),this.triggerInit(e),this.triggerOnNewAegis(e));},ae.prototype.unpatch=function(e){e=this.instances.indexOf(e);-1!==e&&this.instances.splice(e,1);},ae.prototype.countInstance=function(){return this.instances.length},ae.prototype.uninstall=function(){var e;null===(e=null===(e=this.option)||void 0===e?void 0:e.destroy)||void 0===e||e.apply(this);},ae.prototype.walk=function(n){var i=this;this.instances.forEach(function(e){var t=i.canUse(e);t&&n(e,t);});},ae.prototype.canUse=function(e){e=this.getConfig(e);return !(!e||"object"!=typeof e)||!!e},ae.prototype.getConfig=function(e){return null===(e=e.config)||void 0===e?void 0:e[this.name]},ae.prototype.exist=function(e){return -1===this.instances.indexOf(e)},ae.prototype.triggerInit=function(e){var t;this.inited||(this.inited=!0,null===(t=null===(t=this.option)||void 0===t?void 0:t.init)||void 0===t||t.call(this.option,this.getConfig(e)));},ae.prototype.triggerOnNewAegis=function(e){var t;null===(t=null===(t=this.option)||void 0===t?void 0:t.onNewAegis)||void 0===t||t.call(this.option,e,this.getConfig(e));},ae),z=new Y({name:"aid",aid:"",init:function(e){try{var t=!0!==e&&e||window.localStorage.getItem("AEGIS_ID");t||(t="xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx".replace(/[xy]/g,function(e){var t=16*Math.random()|0;return ("x"===e?t:3&t|8).toString(16)}),window.localStorage.setItem("AEGIS_ID",t)),this.aid=t;}catch(e){}},onNewAegis:function(e){e.bean.aid=this.aid,e.config.aid=this.aid;}}),Z=function(e,t){var n,i={fetch:[],static:[]},o=new FormData;return Array.isArray(e)?e.forEach(function(e){var t=U(e);i[e.type].push(t);}):(n=U(e),i[e.type].push(n)),o.append("payload",JSON.stringify(E({duration:i},t))),o},ee=new Y({name:"reportAssetSpeed"}),te=ee=new Y({name:"reportAssetSpeed",collectCur:0,ASSETS_INITIATOR_TYPE:["img","css","script","link","audio","video"],onNewAegis:function(e){var t=this;j()&&(this.collectSuccessLog(e),this.collectFailLog(e),performance.onresourcetimingbufferfull=function(){t.collectCur=0,performance.clearResourceTimings();});},publish:function(t){this.$walk(function(e){e.speedLogPipeline(t);});},collectSuccessLog:function(e){function n(e){for(var t=0,n=e.length;t<n;t++){var i=e[t];-1!==o.ASSETS_INITIATOR_TYPE.indexOf(i.initiatorType)&&-1===i.name.indexOf("cdn-go.cn/aegis/aegis-sdk")&&o.publish(o.generateLog(i,r));}}var t,i,o=this,r=e.config;"function"==typeof window.PerformanceObserver?(n(performance.getEntriesByType("resource")),(t=new window.PerformanceObserver(function(e){n(e.getEntries());})).observe({entryTypes:["resource"]}),e.lifeCycle.on("destroy",function(){0===ee.countInstance()&&t.disconnect();})):(i=setInterval(function(){var e=performance.getEntriesByType("resource"),t=e.slice(o.collectCur);o.collectCur=e.length,n(t);},3e3),e.lifeCycle.on("destroy",function(){0===ee.countInstance()&&clearInterval(i);}));},collectFailLog:function(e){function t(e){var t,n;e&&(e=e.target||e.srcElement,t=(null==e?void 0:e.src)||(null==e?void 0:e.href),e=null==e?void 0:e.tagName,t&&"string"==typeof t&&("IMG"===e&&-1<window.location.href.indexOf(t)||(e="function"==typeof(null===(e=o.api)||void 0===e?void 0:e.resourceTypeHandler)?null===(n=o.api)||void 0===n?void 0:n.resourceTypeHandler(t):"",n=performance.getEntriesByType("resource").find(function(e){return e.name===t}),e={url:g(t),status:400,duration:Number(((null==n?void 0:n.duration)||0).toFixed(2)),method:"get",type:e||"static",isHttps:v(t),urlQuery:g(t,!0),domainLookup:0,connectTime:0},i.publish(e))));}var i=this,o=e.config;window.document.addEventListener("error",t,!0),e.lifeCycle.on("destroy",function(){0===ee.countInstance()&&window.document.removeEventListener("error",t,!0);});},generateLog:function(e,t){var n,i="function"==typeof(null===(n=t.api)||void 0===n?void 0:n.resourceTypeHandler)?null===(i=t.api)||void 0===i?void 0:i.resourceTypeHandler(e.name):"";return {url:g(e.name),method:"get",duration:Number(e.duration.toFixed(2)),status:200,type:i||"static",isHttps:v(e.name),urlQuery:g(e.name,!0),domainLookup:a(e.domainLookupEnd-e.domainLookupStart),connectTime:a(e.connectEnd-e.connectStart)}},destroy:function(){this.option.publish=function(){};}}),ne=!1,ie=[],oe=!1,re=[],se=(new Y({name:"reportApiSpeed"}),new Y({name:"reportApiSpeed",override:!1,onNewAegis:function(e){this.override||(this.override=!0,this.overrideFetch(e.config),this.overrideXhr(e.config));},overrideFetch:function(l){var f=this,e={name:this.name,then:function(r,s,a,u){var e,c,t,n;l.hostUrl&&-1<a.indexOf(l.hostUrl)||(c={url:r.url,isHttps:v(r.url),method:(null==u?void 0:u.method)||"get",duration:s,type:"fetch",status:r.status||0},"fetch"!==(n="function"==typeof(null===(e=l.api)||void 0===e?void 0:e.resourceTypeHandler)?null===(t=l.api)||void 0===t?void 0:t.resourceTypeHandler(r.url):n)&&"static"!==n&&(t=r.headers?r.headers.get("content-type"):"",n=r.ok&&"string"==typeof t&&d(t)?"static":"fetch"),"fetch"===n?r.clone().text().then(function(e){var t=null===(o=l.api)||void 0===o?void 0:o.apiDetail,n=r.status<=0||400<=r.status,i=m(e,l.api,{url:a,ctx:r}),o=i.code,i=i.isErr,t=(n?"FETCH_ERROR: "+e+" \n\n":"")+"req url: "+a+" \n                                \nreq method: "+((null==u?void 0:u.method)||"get")+" \n                                \nreq param: "+(t?y(null==u?void 0:u.body):"")+" \n                                \nres duration: "+s+" \n                                \nres status: "+(r.status||0)+" \n                                \nres retcode: "+o+"\n                                \nres data: "+(t?e:"");c.payload=new L(r,e),c.ret=o,c.isErr=+i,f.publishNormalLog({msg:t,level:n?O.AJAX_ERROR:i?O.RET_ERROR:O.API_RESPONSE,code:o});}):Object.assign(c,{type:"static",urlQuery:g(r.url,!0),domainLookup:h.number,connectTime:h.number}),f.publishSpeed(c));},catch:function(e,t,n,i){if(!(l.hostUrl&&-1<n.indexOf(l.hostUrl))){var o={url:n,isHttps:v(n),method:(null==i?void 0:i.method)||"get",duration:t,type:"fetch",status:0};f.publishSpeed(o);o=null===(o=l.api)||void 0===o?void 0:o.apiDetail,i="AJAX_ERROR: "+e+"\n                          \nres status: 0\n                          \nres duration: "+t+"\n                          \nreq url: "+n+"\n                          \nreq method: "+((null==i?void 0:i.method)||"get")+"\n                          \nreq param: "+(o?y(null==i?void 0:i.body):"");throw f.publishNormalLog({msg:i,level:O.AJAX_ERROR,code:-400}),e}}};this.hackFetchOptions=e,function(t){if(re.find(function(e){return e.name===t.name}))throw new Error("name '"+t.name+"' is already in hackFetch option list");var s;re.push(t),!oe&&window.fetch&&(oe=!0,s=window.fetch,window.fetch=function(e,i){void 0===i&&(i={});for(var o="string"==typeof e?e:e.url,t=0;t<re.length;t++){var n=re[t];try{"function"==typeof n.beforeFetch&&n.beforeFetch(o,i);}catch(e){}}var r=Date.now();return s(e,i).then(function(e){for(var t=0;t<re.length;t++){var n=re[t];try{"function"==typeof n.then&&n.then(e,Date.now()-r,o,i);}catch(e){}}return e}).catch(function(e){for(var t=0;t<re.length;t++){var n=re[t];try{"function"==typeof n.catch&&n.catch(e,Date.now()-r,o,i);}catch(e){}}throw e})});}(this.hackFetchOptions);},getRequestType:function(e,t,n){var i,o="function"==typeof(null===(i=e.api)||void 0===i?void 0:i.resourceTypeHandler)?null===(o=e.api)||void 0===o?void 0:o.resourceTypeHandler(n):"";return "fetch"!==o&&"static"!==o&&(n=t.getResponseHeader("content-type"),o=400<=t.status||"string"!=typeof n||!d(n)?"fetch":"static"),o},overrideXhr:function(d){var p=this,e={name:this.name,send:function(c,l){var f=Date.now();c.addEventListener("loadend",function(){var e,t=c.aegisUrl;if(t){var n=Date.now()-f,i={url:t,isHttps:v(t),status:c.status||0,method:c.aegisMethod||"get",type:"fetch",duration:n,payload:new b(c)};if("fetch"===p.getRequestType(d,c,t))try{var o=null===(e=d.api)||void 0===e?void 0:e.apiDetail,r=m(c.response,d.api,{url:t,ctx:c}),s=r.code,a=r.isErr,u="req url: "+t+" \n                                \nreq method: "+i.method+" \n                                \nreq param: "+(o?y(l):"")+" \n                                \nres duration: "+n+" \n                                \nres status: "+i.status+" \n                                \nres retcode: "+s+"\n                                \nres data: "+(o?y(c.response):"");i.ret=s,i.isErr=+a,p.publishNormalLog({msg:u,level:a?O.RET_ERROR:O.API_RESPONSE,code:s});}catch(e){i.ret="unknown";}else Object.assign(i,{type:"static",urlQuery:g(t,!0),domainLookup:h.number,connectTime:h.number});p.publishSpeed(i);}});}};this.hackXHROptions=e,q(this.hackXHROptions);},publishSpeed:function(n){var i=this;n.url=g(n.url),this.$walk(function(e){var t=i.$getConfig(e);"fetch"===n.type&&t&&"function"==typeof t.urlHandler?e.speedLogPipeline(E(E({},n),{url:encodeURIComponent(t.urlHandler(n.url,n.payload))})):e.speedLogPipeline(n);});},publishNormalLog:function(t){this.$walk(function(e){e.normalLogPipeline(t);});},destroy:function(){var t,e;this.option.publishSpeed=function(){},this.option.publishNormalLog=function(){},this.option.hackXHROptions&&F(this.option.hackXHROptions),this.option.hackFetchOptions&&(t=this.option.hackFetchOptions,-1!==(e=re.findIndex(function(e){return e.name===t.name}))&&re.splice(e,1));}}));function ae(e){this.aegisPlugin=!0,this.name="",this.instances=[],this.inited=!1,e.$walk=this.walk.bind(this),e.$getConfig=this.getConfig.bind(this),this.option=e,this.name=e.name;}function ue(e){var n,t,o=this;this.isGetSample=!1,this.isHidden=!1,this.config={version:0,delay:1e3,onError:!0,repeat:5,random:1,aid:!0,device:!0,pagePerformance:!0,webVitals:!0,speedSample:!0,hostUrl:"https://aegis.qq.com",env:"production",url:"",offlineUrl:"",whiteListUrl:"",pvUrl:"",speedUrl:"",customTimeUrl:"",performanceUrl:"",webVitalsUrl:"",eventUrl:""},this.isWhiteList=!1,this.lifeCycle=new r,this.bean={},this.normalLogPipeline=K([B(this,5),V,function(e,t){var i=n.config;t(e=e.map(function(e){var t,n=i.maxLength||204800;try{if(!e.msg||e.msg.length<=n)return e;e.msg=null===(t=e.msg)||void 0===t?void 0:t.substring(0,n);}catch(t){e.msg=S(e.msg).substring(0,i.maxLength);}return e}));},J((n=this).config),M(this.lifeCycle.emit,this.config),X(this),W(this),function(e,t){var n=JSON.parse(JSON.stringify(e));o.lifeCycle.emit("beforeReport",n);var i=o.config.beforeReport;(e="function"==typeof i?e.filter(function(e){return !1!==i(e)}):e).length&&t(e);},$(this)]),this.eventPipeline=K([B(this,5),function(e){o.sendPipeline([function(e,t){var n=e.map(function(e){return {name:e.name,ext1:e.ext1||o.config.ext1||"",ext2:e.ext2||o.config.ext2||"",ext3:e.ext3||o.config.ext3||""}});t({url:o.config.eventUrl+"?payload="+encodeURIComponent(JSON.stringify(n)),type:T.EVENT,log:e,fail:function(e){"403 forbidden"===e&&o.destroy();}});}],T.EVENT)(e);}]),this.timeMap={},this.customTimePipeline=K([B(this,5),function(e){return o.sendPipeline([function(e,t){t({url:o.config.customTimeUrl+"?payload="+encodeURIComponent(JSON.stringify({custom:e})),type:T.CUSTOM,log:e,fail:function(e){"403 forbidden"===e&&o.destroy();}});}],T.CUSTOM)(e)}]),this.config=(t=this.config,void 0===(e=e.hostUrl)&&(e="https://aegis.qq.com"),t.url=t.url||e+"/collect",t.offlineUrl=t.offlineUrl||e+"/offline",t.whiteListUrl=t.whiteListUrl||e+"/collect/whitelist",t.pvUrl=t.pvUrl||e+"/collect/pv",t.eventUrl=t.eventUrl||e+"/collect/events",t.speedUrl=t.speedUrl||e+"/speed",t.customTimeUrl=t.customTimeUrl||e+"/speed/custom",t.performanceUrl=t.performanceUrl||e+"/speed/performance",t.webVitalsUrl=t.webVitalsUrl||e+"/speed/webvitals",t.setDataReportUrl=t.SetDataReportUrl||e+"/speed/miniProgramData",t),ue.instances.push(this);}(Be=D=D||{})[Be.unknown=100]="unknown",Be[Be.wifi=1]="wifi",Be[Be.net2g=2]="net2g",Be[Be.net3g=3]="net3g",Be[Be.net4g=4]="net4g",Be[Be.net5g=5]="net5g",Be[Be.net6g=6]="net6g",(Fe=H=H||{})[Fe.android=1]="android",Fe[Fe.ios=2]="ios",Fe[Fe.windows=3]="windows",Fe[Fe.macos=4]="macos",Fe[Fe.linux=5]="linux",Fe[Fe.other=100]="other";var ce,le,fe,de,pe,he,ge,ve,me,ye,we=new Y({name:"device",onNewAegis:function(e){e.extendBean("platform",this.getPlatform()),e.extendBean("netType",D.unknown),this.getDpi(e),this.refreshNetworkTypeToBean(e);},getDpi:function(e){e.extendBean("vp",window.innerWidth+" * "+window.innerHeight),window.screen&&e.extendBean("sr",window.screen.width+" * "+window.screen.height);},getPlatform:function(){var t={android:/\bAndroid\s*([^;]+)/,ios:/\b(iPad|iPhone|iPod)\b.*? OS ([\d_]+)/,windows:/\b(Windows NT)/,macos:/\b(Mac OS)/,linux:/\b(Linux)/i},e=Object.keys(t).find(function(e){return t[e].test(navigator.userAgent)});return e?H[e]:H.other},refreshNetworkTypeToBean:function(t){var n=this,e=t.config;e&&("function"==typeof e.getNetworkType?e.getNetworkType:be)(function(e){D[e]||(e=D.unknown),t.extendBean("netType",e),n.NetworkRefreshTimer=setTimeout(function(){n.refreshNetworkTypeToBean(t),clearTimeout(n.NetworkRefreshTimer);},1e4);});}}),be=function(e){var t="",n=navigator.userAgent.match(/NetType\/(\w+)/);n?t=n[1]:navigator.connection&&(t=navigator.connection.effectiveType||navigator.connection.type),e((t=t=t||"unknown",0<=(t=String(t).toLowerCase()).indexOf("4g")?D.net4g:0<=t.indexOf("wifi")?D.wifi:0<=t.indexOf("5g")?D.net5g:0<=t.indexOf("6g")?D.net6g:0<=t.indexOf("3g")?D.net3g:0<=t.indexOf("2g")?D.net2g:D.unknown));},Ee=new Y({name:"onError"}),Re=Ee=new Y({name:"onError",onNewAegis:function(e){this.startListen(e);},startListen:function(r){var i=this,n=window.onerror;window.onerror=function(){for(var e=[],t=0;t<arguments.length;t++)e[t]=arguments[t];i.publishErrorLog({msg:(S(e[0])||"")+" @ ("+(S(e[1])||"")+":"+(e[2]||0)+":"+(e[3]||0)+")\n          \n"+S(e[4]||""),level:O.ERROR}),null==n||n.call.apply(n,u([window],e));};function e(e){e=e&&S(e.reason),i.publishErrorLog({msg:"PROMISE_ERROR: "+e,level:O.PROMISE_ERROR});}window.addEventListener("unhandledrejection",e);function t(e){if(e=(null==e?void 0:e.target)||(null==e?void 0:e.srcElement)){var t=e.src||e.href,e=e.tagName;if("string"==typeof t&&e){var n={msg:e+" load fail: "+t,level:O.INFO};switch(e.toLowerCase()){case"script":n.level=O.SCRIPT_ERROR;break;case"link":n.level=O.CSS_ERROR;break;case"img":if(-1<window.location.href.indexOf(t))return;n.level=O.IMAGE_ERROR;break;case"audio":case"video":n.level=O.MEDIA_ERROR;break;default:return}i.publishErrorLog(n);}}}window.document.addEventListener("error",t,!0),r.lifeCycle.on("destroy",function(){0===Ee.countInstance()&&(window.document.removeEventListener("unhandledrejection",e),window.document.removeEventListener("error",t,!0));});var s=this,o={name:this.name,send:function(i,o){i.addEventListener("loadend",function(){var e,t,n="";this.failType?n=this.failType:this.status?400<=this.status&&(n="error"):n="failed",r.config.hostUrl&&this.aegisUrl&&-1<this.aegisUrl.indexOf(r.config.hostUrl)||(t=null===(e=r.config.api)||void 0===e?void 0:e.apiDetail,n&&(e=i.aegisUrl,e=m(i.response,r.config.api,{url:e,ctx:i}).code,s.publishErrorLog({msg:"AJAX_ERROR: request "+n+"\n                      \nres status: "+(this.status||0)+"\n                      \nres retcode: "+e+"\n                      \nres duration: "+(Date.now()-i.aegisXhrStartTime)+"\n                      \nreq url: "+this.aegisUrl+"\n                      \nreq method: "+this.aegisMethod+"\n                      \nreq param: "+(t?y(o):"")+"\n                      \nres data: "+(t?null===(t=(t=i).responseType&&"text"!==t.responseType?"":t.responseText)||void 0===t?void 0:t.slice(0,1e3):""),level:O.AJAX_ERROR})));}),i.addEventListener("timeout",function(){i.failType="timeout";}),i.addEventListener("error",function(){i.failType="error";}),i.addEventListener("abort",function(){i.failType="abort";});}};this.hackXHROptions=o,q(this.hackXHROptions);},publishErrorLog:function(t){this.$walk(function(e){e.normalLogPipeline(t);});},destroy:function(){this.option.publishErrorLog=function(){},this.option.hackXHROptions&&F(this.option.hackXHROptions);}}),Oe=(new Y({name:"pagePerformance"}),1),Te=new Y({name:"pagePerformance",onNewAegis:function(i){var o=this;if(j())if(ce)this.publish(ce,i);else try{this.getFirstScreenTiming(i,function(e){var t,n=performance.timing;n&&(t=n.loadEventStart-n.domInteractive,ce={dnsLookup:n.domainLookupEnd-n.domainLookupStart,tcp:n.connectEnd-n.connectStart,ssl:0===n.secureConnectionStart?0:n.requestStart-n.secureConnectionStart,ttfb:n.responseStart-n.requestStart,contentDownload:n.responseEnd-n.responseStart,domParse:n.domInteractive-n.domLoading,resourceDownload:t=t<0?1070:t,firstScreenTiming:Math.floor(e)},o.publish(ce,i));});}catch(i){}},publish:function(e,s){var a=this;s.sendPipeline([function(e,t){var n,i=[];for(n in e)i.push(n+"="+e[n]);var o=a.$getConfig(s);if(o){var r=-1===s.config.performanceUrl.indexOf("?")?"?":"&";return "function"==typeof o.urlHandler?t({url:s.config.performanceUrl+r+i.join("&")+"&from="+(encodeURIComponent(o.urlHandler())||window.location.href),beanFilter:["from"],type:T.PERFORMANCE,log:e}):t({url:s.config.performanceUrl+r+i.join("&"),type:T.PERFORMANCE,log:e})}}],T.PERFORMANCE)(e);},getFirstScreenTiming:function(r,n){r.lifeCycle.on("destroy",function(){f&&clearTimeout(f);});var i=["script","style","link","br"],s=[],a=this,u={},c=new MutationObserver(function(e){var t={roots:[],rootsDomNum:[],time:performance.now()};e.forEach(function(e){e&&e.addedNodes&&e.addedNodes.forEach&&e.addedNodes.forEach(function(e){1===e.nodeType&&(e.hasAttribute("AEGIS-FIRST-SCREEN-TIMING")||e.querySelector("[AEGIS-FIRST-SCREEN-TIMING]"))?(Object.prototype.hasOwnProperty.apply(u,[t.time])||(u[t.time]=[]),u[t.time].push(e)):1!==e.nodeType||-1!==i.indexOf(e.nodeName.toLocaleLowerCase())||a.isEleInArray(e,t.roots)||e.hasAttribute("AEGIS-IGNORE-FIRST-SCREEN-TIMING")||(t.roots.push(e),t.rootsDomNum.push(a.walkAndCount(e)||0));});}),t.roots.length&&s.push(t);});c.observe(document,{childList:!0,subtree:!0});var l=function(){var e,i=0,o=0,t=Object.keys(u).filter(function(e){return u[+e].find(function(e){return a.isInFirstScreen(e)})});t.length?(o=Math.max.apply(null,t),("object"!=typeof(null===(t=r.config)||void 0===t?void 0:t.pagePerformance)||null!==(e=r.config.pagePerformance)&&void 0!==e&&e.firstScreenInfo)&&(r.firstScreenInfo={element:null===(e=u[o])||void 0===e?void 0:e[0],timing:o,markDoms:u})):s.forEach(function(e){for(var t,n=0;n<e.roots.length;n++)e.rootsDomNum[n]>i&&a.isInFirstScreen(e.roots[n])&&(i=e.rootsDomNum[n],o=e.time,("object"!=typeof(null===(t=r.config)||void 0===t?void 0:t.pagePerformance)||null!==(t=r.config.pagePerformance)&&void 0!==t&&t.firstScreenInfo)&&(r.firstScreenInfo={element:e.roots[n],timing:o}));}),!o&&Oe?f=setTimeout(function(){return l()},3e3):(c.disconnect(),null==n||n(o)),--Oe;},f=setTimeout(function(){return l()},3e3);},isEleInArray:function(e,t){return !(!e||e===document.documentElement)&&(-1!==t.indexOf(e)||this.isEleInArray(e.parentElement,t))},isInFirstScreen:function(e){if(!e||"function"!=typeof e.getBoundingClientRect)return !1;var t=e.getBoundingClientRect(),n=window.innerHeight,e=window.innerWidth;return 0<=t.left&&t.left<e&&0<=t.top&&t.top<n&&0<t.width&&0<t.height},walkAndCount:function(e){var t=0;if(e&&1===e.nodeType){t+=1;var n=e.children;if(null!=n&&n.length)for(var i=0;i<n.length;i++)t+=this.walkAndCount(n[i]);}return t}});function Se(){pe=[],fe=-1,le=null,me(addEventListener);}function Le(e,t){le||(le=t,fe=e,de=new Date,me(removeEventListener),ve());}function Pe(e){var t,n,i,o;function r(){Le(n,i),o();}function s(){o();}e.cancelable&&(t=(1e12<e.timeStamp?new Date:performance.now())-e.timeStamp,"pointerdown"==e.type?(n=t,i=e,o=function(){removeEventListener("pointerup",r,he),removeEventListener("pointercancel",s,he);},addEventListener("pointerup",r,he),addEventListener("pointercancel",s,he)):Le(t,e));}he={passive:!0,capture:!0},ge=new Date,ve=function(){var t;0<=fe&&fe<de-ge&&(t={entryType:"first-input",name:le.type,target:le.target,cancelable:le.cancelable,startTime:le.timeStamp,processingStart:le.timeStamp+fe},pe.forEach(function(e){e(t);}),pe=[]);},me=function(t){["mousedown","keydown","touchstart","pointerdown"].forEach(function(e){return t(e,Pe,he)});},ye="hidden"===document.visibilityState?0:1/0,addEventListener("visibilitychange",function e(t){"hidden"===document.visibilityState&&(ye=t.timeStamp,removeEventListener("visibilitychange",e,!0));},!0),Se(),self.webVitals={firstInputPolyfill:function(e){pe.push(e),ve();},resetFirstInputPolyfill:Se,get firstHiddenTime(){return ye}};function xe(e,t){return {name:e,value:void 0===t?-1:t,delta:0,entries:[],id:"v1-".concat(Date.now(),"-").concat(Math.floor(8999999999999*Math.random())+1e12)}}function Ce(e,t){try{if(PerformanceObserver.supportedEntryTypes.includes(e)){if("first-input"===e&&!("PerformanceEventTiming"in self))return;var n=new PerformanceObserver(function(e){return e.getEntries().map(t)});return n.observe({type:e,buffered:!0}),n}}catch(e){}}function Ie(t,n){function i(e){"pagehide"!==e.type&&"hidden"!==document.visibilityState||(t(e),n&&(removeEventListener("visibilitychange",i,!0),removeEventListener("pagehide",i,!0)));}addEventListener("visibilitychange",i,!0),addEventListener("pagehide",i,!0);}function Ae(t){addEventListener("pageshow",function(e){e.persisted&&t(e);},!0);}function Ne(e,t,n){var i;return function(){0<=t.value&&(n||De.has(t)||"hidden"===document.visibilityState)&&(t.delta=t.value-(i||0),!t.delta&&void 0!==i||(i=t.value,e(t)));}}function ke(){Ie(function(e){e=e.timeStamp;He=e;},!0);}function _e(){return He<0&&((He=self.webVitals.firstHiddenTime)===1/0&&ke(),Ae(function(){setTimeout(function(){He="hidden"===document.visibilityState?0:1/0,ke();},0);})),{get timeStamp(){return He}}}function Ue(e){var t=e.name,e=e.value;Me[t]=e;}var je,qe,Fe,De=new("function"==typeof WeakSet?WeakSet:Set),He=-1,Me=(new Y({name:"webVitals"}),{FCP:-1,LCP:-1,FID:-1,CLS:-1}),Be=new Y({name:"webVitals",onNewAegis:function(e){var t,n,i,o,r,s,a,u,c,l,f,d,p,h,g,v,m,y,w,b,E,R,O;if(j()&&"function"==typeof window.PerformanceObserver)try{y=Ue,b=_e(),E=xe("FCP"),R=performance.getEntriesByName("first-contentful-paint")[0],O=R?null:Ce("paint",P),(R||O)&&(w=Ne(y,E,void 0),R&&P(R),Ae(function(e){E=xe("FCP"),w=Ne(y,E,void 0),requestAnimationFrame(function(){requestAnimationFrame(function(){E.value=performance.now()-e.timeStamp,De.add(E),w();});});})),d=Ue,g=_e(),v=xe("LCP"),(m=Ce("largest-contentful-paint",L))&&(p=Ne(d,v,void 0),h=function(){De.has(v)||(m.takeRecords().map(L),m.disconnect(),De.add(v),p());},["keydown","click"].forEach(function(e){addEventListener(e,h,{once:!0,capture:!0});}),Ie(h,!0),Ae(function(e){v=xe("LCP"),p=Ne(d,v,void 0),requestAnimationFrame(function(){requestAnimationFrame(function(){v.value=performance.now()-e.timeStamp,De.add(v),p();});});})),a=Ue,u=_e(),c=xe("FID"),l=Ce("first-input",S),f=Ne(a,c,void 0),l&&Ie(function(){l.takeRecords().map(S),l.disconnect();},!0),l||window.webVitals.firstInputPolyfill(S),Ae(function(){c=xe("FID"),f=Ne(a,c,void 0),window.webVitals.resetFirstInputPolyfill(),window.webVitals.firstInputPolyfill(S);}),i=Ue,r=xe("CLS",0),(s=Ce("layout-shift",T))&&(o=Ne(i,r,void 0),Ie(function(){s.takeRecords().map(T),o();}),Ae(function(){r=xe("CLS",0),o=Ne(i,r,void 0);})),t=this.publish.bind(this,e),n=function(e){"pagehide"!==e.type&&"hidden"!==document.visibilityState||(t(e),removeEventListener("visibilitychange",n,!0),removeEventListener("pagehide",n,!0));},addEventListener("visibilitychange",n,!0),addEventListener("pagehide",n,!0);}catch(e){}function T(e){e.hadRecentInput||(r.value+=e.value,r.entries.push(e),o());}function S(e){e.startTime<u.timeStamp&&(c.value=e.processingStart-e.startTime,c.entries.push(e),De.add(c),f());}function L(e){var t=e.startTime;t<g.timeStamp&&(v.value=t,v.entries.push(e)),p();}function P(e){"first-contentful-paint"===e.name&&(O&&O.disconnect(),e.startTime<b.timeStamp&&(E.value=e.startTime,E.entries.push(e),De.add(E),w()));}},publish:function(r){r.sendPipeline([function(e,t){var n,i=[];for(n in e)i.push(n+"="+e[n]);var o=-1===r.config.performanceUrl.indexOf("?")?"?":"&";t({url:r.config.webVitalsUrl+o+i.join("&"),type:T.VITALS,log:e,sendBeacon:!0});}],T.VITALS)(Me);},destroy:function(){this.option.publish=function(){};}}),Ve=0,We=(i(qe=Ge,Fe=je=Q),qe.prototype=null===Fe?Object.create(Fe):(Je.prototype=Fe.prototype,new Je),Ge.prototype.getBean=function(t){var n=this;return void 0===t&&(t=[]),""+Object.getOwnPropertyNames(this.bean).filter(function(e){return -1===t.indexOf(e)}).map(function(e){return e+"="+n.bean[e]}).join("&")},Ge.prototype.resetRequesting=function(){this.requesting=!1;var e=this.requestQueue.shift();e&&this.request(e.options,e.success,e.fail);},Ge.prototype.request=function(e,t,n){var i=this;if(e&&"string"==typeof e.url&&""!==e.url&&this.bean.id)if(this.requesting)this.requestQueue.push({options:e,success:t,fail:n});else {this.requesting=!0;var o=e.url;!1!==e.addBean&&(o=o+(-1===o.indexOf("?")?"?":"&")+this.getBean(e.beanFilter)),e.url=o;var r=e.method||"get",o=this.config.onBeforeRequest;if(!(e=o?o(e,this):e))return this.resetRequesting(),console.log("Sending request blocked");if(!e.url)return this.resetRequesting(),console.warn("Please handle the parameters reasonably, options.url is necessary");if(navigator.sendBeacon&&e.sendBeacon)return this.resetRequesting(),navigator.sendBeacon(e.url,e.data);var s=new XMLHttpRequest;s.sendByAegis=!0,Object.assign(s,{timeout:5e3},e.requestConfig),s.addEventListener("readystatechange",function(){4===s.readyState&&(i.resetRequesting(),400<=s.status||0===s.status?null==n||n(s.response):null==t||t(s.response));}),"get"===r.toLocaleLowerCase()?(s.open("get",function(e,n){if("string"!=typeof e)return "";if("object"==typeof n&&n){var t=Object.getOwnPropertyNames(n).map(function(e){var t=n[e];return e+"="+("string"==typeof t?encodeURIComponent(t):encodeURIComponent(JSON.stringify(t)))}).join("&").replace(/eval/gi,"evaI");return e+(-1===e.indexOf("?")?"?":"&")+t}return e}(e.url,e.data)),s.send()):(s.open("post",e.url),e.contentType&&s.setRequestHeader("Content-Type",e.contentType),"string"==typeof e.data&&(e.data=e.data.replace(/eval/gi,"evaI")),s.send(e.data));}},Ge.useAsyncPlugin=function(s,e){var t=(e=void 0===e?{}:e).exportsConstructor,a=void 0===t?"aegis-plugin-"+Ve:t,t=e.onAegisInit,u=void 0===t?function(){}:t,e=e.onAegisInitAndPluginLoaded,c=void 0===e?function(){}:e;if(Ve+=1,"string"!=typeof s)throw new TypeError("useAsyncPlugin first param must be string");if("function"!=typeof u||"function"!=typeof c)throw new TypeError("onAegisInit and onAegisInitAndPluginLoaded must be function");this.use(new Y({name:"asyncPlugin",onNewAegis:function(t){try{u(t),Ge.asyncPlugin[s]?c(t,window[Ge.asyncPlugin[s]]):(e=s,n=a,i=function(e){e||(Ge.asyncPlugin[s]=a,e=window[a],c(t,e));},o=document.createElement("script"),r=document.head,"function"==typeof n&&(i=n,n=""),o.src=e,o.setAttribute("name",n),o.name=n,o.setAttribute("crossorigin","anonymous"),o.crossorigin="anonymous",o.defer=!0,o.hasLoaded=!1,o.onreadystatechange=function(){o.hasLoaded||o.readyState&&"loaded"!==o.readyState&&"complete"!==o.readyState||(o.hasLoaded=!0,"function"==typeof i&&i(!1),setTimeout(function(){r.contains(o)&&r.removeChild(o);}));},o.onload=o.onreadystatechange,o.onerror=function(){"function"==typeof i&&i(!0),setTimeout(function(){r.contains(o)&&r.removeChild(o);});},"complete"===document.readyState?r.appendChild(o):window.addEventListener("load",function(){r.appendChild(o);}));}catch(e){console.log("error on below is caused by "+s+" "),console.error(e);}var e,n,i,o,r;}}));},Ge.prototype.uploadLogs=function(e,t){this.lifeCycle.emit("uploadLogs",e=void 0===e?{}:e,t=void 0===t?{}:t);},Ge.sessionID="session-"+Date.now(),Ge.asyncPlugin={},Ge.urls={aegisCollect:"https://aegis.qq.com/collect",flog:"https://cdn-go.cn/vasdev/web_webpersistance_v2/v1.8.2/flog.core.min.js",shadowLog:""},Ge),Xe=new Y({name:"offlineLog"}),Q=Xe=new Y({name:"offlineLog",onNewAegis:function(e){var t=We.urls.flog,h=e.config,g=[],v=null,m=null,y=function(e){m=e;},w=function(e){g=g.concat(e);},b=function(e,t){v={conds:e=void 0===e?{}:e,params:t=void 0===t?{}:t};};We.useAsyncPlugin(t,{exportsConstructor:"Flog",onAegisInit:function(e){e.lifeCycle.on("beforeWrite",w),e.lifeCycle.on("uploadLogs",b),e.lifeCycle.on("onConfigChange",y);},onAegisInitAndPluginLoaded:function(i,e){var t,n,o,r,s,a=h.dbConfig,u=void 0===a?{}:a,c=h.url,l=void 0===c?We.urls.aegisCollect:c,f=h.offlineLogExp,d=void 0===f?3:f,a=h.id,c=h.uin;null===(f=i.lifeCycle)||void 0===f||f.remove("beforeWrite",w),null===(f=i.lifeCycle)||void 0===f||f.remove("uploadLogs",b),null===(f=i.lifeCycle)||void 0===f||f.remove("onConfigChange",y);u=Object.assign({lookupUrl:l+"/offlineAuto?id="+a,preservedDay:d,id:a,uin:c,aid:(null===(c=i.bean)||void 0===c?void 0:c.aid)||"",beforeRequest:i.config.beforeRequest,afterRequest:i.config.afterRequest},u,{sessionId:We.sessionID});try{var p=new e(u);g.forEach(function(e){p.add(E(E({},e),{level:e.level===O.INFO_ALL?O.INFO:e.level}));}),null===(t=i.lifeCycle)||void 0===t||t.on("beforeWrite",function(e){(e=void 0===e?[]:e).forEach(function(e){p.add(E(E({},e),{level:e.level===O.INFO_ALL?O.INFO:e.level}));});}),null===(n=i.lifeCycle)||void 0===n||n.on("uploadLogs",function(e,t){var n;void 0===e&&(e={}),void 0===t&&(t={}),p.uploadLogs(Object.assign({id:h.id,uin:h.uin,aid:null===(n=i.bean)||void 0===n?void 0:n.aid},e),t);}),null===(o=i.lifeCycle)||void 0===o||o.on("onConfigChange",function(e){p.setConfig(e);}),null===(r=i.lifeCycle)||void 0===r||r.on("destroy",function(){0===Xe.countInstance()&&(g.length=0,"function"==typeof((v=null)==p?void 0:p.destroy)&&p.destroy());}),p.on("PERREVENT",function(e){i.sendPipeline([function(e,t){var n=R({msg:S(e),level:O.INFO});t({type:T.OFFLINE,data:n,contentType:"application/x-www-form-urlencoded",method:"post",addBean:!1,url:h.url+"?id=893&sessionId="+We.sessionID+"&uin="+h.uin+"&from="+h.id+"&count=1&version=1.24.48",log:e});}],T.OFFLINE)(e);}),m&&(p.setConfig(m),m=null),v&&(null===(s=i.lifeCycle)||void 0===s||s.emit("uploadLogs",v.conds,v.params),v=null),i.flog=p;}catch(i){console.log(i);}}});}}),Fe=(new Y({name:"spa"}),new Y({name:"spa",init:function(){history.pushState=this.wr("pushState")||history.pushState,history.replaceState=this.wr("replaceState")||history.replaceState;var e=(null===(e=location.href)||void 0===e?void 0:e.split("?"))[0];this.$fireUrl=e,this.sendPv=this.sendPv.bind(this),this.onPageChange();},onPageChange:function(){window.addEventListener("replaceState",this.sendPv),window.addEventListener("pushState",this.sendPv),window.addEventListener("popstate",this.sendPv);},wr:function(n){var i=history[n];return "function"==typeof i&&!history.__hasWrittenByTamSpa&&(Object.defineProperty(history,"__hasWrittenByTamSpa",{value:!0,enumerable:!1}),function(){var e=i.apply(this,arguments),t=null;return "function"==typeof Event?t=new Event(n):(t=document.createEvent("HTMLEvents")).initEvent(n,!1,!0),window.dispatchEvent(t),e})},sendPv:function(){var n=this;setTimeout(function(){var t=location.href,e=(null===(e=location.href)||void 0===e?void 0:e.split("?"))[0];e&&e!==n.$fireUrl&&(n.$walk(function(e){e.send({url:e.config.pvUrl+"?from="+encodeURIComponent(t),beanFilter:["from"],type:T.WHITE_LIST});}),n.$fireUrl=e);},0);},destroy:function(){window.removeEventListener("replaceState",this.option.sendPv),window.removeEventListener("pushState",this.option.sendPv),window.removeEventListener("popstate",this.option.sendPv);}}));function Ge(e){var t,n,i,r=je.call(this,e)||this;r.requestQueue=[],r.requesting=!1,r.speedLogPipeline=K([G(r.config),B(r),(i=r,function(t,n){be(function(e){i.extendBean("netType",e),n(t);});}),function(e,t){r.lifeCycle.emit("beforeReportSpeed",e);var o=r.config.beforeReportSpeed;if((e="function"==typeof o?e.filter(function(e){var t,n,i=!1!==o(e);return "fetch"===e.type&&void 0===e.ret&&e.payload&&(t=(n=m(e.payload.data.response,r.config.api,{url:e.payload.data.url,ctx:e.payload.data})||{}).code,n=n.isErr,e.ret=t,e.isErr=+n),i}):e).length)return t(e)},function(e){return r.sendPipeline([function(e,t){t({type:T.SPEED,url:""+r.config.speedUrl,method:"post",data:Z(e,r.bean),log:e});}],T.SPEED)(e)}]),e.asyncPlugin=!0;try{"undefined"!=typeof document&&(e.uin=e.uin||(null!==(t=document.cookie.match(/\buin=\D+(\d*)/))&&void 0!==t?t:[])[1]||(null!==(n=document.cookie.match(/\bilive_uin=\D*(\d+)/))&&void 0!==n?n:[])[1]||""),r.init(e),r.extendBean("sessionId",Ge.sessionID),r.extendBean("from",encodeURIComponent(e.pageUrl||location.href)),"undefined"!=typeof document&&r.extendBean("referer",encodeURIComponent(document.referrer||"")),e.ext1&&r.extendBean("ext1",e.ext1),e.ext2&&r.extendBean("ext2",e.ext2),e.ext3&&r.extendBean("ext3",e.ext3);}catch(e){console.warn(e),console.log("%cThe above error occurred in the process of initializing Aegis, which will affect your normal use of Aegis.\nIt is recommended that you contact us for feedback and thank you for your support.","color: red"),r.sendSDKError(e);}return r}function Je(){this.constructor=qe;}return new Y({name:"ie"}),We.use(Re),We.use(se),We.use(te),We.use(Te),We.use(Be),We.use(z),We.use(we),We.use(Q),We.use(Fe),We});
    });

    class Jessibuca extends Emitter {
      static ERROR = EVENTS_ERROR;
      static TIMEOUT = {
        loadingTimeout: EVENTS.loadingTimeout,
        delayTimeout: EVENTS.delayTimeout
      };

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
        delete _opt.container; // s -> ms

        if (isNotEmpty(_opt.videoBuffer)) {
          _opt.videoBuffer = Number(_opt.videoBuffer) * 1000;
        } // setting


        if (isNotEmpty(_opt.timeout)) {
          if (isEmpty(_opt.loadingTimeout)) {
            _opt.loadingTimeout = _opt.timeout;
          }

          if (isEmpty(_opt.heartTimeout)) {
            _opt.heartTimeout = _opt.timeout;
          }
        }

        this._opt = _opt;
        this.$container = $container;
        this.href = null;
        this.events = new Events(this);
        this.player = new Player($container, _opt);

        this._bindEvents();

        this._initAegis();
      }

      _initAegis() {
        new aegis_min({
          id: '3ogWGfLmpllRGka9pY',
          // 上报 id
          // uin: uuid16(), // 用户唯一 ID（可选）
          reportApiSpeed: false,
          // 接口测速
          reportAssetSpeed: false,
          // 静态资源测速
          spa: false // spa 应用页面跳转的时候开启 pv 计算

        });
      }

      _bindEvents() {
        // 对外的事件
        Object.keys(JESSIBUCA_EVENTS).forEach(key => {
          this.player.on(JESSIBUCA_EVENTS[key], value => {
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
        this.player.mute(true);
      }
      /**
       *
       */


      cancelMute() {
        this.player.mute(false);
      }
      /**
       *
       * @param value {number}
       */


      setVolume(value) {
        this.player.volume = value;
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
        time = Number(time);
        this.player.updateOption({
          timeout: time,
          loadingTimeout: time,
          heartTimeout: time
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
        this.resize();
      }
      /**
       *
       * @returns {Promise<commander.ParseOptionsResult.unknown>}
       */


      pause() {
        return this.player.pause();
      }
      /**
       *
       */


      close() {
        // clear url
        this._opt.url = '';
        return this.player.close();
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
            this.emit(EVENTS.error, EVENTS_ERROR.playError);
            reject();
            return;
          }

          if (url) {
            // url 相等的时候。
            if (this._opt.url) {
              // 存在相同的 url
              if (url === this._opt.url) {
                // 正在播放
                if (this.player.playing) {
                  resolve();
                } else {
                  // pause ->  play
                  this.clearView();
                  this.player.play(this._opt.url).then(() => {
                    resolve();
                  }).catch(() => {
                    this.player.pause().then(() => {
                      reject();
                    });
                  });
                }
              } else {
                // url 发生改变了
                this.player.pause().then(() => {
                  // 清除 画面
                  this.clearView();
                  return this._play(url);
                }).catch(() => {
                  reject();
                });
              }
            } else {
              return this._play(url);
            }
          } else {
            //  url 不存在的时候
            //  就是从 play -> pause -> play
            this.player.play(this._opt.url).then(() => {
              resolve();
            }).catch(() => {
              this.player.pause().then(() => {
                reject();
              });
            });
          }
        });
      }
      /**
       *
       * @param url {string}
       * @returns {Promise<unknown>}
       * @private
       */


      _play(url) {
        return new Promise((resolve, reject) => {
          this._opt.url = url; //  新的url

          const isHttp = url.indexOf("http") === 0; //

          const protocol = isHttp ? PLAYER_PLAY_PROTOCOL.fetch : PLAYER_PLAY_PROTOCOL.websocket; //

          const demuxType = isHttp || url.indexOf(".flv") !== -1 || this._opt.isFlv ? DEMUX_TYPE.flv : DEMUX_TYPE.m7s;
          this.player.updateOption({
            protocol,
            demuxType
          });
          this.player.once(EVENTS_ERROR.mediaSourceH265NotSupport, () => {
            this.close();
          });
          this.player.once(EVENTS_ERROR.webcodecsH265NotSupport, () => {
            this.close();
          });

          if (this.hasLoaded()) {
            this.player.play(url).then(() => {
              resolve();
            }).catch(() => {
              this.player.pause().then(() => {
                reject();
              });
            });
          } else {
            this.player.once(EVENTS.decoderWorkerInit, () => {
              this.player.play(url).then(() => {
                resolve();
              }).catch(() => {
                this.player.pause().then(() => {
                  reject();
                });
              });
            });
          }
        });
      }
      /**
       *
       */


      resize() {
        this.player.resize();
      }
      /**
       *
       * @param time {number} s
       */


      setBufferTime(time) {
        time = Number(time); // s -> ms

        this.player.updateOption({
          videoBuffer: time * 1000
        });
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
      /**
       *
       * @returns {boolean}
       */


      hasLoaded() {
        return this.player.loaded;
      }
      /**
       *
       */


      setKeepScreenOn() {
        this.player.updateOption({
          keepScreenOn: true
        });
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
       * @param fileName {string}
       * @param fileType {string}
       * @returns {Promise<unknown>}
       */


      startRecord(fileName, fileType) {
        return new Promise((resolve, reject) => {
          if (this.player.playing) {
            this.player.startRecord(fileName, fileType);
            resolve();
          } else {
            reject();
          }
        });
      }

      stopRecordAndSave() {
        if (this.player.recording) {
          this.player.stopRecordAndSave();
        }
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
        return this.player.audio.isMute;
      }
      /**
       * 是否在录制视频
       * @returns {*}
       */


      isRecording() {
        return this.player.recorder.recording;
      }
      /**
       *
       */


      destroy() {
        this.player.destroy();
        this.player = null;
        this.off();
      }

    }

    window.Jessibuca = Jessibuca;

    return Jessibuca;

}));
//# sourceMappingURL=jessibuca.js.map
