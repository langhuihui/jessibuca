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
        //
        this.audioEnabled(true);
        // default setting 0
        this.gainNode.gain.value = 0;

        this.playing = false;


        this.audioInfo = {
            encType: '',
            channels: '',
            sampleRate: ''
        }
        this.player.debug.log('AudioContext', 'init');
    }

    updateAudioInfo(data) {
        if (data.encTypeCode) {
            this.audioInfo.encType = AUDIO_ENC_TYPE[data.encTypeCode];
        }

        if (data.channels) {
            this.audioInfo.channels = data.channels;
        }

        if (data.sampleRate) {
            this.audioInfo.sampleRate = data.sampleRate;
        }

        // audio 基本信息
        if (this.audioInfo.sampleRate && this.audioInfo.channels && this.audioInfo.encTypeCode) {

            this.player.emit(EVENTS.audioInfo, this.audioInfo);
        }
    }

    //
    get isPlaying() {
        return this.playing;
    }

    get isMute() {
        return this.gainNode.gain.value === 0 || this.isStateSuspended();
    }

    get volume() {
        return this.gainNode.gain.value;
    }

    get bufferSize() {
        return this.bufferList.length;
    }


    initScriptNode() {
        this.playing = true;

        if (this.hasInitScriptNode) {
            return;
        }
        const channels = this.audioInfo.channels;

        const scriptNode = this.audioContext.createScriptProcessor(1024, 0, channels);

        scriptNode.onaudioprocess = (audioProcessingEvent) => {
            const outputBuffer = audioProcessingEvent.outputBuffer;

            if (this.bufferList.length && this.playing) {
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
        if (flag) {

            if (!this.isMute) {
                this.player.emit(EVENTS.mute, flag);
            }
            this.setVolume(0);
            this.audioEnabled(false);
            this.clear();
        } else {
            if (this.isMute) {
                this.player.emit(EVENTS.mute, flag);
            }
            this.setVolume(0.5);
            this.audioEnabled(true);
        }
    }

    setVolume(volume) {
        volume = parseFloat(volume).toFixed(2);
        if (isNaN(volume)) {
            return;
        }
        this.audioEnabled(true);
        volume = clamp(volume, 0, 1);
        this.gainNode.gain.value = volume;
        this.gainNode.gain.setValueAtTime(volume, this.audioContext.currentTime);
        this.player.emit(EVENTS.volumechange, this.player.volume);
    }

    closeAudio() {
        if (this.hasInitScriptNode) {
            this.scriptNode && this.scriptNode.disconnect(this.gainNode);
            this.gainNode && this.gainNode.disconnect(this.audioContext.destination);
            this.gainNode && this.gainNode.disconnect(this.mediaStreamAudioDestinationNode);
        }
        this.clear();
    }

    // 是否播放。。。
    audioEnabled(flag) {
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

    isStateRunning() {
        return this.audioContext.state === 'running';
    }

    isStateSuspended() {
        return this.audioContext.state === 'suspended';
    }

    clear() {
        // 全部清空。
        while (this.bufferList.length) {
            this.bufferList.shift();
        }
    }

    play(buffer, ts) {
        this.bufferList.push(buffer);
    }

    pause() {
        this.playing = false;
        this.clear();
    }

    resume() {
        this.playing = true;
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
