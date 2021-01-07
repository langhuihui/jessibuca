/**
 * @param opt
 *        container:
 *        contextOptions：
 *        videoBuffer：
 *        forceNoGL：
 *        mute：
 *        decoder：
 * @constructor
 */
function Jessibuca(opt) {
    this.canvasElement = document.createElement("canvas");
    this.canvasElement.style.position = "absolute"
    this.canvasElement.style.top = 0
    this.canvasElement.style.left = 0
    opt.container.appendChild(this.canvasElement)
    this.width = opt.container.clientWidth
    this.height = opt.container.clientHeight
    this.container = opt.container
    this.container.style.overflow = "hidden"
    this.containerOldPostion = {
        position: this.container.style.position,
        top: this.container.style.top,
        left: this.container.style.left,
        width: this.container.style.width,
        height: this.container.style.height
    }
    if (this.containerOldPostion.position != "absolute")
        this.container.style.position = "relative"
    this.contextOptions = opt.contextOptions;
    this.videoBuffer = opt.videoBuffer || 0
    if (!opt.forceNoGL) this.initContextGL();
    this.audioContext = new (window.AudioContext || window.webkitAudioContext)();
    if (opt.mute) {
        this.audioEnabled(true)
        this.audioEnabled(false)
    }
    if (this.contextGL) {
        this.initProgram();
        this.initBuffers();
        this.initTextures();
    }
    this.onresize = () => this.resize()
    window.addEventListener("resize", this.onresize)
    this.decoderWorker = new Worker(opt.decoder || 'ff.js')
    var _this = this;
    this.decoderWorker.onmessage = function (event) {
        var msg = event.data
        switch (msg.cmd) {
            case "init":
                console.log("decoder worker init")
                this.postMessage({cmd: "setVideoBuffer", time: _this.videoBuffer})
                if (_this.onLoad) {
                    _this.onLoad()
                    delete _this.onLoad;
                }
                break
            case "initSize":
                _this.canvasElement.width = msg.w
                _this.canvasElement.height = msg.h
                _this.resize()
                console.log("video size:", msg.w, msg.h)
                if (_this.isWebGL()) {

                } else {
                    _this.initRGB(msg.w, msg.h)
                }
                break
            case "render":
                if (_this.onPlay) {
                    _this.onPlay()
                    // delete _this.onPlay;
                }
                if (_this.contextGL) {
                    _this.drawNextOuptutPictureGL(msg.output);
                } else {
                    _this.drawNextOuptutPictureRGBA(msg.buffer);
                }
                _this._initBtns();
                break
            case "initAudio":
                _this.initAudioPlay(msg.frameCount, msg.samplerate, msg.channels)
                break
            case "playAudio":
                _this.playAudio(msg.buffer)
                break
            case "print":
                if (_this.onLog) {
                    _this.onLog(msg.text)
                }
                console.log(msg.text);
                break
            case "printErr":
                if (_this.onLog) {
                    _this.onLog(msg.text)
                }
                console.error(msg.text);
                break
            default:
                _this[msg.cmd](msg)
        }
    }

    this.container.addEventListener('dblclick', function () {
        _this.fullscreen = !_this.fullscreen;
    }, false);
    this.doms = _initDom(this.container);

    this._initEventListener();
};

function _initDom(container) {
    var playBase64 = 'data:image/png;base64,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';
    var stopBase64 = 'data:image/png;base64,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';
    var screenshotBase64 = 'data:image/png;base64,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';
    var fullscreenBase64 = 'data:image/png;base64,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';
    function _setStyle(dom, cssObj) {
        Object.keys(cssObj).forEach((key) => {
            dom.style[key] = cssObj[key];
        })
    }

    var fragment = document.createDocumentFragment();
    var btnWrap = document.createElement('div');
    var control1 = document.createElement('div');
    var control2 = document.createElement('div');
    var textDom = document.createElement('div');
    var speedDom = document.createElement('div');
    var playDom = document.createElement('div');
    var stopDom = document.createElement('div');
    var screenshotsDom = document.createElement('div');
    var fullscreenDom = document.createElement('div');
    var loadingDom = document.createElement('div');
    loadingDom.innerText = '加载中......';
    var wrapStyle = {
        height: '25px',
        zIndex: 11,
        position: 'absolute',
        left: 0,
        bottom: 0,
        width: '100%',
        background: 'rgba(0,0,0)'
    };
    //
    var loadingStyle = {
        position: 'absolute',
        width: '100%',
        height: '100%',
        textAlign: 'center',
        color: "#fff",
        display: 'none'
    };

    var controlStyle = {
        position: 'absolute',
        top: 0,
        height: '100%',
        display: 'flex',
        alignItems: 'center',
    };
    var styleObj = {
        display: 'inline-block',
        position: 'relative',
        fontSize: '13px',
        color: '#fff',
        lineHeight: '20px'
    };

    var styleObj2 = {
        display: 'none',
        position: 'relative',
        width: '15px',
        height: '15px',
        marginLeft: '5px',
        marginRight: '5px',
        backgroundRepeat: "no-repeat",
        backgroundPosition: "center",
        backgroundSize: '100%',
        cursor: 'pointer',
    };

    _setStyle(btnWrap, wrapStyle);
    _setStyle(loadingDom, loadingStyle);
    _setStyle(control1, Object.assign({}, controlStyle, {
        left: 0
    }));
    _setStyle(control2, Object.assign({}, controlStyle, {
        right: 0
    }));
    _setStyle(textDom, styleObj);
    _setStyle(speedDom, styleObj);
    _setStyle(playDom, Object.assign({}, styleObj2, {
        backgroundImage: "url('" + playBase64 + "')",
    }));

    _setStyle(stopDom, Object.assign({}, styleObj2, {
        backgroundImage: "url('" + stopBase64 + "')"
    }));

    _setStyle(screenshotsDom, Object.assign({}, styleObj2, {
        backgroundImage: "url('" + screenshotBase64 + "')"
    }));


    _setStyle(fullscreenDom, Object.assign({}, styleObj2, {
        backgroundImage: "url('" + fullscreenBase64 + "')"
    }));

    control1.appendChild(textDom);
    control1.appendChild(speedDom);
    control2.appendChild(playDom);
    control2.appendChild(stopDom);
    control2.appendChild(screenshotsDom);
    control2.appendChild(fullscreenDom);
    btnWrap.appendChild(control1);
    btnWrap.appendChild(control2);
    fragment.appendChild(loadingDom);
    fragment.appendChild(btnWrap);


    container.appendChild(fragment);
    return {
        textDom,
        speedDom,
        playDom,
        stopDom,
        loadingDom,
        screenshotsDom,
        fullscreenDom
    };
}

Jessibuca.prototype._initEventListener = function () {
    var _this = this;
    this.doms.playDom.addEventListener('click', function () {
        _this.play();
        _this.doms.stopDom.style.display = 'block';
        _this.doms.playDom.style.display = 'none';
    }, false);

    this.doms.stopDom.addEventListener('click', function () {
        _this.close();
        _this.doms.playDom.style.display = 'block';
        _this.doms.stopDom.style.display = 'none';
    }, false);

    // screenshots
    this.doms.screenshotsDom.addEventListener('click', function () {
        const dataURL = _this.canvasElement.toDataURL('image/png');
        _downloadImg(_dataURLToFile(dataURL));
    }, false)

    this.doms.fullscreenDom.addEventListener('click', function () {
       this.fullscreen = true;
        _this.doms.screenshotsDom.style.display = 'none';
    }, false)};

Jessibuca.prototype._initBtns = function () {
    if (this.doms.loadingDom) {
        this.doms.loadingDom.style.display = 'none';
    }

    // show
    this.doms.stopDom.style.display = 'block';
    this.doms.screenshotsDom.style.display = 'block';
    this.doms.fullscreenDom.style.display = 'block';

    // hide
    this.doms.playDom.style.display = 'none';
}

Jessibuca.prototype.initAudioPlanar = function (msg) {
    var channels = msg.channels
    var samplerate = msg.samplerate
    console.log("initAudioPlanar:", "channles:", channels, "samplerate:", samplerate)
    var context = this.audioContext;
    var isPlaying = false;
    var audioBuffers = [];
    if (!context) return false;
    var _this = this
    this.playAudio = function (buffer) {
        var frameCount = buffer[0][0].length
        var audioBuffer = context.createBuffer(channels, frameCount * buffer.length, samplerate);
        var copyToCtxBuffer = function (fromBuffer) {
            for (var channel = 0; channel < channels; channel++) {
                var nowBuffering = audioBuffer.getChannelData(channel);
                for (var j = 0; j < buffer.length; j++) {
                    for (var i = 0; i < frameCount; i++) {
                        nowBuffering[i + j * frameCount] = fromBuffer[j][channel][i]
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
            if (!fromBuffer) return
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

function _dataURLToFile(dataURL) {
    const arr = dataURL.split(",");
    const bstr = atob(arr[1]);
    const type = arr[0].replace("data:", "").replace(";base64", "")
    let n = bstr.length, u8arr = new Uint8Array(n);
    while (n--) {
        u8arr[n] = bstr.charCodeAt(n);
    }
    return new File([u8arr], 'file', {type});
}

function _downloadImg(content) {
    const aLink = document.createElement("a");
    aLink.download = '' + new Date().getTime();
    aLink.href = URL.createObjectURL(content);
    aLink.click();
    URL.revokeObjectURL(content);
}

/**
 * set audio
 * @param flag
 */
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
        if (audioBuffers.length) {
            playBuffer(audioBuffers.shift())
        }
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
    };
    var playAudio = function (fromBuffer) {
        if (isPlaying) {
            audioBuffers.push(fromBuffer);
            return;
        }
        isPlaying = true;
        copyToCtxBuffer(fromBuffer);
        var source = context.createBufferSource();
        source.buffer = audioBuffer;
        source.connect(context.destination);
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
            var contextOptions = {preserveDrawingBuffer: true};
            if (this.contextOptions) {
                contextOptions = Object.assign(contextOptions, this.contextOptions);
            }

            gl = canvas.getContext(contextName, contextOptions);
        } catch (e) {
            gl = null;
        }

        if (!gl || typeof gl.getParameter !== "function") {
            gl = null;
        }

        ++nameIndex;
    }
    ;

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
Jessibuca.prototype.drawNextOutputPicture = function (data) {
    if (this.contextGL) {
        this.drawNextOuptutPictureGL(data);
    } else {
        this.drawNextOuptutPictureRGBA(data);
    }
};

/**
 * Draw the next output picture using WebGL
 */
Jessibuca.prototype.drawNextOuptutPictureGL = function (data) {
    var gl = this.contextGL;
    var texturePosBuffer = this.texturePosBuffer;
    var yTextureRef = this.yTextureRef;
    var uTextureRef = this.uTextureRef;
    var vTextureRef = this.vTextureRef;
    var croppingParams = this.croppingParams
    var width = this.canvasElement.width
    var height = this.canvasElement.height
    if (croppingParams) {
        gl.viewport(0, 0, croppingParams.width, croppingParams.height);
        var tTop = croppingParams.top / height;
        var tLeft = croppingParams.left / width;
        var tBottom = croppingParams.height / height;
        var tRight = croppingParams.width / width;
        var texturePosValues = new Float32Array([tRight, tTop, tLeft, tTop, tRight, tBottom, tLeft, tBottom]);

        gl.bindBuffer(gl.ARRAY_BUFFER, texturePosBuffer);
        gl.bufferData(gl.ARRAY_BUFFER, texturePosValues, gl.DYNAMIC_DRAW);
    } else {
        gl.viewport(0, 0, this.canvasElement.width, this.canvasElement.height);
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
Jessibuca.prototype.drawNextOuptutPictureRGBA = function (data) {
    // var canvas = this.canvasElement;
    //var argbData = data;
    //var ctx = canvas.getContext('2d');
    // var imageData = ctx.getImageData(0, 0, width, height);
    //this.imageData = this.ctx2d.getImageData(0, 0, width, height);
    this.imageData.data.set(data);
    //Module.print(typeof this.imageData.data);
    var croppingParams = this.croppingParams
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
    this.decoderWorker.postMessage({cmd: "close"})
    this.contextGL.clear(this.contextGL.COLOR_BUFFER_BIT);

    if (this.onClose) {
        this.onClose();
        // delete this.onClose;
    }
}
Jessibuca.prototype.destroy = function () {
    this.decoderWorker.terminate()
    window.removeEventListener("resize", this.onresize)
}
/**
 * play
 * @param url
 */
Jessibuca.prototype.play = function (url) {
    // show loading
    if (this.doms.loadingDom) {
        this.doms.loadingDom.style.display = 'block';
    }
    if (url) {
        this.playUrl = url;
    } else if (this.playUrl) {
        url = this.playUrl
    }

    this.decoderWorker.postMessage({cmd: "play", url: url, isWebGL: this.isWebGL()})
}

Object.defineProperty(Jessibuca.prototype, "fullscreen", {
    set(value) {
        if (value) {
            this.container.requestFullscreen()
        } else {
            document.exitFullscreen();
        }
    }
})
/**
 * resize
 */
Jessibuca.prototype.resize = function () {
    this.width = this.container.clientWidth
    this.height = this.container.clientHeight
    var wScale = this.width / this.canvasElement.width
    var hScale = this.height / this.canvasElement.height
    var scale = wScale > hScale ? hScale : wScale
    this.canvasElement.style.transform = "scale(" + scale + ")"
    this.canvasElement.style.left = ((this.width - this.canvasElement.width) / 2) + "px"
    this.canvasElement.style.top = ((this.height - this.canvasElement.height) / 2) + "px"
}
/**
 * change buffer
 * @param buffer
 */
Jessibuca.prototype.changeBuffer = function (buffer) {
    this.decoderWorker.postMessage({cmd: "setVideoBuffer", time: Number(buffer)});
}

export default Jessibuca
