import {downloadRecord} from "../utils";
import RecordRTC from 'recordrtc';
import {EVENTS, FILE_SUFFIX} from "../constant";
import Emitter from "../utils/emitter";

export default class RecordRTCLoader extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        this.isRecording = false;
        this.recordingTimestamp = 0;
        this.recordingInterval = null;
        player.debug.log('Recorder', 'init');
    }

    setFileName(fileName) {
        this.fileName = fileName;
    }

    get recording() {
        return this.isRecording;
    }

    get recordTime() {
        return this.recordingTimestamp;
    }

    startRecord() {
        const debug = this.player.debug;
        const options = {
            type: 'video',
            mimeType: 'video/webm;codecs=h264',
            onTimeStamp: (timestamp) => {
                debug.log('Recorder', 'record timestamp :' + timestamp);
            },
            disableLogs: !this.player._opt.debug
        }

        try {
            const stream = this.player.video.$videoElement.captureStream(25);
            const audioStream = this.player.audio.mediaStreamAudioDestinationNode.stream;
            stream.addTrack(audioStream.getAudioTracks()[0]);
            this.recorder = RecordRTC(stream, options);
        } catch (e) {
            debug.error('Recorder', e);
            this.emit(EVENTS.recordCreateError);
        }
        if (this.recorder) {
            this.isRecording = true;
            this.emit(EVENTS.recording, true);
            this.recorder.startRecording();
            debug.log('Recorder', 'start recording');
            this.recordingInterval = window.setInterval(() => {
                this.recordingTimestamp += 1;
                this.player.emit(EVENTS.recordingTimestamp, this.recordingTimestamp);
            }, 1000);
        }
    }

    stopRecordAndSave() {
        if (!this.recorder || !this.isRecording) {
            return;
        }
        this.recorder.stopRecording(() => {
            this.player.debug.log('Recorder', 'stop recording');
            downloadRecord(this.recorder.getBlob(), this.fileName, FILE_SUFFIX.mp4);
            this._reset();
            this.emit(EVENTS.recording, false);
        })
    }

    _reset() {
        this.isRecording = false;
        this.recordingTimestamp = 0;
        this.recorder = null;
        this.fileName = null;
        if (this.recordingInterval) {
            clearInterval(this.recordingInterval);
        }
        this.recordingInterval = null;
    }

    destroy() {
        this._reset();
        this.player.debug.log('Recorder', 'destroy');
        this.player = null;
    }
}
