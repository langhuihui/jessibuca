mergeInto(LibraryManager.library, {
    init: function () {
        function arrayBufferCopy(src, dst, dstByteOffset, numBytes) {
            var i;
            var dst32Offset = dstByteOffset / 4;
            var tail = (numBytes % 4);
            var src32 = new Uint32Array(src.buffer, 0, (numBytes - tail) / 4);
            var dst32 = new Uint32Array(dst.buffer);
            for (i = 0; i < src32.length; i++) {
                dst32[dst32Offset + i] = src32[i];
            }
            for (i = numBytes - tail; i < numBytes; i++) {
                dst[dstByteOffset + i] = src[i];
            }
        }
        if (!Date.now) Date.now = function () {
            return new Date().getTime();
        };
        Module.print = function (text) {
            postMessage({ cmd: "print", text: text })
        }
        Module.printErr = function (text) {
            postMessage({ cmd: "printErr", text: text })
        }
        Module.postRun = function () {
            postMessage({ cmd: "init" })
        }
        Module.Jessibuca = Module.Jessica.extend("Jessibuca", {
            __construct: function () {
                this.__parent.__construct.call(this, this);
            },
            __destruct: function () {
                this.__parent.__destruct.call(this);
            },
            onError: function (err) {
                console.log(this, err)
            },
            setStartTime: function (offset) {
                console.log("first timestamp:", offset)
                var startTime = Date.now() - offset;
                this.timespan = function (t) {
                    return t - (Date.now() - startTime);
                }

                function playVideo(_this) {
                    _this.decodeVideoBuffer();
                }
                this.playVideoBuffer = function (t) {
                    // console.log("setTimeout:", t);
                    return setTimeout(playVideo, t, this)
                }
            },
            timespan: function (t) {
                this.setStartTime(t);
                return this.timespan(t);
            },
            resetTimeSpan: function () {
                delete this.timespan;
            },
            play: function (url) {
                console.log('Jessibuca play', url)
                var _this = this;
                var reconnectCount = 0;
                var reconnectTime = 2000;
                function setWebsocket() {
                    this.$play(url);
                    this.ws.onopen = function () {
                        reconnectCount = 0;
                        reconnectTime = 2000;
                        console.log("ws open")
                    };
                    this.ws.onclose = function () {
                        _this.isPlaying = false;
                        _this.ws = null;
                        _this.$close();
                        if (reconnectCount > 3) return;
                        reconnectCount++;
                        console.warn("ws reconnect after " + (reconnectTime / 1000 >> 0) + " second")
                        _this.reconnectId = setTimeout(function () {
                            console.log("ws reconnecting :", reconnectCount);
                            reconnectTime *= 2;
                            setWebsocket.call(_this);
                        }, reconnectTime)
                    };
                    this.ws.onerror = function () {
                        console.warn("ws error");
                    };
                }
                setWebsocket.call(this);
            },
            close: function () {
                clearTimeout(this.reconnectId)
                if (!this.isPlaying) return;
                console.log('close Jessibuca')
                this.isPlaying = false;
                this.ws.onmessage = null;
                this.ws.onclose = null;
                this.ws.onerror = null;
                this.ws.close();
                this.ws = null;
                this.$close();
                delete this.timespan;
            },
            initAudio: function (frameCount, samplerate, channels, outputPtr) {
                var allFrameCount = frameCount * channels;
                var resampled = samplerate < 22050;
                var audioOutputArray = HEAP16.subarray(outputPtr, outputPtr + allFrameCount);
                postMessage({ cmd: "initAudio", frameCount: frameCount, samplerate: samplerate, channels: channels })
                var copyAudioOutputArray = resampled ? function (target) {
                    for (var i = 0; i < allFrameCount; i++) {
                        var j = i << 1;
                        target[j] = target[j + 1] = audioOutputArray[i] / 32768;
                    }
                } : function (target) {
                    for (var i = 0; i < allFrameCount; i++) {
                        target[i] = audioOutputArray[i] / 32768;
                    }
                };
                this.playAudio = function () {
                    var buffer = new Float32Array(resampled ? allFrameCount * 2 : allFrameCount);
                    copyAudioOutputArray(buffer)
                    postMessage({ cmd: "playAudio", buffer: buffer }, [buffer.buffer])
                }
            },
            setBuffer: function (outputArray) {
                for (var i = 0; i < 3; i++) {
                    var buffer = this.buffers[i]
                    if (buffer.length) {
                        buffer = buffer.pop()
                        arrayBufferCopy(outputArray, buffer, 0, buffer.byteLength)
                        // for (var j = 0; j < buffer.byteLength; j++) {
                        //     buffer[j] = outputArray[i][j]
                        // }
                    } else {
                        buffer = Uint8Array.from(outputArray[i])
                    }
                    outputArray[i] = buffer
                }
            },
            setVideoSize: function (w, h, dataPtr) {
                postMessage({ cmd: "initSize", w: w, h: h })
                this.buffers = [[], [], []]
                var size = w * h
                if (this.isWebGL) {
                    this.draw = function () {
                        var y = HEAPU32[dataPtr];
                        var u = HEAPU32[dataPtr + 1];
                        var v = HEAPU32[dataPtr + 2];
                        // console.log(y, u, v);
                        var outputArray = [HEAPU8.subarray(y, y + size), HEAPU8.subarray(u, u + (size >> 2)), HEAPU8.subarray(v, v + (size >> 2))];
                        this.setBuffer(outputArray)
                        // var outputArray = [new Uint8Array(this.buffer, 0, size), new Uint8Array(this.buffer, size, size >> 2), new Uint8Array(this.buffer, size + (size >> 2), size >> 2)]
                        postMessage({ cmd: "render", output: outputArray }, [outputArray[0].buffer, outputArray[1].buffer, outputArray[2].buffer])
                    };
                } else {
                    var outputArray = HEAPU8.subarray(dataPtr, dataPtr + (w * h << 2));
                    var output = { cmd: "render", buffer: outputArray }
                    this.draw = function () {
                        postMessage(output)
                    };
                }
            },

        });
        var decoder = new Module.Jessibuca()
        decoder.videoBuffer = 1
        self.onmessage = function (event) {
            var msg = event.data
            switch (msg.cmd) {
                case "play":
                    decoder.isWebGL = msg.isWebGL
                    decoder.play(msg.url)
                    break
                case "setBuffer":
                    decoder.buffers[0].push(msg.buffers[0])
                    decoder.buffers[1].push(msg.buffers[1])
                    decoder.buffers[2].push(msg.buffers[2])
                    break
				case "setVideoBuffer":
					decoder.videoBuffer = msg.cmd*1000
					break
                case "close":
                    decoder.close()
                    break
            }
        }
    },

})