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
        const channels = msg.channels
        // const samplerate = msg.samplerate
        const context = jessibuca._audioContext;
        if (!context) return false;
        jessibuca._playAudio = function (buffer) {
            let _audioPlayBuffers = [buffer];
            const frameCount = buffer[0].length
            const scriptNode = context.createScriptProcessor(frameCount, 0, channels);
            scriptNode.onaudioprocess = function (audioProcessingEvent) {
                if (_audioPlayBuffers.length) {
                    const buffer = _audioPlayBuffers.shift()
                    for (let channel = 0; channel < channels; channel++) {
                        const nowBuffering = audioProcessingEvent.outputBuffer.getChannelData(channel);
                        for (let i = 0; i < frameCount; i++) {
                            nowBuffering[i] = buffer[channel][i]
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
                _audioPlayBuffers.push(fromBuffer);
                if (_audioPlayBuffers.length > 5) {
                    _audioPlayBuffers.shift();
                }
            }
        };
    }


    jessibuca._destroyAudioContext = () => {
        jessibuca._audioContext.close();
        jessibuca._audioContext = null;
        jessibuca._gainNode = null;
    }
}