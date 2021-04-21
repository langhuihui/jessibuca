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
            _initProgram: function () {
                var gl = this._contextGL;

                var vertexShaderScript = [
                    'attribute vec4 vertexPos;',
                    'attribute vec4 texturePos;',
                    'varying vec2 textureCoord;',

                    'void main()',
                    '{',
                    'gl_Position = vertexPos;',
                    'textureCoord = texturePos.xy;',
                    '}'
                ].join('\n');

                var fragmentShaderScript = [
                    'precision highp float;',
                    'varying highp vec2 textureCoord;',
                    'uniform sampler2D ySampler;',
                    'uniform sampler2D uSampler;',
                    'uniform sampler2D vSampler;',
                    'const mat4 YUV2RGB = mat4',
                    '(',
                    '1.1643828125, 0, 1.59602734375, -.87078515625,',
                    '1.1643828125, -.39176171875, -.81296875, .52959375,',
                    '1.1643828125, 2.017234375, 0, -1.081390625,',
                    '0, 0, 0, 1',
                    ');',

                    'void main(void) {',
                    'highp float y = texture2D(ySampler,  textureCoord).r;',
                    'highp float u = texture2D(uSampler,  textureCoord).r;',
                    'highp float v = texture2D(vSampler,  textureCoord).r;',
                    'gl_FragColor = vec4(y, u, v, 1) * YUV2RGB;',
                    '}'
                ].join('\n');

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

                this._shaderProgram = program;
            }, _initBuffers: function () {
                var gl = this._contextGL;
                var program = this._shaderProgram;

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

                this._texturePosBuffer = texturePosBuffer;
            }, _initTexture: function () {
                var gl = this._contextGL;

                var textureRef = gl.createTexture();
                gl.bindTexture(gl.TEXTURE_2D, textureRef);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
                gl.bindTexture(gl.TEXTURE_2D, null);

                return textureRef;
            }, _initTextures: function () {
                var gl = this._contextGL;
                var program = this._shaderProgram;

                var yTextureRef = this._initTexture();
                var ySamplerRef = gl.getUniformLocation(program, 'ySampler');
                gl.uniform1i(ySamplerRef, 0);
                this._yTextureRef = yTextureRef;

                var uTextureRef = this._initTexture();
                var uSamplerRef = gl.getUniformLocation(program, 'uSampler');
                gl.uniform1i(uSamplerRef, 1);
                this._uTextureRef = uTextureRef;

                var vTextureRef = this._initTexture();
                var vSamplerRef = gl.getUniformLocation(program, 'vSampler');
                gl.uniform1i(vSamplerRef, 2);
                this._vTextureRef = vTextureRef;
            },
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
                if (url.indexOf("http") == 0) {
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
            fetch: function (url) {
                var _this = this;
                this.controller = new AbortController();
                var signal = this.controller.signal;
                fetch(url, { signal }).then(function (res) {
                    var reader = res.body.getReader();
                    _this.fetchNext = function () {
                        reader.read().then(({ done, value }) => _this.onFetchData({ done, data: value })).catch(console.error)
                    }
                    _this.fetchNext()
                }).catch(console.error)
            },
            close: function () {
                clearTimeout(this.reconnectId)
                if (!this.isPlaying) return;
                console.log('close Jessibuca')
                this.isPlaying = false;
                if (this.ws) {
                    this.ws.onmessage = null;
                    this.ws.onclose = null;
                    this.ws.onerror = null;
                    this.ws.close();
                    this.ws = null;
                }
                if (this.controller) this.controller.abort();
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
                    postMessage({ cmd: "playAudio", buffer: audioOutputArray, ts: ts })
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
            playAudioPlanar(data, len, ts) {
                var outputArray = [];
                var frameCount = len / 4 / this.buffersA.length;
                for (var i = 0; i < this.buffersA.length; i++) {
                    var fp = HEAPU32[(data >> 2) + i] >> 2;
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
                    postMessage({ cmd: "playAudio", buffer: this.audioCache, ts: ts }, this.audioCache.flatMap(outputArray => outputArray.map(x => x.buffer)))
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
                var canvas = new OffscreenCanvas(w, h);
                var gl = canvas.getContext("webgl");
                this._contextGL = gl
                this._initProgram();
                this._initBuffers();
                this._initTextures();
                this.buffers = [[], [], []]
                var size = w * h
                var qsize = size >> 2
                //this.sharedBuffer = new SharedArrayBuffer(size * 3 >> 1);
                if (this.isWebGL) {
                    this.draw = function (compositionTime, ts) {
                        var gl = this._contextGL

                        var y = HEAPU32[dataPtr];
                        var u = HEAPU32[dataPtr + 1];
                        var v = HEAPU32[dataPtr + 2];
                        // console.log(y, u, v);
                        var outputArray = [HEAPU8.subarray(y, y + size), HEAPU8.subarray(u, u + qsize), HEAPU8.subarray(v, v + (qsize))];
                        //arrayBufferCopy(HEAPU8.subarray(y, y + size), this.sharedBuffer, 0, size)
                        //arrayBufferCopy(HEAPU8.subarray(u, u + (qsize)), this.sharedBuffer, size, qsize)
                        //arrayBufferCopy(HEAPU8.subarray(v, v + (qsize)), this.sharedBuffer, size + qsize, qsize)
                        this.setBuffer(outputArray)
                        //var outputArray = [new Uint8Array(this.sharedBuffer, 0, size), new Uint8Array(this.sharedBuffer, size, qsize), new Uint8Array(this.sharedBuffer, size + (qsize), qsize)]
                        if (gl) {
                            var yTextureRef = this._yTextureRef;
                            var uTextureRef = this._uTextureRef;
                            var vTextureRef = this._vTextureRef;
                            gl.viewport(0, 0, w, h);
                            gl.activeTexture(gl.TEXTURE0);
                            gl.bindTexture(gl.TEXTURE_2D, yTextureRef);
                            gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, w, h, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, HEAPU8.subarray(y, y + size));
                            gl.activeTexture(gl.TEXTURE1);
                            gl.bindTexture(gl.TEXTURE_2D, uTextureRef);
                            gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, w / 2, h / 2, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, HEAPU8.subarray(u, u + qsize));
                            gl.activeTexture(gl.TEXTURE2);
                            gl.bindTexture(gl.TEXTURE_2D, vTextureRef);
                            gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, w / 2, h / 2, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, HEAPU8.subarray(v, v + (qsize)));
                            gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
                            let image_bitmap = canvas.transferToImageBitmap();
                            postMessage({ cmd: "render", compositionTime: compositionTime, ts: ts, bps: this.bps, delay: this.delay, buffer: image_bitmap }, [image_bitmap])
                        } else {
                            postMessage({ cmd: "render", compositionTime: compositionTime, ts: ts, bps: this.bps, delay: this.delay, output: outputArray }, outputArray.map(x => x.buffer))
                        }
                    };
                } else {
                    var outputArray = HEAPU8.subarray(dataPtr, dataPtr + (w * h << 2));
                    var output = { cmd: "render", buffer: outputArray }
                    this.draw = function () {
                        postMessage(output)
                    };
                }
            },
            getDelay: function (timestamp) {
                this.firstVideoTimestamp = timestamp
                this.firstTimestamp = Date.now()
                this.getDelay = function (timestamp) {
                    this.delay = (timestamp - this.firstVideoTimestamp) - (Date.now() - this.firstTimestamp)
                    return this.delay
                }
                return 0
            },
            init: function (msg) {
                var canvas = msg.canvas
                var gl = canvas.getContext("webgl");
                this._contextGL = gl
                this._initProgram();
                this._initBuffers();
                this._initTextures();
            }
        });
        var decoder = new Module.Jessibuca()
        self.onmessage = function (event) {
            var msg = event.data
            switch (msg.cmd) {
                case "init":
                    decoder.init(msg)
                    break
                case "getProp":
                    postMessage({ cmd: "getProp", value: decoder[msg.prop] })
                    break
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
                    decoder.videoBuffer = (msg.time * 1000) | 0
                    break
                case "close":
                    decoder.close()
                    break
            }
        }
    },

})
