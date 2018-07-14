mergeInto(LibraryManager.library, {
    init: function() {
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
        /**
         * This class can be used to render output pictures from an H264bsdDecoder to a canvas element.
         * If available the content is rendered using WebGL.
         */
        function H264bsdCanvas(canvas, forceNoGL, contextOptions) {
            this.canvasElement = canvas;
            this.contextOptions = contextOptions;

            if (!forceNoGL) this.initContextGL();

            if (this.contextGL) {
                this.initProgram();
                this.initBuffers();
                this.initTextures();
            };
        };

        /**
         * Returns true if the canvas supports WebGL
         */
        H264bsdCanvas.prototype.isWebGL = function() {
            return !!this.contextGL;
        };

        /**
         * Create the GL context from the canvas element
         */
        H264bsdCanvas.prototype.initContextGL = function() {
            var canvas = this.canvasElement;
            var gl = null;

            var validContextNames = ["webgl", "experimental-webgl", "moz-webgl", "webkit-3d"];
            var nameIndex = 0;

            while (!gl && nameIndex < validContextNames.length) {
                var contextName = validContextNames[nameIndex];

                try {
                    if (this.contextOptions) {
                        gl = canvas.getContext(contextName, this.contextOptions);
                    } else {
                        gl = canvas.getContext(contextName);
                    };
                } catch (e) {
                    gl = null;
                }

                if (!gl || typeof gl.getParameter !== "function") {
                    gl = null;
                }

                ++nameIndex;
            };

            this.contextGL = gl;
        };

        /**
         * Initialize GL shader program
         */
        H264bsdCanvas.prototype.initProgram = function() {
            var gl = this.contextGL;

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

            this.shaderProgram = program;
        };

        /**
         * Initialize vertex buffers and attach to shader program
         */
        H264bsdCanvas.prototype.initBuffers = function() {
            var gl = this.contextGL;
            var program = this.shaderProgram;

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

            this.texturePosBuffer = texturePosBuffer;
        };

        /**
         * Initialize GL textures and attach to shader program
         */
        H264bsdCanvas.prototype.initTextures = function() {
            var gl = this.contextGL;
            var program = this.shaderProgram;

            var yTextureRef = this.initTexture();
            var ySamplerRef = gl.getUniformLocation(program, 'ySampler');
            gl.uniform1i(ySamplerRef, 0);
            this.yTextureRef = yTextureRef;

            var uTextureRef = this.initTexture();
            var uSamplerRef = gl.getUniformLocation(program, 'uSampler');
            gl.uniform1i(uSamplerRef, 1);
            this.uTextureRef = uTextureRef;

            var vTextureRef = this.initTexture();
            var vSamplerRef = gl.getUniformLocation(program, 'vSampler');
            gl.uniform1i(vSamplerRef, 2);
            this.vTextureRef = vTextureRef;
        };

        /**
         * Create and configure a single texture
         */
        H264bsdCanvas.prototype.initTexture = function() {
            var gl = this.contextGL;

            var textureRef = gl.createTexture();
            gl.bindTexture(gl.TEXTURE_2D, textureRef);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
            gl.bindTexture(gl.TEXTURE_2D, null);

            return textureRef;
        };

        /**
         * Draw picture data to the canvas.
         * If this object is using WebGL, the data must be an I420 formatted ArrayBuffer,
         * Otherwise, data must be an RGBA formatted ArrayBuffer.
         */
        H264bsdCanvas.prototype.drawNextOutputPicture = function(width, height, croppingParams, data) {
            var gl = this.contextGL;
            if (gl) {
                this.drawNextOuptutPictureGL(width, height, croppingParams, data);
            } else {
                this.drawNextOuptutPictureRGBA(width, height, croppingParams, data);
            }
        };

        /**
         * Draw the next output picture using WebGL
         */
        H264bsdCanvas.prototype.drawNextOuptutPictureGL = function(width, height, croppingParams, data) {
            var gl = this.contextGL;
            var texturePosBuffer = this.texturePosBuffer;
            var yTextureRef = this.yTextureRef;
            var uTextureRef = this.uTextureRef;
            var vTextureRef = this.vTextureRef;

            if (!croppingParams) {
                gl.viewport(0, 0, width, height);
            } else {
                gl.viewport(0, 0, croppingParams.width, croppingParams.height);

                var tTop = croppingParams.top / height;
                var tLeft = croppingParams.left / width;
                var tBottom = croppingParams.height / height;
                var tRight = croppingParams.width / width;
                var texturePosValues = new Float32Array([tRight, tTop, tLeft, tTop, tRight, tBottom, tLeft, tBottom]);

                gl.bindBuffer(gl.ARRAY_BUFFER, texturePosBuffer);
                gl.bufferData(gl.ARRAY_BUFFER, texturePosValues, gl.DYNAMIC_DRAW);
            }
            gl.activeTexture(gl.TEXTURE0);
            gl.bindTexture(gl.TEXTURE_2D, yTextureRef);
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, width, height, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, data[0]);

            gl.activeTexture(gl.TEXTURE1);
            gl.bindTexture(gl.TEXTURE_2D, uTextureRef);
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, width / 2, height / 2, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, data[1]);

            gl.activeTexture(gl.TEXTURE2);
            gl.bindTexture(gl.TEXTURE_2D, vTextureRef);
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, width / 2, height / 2, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, data[2]);

            gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
        };

        /**
         * Draw next output picture using ARGB data on a 2d canvas.
         */
        H264bsdCanvas.prototype.drawNextOuptutPictureRGBA = function(width, height, croppingParams, data) {
            // var canvas = this.canvasElement;
            //var argbData = data;
            //var ctx = canvas.getContext('2d');
            // var imageData = ctx.getImageData(0, 0, width, height);
            //this.imageData = this.ctx2d.getImageData(0, 0, width, height);
            this.imageData.data.set(data);
            //Module.print(typeof this.imageData.data);
            if (!croppingParams) {
                this.ctx2d.putImageData(this.imageData, 0, 0);
            } else {
                this.ctx2d.putImageData(this.imageData, -croppingParams.left, -croppingParams.top, 0, 0, croppingParams.width, croppingParams.height);
            }
        };
        H264bsdCanvas.prototype.ctx2d = null;
        H264bsdCanvas.prototype.imageData = null;
        H264bsdCanvas.prototype.initRGB = function(width, height) {
            this.ctx2d = this.canvasElement.getContext('2d');
            this.imageData = this.ctx2d.getImageData(0, 0, width, height);
            this.clear = function() {
                this.ctx2d.clearRect(0, 0, width, height)
            };
            //Module.print(this.imageData);
        };
        H264bsdCanvas.prototype.clear = function() {
            this.contextGL.clear(this.contextGL.COLOR_BUFFER_BIT);
        }
        window.WebGLCanvas = H264bsdCanvas;
        if (!Date.now) Date.now = function() {
            return new Date().getTime();
        };
        window.H5LiveClient = Module.H5LCBase.extend("H5LC", {
            __construct: function() {
                this.__parent.__construct.call(this, this);
            },
            __destruct: function() {
                this.__parent.__destruct.call(this);
            },
            onError: function(err) {
                console.log(this, err)
            },
            setStartTime: function(offset) {
                console.log("first timestamp:", offset)
                var startTime = Date.now() - offset;
                this.timespan = function(t) {
                    return t - (Date.now() - startTime);
                }

                function playVideo(_this) {
                    _this.decodeVideoBuffer();
                }
                this.playVideoBuffer = function(t) {
                    // console.log("setTimeout:", t);
                    return setTimeout(playVideo, t, this)
                }
            },
            timespan: function(t) {
                this.setStartTime(t);
                return this.timespan(t);
            },
            play: function(url, canvas, forceNoGL, contextOptions) {
                if (!this.webGLCanvas || this.webGLCanvas.canvasElement != canvas) {
                    this.webGLCanvas = new H264bsdCanvas(canvas, forceNoGL, contextOptions);
                }
                var _this = this;
                var reconnectCount = 0;
                var reconnectTime = 2000;

                function setWebsocket() {
                    this.$play(url);
                    this.ws.onopen = function() {
                        reconnectCount = 0;
                        reconnectTime = 2000;
                        console.log("ws open")
                    };
                    this.ws.onclose = function() {
                        _this.isPlaying = false;
                        _this.ws = null;
                        _this.$close();
                        if (reconnectCount > 3) return;
                        reconnectCount++;
                        console.warn("ws reconnect after " + (reconnectTime / 1000 >> 0) + " second")
                        _this.reconnectId = setTimeout(function() {
                            console.log("ws reconnecting :", reconnectCount);
                            reconnectTime *= 2;
                            setWebsocket.call(_this);
                        }, reconnectTime)
                    };
                    this.ws.onerror = function() {
                        console.warn("ws error");
                    };
                }
                setWebsocket.call(this);
            },
            close: function() {
                if (!this.isPlaying) return;
                clearTimeout(this.reconnectId)
                this.isPlaying = false;
                this.ws.onclose = null;
                this.ws.close();
                this.ws = null;
                this.webGLCanvas.clear();
                this.$close();
                delete this.timespan;
            },
            initAudio: function(frameCount, samplerate, channels, outputPtr) {
                var isPlaying = false;
                var audioBuffers = [];
                var allFrameCount = frameCount * channels;
                if (!context) return false;
                var resampled = samplerate < 22050;
                var audioBuffer = resampled ? context.createBuffer(channels, frameCount << 1, samplerate << 1) : context.createBuffer(channels, frameCount, samplerate);
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
            },
            setVideoSize: function(w, h, dataPtr) {
                this.webGLCanvas.canvasElement.width = w;
                this.webGLCanvas.canvasElement.height = h;
                if (this.webGLCanvas.isWebGL()) {
                    this.draw = function() {
                        var y = HEAPU32[dataPtr];
                        var u = HEAPU32[dataPtr + 1];
                        var v = HEAPU32[dataPtr + 2];
                        // console.log(y, u, v);
                        var outputArray = [HEAPU8.subarray(y, y + w * h), HEAPU8.subarray(u, u + (w * h >> 2)), HEAPU8.subarray(v, v + (w * h >> 2))];
                        this.webGLCanvas.drawNextOutputPicture(w, h, this.croppingParams, outputArray);
                    };
                } else {
                    var outputArray = HEAPU8.subarray(dataPtr, dataPtr + (w * h << 2));
                    this.webGLCanvas.initRGB(w, h);
                    this.draw = function() {
                        this.webGLCanvas.drawNextOutputPicture(w, h, this.croppingParams, outputArray);
                    };
                }
            }
        });
    },

})