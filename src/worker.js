import Module from './decoder/decoder'
import createWebGL from './utils/webgl';
import {WORKER_CMD_TYPE, MEDIA_TYPE, WORKER_SEND_TYPE, ENCODED_VIDEO_TYPE} from "./constant";
import {formatVideoDecoderConfigure} from "./utils";

if (!Date.now) Date.now = function () {
    return new Date().getTime();
};
Module.print = function (text) {
    postMessage({cmd: WORKER_CMD_TYPE.print, text: text})
}
Module.printErr = function (text) {
    postMessage({cmd: WORKER_CMD_TYPE.printErr, text: text})
}


Module.postRun = function () {
    var buffer = []
    var wcsVideoDecoder = {};
    if ("VideoEncoder" in self) {
        wcsVideoDecoder = {
            hasInit: false,
            isEmitInfo: false,
            offscreenCanvas: null,
            offscreenCanvasCtx: null,
            decoder: new VideoDecoder({
                output: function (videoFrame) {
                    if (!wcsVideoDecoder.isEmitInfo) {
                        decoder.opt.debug && console.log('Jessibuca: [worker] Webcodecs Video Decoder initSize');
                        postMessage({
                            cmd: WORKER_CMD_TYPE.initVideo,
                            w: videoFrame.codedWidth,
                            h: videoFrame.codedHeight
                        })
                        wcsVideoDecoder.isEmitInfo = true;
                        wcsVideoDecoder.offscreenCanvas = new OffscreenCanvas(videoFrame.codedWidth, videoFrame.codedHeight);
                        wcsVideoDecoder.offscreenCanvasCtx = wcsVideoDecoder.offscreenCanvas.getContext("2d");
                    }

                    wcsVideoDecoder.offscreenCanvasCtx.drawImage(videoFrame, 0, 0, videoFrame.codedWidth, videoFrame.codedHeight);
                    let image_bitmap = wcsVideoDecoder.offscreenCanvas.transferToImageBitmap();
                    postMessage({
                        cmd: WORKER_CMD_TYPE.render,
                        buffer: image_bitmap,
                        delay: decoder.delay,
                        ts: 0
                    }, [image_bitmap])

                    setTimeout(function () {
                        if (videoFrame.close) {
                            videoFrame.close()
                        } else {
                            videoFrame.destroy()
                        }
                    }, 100)

                },
                error: function (error) {
                    console.error(error);
                }
            }),
            decode: function (payload, ts) {
                const isIframe = payload[0] >> 4 === 1;
                if (!wcsVideoDecoder.hasInit) {
                    if (isIframe && payload[1] === 0) {
                        const videoCodec = (payload[0] & 0x0F);
                        decoder.setVideoCodec(videoCodec);
                        const config = formatVideoDecoderConfigure(payload.slice(5));
                        wcsVideoDecoder.decoder.configure(config);
                        wcsVideoDecoder.hasInit = true;
                    }
                } else {
                    const chunk = new EncodedVideoChunk({
                        data: payload.slice(5),
                        timestamp: ts,
                        type: isIframe ? ENCODED_VIDEO_TYPE.key : ENCODED_VIDEO_TYPE.delta
                    })
                    wcsVideoDecoder.decoder.decode(chunk);
                }
            },
            reset() {
                wcsVideoDecoder.hasInit = false;
                wcsVideoDecoder.isEmitInfo = false;
                wcsVideoDecoder.offscreenCanvas = null;
                wcsVideoDecoder.offscreenCanvasCtx = null;
            }
        }
    }

    var decoder = {
        opt: {},
        useOffscreen: function () {
            return !this.opt.forceNoOffscreen && typeof OffscreenCanvas != 'undefined';
        },
        initAudioPlanar: function (channels, samplerate) {
            postMessage({cmd: WORKER_CMD_TYPE.initAudio, sampleRate: samplerate, channels: channels})
            var buffer = []
            var outputArray = [];
            var remain = 0
            this.playAudioPlanar = function (data, len, ts) {
                var frameCount = len;
                var origin = []
                var start = 0
                for (var channel = 0; channel < 2; channel++) {
                    var fp = Module.HEAPU32[(data >> 2) + channel] >> 2;
                    origin[channel] = Module.HEAPF32.subarray(fp, fp + frameCount);
                }
                if (remain) {
                    len = 1024 - remain
                    if (frameCount >= len) {
                        outputArray[0] = Float32Array.of(...buffer[0], ...origin[0].subarray(0, len))
                        if (channels == 2) {
                            outputArray[1] = Float32Array.of(...buffer[1], ...origin[1].subarray(0, len))
                        }
                        postMessage({
                            cmd: WORKER_CMD_TYPE.playAudio,
                            buffer: outputArray,
                            ts
                        }, outputArray.map(x => x.buffer))
                        start = len
                        frameCount -= len
                    } else {
                        remain += frameCount
                        buffer[0] = Float32Array.of(...buffer[0], ...origin[0])
                        if (channels == 2) {
                            buffer[1] = Float32Array.of(...buffer[1], ...origin[1])
                        }
                        return
                    }
                }
                for (remain = frameCount; remain >= 1024; remain -= 1024) {
                    outputArray[0] = origin[0].slice(start, start += 1024)
                    if (channels == 2) {
                        outputArray[1] = origin[1].slice(start - 1024, start)
                    }
                    postMessage({
                        cmd: WORKER_CMD_TYPE.playAudio,
                        buffer: outputArray,
                        ts
                    }, outputArray.map(x => x.buffer))
                }
                if (remain) {
                    buffer[0] = origin[0].slice(start)
                    if (channels == 2) {
                        buffer[1] = origin[1].slice(start)
                    }
                }
            }
        },
        setVideoCodec: function (code) {
            postMessage({cmd: WORKER_CMD_TYPE.videoCode, code})
        },
        setAudioCodec: function (code) {
            postMessage({cmd: WORKER_CMD_TYPE.audioCode, code})
        },
        setVideoSize: function (w, h) {
            postMessage({cmd: WORKER_CMD_TYPE.initVideo, w: w, h: h})
            var size = w * h
            var qsize = size >> 2
            if (decoder.useOffscreen()) {
                this.offscreenCanvas = new OffscreenCanvas(w, h);
                this.offscreenCanvasGL = this.offscreenCanvas.getContext("webgl");
                this.webglObj = createWebGL(this.offscreenCanvasGL)
                this.draw = function (ts, y, u, v) {
                    this.webglObj.render(w, h, Module.HEAPU8.subarray(y, y + size), Module.HEAPU8.subarray(u, u + qsize), Module.HEAPU8.subarray(v, v + (qsize)))
                    let image_bitmap = this.offscreenCanvas.transferToImageBitmap();
                    postMessage({
                        cmd: WORKER_CMD_TYPE.render,
                        buffer: image_bitmap,
                        delay: this.delay,
                        ts
                    }, [image_bitmap])
                }
            } else {
                this.draw = function (ts, y, u, v) {
                    var yuv = [Module.HEAPU8.subarray(y, y + size), Module.HEAPU8.subarray(u, u + qsize), Module.HEAPU8.subarray(v, v + (qsize))];
                    var outputArray = yuv.map(buffer => Uint8Array.from(buffer))
                    postMessage({
                        cmd: WORKER_CMD_TYPE.render,
                        output: outputArray,
                        delay: this.delay,
                        ts
                    }, outputArray.map(x => x.buffer))
                }
            }
        },
        getDelay: function (timestamp) {
            if (!timestamp) {
                return -1
            }
            if (!this.firstTimestamp) {
                this.firstTimestamp = timestamp
                this.startTimestamp = Date.now()
                this.delay = -1;
            } else {
                if (timestamp) {
                    this.delay = (Date.now() - this.startTimestamp) - (timestamp - this.firstTimestamp)
                }
            }
            return this.delay
        },
        resetDelay: function () {
            this.firstTimestamp = null;
            this.startTimestamp = null;
            this.delay = -1;
        },

        init: function () {
            decoder.opt.debug && console.log('Jessibuca: [worker] init');
            const _doDecode = (data) => {
                // this.opt.debug && console.log('Jessibuca: [worker]: _doDecode');
                if (decoder.opt.useWCS && decoder.useOffscreen() && data.type === MEDIA_TYPE.video && wcsVideoDecoder.decode) {
                    wcsVideoDecoder.decode(data.payload, data.ts)
                } else {
                    // this.opt.debug && console.log('Jessibuca: [worker]: _doDecode  wasm');
                    data.decoder.decode(data.payload, data.ts)
                }
            }
            const loop = () => {
                if (buffer.length) {
                    if (this.dropping) {
                        // // dropping
                        data = buffer.shift();
                        //
                        if (data.type === MEDIA_TYPE.audio && data.payload[1] === 0) {
                            _doDecode(data);
                        }
                        while (!data.isIFrame && buffer.length) {
                            // dropping
                            data = buffer.shift();
                            //
                            if (data.type === MEDIA_TYPE.audio && data.payload[1] === 0) {
                                _doDecode(data);
                            }
                        }
                        if (data.isIFrame) {
                            this.dropping = false;
                            _doDecode(data);
                        }
                    } else {
                        var data = buffer[0]
                        if (this.getDelay(data.ts) === -1) {
                            // this.opt.debug && console.log('Jessibuca: [worker]: common dumex delay is -1');
                            buffer.shift()
                            _doDecode(data);
                        } else if (this.delay > this.opt.videoBuffer + 1000) {
                            // this.opt.debug && console.log('Jessibuca: [worker]:', `delay is ${this.delay}, set dropping is true`);
                            this.resetDelay();
                            this.dropping = true
                        } else {
                            while (buffer.length) {
                                data = buffer[0]
                                if (this.getDelay(data.ts) > this.opt.videoBuffer) {
                                    // 丢帧。。。
                                    buffer.shift()
                                    _doDecode(data);
                                } else {
                                    // this.opt.debug && console.log('Jessibuca: [worker]:', `delay is ${this.delay}`);
                                    break
                                }
                            }
                        }
                    }
                }
            }
            this.stopId = setInterval(loop, 10);
        },
        close: function () {
            this.opt.debug && console.log('Jessibuca: [worker]: close');
            clearInterval(this.stopId);
            this.stopId = null;
            audioDecoder.clear();
            videoDecoder.clear();
            wcsVideoDecoder.reset && wcsVideoDecoder.reset();
            this.firstTimestamp = null;
            this.startTimestamp = null;
            this.delay = -1;
            this.dropping = false;

            if (this.webglObj) {
                this.webglObj.destroy();
                this.offscreenCanvas = null;
                this.offscreenCanvasGL = null;
                this.offscreenCanvasCtx = null;
            }
            buffer = [];
            delete this.playAudioPlanar;
            delete this.draw;
        },
        pushBuffer: function (bufferData, options) {
            // 音频
            if (options.type === MEDIA_TYPE.audio) {
                buffer.push({
                    ts: options.ts,
                    payload: bufferData,
                    decoder: audioDecoder,
                    type: MEDIA_TYPE.audio,
                })
            } else if (options.type === MEDIA_TYPE.video) {
                buffer.push({
                    ts: options.ts,
                    payload: bufferData,
                    decoder: videoDecoder,
                    type: MEDIA_TYPE.video,
                    isIFrame: options.isIFrame
                })
            }
        }
    }
    var audioDecoder = new Module.AudioDecoder(decoder)
    var videoDecoder = new Module.VideoDecoder(decoder)
    postMessage({cmd: WORKER_SEND_TYPE.init})
    self.onmessage = function (event) {
        var msg = event.data
        switch (msg.cmd) {
            case WORKER_SEND_TYPE.init:
                decoder.opt = JSON.parse(msg.opt)
                audioDecoder.sample_rate = msg.sampleRate
                decoder.init();
                break
            case WORKER_SEND_TYPE.decode:
                decoder.pushBuffer(msg.buffer, msg.options)
                break
            case WORKER_SEND_TYPE.audioDecode:
                audioDecoder.decode(msg.buffer, msg.ts);
                break;
            case WORKER_SEND_TYPE.videoDecode:
                videoDecoder.decode(msg.buffer, msg.ts);
                break;
            case WORKER_SEND_TYPE.close:
                decoder.close()
                break
        }
    }

}
