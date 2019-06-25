mergeInto(LibraryManager.library, {
    init: function () {
        var AudioContext = (window.AudioContext || window.webkitAudioContext || window.mozAudioContext)
        var context = AudioContext ? new AudioContext() : null
        if (!context) {
            cc.warn('no audio support')
        }
        const MVP = `
        uniform mat4 viewProj;
        attribute vec3 a_position;
        attribute vec2 a_uv0;
        varying vec2 uv0;
        void main () {
            vec4 pos = viewProj * vec4(a_position, 1);
            gl_Position = pos;
            uv0 = a_uv0;
        }`;
        const fragmentShaderScript = `
        precision highp float;
        varying highp vec2 uv0;
        uniform sampler2D ySampler;
        uniform sampler2D uSampler;
        uniform sampler2D vSampler;
        const mat4 YUV2RGB = mat4
        (
        1.1643828125, 0, 1.59602734375, -.87078515625,
        1.1643828125, -.39176171875, -.81296875, .52959375,
        1.1643828125, 2.017234375, 0, -1.081390625,
        0, 0, 0, 1
        );
        void main(void) {
        highp float y = texture2D(ySampler,  uv0).r;
        highp float u = texture2D(uSampler,  uv0).r;
        highp float v = texture2D(vSampler,  uv0).r;
        gl_FragColor = vec4(y, u, v, 1) * YUV2RGB;
        }`

        const renderer = cc.renderer;
        const renderEngine = renderer.renderEngine;
        const Texture2D = renderEngine.Texture2D;
        const gfx = renderEngine.gfx;
        const Pass = renderEngine.renderer.Pass;
        const Technique = renderEngine.renderer.Technique;
        const Effect = renderEngine.renderer.Effect;
        const PARAM_TEXTURE_2D = renderEngine.renderer.PARAM_TEXTURE_2D;
        const option = { format: gfx.TEXTURE_FMT_L8, minFilter: gfx.FILTER_LINEAR, magFilter: gfx.FILTER_LINEAR, wrapS: gfx.WRAP_CLAMP, wrapT: gfx.WRAP_CLAMP }
        const YUV2RGB_Material = cc.Class({
            extends: renderEngine.Material,
            create(sprite) {
                sprite.setState(0)
                let name = "YUV2RGB";
                let lib = renderer._forward._programLib;
                !lib._templates[name] && lib.define(name, MVP, fragmentShaderScript, []);
                let pass = new Pass(name);
                pass.setDepth(false, false);
                pass.setCullMode(gfx.CULL_NONE);
                pass.setBlend(
                    gfx.BLEND_FUNC_ADD,
                    gfx.BLEND_SRC_ALPHA,
                    gfx.BLEND_ONE_MINUS_SRC_ALPHA,
                    gfx.BLEND_FUNC_ADD,
                    gfx.BLEND_SRC_ALPHA,
                    gfx.BLEND_ONE_MINUS_SRC_ALPHA
                );
                let mainTech = new Technique(
                    ['transparent'],
                    [
                        { name: 'ySampler', type: PARAM_TEXTURE_2D, val: this.ySampler = new Texture2D(renderer.device, option) },
                        { name: 'uSampler', type: PARAM_TEXTURE_2D, val: this.uSampler = new Texture2D(renderer.device, option) },
                        { name: 'vSampler', type: PARAM_TEXTURE_2D, val: this.vSampler = new Texture2D(renderer.device, option) },
                    ],
                    [pass]
                );
                this._effect = this.effect = new Effect([mainTech], {}, []);
                this._mainTech = mainTech;
                this._texture = sprite.spriteFrame.getTexture();
                this.updateHash();
                sprite._state = 114;
                sprite._material = sprite._renderData._material = this;
                this.samplerOptions = [{ level: 0 }, { level: 0 }, { level: 0 }]
                return this
            },
            isWebGL() { return true },
            drawNextOutputPicture() {
                this.updateHash(Math.random().toString(36))
                this.ySampler.updateImage(this.samplerOptions[0])
                this.uSampler.updateImage(this.samplerOptions[1])
                this.vSampler.updateImage(this.samplerOptions[2])
            }
        })
        Module.print = text => cc.log(text);
        Module.printErr = text => cc.error(text)
        Module.H5LiveClient = Module.H5LCBase.extend("H5LC", {
            __construct: function (component) {
                cc.dynamicAtlasManager.enabled = false;
                let sprite = component.node.getComponent(cc.Sprite);
                this.webGLCanvas = component._material = new YUV2RGB_Material().create(sprite);
                this.__parent.__construct.call(this, this);
            },
            __destruct: function () {
                this.__parent.__destruct.call(this);
            },
            onError: function (err) {
                cc.log(this, err)
            },
            setStartTime: function (offset) {
                cc.log("first timestamp:", offset)
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
                var _this = this;
                var reconnectCount = 0;
                var reconnectTime = 2000;

                cc.log('H5LiverClient play', url)

                function setWebsocket() {
                    this.$play(url);
                    this.ws.onopen = function () {
                        reconnectCount = 0;
                        reconnectTime = 2000;
                        cc.log("ws open")
                    };
                    this.ws.onclose = function () {
                        _this.isPlaying = false;
                        _this.ws = null;
                        _this.$close();
                        if (reconnectCount > 3) return;
                        reconnectCount++;
                        cc.warn("ws reconnect after " + (reconnectTime / 1000 >> 0) + " second")
                        _this.reconnectId = setTimeout(function () {
                            cc.log("ws reconnecting :", reconnectCount);
                            reconnectTime *= 2;
                            setWebsocket.call(_this);
                        }, reconnectTime)
                    };
                    this.ws.onerror = function () {
                        cc.warn("ws error");
                    };
                }
                setWebsocket.call(this);
            },
            close: function () {
                clearTimeout(this.reconnectId)
                if (!this.isPlaying) return;
                cc.log('close H5LiverClient')
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
                var isPlaying = false;
                var audioBuffers = [];
                var allFrameCount = frameCount * channels;
                if (!context) return false;
                var resampled = samplerate < 22050;
                var audioBuffer = resampled ? context.createBuffer(channels, frameCount << 1, samplerate << 1) : context.createBuffer(channels, frameCount, samplerate);
                var audioOutputArray = HEAP16.subarray(outputPtr, outputPtr + allFrameCount);
                var playNextBuffer = function () {
                    isPlaying = false;
                    if (audioBuffers.length) {
                        playAudio(audioBuffers.shift());
                    }
                    if (audioBuffers.length > 1) audioBuffers.shift();
                    //console.log(audioBuffers.length)
                };
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
                var copyToCtxBuffer = channels > 1 ? function (fromBuffer) {
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
                } : function (fromBuffer) {
                    var nowBuffering = audioBuffer.getChannelData(0);
                    if (fromBuffer) nowBuffering.set(fromBuffer);
                    else copyAudioOutputArray(nowBuffering);
                };
                var playAudio = function (fromBuffer) {
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
            },
            setVideoSize: function (w, h, dataPtr) {
                const size = w * h;
                const size4 = size >> 2;
                const samplerOptions = this.webGLCanvas.samplerOptions
                samplerOptions[0].width = w
                samplerOptions[0].height = h
                samplerOptions[1].width = samplerOptions[2].width = w >> 1
                samplerOptions[1].height = samplerOptions[2].height = h >> 1
                this.draw = function () {
                    var y = HEAPU32[dataPtr];
                    var u = HEAPU32[dataPtr + 1];
                    var v = HEAPU32[dataPtr + 2];
                    samplerOptions[0].image = HEAPU8.subarray(y, y + size)
                    samplerOptions[1].image = HEAPU8.subarray(u, u + size4)
                    samplerOptions[2].image = HEAPU8.subarray(v, v + size4)
                    this.webGLCanvas.drawNextOutputPicture();
                };
            }
        });
    }
})