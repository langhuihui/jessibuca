(function (factory) {
    typeof define === 'function' && define.amd ? define(factory) :
    factory();
})((function () { 'use strict';

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

    // 播放协议
    const DEBUG_LEVEL = {
      debug: 'debug',
      warn: 'warn'
    };

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
      zooming: 'zooming',
      videoInfo: 'videoInfo',
      timeUpdate: 'timeUpdate',
      audioInfo: "audioInfo",
      log: 'log',
      error: "error",
      kBps: 'kBps',
      timeout: 'timeout',
      delayTimeout: 'delayTimeout',
      delayTimeoutRetryEnd: 'delayTimeoutRetryEnd',
      loadingTimeout: 'loadingTimeout',
      loadingTimeoutRetryEnd: 'loadingTimeoutRetryEnd',
      stats: 'stats',
      performance: "performance",
      faceDetectActive: 'faceDetectActive',
      objectDetectActive: 'objectDetectActive',
      // record
      record: 'record',
      recording: 'recording',
      recordingTimestamp: 'recordingTimestamp',
      recordStart: 'recordStart',
      recordEnd: 'recordEnd',
      recordCreateError: 'recordCreateError',
      recordBlob: 'recordBlob',
      buffer: 'buffer',
      videoFrame: 'videoFrame',
      start: 'start',
      metadata: 'metadata',
      resize: 'resize',
      volumechange: 'volumechange',
      destroy: 'destroy',
      beforeDestroy: 'beforeDestroy',
      // stream
      streamEnd: 'streamEnd',
      streamRate: 'streamRate',
      streamAbps: 'streamAbps',
      streamVbps: 'streamVbps',
      streamDts: 'streamDts',
      streamSuccess: 'streamSuccess',
      streamMessage: 'streamMessage',
      streamError: 'streamError',
      streamStats: 'streamStats',
      // MSE
      mseSourceOpen: 'mseSourceOpen',
      mseSourceClose: 'mseSourceClose',
      mseSourceended: 'mseSourceended',
      mseSourceBufferError: 'mseSourceBufferError',
      mseAddSourceBufferError: 'mseAddSourceBufferError',
      mseSourceBufferBusy: 'mseSourceBufferBusy',
      mseSourceBufferFull: 'mseSourceBufferFull',
      // VIDEO
      videoWaiting: 'videoWaiting',
      videoTimeUpdate: 'videoTimeUpdate',
      videoSyncAudio: 'videoSyncAudio',
      //
      playToRenderTimes: 'playToRenderTimes',
      playbackTime: 'playbackTime',
      playbackTimestamp: 'playbackTimestamp',
      playbackPrecision: 'playbackPrecision',
      playbackJustTime: 'playbackJustTime',
      playbackStats: 'playbackStats',
      playbackSeek: 'playbackSeek',
      playbackPause: 'playbackPause',
      playbackPauseOrResume: 'playbackPauseOrResume',
      playbackRateChange: 'playbackRateChange',
      playbackPreRateChange: 'playbackPreRateChange',
      ptz: 'ptz',
      streamQualityChange: 'streamQualityChange',
      visibilityChange: "visibilityChange",
      netBuf: 'netBuf',
      close: 'close',
      networkDelayTimeout: 'networkDelayTimeout',
      togglePerformancePanel: 'togglePerformancePanel',
      viewResizeChange: 'viewResizeChange',
      flvDemuxBufferSizeTooLarge: 'flvDemuxBufferSizeTooLarge',
      // talk
      talkGetUserMediaSuccess: 'talkGetUserMediaSuccess',
      talkGetUserMediaFail: 'talkGetUserMediaFail',
      talkGetUserMediaTimeout: 'talkGetUserMediaTimeout',
      talkStreamStart: 'talkStreamStart',
      talkStreamOpen: 'talkStreamOpen',
      talkStreamClose: 'talkStreamClose',
      talkStreamError: 'talkStreamError',
      talkStreamInactive: 'talkStreamInactive',
      webrtcDisconnect: 'webrtcDisconnect',
      webrtcFailed: 'webrtcFailed',
      webrtcClosed: 'webrtcClosed',
      // crash
      crashLog: 'crashLog',
      // dom
      focus: 'focus',
      blur: 'blur',
      visibilityHiddenTimeout: 'visibilityHiddenTimeout',
      // websocket
      websocketOpen: 'websocketOpen',
      websocketClose: 'websocketClose',
      websocketError: 'websocketError',
      websocketMessage: 'websocketMessage',
      // ai
      aiObjectDetectorInfo: 'aiObjectDetectorInfo',
      aiFaceDetector: 'aiFaceDetector',
      //
      playFailedAndPaused: 'playFailedAndPaused',
      // audio
      audioResumeState: 'audioResumeState'
    }; // jessibuca events
    const TALK_EVENTS = {
      talkStreamClose: EVENTS.talkStreamClose,
      talkStreamError: EVENTS.talkStreamError,
      talkStreamInactive: EVENTS.talkStreamInactive,
      talkGetUserMediaTimeout: EVENTS.talkGetUserMediaTimeout
    };
    const EVENTS_ERROR = {
      playError: 'playIsNotPauseOrUrlIsNull',
      fetchError: "fetchError",
      websocketError: 'websocketError',
      webcodecsH265NotSupport: 'webcodecsH265NotSupport',
      webcodecsDecodeError: 'webcodecsDecodeError',
      webcodecsUnsupportedConfigurationError: 'webcodecsUnsupportedConfigurationError',
      mediaSourceH265NotSupport: 'mediaSourceH265NotSupport',
      mediaSourceDecoderConfigurationError: 'mediaSourceDecoderConfigurationError',
      mediaSourceFull: EVENTS.mseSourceBufferFull,
      mseSourceBufferError: EVENTS.mseSourceBufferError,
      mseAddSourceBufferError: EVENTS.mseAddSourceBufferError,
      mediaSourceAppendBufferError: 'mediaSourceAppendBufferError',
      mediaSourceBufferListLarge: 'mediaSourceBufferListLarge',
      mediaSourceAppendBufferEndTimeout: 'mediaSourceAppendBufferEndTimeout',
      mediaSourceTsIsMaxDiff: 'mediaSourceTsIsMaxDiff',
      mediaSourceUseCanvasRenderPlayFailed: 'mediaSourceUseCanvasRenderPlayFailed',
      wasmDecodeError: 'wasmDecodeError',
      wasmUseVideoRenderError: 'wasmUseVideoRenderError',
      hlsError: 'hlsError',
      webrtcError: 'webrtcError',
      webglAlignmentError: 'webglAlignmentError',
      wasmWidthOrHeightChange: 'wasmWidthOrHeightChange',
      mseWidthOrHeightChange: 'mseWidthOrHeightChange',
      wcsWidthOrHeightChange: 'wcsWidthOrHeightChange',
      tallWebsocketClosedByError: 'tallWebsocketClosedByError',
      flvDemuxBufferSizeTooLarge: EVENTS.flvDemuxBufferSizeTooLarge,
      wasmDecodeVideoNoResponseError: 'wasmDecodeVideoNoResponseError',
      audioChannelError: 'audioChannelError',
      simdH264DecodeVideoWidthIsTooLarge: 'simdH264DecodeVideoWidthIsTooLarge',
      webglContextLostError: 'webglContextLostError'
    };
    const WEBSOCKET_STATUS = {
      notConnect: 'notConnect',
      open: 'open',
      close: 'close',
      error: 'error'
    };
    const TALK_ENC_TYPE = {
      g711a: 'g711a',
      g711u: 'g711u'
    };
    //     RTP_PAYLOAD_TYPE_PCMA    = 8,   // g711a
    //     RTP_PAYLOAD_TYPE_JPEG    = 26,
    //     RTP_PAYLOAD_TYPE_H264    = 96,
    //     RTP_PAYLOAD_TYPE_H265    = 97,
    //     RTP_PAYLOAD_TYPE_OPUS    = 98,
    //     RTP_PAYLOAD_TYPE_AAC     = 99,
    //     RTP_PAYLOAD_TYPE_G726    = 100,
    //     RTP_PAYLOAD_TYPE_G726_16 = 101,
    //     RTP_PAYLOAD_TYPE_G726_24 = 102,
    //     RTP_PAYLOAD_TYPE_G726_32 = 103,
    //     RTP_PAYLOAD_TYPE_G726_40 = 104,
    //     RTP_PAYLOAD_TYPE_SPEEX   = 105,

    const RTP_PAYLOAD_TYPE = {
      pcma: 8,
      g711a: 8,
      pcmu: 0,
      g711u: 0,
      jpeg: 26,
      h264: 96,
      h265: 97,
      opus: 98
    };
    const TALK_PACKET_TYPE = {
      empty: 'empty',
      // 裸的协议
      rtp: 'rtp',
      opus: 'opus'
    };
    const WEBSOCKET_EVENTS = {
      open: 'open',
      close: 'close',
      error: 'error',
      message: 'message'
    };
    const TALK_ENGINE = {
      worklet: 'worklet',
      script: 'script'
    }; // default talk options

    const DEFAULT_TALK_OPTIONS = {
      encType: TALK_ENC_TYPE.g711a,
      packetType: TALK_PACKET_TYPE.rtp,
      // 默认的包个格式化
      rtpSsrc: '0000000000',
      // 10 位
      numberChannels: 1,
      // 采样通道
      sampleRate: 8000,
      // 采样率
      sampleBitsWidth: 16,
      // 采样精度
      debug: false,
      debugLevel: DEBUG_LEVEL.warn,
      // debug level
      testMicrophone: false,
      // 测试麦克风获取
      audioBufferLength: 160,
      // 默认走的是 20ms 8000 采样率 16 位精度
      engine: TALK_ENGINE.worklet,
      //
      checkGetUserMediaTimeout: false,
      // 检测 getUserMedia 超时
      getUserMediaTimeout: 10 * 1000 // getUserMedia 超时时间 10s

    };

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
    function isFunction(fn) {
      return typeof fn === "function";
    }
    function isSupportGetUserMedia() {
      let result = false;
      const navigator = window.navigator;

      if (navigator) {
        result = !!(navigator.mediaDevices && navigator.mediaDevices.getUserMedia);

        if (!result) {
          result = !!(navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia);
        }
      }

      return result;
    }
    function createWorkletModuleUrl(func) {
      function functionToString(str) {
        return str.trim().match(/^function\s*\w*\s*\([\w\s,]*\)\s*{([\w\W]*?)}$/)[1];
      }

      const funcStr = functionToString(func.toString());
      const blob = new Blob([funcStr], {
        type: 'application/javascript'
      });
      return URL.createObjectURL(blob);
    }
    function clone(obj) {
      let result = ''; //

      if (typeof obj === 'object') {
        try {
          result = JSON.stringify(obj);
          result = JSON.parse(result);
        } catch (e) {
          result = obj;
        }
      } else {
        result = obj;
      }

      return result;
    }
    /**
     *
     * @returns {object:DEFAULT_TALK_OPTIONS}
     */

    function getDefaultTalkOptions() {
      return clone(DEFAULT_TALK_OPTIONS);
    }

    class Resampler {
      constructor(options) {
        const {
          fromSampleRate,
          toSampleRate,
          channels,
          inputBufferSize
        } = options;

        if (!fromSampleRate || !toSampleRate || !channels) {
          throw new Error("Invalid settings specified for the resampler.");
        }

        this.resampler = null;
        this.fromSampleRate = fromSampleRate;
        this.toSampleRate = toSampleRate;
        this.channels = channels || 0;
        this.inputBufferSize = inputBufferSize;
        this.initialize();
      }

      initialize() {
        if (this.fromSampleRate == this.toSampleRate) {
          // Setup resampler bypass - Resampler just returns what was passed through
          this.resampler = buffer => {
            return buffer;
          };

          this.ratioWeight = 1;
        } else {
          if (this.fromSampleRate < this.toSampleRate) {
            // Use generic linear interpolation if upsampling,
            // as linear interpolation produces a gradient that we want
            // and works fine with two input sample points per output in this case.
            this.linearInterpolation();
            this.lastWeight = 1;
          } else {
            // Custom resampler I wrote that doesn't skip samples
            // like standard linear interpolation in high downsampling.
            // This is more accurate than linear interpolation on downsampling.
            this.multiTap();
            this.tailExists = false;
            this.lastWeight = 0;
          } // Initialize the internal buffer:


          this.initializeBuffers();
          this.ratioWeight = this.fromSampleRate / this.toSampleRate;
        }
      }

      bufferSlice(sliceAmount) {
        //Typed array and normal array buffer section referencing:
        try {
          return this.outputBuffer.subarray(0, sliceAmount);
        } catch (error) {
          try {
            //Regular array pass:
            this.outputBuffer.length = sliceAmount;
            return this.outputBuffer;
          } catch (error) {
            //Nightly Firefox 4 used to have the subarray function named as slice:
            return this.outputBuffer.slice(0, sliceAmount);
          }
        }
      }

      initializeBuffers() {
        this.outputBufferSize = Math.ceil(this.inputBufferSize * this.toSampleRate / this.fromSampleRate / this.channels * 1.000000476837158203125) + this.channels + this.channels;

        try {
          this.outputBuffer = new Float32Array(this.outputBufferSize);
          this.lastOutput = new Float32Array(this.channels);
        } catch (error) {
          this.outputBuffer = [];
          this.lastOutput = [];
        }
      }

      linearInterpolation() {
        this.resampler = buffer => {
          let bufferLength = buffer.length,
              channels = this.channels,
              outLength,
              ratioWeight,
              weight,
              firstWeight,
              secondWeight,
              sourceOffset,
              outputOffset,
              outputBuffer,
              channel;

          if (bufferLength % channels !== 0) {
            throw new Error("Buffer was of incorrect sample length.");
          }

          if (bufferLength <= 0) {
            return [];
          }

          outLength = this.outputBufferSize;
          ratioWeight = this.ratioWeight;
          weight = this.lastWeight;
          firstWeight = 0;
          secondWeight = 0;
          sourceOffset = 0;
          outputOffset = 0;
          outputBuffer = this.outputBuffer;

          for (; weight < 1; weight += ratioWeight) {
            secondWeight = weight % 1;
            firstWeight = 1 - secondWeight;
            this.lastWeight = weight % 1;

            for (channel = 0; channel < this.channels; ++channel) {
              outputBuffer[outputOffset++] = this.lastOutput[channel] * firstWeight + buffer[channel] * secondWeight;
            }
          }

          weight -= 1;

          for (bufferLength -= channels, sourceOffset = Math.floor(weight) * channels; outputOffset < outLength && sourceOffset < bufferLength;) {
            secondWeight = weight % 1;
            firstWeight = 1 - secondWeight;

            for (channel = 0; channel < this.channels; ++channel) {
              outputBuffer[outputOffset++] = buffer[sourceOffset + (channel > 0 ? channel : 0)] * firstWeight + buffer[sourceOffset + (channels + channel)] * secondWeight;
            }

            weight += ratioWeight;
            sourceOffset = Math.floor(weight) * channels;
          }

          for (channel = 0; channel < channels; ++channel) {
            this.lastOutput[channel] = buffer[sourceOffset++];
          }

          return this.bufferSlice(outputOffset);
        };
      }

      multiTap() {
        this.resampler = buffer => {
          let bufferLength = buffer.length,
              outLength,
              output_variable_list,
              channels = this.channels,
              ratioWeight,
              weight,
              channel,
              actualPosition,
              amountToNext,
              alreadyProcessedTail,
              outputBuffer,
              outputOffset,
              currentPosition;

          if (bufferLength % channels !== 0) {
            throw new Error("Buffer was of incorrect sample length.");
          }

          if (bufferLength <= 0) {
            return [];
          }

          outLength = this.outputBufferSize;
          output_variable_list = [];
          ratioWeight = this.ratioWeight;
          weight = 0;
          actualPosition = 0;
          amountToNext = 0;
          alreadyProcessedTail = !this.tailExists;
          this.tailExists = false;
          outputBuffer = this.outputBuffer;
          outputOffset = 0;
          currentPosition = 0;

          for (channel = 0; channel < channels; ++channel) {
            output_variable_list[channel] = 0;
          }

          do {
            if (alreadyProcessedTail) {
              weight = ratioWeight;

              for (channel = 0; channel < channels; ++channel) {
                output_variable_list[channel] = 0;
              }
            } else {
              weight = this.lastWeight;

              for (channel = 0; channel < channels; ++channel) {
                output_variable_list[channel] = this.lastOutput[channel];
              }

              alreadyProcessedTail = true;
            }

            while (weight > 0 && actualPosition < bufferLength) {
              amountToNext = 1 + actualPosition - currentPosition;

              if (weight >= amountToNext) {
                for (channel = 0; channel < channels; ++channel) {
                  output_variable_list[channel] += buffer[actualPosition++] * amountToNext;
                }

                currentPosition = actualPosition;
                weight -= amountToNext;
              } else {
                for (channel = 0; channel < channels; ++channel) {
                  output_variable_list[channel] += buffer[actualPosition + (channel > 0 ? channel : 0)] * weight;
                }

                currentPosition += weight;
                weight = 0;
                break;
              }
            }

            if (weight === 0) {
              for (channel = 0; channel < channels; ++channel) {
                outputBuffer[outputOffset++] = output_variable_list[channel] / ratioWeight;
              }
            } else {
              this.lastWeight = weight;

              for (channel = 0; channel < channels; ++channel) {
                this.lastOutput[channel] = output_variable_list[channel];
              }

              this.tailExists = true;
              break;
            }
          } while (actualPosition < bufferLength && outputOffset < outLength);

          return this.bufferSlice(outputOffset);
        };
      }

      resample(buffer) {
        if (this.fromSampleRate == this.toSampleRate) {
          this.ratioWeight = 1;
        } else {
          if (this.fromSampleRate < this.toSampleRate) {
            this.lastWeight = 1;
          } else {
            this.tailExists = false;
            this.lastWeight = 0;
          }

          this.initializeBuffers();
          this.ratioWeight = this.fromSampleRate / this.toSampleRate;
        }

        return this.resampler(buffer);
      }

    }

    /**
     * 将无符号Float32Array数组转化成有符号的Int16Array数组
     * @param {Float32Array} input unsinged Float32Array
     * @return {Int16Array} singed int16
     */
    function floatTo16BitPCM(input) {
      let i = input.length;
      let output = new Int16Array(i);

      while (i--) {
        let s = Math.max(-1, Math.min(1, input[i]));
        output[i] = s < 0 ? s * 0x8000 : s * 0x7FFF;
      }

      return output;
    }
    /**
     * 将无符号Float32Array数组转化成有符号的Int8Array数组
     * @param {Float32Array} input unsinged Float32Array
     * @return {Int8Array} singed int8
     */

    function floatTo8BitPCM(input) {
      let i = input.length;
      let output = new Int8Array(i);

      while (i--) {
        let s = Math.max(-1, Math.min(1, input[i]));
        const temp = s < 0 ? s * 0x8000 : s * 0x7FFF;
        output[i] = parseInt(255 / (65535 / (32768 + temp)), 10);
      }

      return output;
    }

    const QUANT_MASK = 0xf;
    const SEG_SHIFT = 4;
    const BIAS = 0x84;
    const segEnd = [0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF];

    function _search(val, table, size) {
      for (let i = 0; i < size; i++) {
        if (val <= table[i]) {
          return i;
        }
      }

      return size;
    } // alaw


    function _linear2alaw(pcmVal) {
      let mask;
      let seg;
      let aval;

      if (pcmVal >= 0) {
        mask = 0xD5;
      } else {
        mask = 0x55; // pcmVal = -pcmVal - 8;

        pcmVal = -pcmVal - 1;

        if (pcmVal < 0) {
          pcmVal = 32767;
        }
      }
      /* Convert the scaled magnitude to segment number. */


      seg = _search(pcmVal, segEnd, 8);

      if (seg >= 8) {
        return 0x7F ^ mask;
      } else {
        aval = seg << SEG_SHIFT;

        if (seg < 2) {
          aval |= pcmVal >> 4 & QUANT_MASK;
        } else {
          aval |= pcmVal >> seg + 3 & QUANT_MASK;
        }

        return aval ^ mask;
      }
    }


    function _linear2ulaw(pcmVal) {
      let mask = 0;

      if (pcmVal < 0) {
        pcmVal = BIAS - pcmVal;
        mask = 0x7F;
      } else {
        pcmVal += BIAS;
        mask = 0xFF;
      }

      let seg = _search(pcmVal, segEnd, 8);

      if (seg >= 8) {
        return 0x7F ^ mask;
      } else {
        let uval = seg << 4 | pcmVal >> seg + 3 & 0xF;
        return uval ^ mask;
      }
    }


    function g711aEncoder(typedArray) {
      const g711Array = [];
      const tempArray = Array.prototype.slice.call(typedArray);
      tempArray.forEach((i, index) => {
        g711Array[index] = _linear2alaw(i);
      });
      return g711Array;
    } // g711a to pcm(float32Array)

    function g711uEncoder(typedArray) {
      const g711Array = [];
      const tempArray = Array.prototype.slice.call(typedArray);
      tempArray.forEach((i, index) => {
        g711Array[index] = _linear2ulaw(i);
      });
      return g711Array;
    } // g711u

    class Debug {
      constructor(master) {
        this.log = function (name) {
          if (master._opt.debug && master._opt.debugLevel == DEBUG_LEVEL.debug) {
            const prefix = master._opt.debugUuid ? `[${master._opt.debugUuid}]` : '';

            for (var _len = arguments.length, args = new Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
              args[_key - 1] = arguments[_key];
            }

            console.log(`JbPro${prefix}:[\u2705\u2705\u2705][${name}]`, ...args);
          }
        };

        this.warn = function (name) {
          if (master._opt.debug && (master._opt.debugLevel == DEBUG_LEVEL.debug || master._opt.debugLevel == DEBUG_LEVEL.warn)) {
            const prefix = master._opt.debugUuid ? `[${master._opt.debugUuid}]` : '';

            for (var _len2 = arguments.length, args = new Array(_len2 > 1 ? _len2 - 1 : 0), _key2 = 1; _key2 < _len2; _key2++) {
              args[_key2 - 1] = arguments[_key2];
            }

            console.log(`JbPro${prefix}:[\u2757\u2757\u2757][${name}]`, ...args);
          }
        };

        this.error = function (name) {
          const prefix = master._opt.debugUuid ? `[${master._opt.debugUuid}]` : '';

          for (var _len3 = arguments.length, args = new Array(_len3 > 1 ? _len3 - 1 : 0), _key3 = 1; _key3 < _len3; _key3++) {
            args[_key3 - 1] = arguments[_key3];
          }

          console.error(`JbPro${prefix}:[\u274C\u274C\u274C][${name}]`, ...args);
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

        const destroy = () => {
          if (isFunction(target.removeEventListener)) {
            target.removeEventListener(name, callback, option);
          }
        };

        this.destroys.push(destroy);
        return destroy;
      }

      destroy() {
        this.master.debug && this.master.debug.log(`Events`, 'destroy');
        this.destroys.forEach(event => event());
      }

    }

    class Talk extends Emitter {
      constructor(player) {
        let options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
        super();

        if (player) {
          this.player = player;
        }

        this.tag = 'talk';
        const defaultOptions = getDefaultTalkOptions();
        this._opt = Object.assign({}, defaultOptions, options);
        this._opt.sampleRate = parseInt(this._opt.sampleRate, 10);
        this._opt.sampleBitsWidth = parseInt(this._opt.sampleBitsWidth, 10);
        this.audioContext = null;
        this.gainNode = null;
        this.recorder = null;
        this.workletRecorder = null;
        this.biquadFilter = null;
        this.userMediaStream = null; // buffersize

        this.bufferSize = 512;
        this._opt.audioBufferLength = this.calcAudioBufferLength();
        this.audioBufferList = []; // socket

        this.socket = null;
        this.socketStatus = WEBSOCKET_STATUS.notConnect; //

        this.mediaStreamSource = null;
        this.heartInterval = null;
        this.checkGetUserMediaTimeout = null;
        this.wsUrl = null;
        this.startTimestamp = 0; // 报文数据

        this.sequenceId = 0;
        this.tempTimestamp = null;
        this.tempRtpBufferList = [];
        this.events = new Events(this);

        this._initTalk(); //


        if (!this.player) {
          this.debug = new Debug(this);
        }

        this.log(this.tag, 'init', this._opt);
      }

      destroy() {
        //
        if (this.userMediaStream) {
          this.userMediaStream.getTracks && this.userMediaStream.getTracks().forEach(track => {
            track.stop();
          });
          this.userMediaStream = null;
        }

        if (this.mediaStreamSource) {
          this.mediaStreamSource.disconnect();
          this.mediaStreamSource = null;
        }

        if (this.recorder) {
          this.recorder.disconnect();
          this.recorder.onaudioprocess = null;
        }

        if (this.biquadFilter) {
          this.biquadFilter.disconnect();
          this.biquadFilter = null;
        }

        if (this.gainNode) {
          this.gainNode.disconnect();
          this.gainNode = null;
        }

        if (this.workletRecorder) {
          this.workletRecorder.disconnect();
          this.workletRecorder = null;
        }

        if (this.socket) {
          if (this.socketStatus === WEBSOCKET_STATUS.open) {
            this._sendClose();
          }

          this.socket.close();
          this.socket = null;
        }

        this._stopHeartInterval();

        this._stopCheckGetUserMediaTimeout();

        this.audioContext = null;
        this.gainNode = null;
        this.recorder = null;
        this.audioBufferList = [];
        this.sequenceId = 0;
        this.wsUrl = null;
        this.tempTimestamp = null;
        this.tempRtpBufferList = [];
        this.startTimestamp = 0;
        this.log('talk', 'destroy');
      }

      addRtpToBuffer(rtp) {
        const len = rtp.length + this.tempRtpBufferList.length;
        const buffer = new Uint8Array(len);
        buffer.set(this.tempRtpBufferList, 0);
        buffer.set(rtp, this.tempRtpBufferList.length);
        this.tempRtpBufferList = buffer; //console.log('addRtpToBuffer length and byteLength ', this.tempRtpBufferList.length, this.tempRtpBufferList.byteLength)
      }

      downloadRtpFile() {
        const blob = new Blob([this.tempRtpBufferList]);

        try {
          const oa = document.createElement('a');
          oa.href = window.URL.createObjectURL(blob);
          oa.download = Date.now() + '.rtp';
          oa.click();
        } catch (e) {
          console.error('downloadRtpFile', e);
        }
      }

      calcAudioBufferLength() {
        const {
          sampleRate,
          sampleBitsWidth
        } = this._opt;
        return sampleRate * 8 * (20 / 1000) / 8;
      }

      get socketStatusOpen() {
        return this.socketStatus === WEBSOCKET_STATUS.open;
      }

      log() {
        for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
          args[_key] = arguments[_key];
        }

        this._log('log', ...args);
      }

      warn() {
        for (var _len2 = arguments.length, args = new Array(_len2), _key2 = 0; _key2 < _len2; _key2++) {
          args[_key2] = arguments[_key2];
        }

        this._log('warn', ...args);
      }

      error() {
        for (var _len3 = arguments.length, args = new Array(_len3), _key3 = 0; _key3 < _len3; _key3++) {
          args[_key3] = arguments[_key3];
        }

        this._log('error', ...args);
      }

      _log(type) {
        for (var _len4 = arguments.length, args = new Array(_len4 > 1 ? _len4 - 1 : 0), _key4 = 1; _key4 < _len4; _key4++) {
          args[_key4 - 1] = arguments[_key4];
        }

        if (this.player) {
          this.player.debug[type](...args);
        } else if (this.debug) {
          this.debug[type](...args);
        } else {
          console[type](...args);
        }
      }

      _getSequenceId() {
        return ++this.sequenceId;
      }

      _createWebSocket() {
        return new Promise((resolve, reject) => {
          const proxy = this.events.proxy;
          this.socket = new WebSocket(this.wsUrl);
          this.socket.binaryType = 'arraybuffer';
          this.emit(EVENTS.talkStreamStart);
          proxy(this.socket, WEBSOCKET_EVENTS.open, () => {
            this.socketStatus = WEBSOCKET_STATUS.open;
            this.log(this.tag, 'websocket open -> do talk');
            this.emit(EVENTS.talkStreamOpen);
            resolve();

            this._doTalk();
          });
          proxy(this.socket, WEBSOCKET_EVENTS.message, event => {
            this.log(this.tag, 'websocket message', event.data);
          });
          proxy(this.socket, WEBSOCKET_EVENTS.close, e => {
            this.socketStatus = WEBSOCKET_STATUS.close;
            this.log(this.tag, 'websocket close');
            this.emit(EVENTS.talkStreamClose);
            reject(e);
          });
          proxy(this.socket, WEBSOCKET_EVENTS.error, error => {
            this.socketStatus = WEBSOCKET_STATUS.error;
            this.error(this.tag, 'websocket error', error);
            this.emit(EVENTS.talkStreamError, error);
            reject(error);
          });
        });
      }

      _sendClose() {}

      _initTalk() {
        this._initMethods();

        if (this._opt.engine === TALK_ENGINE.worklet) {
          this._initWorklet();
        } else if (this._opt.engine === TALK_ENGINE.script) {
          this._initScriptProcessor();
        }

        this.log(this.tag, 'audioContext samplerate', this.audioContext.sampleRate);
      }

      _initMethods() {
        //
        this.audioContext = new (window.AudioContext || window.webkitAudioContext)({
          sampleRate: 48000
        });
        this.gainNode = this.audioContext.createGain(); // default 1

        this.gainNode.gain.value = 1; // 消音器

        this.biquadFilter = this.audioContext.createBiquadFilter();
        this.biquadFilter.type = "lowpass";
        this.biquadFilter.frequency.value = 3000;
        this.resampler = new Resampler({
          fromSampleRate: this.audioContext.sampleRate,
          toSampleRate: this._opt.sampleRate,
          channels: this._opt.numberChannels,
          inputBufferSize: this.bufferSize
        });
      }

      _initScriptProcessor() {
        //
        const createScript = this.audioContext.createScriptProcessor || this.audioContext.createJavaScriptNode;
        this.recorder = createScript.apply(this.audioContext, [this.bufferSize, this._opt.numberChannels, this._opt.numberChannels]);

        this.recorder.onaudioprocess = e => this._onaudioprocess(e);
      }

      _initWorklet() {
        function workletProcess() {
          class TalkProcessor extends AudioWorkletProcessor {
            constructor(options) {
              super();
              this._cursor = 0;
              this._bufferSize = options.processorOptions.bufferSize;
              this._buffer = new Float32Array(this._bufferSize);
            }

            process(inputs, outputs, parameters) {
              if (!inputs.length || !inputs[0].length) {
                return true;
              }

              for (let i = 0; i < inputs[0][0].length; i++) {
                this._cursor += 1;

                if (this._cursor === this._bufferSize) {
                  this._cursor = 0;
                  this.port.postMessage({
                    eventType: 'data',
                    buffer: this._buffer
                  });
                }

                this._buffer[this._cursor] = inputs[0][0][i];
              }

              return true;
            }

          }

          registerProcessor('talk-processor', TalkProcessor);
        }

        this.audioContext.audioWorklet.addModule(createWorkletModuleUrl(workletProcess)).then(() => {
          const workletNode = new AudioWorkletNode(this.audioContext, 'talk-processor', {
            processorOptions: {
              bufferSize: this.bufferSize
            }
          });
          workletNode.connect(this.gainNode);

          workletNode.port.onmessage = e => {
            if (e.data.eventType === 'data') {
              this._encodeAudioData(e.data.buffer);
            }
          };

          this.workletRecorder = workletNode;
        });
      }

      _onaudioprocess(e) {
        // 数组里的每个数字都是32位的单精度浮点数
        // 默认是单精度
        const float32Array = e.inputBuffer.getChannelData(0); // send 出去。

        this._encodeAudioData(new Float32Array(float32Array));
      }

      _encodeAudioData(float32Array) {
        // 没有说话
        if (float32Array[0] === 0 && float32Array[1] === 0) {
          this.log(this.tag, 'empty audio data');
          return;
        }

        const resampleBuffer = this.resampler.resample(float32Array); // default 32Bit

        let tempArrayBuffer = resampleBuffer;

        if (this._opt.sampleBitsWidth === 16) {
          tempArrayBuffer = floatTo16BitPCM(resampleBuffer);
        } else if (this._opt.sampleBitsWidth === 8) {
          tempArrayBuffer = floatTo8BitPCM(resampleBuffer);
        }

        if (tempArrayBuffer.buffer !== null) {
          let typedArray = null;

          if (this._opt.encType === TALK_ENC_TYPE.g711a) {
            typedArray = g711aEncoder(tempArrayBuffer);
          } else if (this._opt.encType === TALK_ENC_TYPE.g711u) {
            typedArray = g711uEncoder(tempArrayBuffer);
          } //


          const unit8Array = Uint8Array.from(typedArray);

          for (let i = 0; i < unit8Array.length; i++) {
            let audioBufferLength = this.audioBufferList.length;
            this.audioBufferList[audioBufferLength++] = unit8Array[i]; //

            if (this.audioBufferList.length === this._opt.audioBufferLength) {
              this._sendTalkMsg(new Uint8Array(this.audioBufferList));

              this.audioBufferList = [];
            }
          }
        }
      }

      _parseAudioMsg(typedArray) {
        let typeArray2 = null; // rtp reumx just support g711a or g711u

        if (this._opt.packetType === TALK_PACKET_TYPE.rtp && (this._opt.encType === TALK_ENC_TYPE.g711a || this._opt.encType === TALK_ENC_TYPE.g711u)) {
          typeArray2 = this.rtpPacket(typedArray);
        } else if (this._opt.packetType === TALK_PACKET_TYPE.opus) {
          typeArray2 = this.opusPacket(typedArray);
        } else if (this._opt.packetType === TALK_PACKET_TYPE.empty) {
          // 默认
          typeArray2 = typedArray;
        }

        return typeArray2;
      }

      rtpPacket(typedArray) {
        const rtpHeader = []; //2 bits RTP的版本，这里统一为2

        const version = 2; //1 bit 如果置1，在packet的末尾被填充，填充有时是方便一些针对固定长度的算法的封装

        const padding = 0; //1 bit 如果置1，在RTP Header会跟着一个header extension

        const extension = 0; //4 bits 表示头部后 特约信源 的个数

        const csrcCount = 0; //1 bit 不同的有效载荷有不同的含义，marker=1; 对于视频，标记一帧的结束；对于音频，标记会话的开始。

        const marker = 1; //7 bits 表示所传输的多媒体的类型，

        let playloadType = 0; //16 bits 每个RTP packet的sequence number会自动加一，以便接收端检测丢包情况

        let sequenceNumber = 0; //32 bits 时间戳

        let timestamp = 0; //32 bits 同步源的id，每两个同步源的id不能相同

        const ssrc = this._opt.rtpSsrc; //

        const frameLen = typedArray.length;

        if (this._opt.encType === TALK_ENC_TYPE.g711a) {
          playloadType = RTP_PAYLOAD_TYPE.g711a;
        } else if (this._opt.encType === TALK_ENC_TYPE.g711u) {
          playloadType = RTP_PAYLOAD_TYPE.g711u;
        }

        if (!this.startTimestamp) {
          this.startTimestamp = now();
        }

        timestamp = now() - this.startTimestamp;
        sequenceNumber = this._getSequenceId(); // frame length
        // 需要在rtp头前面加两个字节，表示数据包长度（整个rtp包长度）
        // 国标流udp不需要两个字节长度
        // 国标流tcp需要两个字节长度
        // websocket目前是按照tcp的做法做的

        const rtpFrameLen = frameLen + 12;
        rtpHeader[0] = 0xFF & rtpFrameLen >> 8;
        rtpHeader[1] = 0xFF & rtpFrameLen >> 0;
        rtpHeader[2] = (version << 6) + (padding << 5) + (extension << 4) + csrcCount;
        rtpHeader[3] = (marker << 7) + playloadType;
        rtpHeader[4] = sequenceNumber / (0xff + 1);
        rtpHeader[5] = sequenceNumber % (0xff + 1);
        rtpHeader[6] = timestamp / (0xffff + 1) / (0xff + 1);
        rtpHeader[7] = timestamp / (0xffff + 1) % (0xff + 1);
        rtpHeader[8] = timestamp % (0xffff + 1) / (0xff + 1);
        rtpHeader[9] = timestamp % (0xffff + 1) % (0xff + 1);
        rtpHeader[10] = ssrc / (0xffff + 1) / (0xff + 1);
        rtpHeader[11] = ssrc / (0xffff + 1) % (0xff + 1);
        rtpHeader[12] = ssrc % (0xffff + 1) / (0xff + 1);
        rtpHeader[13] = ssrc % (0xffff + 1) % (0xff + 1);
        let typeArray2 = rtpHeader.concat([...typedArray]);
        let binary = new Uint8Array(typeArray2.length);

        for (let ii = 0; ii < typeArray2.length; ii++) {
          binary[ii] = typeArray2[ii];
        }

        return binary;
      }

      opusPacket(typedArray) {
        //TODO:待完成
        return typedArray;
      }

      _sendTalkMsg(typedArray) {
        if (this.tempTimestamp === null) {
          this.tempTimestamp = now();
        }

        const timestamp = now();
        const diff = timestamp - this.tempTimestamp;

        const typedArray2 = this._parseAudioMsg(typedArray);

        this.log(this.tag, `'send talk msg and diff is ${diff} and byteLength is ${typedArray2.byteLength} and length is ${typedArray2.length}, and g711 length is ${typedArray.length}`);

        if (this._opt.packetType === TALK_PACKET_TYPE.rtp) {
          this.addRtpToBuffer(typedArray2);
        }

        if (typedArray2) {
          if (this.socketStatusOpen) {
            this.socket.send(typedArray2.buffer);
          } else {
            this.emit(EVENTS_ERROR.tallWebsocketClosedByError);
          }
        }

        this.tempTimestamp = timestamp;
      }

      _doTalk() {
        this._getUserMedia(); // this._getUserMedia2();
        // this._getUserMedia3();

      }

      _getUserMedia() {
        this.log(this.tag, 'getUserMedia'); // 老的浏览器可能根本没有实现 mediaDevices，所以我们可以先设置一个空的对象

        if (window.navigator.mediaDevices === undefined) {
          window.navigator.mediaDevices = {};
        } // 一些浏览器部分支持 mediaDevices。我们不能直接给对象设置 getUserMedia
        // 因为这样可能会覆盖已有的属性。这里我们只会在没有 getUserMedia 属性的时候添加它。


        if (window.navigator.mediaDevices.getUserMedia === undefined) {
          this.log(this.tag, 'window.navigator.mediaDevices.getUserMedia is undefined and init function');

          window.navigator.mediaDevices.getUserMedia = function (constraints) {
            // 首先，如果有 getUserMedia 的话，就获得它
            var getUserMedia = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia; // 一些浏览器根本没实现它 - 那么就返回一个 error 到 promise 的 reject 来保持一个统一的接口
            // 由于受浏览器的限制，navigator.mediaDevices.getUserMedia在https协议下是可以正常使用的，
            // 而在http协议下只允许localhost/127.0.0.1这两个域名访问，
            // 因此在开发时应做好容灾处理，上线时则需要确认生产环境是否处于https协议下。

            if (!getUserMedia) {
              return Promise.reject(new Error('getUserMedia is not implemented in this browser'));
            } // 否则，为老的 navigator.getUserMedia 方法包裹一个 Promise


            return new Promise(function (resolve, reject) {
              getUserMedia.call(navigator, constraints, resolve, reject);
            });
          };
        }

        if (this._opt.checkGetUserMediaTimeout) {
          this._startCheckGetUserMediaTimeout();
        }

        window.navigator.mediaDevices.getUserMedia({
          audio: {
            latency: true,
            noiseSuppression: true,
            autoGainControl: true,
            echoCancellation: true,
            sampleRate: 48000,
            channelCount: 1
          },
          video: false
        }).then(stream => {
          this.log(this.tag, 'getUserMedia success');
          this.userMediaStream = stream;
          this.mediaStreamSource = this.audioContext.createMediaStreamSource(stream);
          this.mediaStreamSource.connect(this.biquadFilter);

          if (this.recorder) {
            this.biquadFilter.connect(this.recorder);
            this.recorder.connect(this.gainNode);
          } else if (this.workletRecorder) {
            this.biquadFilter.connect(this.workletRecorder);
            this.workletRecorder.connect(this.gainNode);
          }

          this.gainNode.connect(this.audioContext.destination);
          this.emit(EVENTS.talkGetUserMediaSuccess); // check stream inactive

          if (stream.oninactive === null) {
            stream.oninactive = e => {
              this._handleStreamInactive(e);
            };
          }
        }).catch(e => {
          this.error(this.tag, 'getUserMedia error', e.toString());
          this.emit(EVENTS.talkGetUserMediaFail, e.toString());
        }).finally(() => {
          this.log(this.tag, 'getUserMedia finally');

          this._stopCheckGetUserMediaTimeout();
        });
      }

      _getUserMedia2() {
        this.log(this.tag, 'getUserMedia');
        navigator.mediaDevices ? navigator.mediaDevices.getUserMedia({
          audio: true
        }).then(stream => {
          this.log(this.tag, 'getUserMedia2 success');
        }) : navigator.getUserMedia({
          audio: true
        }, this.log(this.tag, 'getUserMedia2 success'), this.log(this.tag, 'getUserMedia2 fail'));
      }

      async _getUserMedia3() {
        this.log(this.tag, 'getUserMedia3');

        try {
          const stream = await navigator.mediaDevices.getUserMedia({
            audio: {
              latency: true,
              noiseSuppression: true,
              autoGainControl: true,
              echoCancellation: true,
              sampleRate: 48000,
              channelCount: 1
            },
            video: false
          });
          console.log('getUserMedia() got stream:', stream);
          this.log(this.tag, 'getUserMedia3 success');
        } catch (e) {
          this.log(this.tag, 'getUserMedia3 fail');
        }
      }

      _handleStreamInactive(e) {
        if (this.userMediaStream) {
          this.error(this.tag, 'stream oninactive');
          this.emit(EVENTS.talkStreamInactive);
        }
      }

      _startCheckGetUserMediaTimeout() {
        this._stopCheckGetUserMediaTimeout();

        this.checkGetUserMediaTimeout = setTimeout(() => {
          this.log(this.tag, 'check getUserMedia timeout');
          this.emit(EVENTS.talkGetUserMediaTimeout);
        }, this._opt.getUserMediaTimeout);
      }

      _stopCheckGetUserMediaTimeout() {
        if (this.checkGetUserMediaTimeout) {
          this.log(this.tag, 'stop checkGetUserMediaTimeout');
          clearTimeout(this.checkGetUserMediaTimeout);
          this.checkGetUserMediaTimeout = null;
        }
      }

      _startHeartInterval() {
        // 定时发送心跳，
        this.heartInterval = setInterval(() => {
          this.log(this.tag, 'heart interval');
          let data = [0x23, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00];
          data = new Uint8Array(data);
          this.socket.send(data.buffer);
        }, 15 * 1000);
      }

      _stopHeartInterval() {
        if (this.heartInterval) {
          this.log(this.tag, 'stop heart interval');
          clearInterval(this.heartInterval);
          this.heartInterval = null;
        }
      }

      startTalk(wsUrl) {
        return new Promise((resolve, reject) => {
          if (!isSupportGetUserMedia()) {
            return reject('not support getUserMedia');
          }

          this.wsUrl = wsUrl;

          if (this._opt.testMicrophone) {
            this._doTalk();

            return resolve();
          }

          this._createWebSocket().catch(e => {
            reject(e);
          }); // reject


          this.once(EVENTS.talkGetUserMediaFail, () => {
            reject('getUserMedia fail');
          }); // only get user media success and resolve

          this.once(EVENTS.talkGetUserMediaSuccess, () => {
            resolve();
          });
        });
      }

      setVolume(volume) {
        volume = parseFloat(volume).toFixed(2);

        if (isNaN(volume)) {
          return;
        }

        volume = clamp(volume, 0, 1);
        this.gainNode.gain.value = volume;
      }

      getOption() {
        return this._opt;
      }

      get volume() {
        return this.gainNode ? parseFloat(this.gainNode.gain.value * 100).toFixed(0) : null;
      }

    }

    class JessibucaProTalk extends Emitter {
      /**@type {import('./constant').DEFAULT_TALK_OPTIONS}*/
      _opt = {};

      constructor() {
        let options = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : {};
        super();
        this.talk = null;
        this._opt = options;
        this.LOG_TAG = 'JessibucaProTalk';
        this.debug = new Debug(this);
        this.debug.log(this.LOG_TAG, 'init', JSON.stringify(options));
      }

      destroy() {
        this.debug.log(this.LOG_TAG, 'destroy()');
        this.off();

        if (this.talk) {
          this.talk.destroy();
          this.talk = null;
        }

        this.debug.log(this.LOG_TAG, 'destroy');
      }

      _initTalk() {
        let options = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : {};

        if (this.talk) {
          this.talk.destroy();
          this.talk = null;
        }

        const opt = Object.assign({}, clone(this._opt), options);
        this.talk = new Talk(null, opt);
        this.debug.log(this.LOG_TAG, '_initTalk', this.talk.getOption());

        this._bindTalkEvents();
      }

      _bindTalkEvents() {
        // 对外的事件
        Object.keys(TALK_EVENTS).forEach(key => {
          this.talk.on(TALK_EVENTS[key], value => {
            this.emit(key, value);
          });
        });
      }
      /**
       *
       * @param wsUrl
       * @param options
       * @returns {Promise<unknown>}
       */


      startTalk(wsUrl) {
        let options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
        return new Promise((resolve, reject) => {
          this.debug.log(this.LOG_TAG, 'startTalk', wsUrl, JSON.stringify(options));

          this._initTalk(options);

          this.talk.startTalk(wsUrl).then(() => {
            resolve();
            this.talk.once(EVENTS.talkStreamClose, () => {
              this.debug.warn(this.LOG_TAG, 'talkStreamClose');
              this.stopTalk().catch(e => {});
            });
            this.talk.once(EVENTS.talkStreamError, e => {
              this.debug.error(this.LOG_TAG, 'talkStreamError');
              this.stopTalk().catch(e => {});
            });
            this.talk.once(EVENTS.talkStreamInactive, () => {
              this.debug.warn(this.LOG_TAG, 'talkStreamInactive');
              this.stopTalk().catch(e => {});
            });
          }).catch(e => {
            reject(e);
          });
        });
      }
      /**
       *
       * @returns {Promise<unknown>}
       */


      stopTalk() {
        return new Promise((resolve, reject) => {
          this.debug.log(this.LOG_TAG, 'stopTalk()');

          if (!this.talk) {
            reject('talk is not init');
          }

          this.talk.destroy();
          resolve();
        });
      }
      /**
       *
       * @returns {Promise<unknown>}
       */


      getTalkVolume() {
        return new Promise((resolve, reject) => {
          if (!this.talk) {
            reject('talk is not init');
          }

          let result = this.talk.volume;
          resolve(result);
        });
      }
      /**
       *
       * @param volume
       * @returns {Promise<unknown>}
       */


      setTalkVolume(volume) {
        return new Promise((resolve, reject) => {
          this.debug.log(this.LOG_TAG, 'setTalkVolume', volume);

          if (!this.talk) {
            reject('talk is not init');
          }

          this.talk.setVolume(volume / 100);
          resolve();
        });
      }

      downloadTempRtpFile() {
        return new Promise((resolve, reject) => {
          if (this.talk) {
            this.talk.downloadRtpFile();
            resolve();
          } else {
            reject('talk is not init');
          }
        });
      }

    }

    JessibucaProTalk.EVENTS = TALK_EVENTS;
    window.JessibucaProTalk = JessibucaProTalk;
    window.WebPlayerProTalk = JessibucaProTalk;

}));
//# sourceMappingURL=jessibuca-pro-talk-demo.js.map
