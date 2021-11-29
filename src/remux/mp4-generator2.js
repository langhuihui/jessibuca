class MP4 {

    static init() {
        MP4.types = {
            avc1: [],
            avcC: [],
            hvc1: [],
            hvcC: [],
            btrt: [],
            dinf: [],
            dref: [],
            esds: [],
            ftyp: [],
            hdlr: [],
            mdat: [],
            mdhd: [],
            mdia: [],
            mfhd: [],
            minf: [],
            moof: [],
            moov: [],
            mp4a: [],
            mvex: [],
            mvhd: [],
            sdtp: [],
            stbl: [],
            stco: [],
            stsc: [],
            stsd: [],
            stsz: [],
            stts: [],
            tfdt: [],
            tfhd: [],
            traf: [],
            trak: [],
            trun: [],
            trex: [],
            tkhd: [],
            vmhd: [],
            smhd: []
        };
        for (const e in MP4.types) {
            if (MP4.types.hasOwnProperty(e)) {
                MP4.types[e] = [
                    e.charCodeAt(0),
                    e.charCodeAt(1),
                    e.charCodeAt(2),
                    e.charCodeAt(3)
                ]
            }
        }
        const A = MP4.constants = {};
        A.FTYP = new Uint8Array([105, 115, 111, 109, 0, 0, 0, 1, 105, 115, 111, 109, 97, 118, 99, 49]),
            A.STSD_PREFIX = new Uint8Array([0, 0, 0, 0, 0, 0, 0, 1]),
            A.STTS = new Uint8Array([0, 0, 0, 0, 0, 0, 0, 0]),
            A.STSC = A.STCO = A.STTS,
            A.STSZ = new Uint8Array([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]),
            A.HDLR_VIDEO = new Uint8Array([0, 0, 0, 0, 0, 0, 0, 0, 118, 105, 100, 101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 86, 105, 100, 101, 111, 72, 97, 110, 100, 108, 101, 114, 0]),
            A.HDLR_AUDIO = new Uint8Array([0, 0, 0, 0, 0, 0, 0, 0, 115, 111, 117, 110, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 111, 117, 110, 100, 72, 97, 110, 100, 108, 101, 114, 0]),
            A.DREF = new Uint8Array([0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 12, 117, 114, 108, 32, 0, 0, 0, 1]),
            A.SMHD = new Uint8Array([0, 0, 0, 0, 0, 0, 0, 0]),
            A.VMHD = new Uint8Array([0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0])
    }

    static box(A) {
        let e = 8,
            t = null,
            r = Array.prototype.slice.call(arguments, 1),
            i = r.length;
        for (let h = 0; h < i; h++) {
            e += r[h].byteLength;
        }
        t = new Uint8Array(e),
            t[0] = e >>> 24 & 255,
            t[1] = e >>> 16 & 255,
            t[2] = e >>> 8 & 255,
            t[3] = e & 255,
            t.set(A, 4);
        let o = 8;
        for (let h = 0; h < i; h++) {
            t.set(r[h], o),
                o += r[h].byteLength;
        }
        return t
    }

    static generateInitSegment(A) {
        let e = MP4.box(MP4.types.ftyp, MP4.constants.FTYP),
            t = MP4.moov(A),
            r = new Uint8Array(e.byteLength + t.byteLength);
        return r.set(e, 0), r.set(t, e.byteLength), r
    }

    static moov(A) {
        let e = MP4.mvhd(A.timescale, A.duration),
            t = MP4.trak(A),
            r = MP4.mvex(A);
        return MP4.box(MP4.types.moov, e, t, r)
    }

    static mvhd(A, e) {
        return MP4.box(MP4.types.mvhd, new Uint8Array([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, A >>> 24 & 255, A >>> 16 & 255, A >>> 8 & 255, A & 255, e >>> 24 & 255, e >>> 16 & 255, e >>> 8 & 255, e & 255, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255]))
    }

    static trak(A) {
        return MP4.box(MP4.types.trak, MP4.tkhd(A), MP4.mdia(A))
    }

    static tkhd(A) {
        let e = A.id, t = A.duration, r = A.codecWidth, i = A.codecHeight;
        return MP4.box(MP4.types.tkhd, new Uint8Array([0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, e >>> 24 & 255, e >>> 16 & 255, e >>> 8 & 255, e & 255, 0, 0, 0, 0, t >>> 24 & 255, t >>> 16 & 255, t >>> 8 & 255, t & 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0, r >>> 8 & 255, r & 255, 0, 0, i >>> 8 & 255, i & 255, 0, 0]))
    }

    static mdia(A) {
        return MP4.box(MP4.types.mdia, MP4.mdhd(A), MP4.hdlr(A), MP4.minf(A))
    }

    static mdhd(A) {
        let e = A.timescale, t = A.duration;
        return MP4.box(MP4.types.mdhd, new Uint8Array([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, e >>> 24 & 255, e >>> 16 & 255, e >>> 8 & 255, e & 255, t >>> 24 & 255, t >>> 16 & 255, t >>> 8 & 255, t & 255, 85, 196, 0, 0]))
    }

    static hdlr(A) {
        let e = null;
        return A.type === "audio" ? e = MP4.constants.HDLR_AUDIO : e = MP4.constants.HDLR_VIDEO, MP4.box(MP4.types.hdlr, e)
    }

    static minf(A) {
        let e = null;
        return A.type === "audio" ? e = MP4.box(MP4.types.smhd, MP4.constants.SMHD) : e = MP4.box(MP4.types.vmhd, MP4.constants.VMHD), MP4.box(MP4.types.minf, e, MP4.dinf(), MP4.stbl(A))
    }

    static dinf() {
        return MP4.box(MP4.types.dinf, MP4.box(MP4.types.dref, MP4.constants.DREF))
    }

    static stbl(A) {
        return MP4.box(MP4.types.stbl, MP4.stsd(A), MP4.box(MP4.types.stts, MP4.constants.STTS), MP4.box(MP4.types.stsc, MP4.constants.STSC), MP4.box(MP4.types.stsz, MP4.constants.STSZ), MP4.box(MP4.types.stco, MP4.constants.STCO))
    }

    static stsd(A) {
        return A.type === "audio" ?
            MP4.box(MP4.types.stsd, MP4.constants.STSD_PREFIX, MP4.mp4a(A)) :
            A.videoType === 'avc' ?
                MP4.box(MP4.types.stsd, MP4.constants.STSD_PREFIX, MP4.avc1(A)) :
                MP4.box(MP4.types.stsd, MP4.constants.STSD_PREFIX, MP4.hvc1(A))
    }

    static mp4a(A) {
        let e = A.channelCount, t = A.audioSampleRate,
            r = new Uint8Array([0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, e, 0, 16, 0, 0, 0, 0, t >>> 8 & 255, t & 255, 0, 0]);
        return MP4.box(MP4.types.mp4a, r, MP4.esds(A))
    }

    static esds(A) {
        let e = A.config || [], t = e.length,
            r = new Uint8Array([0, 0, 0, 0, 3, 23 + t, 0, 1, 0, 4, 15 + t, 64, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5].concat([t]).concat(e).concat([6, 1, 2]));
        return MP4.box(MP4.types.esds, r)
    }

    static avc1(A) {
        let e = A.avcc, t = A.codecWidth;
        const r = A.codecHeight;
        let i = new Uint8Array([0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, t >>> 8 & 255, t & 255, r >>> 8 & 255, r & 255, 0, 72, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 255, 255]);
        return MP4.box(MP4.types.avc1, i, MP4.box(MP4.types.avcC, e))
    }

    static hvc1(A) {
        let e = A.avcc, t = A.codecWidth;
        const r = A.codecHeight;
        let i = new Uint8Array([0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, t >>> 8 & 255, t & 255, r >>> 8 & 255, r & 255, 0, 72, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 255, 255]);
        return MP4.box(MP4.types.hvc1, i, MP4.box(MP4.types.hvcC, e))
    }

    static mvex(A) {
        return MP4.box(MP4.types.mvex, MP4.trex(A))
    }

    static trex(A) {
        let e = A.id,
            t = new Uint8Array([0, 0, 0, 0, e >>> 24 & 255, e >>> 16 & 255, e >>> 8 & 255, e & 255, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1]);
        return MP4.box(MP4.types.trex, t)
    }

    static moof(A, e) {
        return MP4.box(MP4.types.moof, MP4.mfhd(A.sequenceNumber), MP4.traf(A, e))
    }

    static mfhd(A) {
        let e = new Uint8Array([0, 0, 0, 0, A >>> 24 & 255, A >>> 16 & 255, A >>> 8 & 255, A & 255]);
        return MP4.box(MP4.types.mfhd, e)
    }

    static traf(A, e) {
        let t = A.id,
            r = MP4.box(MP4.types.tfhd, new Uint8Array([0, 0, 0, 0, t >>> 24 & 255, t >>> 16 & 255, t >>> 8 & 255, t & 255])),
            i = MP4.box(MP4.types.tfdt, new Uint8Array([0, 0, 0, 0, e >>> 24 & 255, e >>> 16 & 255, e >>> 8 & 255, e & 255])),
            o = MP4.sdtp(A), h = MP4.trun(A, o.byteLength + 16 + 16 + 8 + 16 + 8 + 8);
        return MP4.box(MP4.types.traf, r, i, h, o)
    }

    static sdtp(A) {
        let e = new Uint8Array(4 + 1), t = A.flags;
        return e[4] = t.isLeading << 6 | t.dependsOn << 4 | t.isDependedOn << 2 | t.hasRedundancy, MP4.box(MP4.types.sdtp, e)
    }

    static trun(A, e) {
        let t = 12 + 16, r = new Uint8Array(t);
        e += 8 + t, r.set([0, 0, 15, 1, 0, 0, 0, 1, e >>> 24 & 255, e >>> 16 & 255, e >>> 8 & 255, e & 255], 0);
        let i = A.duration, o = A.size, h = A.flags, u = A.cts;
        return r.set([i >>> 24 & 255, i >>> 16 & 255, i >>> 8 & 255, i & 255, o >>> 24 & 255, o >>> 16 & 255, o >>> 8 & 255, o & 255, h.isLeading << 2 | h.dependsOn, h.isDependedOn << 6 | h.hasRedundancy << 4 | h.isNonSync, 0, 0, u >>> 24 & 255, u >>> 16 & 255, u >>> 8 & 255, u & 255], 12), MP4.box(MP4.types.trun, r)
    }

    static mdat(A) {
        return MP4.box(MP4.types.mdat, A)
    }
}

MP4.init();

export default MP4;
