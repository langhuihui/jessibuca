interface SamplesObj {

    buffers: Float32Array[],
    pts: number

}


class SpliteBuffer {

    sampleRate:number = 0;
    channels:number  = 0;
    samplesPerPacket:number = 0;
    samplesList: SamplesObj[] = [];
    curSamples:number = 0;

    constructor(sampleRate: number, channels: number, samplesPerPacket: number) {

        this.sampleRate = sampleRate;
        this.channels = channels;
        this.samplesPerPacket = samplesPerPacket;

    }

    addBuffer(buffers: Float32Array[], pts: number) {

        this.samplesList.push({buffers, pts});
        this.curSamples += buffers[0].length;
    }


    spliteOnce(f: (buffers: Float32Array[], pts: number) => void) {

        if (this.curSamples < this.samplesPerPacket) {

            return;
        }

        let newbuffers = [];
        let pts: number = 0;
        let ptsSet = false;

        for(let i = 0; i < this.channels; i++ ) {

            newbuffers.push(new Float32Array(this.samplesPerPacket));
        }

        let needSamples = this.samplesPerPacket;
        let copySamples = 0;

        while(true) {

            if (needSamples === 0) {

                break;
            }

            let first = this.samplesList[0];

            if (!ptsSet) {
                pts = first.pts;
                ptsSet = true;
            }

            if (needSamples >= first.buffers[0].length) {

                for(let i = 0; i < this.channels; i++) {

                    newbuffers[i].set(first.buffers[i], copySamples);
                }

                needSamples -= first.buffers[0].length
                copySamples += first.buffers[0].length;

                this.samplesList.shift();

            }  else {

                for(let i = 0; i < this.channels; i++) {

                    newbuffers[i].set(first.buffers[i].slice(0, needSamples), copySamples);
                    first.buffers[i] = first.buffers[i].slice(needSamples)
                }


                first.pts += Math.floor(needSamples*1000/this.sampleRate); 

                copySamples += needSamples;
                needSamples = 0;
            }

        }

        this.curSamples -= this.samplesPerPacket;

        f(newbuffers, pts);
    
    }

    splite(f: (buffers: Float32Array[], pts: number) => void) {

        while(this.curSamples >= this.samplesPerPacket) {

            this.spliteOnce(f);
        }
    }    

}


export default SpliteBuffer;