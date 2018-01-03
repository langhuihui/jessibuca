mergeInto(LibraryManager.library, {
    init: function() {
        var fc = window["FlvClient"] = Module["FlvClient"];
        window.AudioContext = window.AudioContext || window.webkitAudioContext;
        var context = new window.AudioContext();
        if (/(iPhone|iPad|iPod|iOS|Safari)/i.test(navigator.userAgent)) {
            function _unlock() {
                context.resume();
                var source = context.createBufferSource();
                source.buffer = context.createBuffer(1, 1, 22050);
                source.connect(context.destination);
                if (source.noteOn)
                    source.noteOn(0);
                else
                    source.start(0);
            }
            document.addEventListener("mousedown", _unlock, true);
            document.addEventListener("touchend", _unlock, true);
        }

        fc.prototype.checkVideoBuffer = function(t) {
            return setTimeout(this.decodeVideoBuffer.bind(this), t);
        };
        fc.prototype.onNetStatus = function(info) {

        };
        fc.prototype.initAudio = function(frameCount, samplerate, channels) {
            var isPlaying = false;
            var audioBuffers = [];
            var allFrameCount = frameCount * channels;
            var audioBufferTime = frameCount * 1000 / samplerate;
            //var AudioContext = window.AudioContext || window.webkitAudioContext || window.mozAudioContext || window.msAudioContext;
            //var audioCtx = this.audioContext;//new AudioContext();
            if (!context) return false;
            var setCurrentAudioData;
            var resampled = samplerate < 22050;
            var audioBuffer = resampled ? context.createBuffer(channels, frameCount << 1, samplerate << 1) : context.createBuffer(channels, frameCount, samplerate);
            var outputPtr = this._initAudio(frameCount, channels);
            var audioOutputArray = HEAP16.subarray(outputPtr, outputPtr + allFrameCount);
            var playNextBuffer = function() {
                isPlaying = false;
                if (audioBuffers.length) {
                    playAudio(audioBuffers.shift());
                }
                if (audioBuffers.length > 1) audioBuffers.shift();
                //console.log(audioBuffers.length)
            };
            var copyAudioOutputArray = resampled ? function(target) {
                for (var i = 0; i < allFrameCount; i++) {
                    var j = i << 1;
                    target[j] = target[j + 1] = audioOutputArray[i] / 32768;
                }
            } : function(target) {
                for (var i = 0; i < allFrameCount; i++) {

                    target[i] = audioOutputArray[i] / 32768;
                }
            };
            var copyToCtxBuffer = channels > 1 ? function(fromBuffer) {
                for (var channel = 0; channel < channels; channel++) {
                    var nowBuffering = audioBuffer.getChannelData(channel);
                    if (fromBuffer) {
                        for (var i = 0; i < frameCount; i++) {
                            nowBuffering[i] = fromBuffer[i * (channel + 1)];
                        }
                    } else {
                        for (var i = 0; i < frameCount; i++) {
                            nowBuffering[i] = audioOutputArray[i * (channel + 1)] / 32768;
                        }
                    }
                }
            } : function(fromBuffer) {
                var nowBuffering = audioBuffer.getChannelData(0);
                if (fromBuffer) nowBuffering.set(fromBuffer);
                else copyAudioOutputArray(nowBuffering);
            };
            var playAudio = function(fromBuffer) {
                if (isPlaying) {
                    var buffer = new Float32Array(resampled ? allFrameCount * 2 : allFrameCount);
                    copyAudioOutputArray(buffer);
                    audioBuffers.push(buffer);
                    return;
                }
                isPlaying = true;
                copyToCtxBuffer(fromBuffer);
                var source = context.createBufferSource();
                source.buffer = audioBuffer;
                source.connect(context.destination);
                source.onended = playNextBuffer;
                //setTimeout(playNextBuffer, audioBufferTime-audioBuffers.length*200);
                source.start();
            };
            this.playAudio = playAudio;
        };
        fc.prototype.play = function(url, webGLCanvas, croppingParams) {

            var canvas = webGLCanvas.canvasElement;
            this.setVideoSize = function(w, h, dataPtr) {
                canvas.width = w;
                canvas.height = h;
                if (webGLCanvas.isWebGL()) {
                    this.draw = function() {
                        var y = HEAPU32[dataPtr];
                        var u = HEAPU32[dataPtr + 1];
                        var v = HEAPU32[dataPtr + 2];
                        var outputArray = [HEAPU8.subarray(y, y + w * h), HEAPU8.subarray(u, u + (w * h >> 2)), HEAPU8.subarray(v, v + (w * h >> 2))];
                        webGLCanvas.drawNextOutputPicture(w, h, croppingParams, outputArray);
                    };
                } else {
                    var outputArray = HEAPU8.subarray(dataPtr, dataPtr + (w * h << 2));
                    webGLCanvas.initRGB(w, h);
                    this.draw = function() {
                        webGLCanvas.drawNextOutputPicture(w, h, croppingParams, outputArray);
                    };
                }
            }.bind(this);

            this.getWebGL = function() {
                return webGLCanvas;
            };
            var _this = this;
            var ws = new WebSocket(url);
            ws.onmessage = function(data) {
                _this.$onWsMessage(data);
            };
            ws.onopen = function() {
                Module.print("websocket open");
            };
            ws.binaryType = "arraybuffer";
            if (this.onWsError) ws.onerror = this.onWsError;
            ws.onclose = function() {
                _this.$close();
                if (this.onWsClose) this.onWsClose();
            };
            this.close = function() {
                ws.close();
            };
            this.$play(this, webGLCanvas.isWebGL());
        };
    }
})