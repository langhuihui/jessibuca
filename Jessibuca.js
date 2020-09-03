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
                this.audioCache = []
            },
            __destruct: function () {
                this.__parent.__destruct.call(this);
            },
            onError: function (err) {
                console.log(this, err)
            },
            play: function (url) {
                console.log('Jessibuca play', url)
                if(url.indexOf("http")==0){
                    this.$play(url);
                    return;
                }
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
            fetch: function(url){
                var _this = this;
                this.controller = new AbortController();
                var signal = this.controller.signal;
                fetch(url,{signal}).then(function(res){
                    var reader = res.body.getReader();
                    _this.fetchNext = function(){
                        reader.read().then(({done, value})=>_this.onFetchData({done,data:value}))
                    }
                    _this.fetchNext()
                }).catch(console.error)
            },
            close: function () {
                clearTimeout(this.reconnectId)
                if (!this.isPlaying) return;
                console.log('close Jessibuca')
                this.isPlaying = false;
                if(this.ws){
                    this.ws.onmessage = null;
                    this.ws.onclose = null;
                    this.ws.onerror = null;
                    this.ws.close();
                    this.ws = null;
                }
                if(this.controller)this.controller.abort();
                this.$close();
                delete this.timespan;
            },
            initAudio: function (frameCount, samplerate, channels, outputPtr) {
                var allFrameCount = frameCount * channels;
                var resampled = samplerate < 22050;
                var audioOutputArray = HEAP16.subarray(outputPtr, outputPtr + allFrameCount);
                postMessage({ cmd: "initAudio", frameCount: frameCount, samplerate: samplerate, channels: channels })
                // var copyAudioOutputArray = resampled ? function (target) {
                //     for (var i = 0; i < allFrameCount; i++) {
                //         var j = i << 1;
                //         target[j] = target[j + 1] = audioOutputArray[i] / 32768;
                //     }
                // } : function (target) {
                //     for (var i = 0; i < allFrameCount; i++) {
                //         target[i] = audioOutputArray[i] / 32768;
                //     }
                // };
                this.playAudio = function (ts) {
                    // var buffer = new Float32Array(resampled ? allFrameCount * 2 : allFrameCount);
                    // copyAudioOutputArray(buffer)
                    // postMessage({ cmd: "playAudio", buffer: buffer }, [buffer.buffer])
                    postMessage({ cmd: "playAudio", buffer: audioOutputArray ,ts:ts})
                }
            },
            playAudio(data, len) {
                var buffer = HEAPU8.subarray(data, data + len);
                this.audioCache.push(buffer.buffer.slice(buffer.byteOffset, buffer.byteOffset + buffer.length))
                if (this.audioCache.length >= this.audioBuffer) {
                    postMessage({ cmd: "playAudio", buffer: this.audioCache }, this.audioCache)
                    this.audioCache.length = 0
                }
            },
            initAudioPlanar(channels, samplerate) {
                this.buffersA = [];
                for (var i = 0; i < channels; i++) {
                    this.buffersA.push([]);
                }
                postMessage({ cmd: "initAudioPlanar", samplerate: samplerate, channels: channels })
            },
            playAudioPlanar(data, len,ts) {
                var outputArray = [];
                var frameCount = len / 4 / this.buffersA.length;
                for (var i = 0; i < this.buffersA.length; i++) {
                    var fp = HEAPU32[(data>>2) + i]>>2;
                    var float32 = HEAPF32.subarray(fp, fp + frameCount);
                    var buffer = this.buffersA[i]
                    if (buffer.length) {
                        buffer = buffer.pop();
                        for (var j = 0; j < buffer.length; j++) {
                            buffer[j] = float32[j];
                        }
                    } else {
                        buffer = Float32Array.from(float32);
                    }
                    outputArray[i] = buffer;
                }
                this.audioCache.push(outputArray)
                if (this.audioCache.length >= this.audioBuffer) {
                    postMessage({ cmd: "playAudio", buffer: this.audioCache,ts:ts }, this.audioCache.flatMap(outputArray=>outputArray.map(x=>x.buffer)))
                    this.audioCache.length = 0
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
                    this.draw = function (compositionTime,ts) {
                        var y = HEAPU32[dataPtr];
                        var u = HEAPU32[dataPtr + 1];
                        var v = HEAPU32[dataPtr + 2];
                        // console.log(y, u, v);
                        var outputArray = [HEAPU8.subarray(y, y + size), HEAPU8.subarray(u, u + (size >> 2)), HEAPU8.subarray(v, v + (size >> 2))];
                        this.setBuffer(outputArray)
                        // var outputArray = [new Uint8Array(this.buffer, 0, size), new Uint8Array(this.buffer, size, size >> 2), new Uint8Array(this.buffer, size + (size >> 2), size >> 2)]
                        postMessage({ cmd: "render", output: outputArray, compositionTime: compositionTime,ts :ts}, [outputArray[0].buffer, outputArray[1].buffer, outputArray[2].buffer])
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
        self.onmessage = function (event) {
            var msg = event.data
            switch (msg.cmd) {
                case "play":
                    decoder.isWebGL = msg.isWebGL
                    decoder.play(msg.url)
                    break
                case "setBuffer":
                    decoder.buffers[0].push(msg.buffer[0])
                    decoder.buffers[1].push(msg.buffer[1])
                    decoder.buffers[2].push(msg.buffer[2])
                    break
                case "setBufferA":
                    decoder.buffersA.forEach((array, i) => array.push(msg.buffers[i]))
                    break
                case "setVideoBuffer":
                    decoder.videoBuffer =(msg.time * 1000)|0
                    break
                case "close":
                    decoder.close()
                    break
            }
        }
    },

})