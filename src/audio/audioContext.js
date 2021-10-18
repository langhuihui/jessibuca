import {audioContextUnlock} from "../utils";

export default (jessibuca) => {
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
                console.log("绘制波纹", buffer)
                //绘制波纹
                //WaveViews.draw(buffer);
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
        jessibuca._playAudio = (fromBuffer) => _audioPlayBuffers.push(fromBuffer)
    }


    jessibuca._destroyAudioContext = () => {
        jessibuca._audioContext.close();
        jessibuca._audioContext = null;
        jessibuca._gainNode = null;
    }

    let WaveViews = {};

     //绘制画布
    let cvs = document.getElementById("cvs");
    console.log("WaveViews caverns", cvs)
    if(typeof(cvs) == 'undefined' || cvs == null){
        console.error('WaveViews not found')
    }else{
        let ctx = cvs.getContext('2d');
        console.log("ctx 2d", ctx)
        WaveViews.draw = function (audioBuffer) {
            //console.log(audioBuffer.getChannelData(0))
            let lth;
            let arr;
            try {
                lth = audioBuffer.getChannelData(0).length;
                arr = audioBuffer.getChannelData(0);
            } catch (e) {
                lth = audioBuffer.getChannelData(1).length;
                arr = audioBuffer.getChannelData(1);
            }
            if (lth == null || arr == null) {
                return;
            }
            ctx.clearRect(0, 0, cvs.width, cvs.height); //清理画布
            let w = Math.floor(lth / cvs.width / 2);
            ctx.fillStyle = '#33ee56'
            for (let i = 0; i < cvs.width; i++) {
                //中线
                ctx.fillRect(i, cvs.height / 2, 10, 1);
                //计算最高值
                let waveHeight = (arr[i * w] * cvs.height) > cvs.height ? cvs.height : (arr[i * w] * cvs.height);
                //双向波纹
                //ctx.fillRect(i, (cvs.height - arr[i * w] * cvs.height) , 1, waveHeight*8);
                ctx.fillRect(i, (cvs.height - arr[i * w] * cvs.height) / 2, 1, waveHeight * 8);
            }
            ctx.save();
        }
    }


}
