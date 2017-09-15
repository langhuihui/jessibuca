mergeInto(LibraryManager.library, {
    init: function() {
        var mc = window["MonaClient"] = Module["MonaClient"];
        var ns = window["NetStream"] = Module["NetStream"];
        ns.prototype.attachCanvas = function(canvas) {
            var webGLCanvas = new WebGLCanvas(canvas, Module["noWebGL"], {});
            this.setVideoSize = function(w, h, dataPtr) {
                canvas.width = w;
                canvas.height = h;
                if (webGLCanvas.isWebGL()) {
                    this.draw = function() {
                        var y = HEAPU32[dataPtr];
                        var u = HEAPU32[dataPtr + 1];
                        var v = HEAPU32[dataPtr + 2];
                        var outputArray = [HEAPU8.subarray(y, y + w * h), HEAPU8.subarray(u, u + (w * h >> 2)), HEAPU8.subarray(v, v + (w * h >> 2))];
                        webGLCanvas.drawNextOutputPicture(w, h, null, outputArray);
                    };
                } else {
                    var outputArray = HEAPU8.subarray(dataPtr, dataPtr + (w * h << 2));
                    webGLCanvas.initRGB(w, h);
                    this.draw = function() {
                        webGLCanvas.drawNextOutputPicture(w, h, null, outputArray);
                    };
                }
            }.bind(this);

            this.getWebGL = function() {
                return webGLCanvas;
            };
            this._attachCanvas(this, webGLCanvas.isWebGL());
        };
        ns.prototype.checkVideoBuffer = function(t) { return setTimeout(this.decodeVideoBuffer.bind(this), t); };
        ns.prototype.onNetStatus = function(info) {

        };
        ns.prototype.initAudio = function(frameCount, samplerate, channels) {
            var isPlaying = false;
            var audioBuffers = [];
            var allFrameCount = frameCount * channels;
            var audioBufferTime = frameCount * 1000 / samplerate;
            //var AudioContext = window.AudioContext || window.webkitAudioContext || window.mozAudioContext || window.msAudioContext;
            var audioCtx = this.audioContext; //new AudioContext();
            if (!audioCtx) return false;
            var setCurrentAudioData;
            var resampled = samplerate < 22050;
            var audioBuffer = resampled ? audioCtx.createBuffer(channels, frameCount << 1, samplerate << 1) : audioCtx.createBuffer(channels, frameCount, samplerate);
            var outputPtr = this._initAudio(this, frameCount, channels);
            var audioOutputArray = HEAP16.subarray(outputPtr, outputPtr + allFrameCount);
            var playNextBuffer = function() {
                isPlaying = false;
                if (audioBuffers.length) {
                    playAudio(audioBuffers.shift());
                }
                if (audioBuffers.length > 1) audioBuffers.shift();
                console.log(audioBuffers.length)
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
                var source = audioCtx.createBufferSource();
                source.buffer = audioBuffer;
                source.connect(audioCtx.destination);
                source.onended = playNextBuffer;
                //setTimeout(playNextBuffer, audioBufferTime-audioBuffers.length*200);
                source.start();
            };
            this.playAudio = playAudio;
        };
        mc.prototype.connect = function(url, appName, roomName) {
            var ws = this.$connect(this, url, appName, roomName || "");
            this.send = function(ptr, length, output) {
                var outputArray = HEAPU8.subarray(ptr, ptr + length);
                ws.send(outputArray);
            };
        };
    }
})