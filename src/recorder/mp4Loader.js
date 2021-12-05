import Emitter from "../utils/emitter";
import {EVENTS, FILE_SUFFIX} from "../constant";
import MP4 from "../remux/mp4-generator";
import {saveBlobToFile} from "../utils";

export default class MP4RecorderLoader extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        this.fileName = '';
        this.fileType = FILE_SUFFIX.webm;
        //
        this.metaDataPrame = {
            timescale: 1000,
            duration: 0
        }

        this.isRecording = false;
        this.recordingTimestamp = 0;
        this.recordingInterval = null;
        this.metaData = {};

        //
        this.track = {};
        //
        this.realTrack = {
            type: 'video',
            id: 1,
            codec: null,
            sequenceNumber: 0,
            samples: [],
            refSampleDuration: 25,// 默认video的fps为25帧，低于25的默认补帧到25
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
        this.recordingInterval = window.setInterval(() => {
            this.recordingTimestamp += 1;
            this.player.emit(EVENTS.recordingTimestamp, this.recordingTimestamp);
        }, 1000);
    }

    //
    handleAddDecoderConfigurationRecord() {
        // 需要update avcc ,sps,pps 数据。
        // this.realTrack.avcc =
    }

    //
    handleAddTrack() {
        let DRFlag = new Uint8Array();
        const isKeyFrameUnit = false;
        const sampleItem = {
            dts: 0,
            pts: 0,
            cts: 0,
            // units: [],
            // size: 0,
            data:'',
            isKeyframe: false,
            duration: 0,
            originalDts: 0,
            flags: {
                isLeading: 0,
                dependsOn: isKeyFrameUnit ? 2 : 1,
                isDependedOn: isKeyFrameUnit ? 1 : 0,
                hasRedundancy: 0,
                isNonSync: isKeyFrameUnit ? 0 : 1
            }
        }
        this.realTrack.sequenceNumber++;
        this.realTrack.samples.push(sampleItem)
        this.metaDataPrame.duration += this.realTrack.refSampleDuration;
    }

    stopRecordAndSave() {
        if (!this.isRecording) {
            return;
        }

        const metaBox = MP4.generateInitSegment(this.metaDataPrame, [this.track], this.mdatBytesLength)

        const blob = new Blob([metaBox], {'type': 'application/octet-stream'})
        const fileName = this.fileName + '.' + this.fileType;
        saveBlobToFile(fileName, blob);

        this._reset();
    }

    _reset() {

        if (this.recordingInterval) {
            clearInterval(this.recordingInterval);
            this.recordingInterval = null;
        }

        this.isRecording = false;
        this.recordingTimestamp = 0;
        this.recordingInterval = null;
        this.metaData = {};

        //
        this.track = {};
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
    }

    destroy() {
        this._reset();
        this.player.debug.log('Recorder', 'destroy');
        this.player = null;
    }
}
