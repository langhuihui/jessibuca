import Emitter from "../utils/emitter";
import {AUDIO_ENC_TYPE, EVENTS, VIDEO_ENC_TYPE} from "../constant";
import {clamp, noop} from "../utils";

export default class AudioContextLoader extends Emitter {
    constructor(player) {
        super();
        this.bufferList = [];
        this.player = player;
        this.scriptNode = null;
        this.hasInitScriptNode = false;
        this.audioContextChannel = null;

        this.audioContext = new (window.AudioContext || window.webkitAudioContext)();
        //
        this.gainNode = this.audioContext.createGain();
        // Get an AudioBufferSourceNode.
        // This is the AudioNode to use when we want to play an AudioBuffer
        const source = this.audioContext.createBufferSource();
        // set the buffer in the AudioBufferSourceNode
        source.buffer = this.audioContext.createBuffer(1, 1, 22050);
        // connect the AudioBufferSourceNode to the
        // destination so we can hear the sound
        source.connect(this.audioContext.destination);
        // noteOn as start
        // start the source playing
        if (source.noteOn) {
            source.noteOn(0);
        } else {
            source.start(0);
        }
        this.audioBufferSourceNode = source;
        //
        this.mediaStreamAudioDestinationNode = this.audioContext.createMediaStreamDestination();
        this._audioEnabled(true);
        // default setting 0
        this.gainNode.gain.value = 0;

        this.audioInfo = {
            encType: '',
            encTypeCode: '',
            channels: '',
            sampleRate: ''
        }
    }


    updateAudioInfo(data) {
        if (data.encTypeCode) {
            this.audioInfo.encTypeCode = data.encTypeCode;
            this.audioInfo.encType = AUDIO_ENC_TYPE[data.encTypeCode];
        }

        if (data.channels) {
            this.audioInfo.channels = data.channels;
        }

        if (data.sampleRate) {
            this.audioInfo.sampleRate = data.sampleRate;
        }
    }

    get isMute() {
        return this.gainNode.gain.value === 0;
    }

    get volume() {
        return this.gainNode.gain.value;
    }

    get bufferSize() {
        return this.bufferList.length;
    }

    initScriptNode() {
        if (this.hasInitScriptNode) {
            return;
        }

        const channels = this.audioInfo.channels;

        const scriptNode = this.audioContext.createScriptProcessor(1024, 0, channels);

        scriptNode.onaudioprocess = (audioProcessingEvent) => {
            const outputBuffer = audioProcessingEvent.outputBuffer;
            if (this.bufferList.length) {
                const buffer = this.bufferList.shift();
                for (let channel = 0; channel < channels; channel++) {
                    const b = buffer[channel]
                    const nowBuffering = outputBuffer.getChannelData(channel);
                    for (let i = 0; i < 1024; i++) {
                        nowBuffering[i] = b[i] || 0
                    }
                }
            }
        }

        scriptNode.connect(this.gainNode);
        this.scriptNode = scriptNode;
        this.gainNode.connect(this.audioContext.destination);
        this.gainNode.connect(this.mediaStreamAudioDestinationNode);
        this.hasInitScriptNode = true;
    }


    mute(flag) {
        this.setVolume(flag ? 0 : 0.5);

        // 如果是静音状态，直接清空buffer
        if (flag) {
            this.clear();
        }
    }

    setVolume(volume) {
        volume = parseFloat(volume).toFixed(2);
        if (isNaN(volume)) {
            return;
        }
        volume = clamp(volume, 0, 1);
        this.gainNode.gain.value = volume;
        this.gainNode.gain.setValueAtTime(volume, this.audioContext.currentTime);
        this.player.emit(EVENTS.volumechange, this.player.volume);
    }

    closeAudio() {
        if (this.hasInitScriptNode) {
            // this.audioBufferSourceNode && this.audioBufferSourceNode.disconnect(this.scriptNode);
            this.scriptNode && this.scriptNode.disconnect(this.gainNode);
            this.gainNode && this.gainNode.disconnect(this.audioContext.destination);
            this.gainNode && this.gainNode.disconnect(this.mediaStreamAudioDestinationNode);
        }
        this.bufferList = [];
    }

    // 是否播放。。。
    _audioEnabled(flag) {
        if (flag) {
            if (this.audioContext.state === 'suspended') {
                // resume
                this.audioContext.resume();
            }
        } else {
            if (this.audioContext.state === 'running') {
                // suspend
                this.audioContext.suspend();
            }
        }
    }

    clear() {
        // 全部清空。
        while (this.bufferList.length) {
            this.bufferList.shift();
        }
    }


    destroy() {
        this.closeAudio();
        this.audioContext.close();
        this.audioContext = null;
        this.gainNode = null;
        if (this.scriptNode) {
            this.scriptNode.onaudioprocess = noop;
            this.scriptNode = null;
        }
        this.audioBufferSourceNode = null;
        this.mediaStreamAudioDestinationNode = null;
        this.hasInitScriptNode = false;
    }

}
