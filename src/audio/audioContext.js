import { audioContextUnlock } from "../utils";

export default (jessibuca) => {
    let WaveViews = {};
    let bufferArray = []
    let cvs;
    WaveViews.draw = function () {
        //绘制画布
        cvs = document.getElementById("cvs");
        if (typeof (cvs) == 'undefined' || cvs == null) {
            console.error('WaveViews not found')
        } else {
            let ctx = cvs.getContext('2d');
            let audioBuffer = [[], []];
            bufferArray.forEach((val) => {
                audioBuffer[0] = audioBuffer[0].concat(...val[0])
                audioBuffer[1] = audioBuffer[1].concat(...val[1])
            })
            bufferArray.length = 0
            let lth;
            let arr;
            try {
                lth = audioBuffer[0].length;
                arr = audioBuffer[0];
            } catch (e) {
                lth = audioBuffer[1].length;
                arr = audioBuffer[1];
            }
            if (lth == null || arr == null) {
                return;
            }
            ctx.clearRect(0, 0, cvs.width, cvs.height); //清理画布
            let w = Math.floor(lth / cvs.width / 2);
            ctx.fillStyle = '#33ee56'
            var max = cvs.width / 2
            for (let i = 0; i < max; i++) {
                //中线
                ctx.fillRect(i, cvs.height / 2, cvs.width, 1);
                //计算最高值
                let waveHeight = (arr[i * w] * cvs.height) > cvs.height ? cvs.height : (arr[i * w] * cvs.height);
                //双向波纹
                // ctx.fillRect(2*i, (cvs.height - arr[i * w] * cvs.height) , 1, waveHeight*8);
                ctx.fillRect(2 * i, (cvs.height - arr[i * w] * cvs.height) / 2, 1, waveHeight * 8);
            }
        }
    }

    jessibuca._audioContext = new (window.AudioContext || window.webkitAudioContext)();
    jessibuca._gainNode = jessibuca._audioContext.createGain();
    //
    jessibuca._audioEnabled = (flag) => {
        if (flag) {
            audioContextUnlock(jessibuca._audioContext)
            jessibuca._audioEnabled = (flag) => {
                if (flag) {
                    // resume
                    jessibuca._audioContext.resume();

                } else {
                    // suspend
                    jessibuca._audioContext.suspend();
                }
            }
            jessibuca._audioContext.resume();
        } else {
            // suspend
            jessibuca._audioContext.suspend();
        }
    }
    // default not mute
    jessibuca._audioEnabled(true);
    //
    jessibuca._mute = () => {
        jessibuca._audioEnabled(false);
        jessibuca.quieting = true;
    }
    //
    jessibuca._cancelMute = () => {
        jessibuca._audioEnabled(true);
        jessibuca.quieting = false;
    }
    //
    jessibuca._audioResume = () => {
        jessibuca._cancelMute();
    }
    //
    jessibuca._initAudioPlanar = (msg) => {
        const context = jessibuca._audioContext;
        if (!context) return false;
        let _audioPlayBuffers = [];
        const scriptNode = context.createScriptProcessor(1024, 0, 2);
        scriptNode.onaudioprocess = function (audioProcessingEvent) {
            if (_audioPlayBuffers.length) {
                const buffer = _audioPlayBuffers.shift()
                for (let channel = 0; channel < msg.channels; channel++) {
                    const b = buffer[channel]
                    const nowBuffering = audioProcessingEvent.outputBuffer.getChannelData(channel);
                    for (let i = 0; i < 1024; i++) {
                        nowBuffering[i] = b[i]
                    }
                }
            }
        };
        scriptNode.connect(jessibuca._gainNode);
        jessibuca._closeAudio = () => {
            scriptNode.disconnect(jessibuca._gainNode)
            jessibuca._gainNode.disconnect(context.destination);
            delete jessibuca._closeAudio
            _audioPlayBuffers = [];
        }
        jessibuca._gainNode.connect(context.destination);
        jessibuca._playAudio = (fromBuffer) => {
            _audioPlayBuffers.push(fromBuffer)
            bufferArray.push(fromBuffer)
            if (bufferArray.length === 6) {
                WaveViews.draw();
            }
        }
    }
    jessibuca._destroyAudioContext = () => {
        jessibuca._audioContext.close();
        jessibuca._audioContext = null;
        jessibuca._gainNode = null;
    }
}
