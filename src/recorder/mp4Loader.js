import Emitter from "../utils/emitter";
import {FILE_SUFFIX} from "../constant";
import MP4 from "../remux/mp4-generator";

export default class MP4RecorderLoader extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        this.fileName = '';
        this.fileType = FILE_SUFFIX.webm;
        this.isRecording = false;
        this.recordingTimestamp = 0;
        this.recordingInterval = null;
        this.metaData = {};
        //
        this.metaDataPrame = {
            timescale: 1000,
            duration: 0
        }
        //
        this.track = {

        };
        //
        this.realTrack = {
            type: 'video',
            id: 1,
            codec: null,
            sequenceNumber: 0,
            samples: [],
            refSampleDuration: 0,
            length: 0,
            avcc: new Uint8Array(),
            addSampleNum: 1,
            duration: 0,
            codecWidth: 0,
            codecHeight: 0,
            presentWidth: 0,
            presentHeight: 0,
            sps: new Uint8Array(),
            pps: new Uint8Array(),
            timescale: 0,
        };


        this.mdatBytesLength = 0;


        player.debug.log('MP4Recorder', 'init');
    }

    setFileName(fileName, fileType) {
        this.fileName = fileName;

        if (FILE_SUFFIX.mp4 === fileType || FILE_SUFFIX.webm === fileType) {
            this.fileType = fileType;
        }
    }

    get recording() {
        return this.isRecording;
    }

    get recordTime() {
        return this.recordingTimestamp;
    }

    startRecord() {

    }

    stopRecordAndSave() {
        this.isRecording = false;
        const metaBox = MP4.generateInitSegment(this.metaDataPrame, [this.track], this.mdatBytesLength)

    }

    _reset() {

    }

    destroy() {
        this._reset();
        this.player.debug.log('Recorder', 'destroy');
        this.player = null;
    }
}
