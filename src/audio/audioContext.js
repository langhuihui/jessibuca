import { audioContextUnlock } from "../utils";

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
        let lastBuffer
        const scriptNode = context.createScriptProcessor(1024, 0, 2);
        scriptNode.onaudioprocess = function (audioProcessingEvent) {
            if (_audioPlayBuffers.length) {
                const buffer = _audioPlayBuffers.shift()
                for (let channel = 0; channel < 2; channel++) {
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
            if (lastBuffer) {
                let need = 1024 - lastBuffer[0].length
                if (fromBuffer[0].length >= need) {
                    _audioPlayBuffers.push(lastBuffer.map((x, i) => x.concat(fromBuffer[i].slice(0, need))));
                    lastBuffer = null
                    if (fromBuffer[0].length > need) {
                        jessibuca._playAudio(fromBuffer.map(x => x.slice(need)))
                    }
                } else {
                    lastBuffer.forEach((x, i) => x.push(...fromBuffer[i]));
                }
            } else {
                let remain = fromBuffer[0].length
                let start = 0
                while (remain) {
                    if (remain >= 1024) {
                        remain -= 1024
                        _audioPlayBuffers.push(fromBuffer.map(x => x.slice(start, 1024 + start)));
                        start += 1024
                    } else {
                        remain = 0
                        lastBuffer = fromBuffer.map(x => x.slice(start))
                        console.log(lastBuffer)
                        break
                    }
                }
            }
        }
    }


    jessibuca._destroyAudioContext = () => {
        jessibuca._audioContext.close();
        jessibuca._audioContext = null;
        jessibuca._gainNode = null;
    }
}