window.AudioContext = window.AudioContext || window.webkitAudioContext;
function Jessibuca(opt) {
    this.canvasElement = opt.canvas;
    this.contextOptions = opt.contextOptions;
    this.videoBuffer = opt.videoBuffer || 0
    if (!opt.forceNoGL) this.initContextGL();
    this.audioContext = new window.AudioContext();
    if (opt.mute) {
        this.audioEnabled(true)
        this.audioEnabled(false)
    }
    if (this.contextGL) {
        this.initProgram();
        this.initBuffers();
        this.initTextures();
    };
    this.decoderWorker = new Worker(opt.decoder || 'ff.js')
    var _this = this
    this.decoderWorker.onmessage = function (event) {
        var msg = event.data
        switch (msg.cmd) {
            case "init":
                console.log("decoder worker init")
                this.postMessage({ cmd: "setVideoBuffer", time: _this.videoBuffer })
                if (_this.onLoad) {
                    _this.onLoad()
                    delete _this.onLoad;
                }
                break
            case "initSize":
                _this.width = msg.w
                _this.height = msg.h
                if (_this.isWebGL()) {
                    // var buffer = new ArrayBuffer(msg.w * msg.h + (msg.w * msg.h >> 1))
                    // this.postMessage({ cmd: "setBuffer", buffer: buffer }, [buffer])
                }
                else {
                    _this.initRGB(msg.w, msg.h)
                }
                break
            case "render":
                if (_this.onPlay) {
                    _this.onPlay()
                    delete _this.onPlay;
                }
                // if (msg.compositionTime) {
                //     console.log(msg.compositionTime)
                //     setTimeout(draw, msg.compositionTime, msg.output)
                // } else {
                //     draw(msg.output)
                // }
                if (_this.contextGL) {
                    _this.drawNextOuptutPictureGL(_this.width, _this.height, null, msg.output);
                   // this.postMessage({ cmd: "setBuffer", buffer: msg.output }, [msg.output[0].buffer, msg.output[1].buffer, msg.output[2].buffer])
                } else {
                    _this.drawNextOuptutPictureRGBA(_this.width, _this.height, null, msg.buffer);
                }
                break
            case "initAudio":
                _this.initAudioPlay(msg.frameCount, msg.samplerate, msg.channels)
                break
            case "playAudio":
                _this.playAudio(msg.buffer)
                break
            case "print":
                console.log(msg.text);
                break
            case "printErr":
                console.error(msg.text);
                break
            default:
                _this[msg.cmd](msg)
        }
    }
};
Jessibuca.prototype.initAudioPlanar = function (msg) {
    var channels = msg.channels
    var samplerate = msg.samplerate
    console.log("initAudioPlanar:","channles:",channels,"samplerate:",samplerate)
    var context = this.audioContext;
    var isPlaying = false;
    var audioBuffers = [];
    if (!context) return false;
    var _this = this
    this.playAudio = function (buffer) {
        var frameCount = buffer[0][0].length
        var audioBuffer = context.createBuffer(channels, frameCount*buffer.length, samplerate);
        var copyToCtxBuffer = function (fromBuffer) {
            for (var channel = 0; channel < channels; channel++) {
                var nowBuffering = audioBuffer.getChannelData(channel);
                for( var j=0;j<buffer.length;j++){
                    for (var i = 0; i < frameCount; i++) {
                        nowBuffering[i+j*frameCount] = fromBuffer[j][channel][i]
                    }
                    //postMessage({ cmd: "setBufferA", buffer: fromBuffer[j] }, '*', fromBuffer[j].map(x => x.buffer))
                }
            }
        }
        var playNextBuffer = function () {
            isPlaying = false;
            //console.log("~", audioBuffers.length)
            if (audioBuffers.length) {
                playAudio(audioBuffers.shift());
            }
            //if (audioBuffers.length > 1) audioBuffers.shift();
        };
        var playAudio = function (fromBuffer) {
            if(!fromBuffer)return
            if (isPlaying) {
                audioBuffers.push(fromBuffer);
                //console.log(audioBuffers.length)
                return;
            }
            isPlaying = true;
            copyToCtxBuffer(fromBuffer);
            var source = context.createBufferSource();
            source.buffer = audioBuffer;
            source.connect(context.destination);
            // source.onended = playNextBuffer;
            source.start();
        };
        _this.playAudio = playAudio
        _this.audioInterval = setInterval(playNextBuffer, audioBuffer.duration * 1000);
        playAudio(buffer)
    };
}

function _unlock(context) {
    context.resume();
    var source = context.createBufferSource();
    source.buffer = context.createBuffer(1, 1, 22050);
    source.connect(context.destination);
    if (source.noteOn)
        source.noteOn(0);
    else
        source.start(0);
}
// document.addEventListener("mousedown", _unlock, true);
// document.addEventListener("touchend", _unlock, true);
Jessibuca.prototype.audioEnabled = function (flag) {
    if (flag) {
        _unlock(this.audioContext)
        this.audioEnabled = function (flag) {
            if (flag) {
                this.audioContext.resume();
            } else {
                this.audioContext.suspend();
            }
        }
    } else {
        this.audioContext.suspend();
    }
}
Jessibuca.prototype.playAudio = function (data) {
    var context = this.audioContext;
    var isPlaying = false;
    var isDecoding = false;
    if (!context) return false;
    var audioBuffers = [];
    var decodeQueue = []
    var _this = this
    var playNextBuffer = function (e) {
        // isPlaying = false;
        if (audioBuffers.length) {
            playBuffer(audioBuffers.shift())
        }
        //if (audioBuffers.length > 1) audioBuffers.shift();
    };
    var playBuffer = function (buffer) {
        isPlaying = true;
        var audioBufferSouceNode = context.createBufferSource();
        audioBufferSouceNode.buffer = buffer;
        audioBufferSouceNode.connect(context.destination);
        // audioBufferSouceNode.onended = playNextBuffer;
        audioBufferSouceNode.start();
        if (!_this.audioInterval) {
            _this.audioInterval = setInterval(playNextBuffer, buffer.duration * 1000 - 1);
        }
        // setTimeout(playNextBuffer, buffer.duration * 1000)
    }
    var decodeAudio = function () {
        if (decodeQueue.length) {
            context.decodeAudioData(decodeQueue.shift(), tryPlay, decodeAudio);
        } else {
            isDecoding = false
        }
    }
    var tryPlay = function (buffer) {
        decodeAudio()
        if (isPlaying) {
            audioBuffers.push(buffer);
        } else {
            playBuffer(buffer)
        }
    }
    var playAudio = function (data) {
        decodeQueue.push(...data)
        if (!isDecoding) {
            isDecoding = true
            decodeAudio()
        }
    }
    this.playAudio = playAudio
    playAudio(data)
}
Jessibuca.prototype.initAudioPlay = function (frameCount, samplerate, channels) {
    var context = this.audioContext;
    var isPlaying = false;
    var audioBuffers = [];
    if (!context) return false;
    var _this = this
    var resampled = samplerate < 22050;
    if (resampled) {
        console.log("resampled!")
    }
    var audioBuffer = resampled ? context.createBuffer(channels, frameCount << 1, samplerate << 1) : context.createBuffer(channels, frameCount, samplerate);
    var playNextBuffer = function () {
        isPlaying = false;
        //console.log("~", audioBuffers.length)
        if (audioBuffers.length) {
            playAudio(audioBuffers.shift());
        }
        //if (audioBuffers.length > 1) audioBuffers.shift();
    };

    var copyToCtxBuffer = channels > 1 ? function (fromBuffer) {
        for (var channel = 0; channel < channels; channel++) {
            var nowBuffering = audioBuffer.getChannelData(channel);
            if (resampled) {
                for (var i = 0; i < frameCount; i++) {
                    nowBuffering[i * 2] = nowBuffering[i * 2 + 1] = fromBuffer[i * (channel + 1)] / 32768;
                }
            } else
                for (var i = 0; i < frameCount; i++) {
                    nowBuffering[i] = fromBuffer[i * (channel + 1)] / 32768;
                }

        }
    } : function (fromBuffer) {
        var nowBuffering = audioBuffer.getChannelData(0);
        for (var i = 0; i < nowBuffering.length; i++) {
            nowBuffering[i] = fromBuffer[i] / 32768;
        }
        // nowBuffering.set(fromBuffer);
    };
    var playAudio = function (fromBuffer) {
        if (isPlaying) {
            audioBuffers.push(fromBuffer);
            //console.log(audioBuffers.length)
            return;
        }
        isPlaying = true;
        copyToCtxBuffer(fromBuffer);
        var source = context.createBufferSource();
        source.buffer = audioBuffer;
        source.connect(context.destination);
        // source.onended = playNextBuffer;
        if (!_this.audioInterval) {
            _this.audioInterval = setInterval(playNextBuffer, audioBuffer.duration * 1000);
        }
        source.start();
    };
    this.playAudio = playAudio;
}
/**
 * Returns true if the canvas supports WebGL
 */
Jessibuca.prototype.isWebGL = function () {
    return !!this.contextGL;
};

/**
 * Create the GL context from the canvas element
 */
Jessibuca.prototype.initContextGL = function () {
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
Jessibuca.prototype.initProgram = function () {
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
Jessibuca.prototype.initBuffers = function () {
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
Jessibuca.prototype.initTextures = function () {
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
Jessibuca.prototype.initTexture = function () {
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
Jessibuca.prototype.drawNextOutputPicture = function (width, height, croppingParams, data) {
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
Jessibuca.prototype.drawNextOuptutPictureGL = function (width, height, croppingParams, data) {
    var gl = this.contextGL;
    var texturePosBuffer = this.texturePosBuffer;
    var yTextureRef = this.yTextureRef;
    var uTextureRef = this.uTextureRef;
    var vTextureRef = this.vTextureRef;
    this.contextGL.viewport(0, 0, this.canvasElement.width, this.canvasElement.height);
    // if (!croppingParams) {
    //     gl.viewport(0, 0, width, height);
    // } else {
    //     gl.viewport(0, 0, croppingParams.width, croppingParams.height);

    //     var tTop = croppingParams.top / height;
    //     var tLeft = croppingParams.left / width;
    //     var tBottom = croppingParams.height / height;
    //     var tRight = croppingParams.width / width;
    //     var texturePosValues = new Float32Array([tRight, tTop, tLeft, tTop, tRight, tBottom, tLeft, tBottom]);

    //     gl.bindBuffer(gl.ARRAY_BUFFER, texturePosBuffer);
    //     gl.bufferData(gl.ARRAY_BUFFER, texturePosValues, gl.DYNAMIC_DRAW);
    // }
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
Jessibuca.prototype.drawNextOuptutPictureRGBA = function (width, height, croppingParams, data) {
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
Jessibuca.prototype.ctx2d = null;
Jessibuca.prototype.imageData = null;
Jessibuca.prototype.initRGB = function (width, height) {
    this.ctx2d = this.canvasElement.getContext('2d');
    this.imageData = this.ctx2d.getImageData(0, 0, width, height);
    this.clear = function () {
        this.ctx2d.clearRect(0, 0, width, height)
    };
    //Module.print(this.imageData);
};
Jessibuca.prototype.close = function () {
    if (this.audioInterval) {
        clearInterval(this.audioInterval)
    }
    delete this.playAudio
    this.decoderWorker.postMessage({ cmd: "close" })
    this.contextGL.clear(this.contextGL.COLOR_BUFFER_BIT);
}
Jessibuca.prototype.destroy = function () {
    this.decoderWorker.terminate()
}
Jessibuca.prototype.play = function (url) {
    this.decoderWorker.postMessage({ cmd: "play", url: url, isWebGL: this.isWebGL() })
}
