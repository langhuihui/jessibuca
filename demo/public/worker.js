var supportedWasm = (() => {
    try {
        if (typeof WebAssembly === "object"
            && typeof WebAssembly.instantiate === "function") {
            const module = new WebAssembly.Module(Uint8Array.of(0x0, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00));
            if (module instanceof WebAssembly.Module)
                return new WebAssembly.Instance(module) instanceof WebAssembly.Instance;
        }
    } catch (e) {
    }
    return false;
})();
importScripts(supportedWasm ? 'ff_wasm.js' : 'ff.js')
importScripts('webgl.js')
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
function dispatchData(input) {
    let need = input.next()
    let buffer = null
    return (value) => {
        var data = new Uint8Array(value)
        if (buffer) {
            var combine = new Uint8Array(buffer.length + data.length)
            combine.set(buffer)
            combine.set(data, buffer.length)
            data = combine
            buffer = null
        }
        while (data.length >= need.value) {
            var remain = data.slice(need.value)
            need = input.next(data.slice(0, need.value))
            data = remain
        }
        if (data.length > 0) {
            buffer = data
        }
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
    var buffer = []
    var decoder = {
        opt: {},
        initAudioPlanar: function (channels, samplerate) {
            var buffersA = [];
            for (var i = 0; i < channels; i++) {
                buffersA.push([]);
            }
            postMessage({ cmd: "initAudioPlanar", samplerate: samplerate, channels: channels })
            this.playAudioPlanar = function (data, len) {
                var outputArray = [];
                var frameCount = len / 4 / buffersA.length;
                for (var i = 0; i < buffersA.length; i++) {
                    var fp = HEAPU32[(data >> 2) + i] >> 2;
                    var float32 = HEAPF32.subarray(fp, fp + frameCount);
                    var buffer = buffersA[i]
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
                postMessage({ cmd: "playAudio", buffer: outputArray }, outputArray.map(x => x.buffer))
            }
        },
        inputFlv: function* () {
            yield 9
            var tmp = new ArrayBuffer(4)
            var tmp8 = new Uint8Array(tmp)
            var tmp32 = new Uint32Array(tmp)
            while (true) {
                tmp8[3] = 0
                var t = yield 15
                var type = t[4]
                tmp8[0] = t[7]
                tmp8[1] = t[6]
                tmp8[2] = t[5]
                var length = tmp32[0]
                tmp8[0] = t[10]
                tmp8[1] = t[9]
                tmp8[2] = t[8]
                var ts = tmp32[0]
                if (ts === 0xFFFFFF) {
                    tmp8[3] = t[11]
                    ts = tmp32[0]
                }
                var payload = yield length
                switch (type) {
                    case 8:
                        buffer.push({ ts, payload, decoder: audioDecoder, type: 0 })
                        break
                    case 9:
                        buffer.push({ ts, payload, decoder: videoDecoder, type: payload[0] >> 4 })
                        break
                }
            }
        },
        play: function (url) {
            console.log('Jessibuca play', url)
            this.getDelay = function (timestamp) {
                if (!timestamp) return -1
                this.firstTimestamp = timestamp
                this.startTimestamp = Date.now()
                this.getDelay = function (timestamp) {
                    this.delay = (Date.now() - this.startTimestamp) - (timestamp - this.firstTimestamp)
                    return this.delay
                }
                return -1
            }
            var loop = this.opt.vod ? () => {
                if (buffer.length) {
                    var data = buffer[0]
                    if (this.getDelay(data.ts) === -1) {
                        buffer.shift()
                        data.decoder.decode(data.payload)
                    } else {
                        while (buffer.length) {
                            data = buffer[0]
                            if (this.getDelay(data.ts) > this.videoBuffer) {
                                buffer.shift()
                                data.decoder.decode(data.payload)
                            } else {
                                break
                            }
                        }
                    }
                }
            } : () => {
                if (buffer.length) {
                    if (this.dropping) {
                        data = buffer.shift()
                        if (data.type == 1) {
                            this.dropping = false
                            data.decoder.decode(data.payload)
                        } else if (data.type == 0) {
                            data.decoder.decode(data.payload)
                        }
                    } else {
                        var data = buffer[0]
                        if (this.getDelay(data.ts) === -1) {
                            buffer.shift()
                            data.decoder.decode(data.payload)
                        } else if (this.delay > this.videoBuffer + 1000) {
                            this.dropping = true
                        } else {
                            while (buffer.length) {
                                data = buffer[0]
                                if (this.getDelay(data.ts) > this.videoBuffer) {
                                    buffer.shift()
                                    data.decoder.decode(data.payload)
                                } else {
                                    break
                                }
                            }
                        }
                    }
                }
            }
            this.stopId = setInterval(loop, 10)
            if (url.indexOf("http") == 0) {
                this.flvMode = true
                var _this = this;
                var controller = new AbortController();
                fetch(url, { signal: controller.signal }).then(function (res) {
                    var reader = res.body.getReader();
                    var input = _this.inputFlv()
                    var dispatch = dispatchData(input)
                    var fetchNext = function () {
                        reader.read().then(({ done, value }) => {
                            if (done) {
                                input.return(null)
                            } else {
                                dispatch(value)
                                fetchNext()
                            }
                        }).catch((e) => input.throw(e))
                    }
                    fetchNext()
                }).catch((err) => {
                    postMessage({ cmd: "printErr", text: err.message })
                })
                this._close = function () {
                    controller.abort()
                }
            } else {
                this.flvMode = url.indexOf(".flv") != -1
                this.ws = new WebSocket(url)
                this.ws.binaryType = "arraybuffer"
                if (this.flvMode) {
                    var dispatch = dispatchData(this.inputFlv())
                    this.ws.onmessage = evt => dispatch(evt.data)
                } else {
                    this.ws.onmessage = evt => {
                        var dv = new DataView(evt.data)
                        switch (dv.getUint8(0)) {
                            case 1:
                                buffer.push({ ts: dv.getUint32(1, false), payload: new Uint8Array(evt.data, 5), decoder: audioDecoder, type: 0 })
                                break
                            case 2:
                                buffer.push({ ts: dv.getUint32(1, false), payload: new Uint8Array(evt.data, 5), decoder: videoDecoder, type: dv.getUint8(5) >> 4 })
                                break
                        }
                    }
                }
                this._close = function () {
                    this.ws.close()
                }
            }
            this.setVideoSize = function (w, h) {
                postMessage({ cmd: "initSize", w: w, h: h })
                var size = w * h
                var qsize = size >> 2
                if (!this.opt.forceNoOffscreen && typeof OffscreenCanvas != 'undefined') {
                    var canvas = new OffscreenCanvas(w, h);
                    var gl = canvas.getContext("webgl");
                    var render = createWebGL(gl)
                    this.draw = function (compositionTime, y, u, v) {
                        render(w, h, HEAPU8.subarray(y, y + size), HEAPU8.subarray(u, u + qsize), HEAPU8.subarray(v, v + (qsize)))
                        let image_bitmap = canvas.transferToImageBitmap();
                        postMessage({ cmd: "render", compositionTime: compositionTime, bps: this.bps, delay: this.delay, buffer: image_bitmap }, [image_bitmap])
                    }
                } else {
                    this.draw = function (compositionTime, y, u, v) {
                        var yuv = [HEAPU8.subarray(y, y + size), HEAPU8.subarray(u, u + qsize), HEAPU8.subarray(v, v + (qsize))];
                        var outputArray = yuv.map(buffer => Uint8Array.from(buffer))
                        // arrayBufferCopy(HEAPU8.subarray(y, y + size), this.sharedBuffer, 0, size)
                        // arrayBufferCopy(HEAPU8.subarray(u, u + (qsize)), this.sharedBuffer, size, qsize)
                        // arrayBufferCopy(HEAPU8.subarray(v, v + (qsize)), this.sharedBuffer, size + qsize, qsize)
                        postMessage({ cmd: "render", compositionTime: compositionTime, bps: this.bps, delay: this.delay, output: outputArray }, outputArray.map(x => x.buffer))
                    }
                }
            }
        }, close: function () {
            if (this._close) {
                console.log("jessibuca closed")
                this._close()
                audioDecoder.clear()
                videoDecoder.clear()
                clearInterval(this.stopId)
                this.firstTimestamp = 0
                this.startTimestamp = 0
                delete this.getDelay
            }
        }
    }
    var audioDecoder = new Module.AudioDecoder(decoder)
    var videoDecoder = new Module.VideoDecoder(decoder)
    postMessage({ cmd: "init" })
    self.onmessage = function (event) {
        var msg = event.data
        switch (msg.cmd) {
            case "init":
                decoder.opt = msg.opt
                break
            case "getProp":
                postMessage({ cmd: "getProp", value: decoder[msg.prop] })
                break
            case "setProp":
                decoder[msg.prop] = msg.value
                break
            case "play":
                decoder.play(msg.url)
                break
            case "setVideoBuffer":
                decoder.videoBuffer = (msg.time * 1000) | 0
                break
            case "close":
                decoder.close()
                break
        }
    }
}