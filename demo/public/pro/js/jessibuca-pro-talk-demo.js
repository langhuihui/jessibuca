!function (e, t) {
    "object" == typeof exports && "undefined" != typeof module ? module.exports = t() : "function" == typeof define && define.amd ? define(t) : (e = "undefined" != typeof globalThis ? globalThis : e || self)["jessibuca-talk"] = t()
}(this, (function () {
    "use strict";

    function e(t, s) {
        const i = r();
        return (e = function (e, t) {
            return i[e -= 112]
        })(t, s)
    }

    const t = e;

    function r() {
        const e = ["2285184GTpcqY", "10DrdigE", "slice", "2527ItbiWP", "2850023AIdfdI", "off", "672pKGrxl", "push", "744ITTKhq", "apply", "2617029XfozFq", "emit", "1804250gORjUu", "keys", "102RPjcTt", "length", "4371TmToyf", "1949upPqSh", "forEach", "once"];
        return (r = function () {
            return e
        })()
    }

    !function (t, r) {
        const s = e, i = t();
        for (; ;) try {
            if (192981 === -parseInt(s(125)) / 1 * (parseInt(s(122)) / 2) + parseInt(s(124)) / 3 * (parseInt(s(116)) / 4) + -parseInt(s(120)) / 5 + parseInt(s(128)) / 6 + -parseInt(s(131)) / 7 * (parseInt(s(114)) / 8) + parseInt(s(118)) / 9 * (parseInt(s(129)) / 10) + -parseInt(s(112)) / 11) break;
            i.push(i.shift())
        } catch (e) {
            i.push(i.shift())
        }
    }(r);

    class s {
        on(t, r, s) {
            const i = e, o = this.e || (this.e = {});
            return (o[t] || (o[t] = []))[i(115)]({fn: r, ctx: s}), this
        }

        [t(127)](t, r, s) {
            const i = this;

            function o() {
                const n = e;
                i[n(113)](t, o);
                for (var a = arguments[n(123)], c = new Array(a), l = 0; l < a; l++) c[l] = arguments[l];
                r.apply(s, c)
            }

            return o._ = r, this.on(t, o, s)
        }

        [t(119)](e) {
            const r = t, s = ((this.e || (this.e = {}))[e] || [])[r(130)]();
            for (var i = arguments[r(123)], o = new Array(i > 1 ? i - 1 : 0), n = 1; n < i; n++) o[n - 1] = arguments[n];
            for (let e = 0; e < s.length; e += 1) s[e].fn[r(117)](s[e].ctx, o);
            return this
        }

        [t(113)](e, r) {
            const s = t, i = this.e || (this.e = {});
            if (!e) return Object[s(121)](i)[s(126)]((e => {
                delete i[e]
            })), void delete this.e;
            const o = i[e], n = [];
            if (o && r) for (let e = 0, t = o[s(123)]; e < t; e += 1) o[e].fn !== r && o[e].fn._ !== r && n[s(115)](o[e]);
            return n[s(123)] ? i[e] = n : delete i[e], this
        }
    }

    const i = v;
    !function (e, t) {
        const r = v, s = e();
        for (; ;) try {
            if (385214 === parseInt(r(353)) / 1 + -parseInt(r(339)) / 2 * (parseInt(r(151)) / 3) + parseInt(r(350)) / 4 * (parseInt(r(331)) / 5) + -parseInt(r(365)) / 6 + parseInt(r(123)) / 7 + -parseInt(r(398)) / 8 * (-parseInt(r(233)) / 9) + parseInt(r(360)) / 10 * (-parseInt(r(448)) / 11)) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(E);
    const o = {websocket: 1, fetch: 2, hls: 3, webrtc: 4, webTransport: 5, aliyunRtc: 6, ts: 7};
    i(505), i(508), i(283), i(491), i(245), i(508), i(283), i(491), i(245), i(184), i(539);
    const n = {player: "player", playerAudio: i(457), playbackTF: "playbackTF", talk: i(351), playVod: i(472)},
        a = {mp4: "mp4", webm: i(518), flv: i(108), mov: i(149)}, c = {
            flv: "flv",
            m7s: i(487),
            hls: i(283),
            webrtc: i(491),
            webTransport: i(245),
            nakedFlow: i(219),
            fmp4: i(273),
            mpeg4: "mpeg4",
            aliyunRtc: i(184),
            ts: "ts",
            g711: i(432),
            pcm: i(441)
        };
    i(374), i(506), i(535), i(386), i(537), i(231), i(409);
    const l = {
            mse: "mse",
            wcs: i(117),
            offscreen: "offscreen",
            wasm: i(501),
            simd: i(502),
            mt: "mt",
            webrtc: i(491),
            hls: i(283),
            aliyunRtc: i(184)
        }, u = (i(366), "video"), h = {debug: "debug", warn: i(275)}, d = {click: "click", mouseDownAndUp: i(515)},
        p = {normal: i(188), simple: i(261)}, f = {normal: i(188), small: i(140)};
    i(310), i(471);
    i(538), i(115), i(212);
    const m = {
        playIsNotPauseOrUrlIsNull: i(244),
        webglAlignmentError: i(172),
        webglContextLostError: "webgl 上下文丢失",
        mediaSourceH265NotSupport: i(402),
        mseSourceBufferFull: i(242),
        mediaSourceAppendBufferError: "初始化解码器失败",
        mseSourceBufferError: i(520),
        mseAddSourceBufferError: i(385),
        mseWorkerAddSourceBufferError: "初始化解码器失败",
        mediaSourceDecoderConfigurationError: i(385),
        mediaSourceTsIsMaxDiff: i(361),
        mseWidthOrHeightChange: "流异常",
        mediaSourceAudioG711NotSupport: "硬解码不支持G711a/u音频格式",
        mediaSourceAudioInitTimeout: "初始化音频解码器失败",
        mediaSourceAudioNoDataTimeout: "等待音频数据超时",
        mediaSourceUseCanvasRenderPlayFailed: i(257),
        mediaSourceBufferedIsZeroError: "流异常",
        webcodecsH265NotSupport: i(402),
        webcodecsUnsupportedConfigurationError: i(385),
        webcodecsDecodeConfigureError: i(385),
        webcodecsDecodeError: i(520),
        webcodecsAudioInitTimeout: i(567),
        webcodecsAudioNoDataTimeout: "等待音频数据超时",
        wcsWidthOrHeightChange: "解码失败",
        wasmDecodeError: "解码失败",
        simdDecodeError: i(520),
        wasmWidthOrHeightChange: i(361),
        wasmUseVideoRenderError: i(131),
        videoElementPlayingFailed: "video自动渲染失败",
        simdH264DecodeVideoWidthIsTooLarge: i(139),
        networkDelayTimeout: i(421),
        fetchError: i(318),
        streamEnd: i(141),
        websocket1006Error: "流异常结束",
        websocketError: i(318),
        webrtcError: i(318),
        hlsError: "请求失败",
        decoderWorkerInitError: i(442),
        decoderWorkerWasmError: "wasm解码失败",
        videoElementPlayingFailedForWebrtc: "video自动渲染失败",
        videoInfoError: "解析视频分辨率失败",
        webrtcStreamH265: i(326),
        delayTimeout: i(237),
        loadingTimeout: i(379),
        loadingTimeoutRetryEnd: i(379),
        delayTimeoutRetryEnd: "播放超时重播失败",
        flvDemuxBufferSizeTooLarge: i(207),
        audioChannelError: "音频通道错误",
        aliyunRtcError: "请求失败",
        mseWaitVideoCanplayTimeout: i(361),
        initDecoderWorkerTimeout: "初始化解码worker超时"
    };
    i(228), i(275), i(279), i(314), i(334), i(358), i(101), i(188), i(508), i(108), i(501), i(539), i(148), i(161), i(504), i(447);
    i(467), i(428), i(499), i(315), i(349), i(497), i(352), i(517), i(548), i(267), i(414), i(343), i(167), i(247), i(388), i(296), i(147), i(382), i(377), i(481), i(217), i(183), i(254), i(500), i(523), i(236), i(513), i(461), i(392), i(248), i(324), i(427), i(495), i(122), i(259), i(109), i(345), i(119), i(168), i(454), i(303), i(553), i(484), i(106), i(329), i(382), i(466), i(286), i(372), i(214), i(555), i(220), i(192), i(456), i(341), i(556), i(185), i(155), i(528), i(202), i(307), i(205), i(547), i(508), i(531), i(480), i(528), i(202), i(446), i(287), i(109);
    const g = {
        fullscreen: i(525),
        fullScreenByStart: i(179),
        webFullscreen: i(265),
        decoderWorkerInit: i(103),
        play: "play",
        playing: "playing",
        pause: i(152),
        mute: i(308),
        load: "load",
        loading: i(240),
        zooming: i(285),
        drawingRect: "drawingRect",
        drawingRectInfo: "drawingRectInfo",
        videoInfo: i(252),
        timeUpdate: i(278),
        audioInfo: i(135),
        log: "log",
        error: i(470),
        kBps: i(183),
        timeout: i(198),
        delayTimeout: i(130),
        delayTimeoutRetryEnd: i(120),
        loadingTimeout: "loadingTimeout",
        loadingTimeoutRetryEnd: "loadingTimeoutRetryEnd",
        stats: i(389),
        performance: i(417),
        videoSmooth: i(289),
        faceDetectActive: "faceDetectActive",
        objectDetectActive: i(153),
        occlusionDetectActive: i(297),
        imageDetectActive: "imageDetectActive",
        record: "record",
        recording: "recording",
        recordingTimestamp: i(391),
        recordStart: i(157),
        recordEnd: i(270),
        recordCreateError: "recordCreateError",
        recordBlob: i(280),
        recordCancel: i(338),
        buffer: i(202),
        videoFrame: i(482),
        videoSEI: i(347),
        videoSEISyncPts: "videoSEISyncPts",
        start: i(269),
        metadata: i(395),
        resize: i(246),
        volumechange: "volumechange",
        volume: i(546),
        destroy: i(480),
        beforeDestroy: i(532),
        streamEnd: i(450),
        streamRate: i(128),
        streamAbps: "streamAbps",
        streamVbps: i(169),
        streamDts: i(426),
        streamSuccess: i(540),
        streamMessage: "streamMessage",
        streamError: "streamError",
        streamStats: i(284),
        streamEndInner: i(288),
        mseSourceOpen: "mseSourceOpen",
        mseSourceClose: i(235),
        mseSourceended: i(208),
        mseSourceStartStreaming: i(174),
        mseSourceEndStreaming: i(410),
        mseSourceBufferError: i(449),
        mseAddSourceBufferError: i(516),
        mseWorkerAddSourceBufferError: "mseWorkerAddSourceBufferError",
        mseSourceBufferBusy: i(327),
        mseSourceBufferFull: i(405),
        videoWaiting: i(255),
        videoTimeUpdate: i(496),
        videoSyncAudio: i(298),
        playToRenderTimes: "playToRenderTimes",
        playbackTime: i(164),
        playbackTimestamp: i(291),
        playbackTimeScroll: i(458),
        playbackPrecision: i(137),
        playbackShowPrecisionChange: i(412),
        playbackJustTime: i(224),
        playbackStats: i(463),
        playbackSeek: "playbackSeek",
        playbackPause: "playbackPause",
        playbackPauseOrResume: i(215),
        playbackRateChange: i(440),
        playbackPreRateChange: "playbackPreRateChange",
        playbackEnd: "playbackEnd",
        playVodStats: "playVodStats",
        playVodPause: "playVodPause",
        playVodPauseOrResume: "playVodPauseOrResume",
        playVodRateChange: i(210),
        playVodTime: "playVodTime",
        playVodTimeCache: "playVodTimeCache",
        playVodSeek: "playVodSeek",
        playVodSeekEnd: i(299),
        playVodVideoSeekingByMse: i(460),
        playVodVideoSeekedByMse: "playVodVideoSeekedByMse",
        playVodCanplay: i(322),
        playVodWaiting: i(451),
        playVodEndedAndReplay: "playVodEndedAndReplay",
        playVodEnded: i(363),
        ptz: i(337),
        streamQualityChange: i(550),
        visibilityChange: i(193),
        netBuf: i(418),
        close: i(555),
        networkDelayTimeout: i(459),
        togglePerformancePanel: "togglePerformancePanel",
        viewResizeChange: "viewResizeChange",
        flvDemuxBufferSizeTooLarge: i(354),
        talkGetUserMediaSuccess: i(380),
        talkGetUserMediaFail: i(100),
        talkGetUserMediaTimeout: "talkGetUserMediaTimeout",
        talkStreamStart: i(453),
        talkStreamOpen: "talkStreamOpen",
        talkStreamClose: i(564),
        talkStreamError: i(125),
        talkStreamInactive: i(262),
        talkStreamMsg: "talkStreamMsg",
        webrtcDisconnect: i(493),
        webrtcFailed: i(166),
        webrtcClosed: i(142),
        webrtcChecking: i(411),
        webrtcConnected: i(194),
        webrtcCompleted: i(158),
        webrtcOnConnectionStateChange: i(536),
        webrtcOnIceConnectionStateChange: i(545),
        crashLog: i(503),
        focus: i(551),
        blur: i(511),
        inView: i(403),
        visibilityHiddenTimeout: i(342),
        websocketOpen: i(109),
        websocketClose: i(287),
        websocketError: i(446),
        websocketMessage: i(554),
        aiObjectDetectorInfo: i(309),
        aiFaceDetectorInfo: i(464),
        aiOcclusionDetectResult: "aiOcclusionDetectResult",
        aiImageDetectResult: i(173),
        playFailedAndPaused: i(258),
        audioResumeState: "audioResumeState",
        webrtcStreamH265: i(390),
        flvMetaData: i(465),
        talkFailedAndStop: i(527),
        removeLoadingBgImage: i(529),
        memoryLog: i(112),
        downloadMemoryLog: i(234),
        pressureObserverCpu: "pressureObserverCpu",
        currentPts: i(154),
        online: "online",
        offline: i(485),
        networkState: i(111),
        clickInitPlayer: i(439)
    };
    i(252), i(135), i(470), i(183), i(269), i(198), i(478), i(130), i(120), i(265), i(152), i(391), i(280), i(338), i(565), i(126), i(215), i(250), i(440), i(132), i(363), i(443), i(210), i(171), i(299), i(337), i(550), i(285), i(503), i(551), i(342), i(193), i(109), i(309), i(145), i(173), i(258), i(347), i(396), i(545), i(154), i(546), i(378), i(439);
    const b = {
        talkStreamClose: g[i(564)],
        talkStreamError: g[i(125)],
        talkStreamInactive: g[i(262)],
        talkGetUserMediaTimeout: g[i(552)],
        talkFailedAndStop: g[i(527)],
        talkStreamMsg: g.talkStreamMsg
    };
    i(272), i(278), i(135), i(470), i(198), i(130), i(413), i(152), i(308), i(389), i(565), i(503), i(109), i(287), i(258), i(281);
    const w = {talkStreamError: g[i(125)], talkStreamClose: g[i(564)]}, y = {
        playError: i(557),
        fetchError: i(549),
        fetchErrorWorker: i(307),
        websocketError: i(446),
        websocketOpenTimeout: "websocketOpenTimeout",
        webcodecsH265NotSupport: "webcodecsH265NotSupport",
        webcodecsDecodeError: i(416),
        webcodecsUnsupportedConfigurationError: "webcodecsUnsupportedConfigurationError",
        webcodecsDecodeConfigureError: i(479),
        webcodecsAudioInitTimeout: "webcodecsAudioInitTimeout",
        webcodecsAudioNoDataTimeout: i(241),
        mediaSourceH265NotSupport: "mediaSourceH265NotSupport",
        mediaSourceAudioG711NotSupport: i(162),
        mediaSourceAudioInitTimeout: i(476),
        mediaSourceAudioNoDataTimeout: i(165),
        mediaSourceDecoderConfigurationError: i(357),
        mediaSourceFull: g[i(405)],
        mseSourceBufferError: g.mseSourceBufferError,
        mseAddSourceBufferError: g[i(516)],
        mseWorkerAddSourceBufferError: g.mseWorkerAddSourceBufferError,
        mediaSourceAppendBufferError: i(510),
        mediaSourceTsIsMaxDiff: "mediaSourceTsIsMaxDiff",
        mediaSourceUseCanvasRenderPlayFailed: "mediaSourceUseCanvasRenderPlayFailed",
        mediaSourceBufferedIsZeroError: i(107),
        wasmDecodeError: i(394),
        wasmUseVideoRenderError: "wasmUseVideoRenderError",
        simdH264DecodeVideoWidthIsTooLarge: i(259),
        simdDecodeError: "simdDecodeError",
        hlsError: i(530),
        webrtcError: i(430),
        webrtcClosed: g[i(142)],
        webrtcIceCandidateError: "webrtcIceCandidateError",
        webglAlignmentError: "webglAlignmentError",
        wasmWidthOrHeightChange: "wasmWidthOrHeightChange",
        mseWidthOrHeightChange: i(507),
        wcsWidthOrHeightChange: i(104),
        widthOrHeightChange: i(143),
        tallWebsocketClosedByError: "tallWebsocketClosedByError",
        flvDemuxBufferSizeTooLarge: g[i(354)],
        audioChannelError: i(317),
        webglContextLostError: i(127),
        videoElementPlayingFailed: i(336),
        videoElementPlayingFailedForWebrtc: i(116),
        decoderWorkerInitError: i(196),
        decoderWorkerWasmError: "decoderWorkerWasmError",
        videoInfoError: i(524),
        streamEnd: g[i(450)],
        websocket1006Error: "websocket1006Error",
        delayTimeout: g[i(130)],
        loadingTimeout: g[i(519)],
        networkDelayTimeout: g.networkDelayTimeout,
        fullscreenError: i(160),
        aliyunRtcError: i(534),
        mseWaitVideoCanplayTimeout: i(489),
        initDecoderWorkerTimeout: i(387),
        notSupportNewWasm: "notSupportNewWasm",
        playVodMp4Error: i(177),
        mp4Error: i(490), ...w
    }, k = {notConnect: i(437), open: i(163), close: i(555), error: i(470), timeout: i(198)};

    function E() {
        const e = ["video自动渲染失败", "playbackEnd", 'video/mp4;codecs="hev1.4.10.L120.90"', 'video/mp4; codecs="avc1.64002A"', "audioInfo", "timeupdate", "playbackPrecision", "webgl2", "不支持该分辨率的视频", "small", "请求结束", "webrtcClosed", "widthOrHeightChange", "focusNear", "aiOcclusionDetectResult", "10min", "defaultStreamQuality", "vertical", "mov", "wcsDecodeError", "6YKigaL", "pause", "objectDetectActive", "currentPts", "playbackIsPause", "遮挡物检查", "recordStart", "webrtcCompleted", "multiSelected", "fullscreenError", "click", "mediaSourceAudioG711NotSupport", "open", "playbackTime", "mediaSourceAudioNoDataTimeout", "webrtcFailed", "isWebrtcForSRS", "videoSequenceHeader", "streamVbps", "ALAW", "playVodTime", "Webgl 渲染失败", "aiImageDetectResult", "mseSourceStartStreaming", "Decoder failure", "buffering", "playVodMp4Error", "half-hour", "fullScreenByStart", "sourcebuffer-created", "Unsupported configuration", "H265(HEVC)", "kBps", "aliyunRtc", "sendWsMessage", "webgl", "object", "normal", "worklet", "MULAW(g711u)", "MEDIA_ERR_NETWORK", "resetDecode", "visibilityChange", "webrtcConnected", "inbound-rtp", "decoderWorkerInitError", "decoder-pro-audio-player.js", "timeout", "mp3", "A key frame is required after configure() or flush()", "tcp", "buffer", "stream", "remote-candidate", "fetchClose", "ALAW(g711a)", "FLV待解封装buffer过大", "mseSourceended", "seeked", "playVodRateChange", "mse", "jbprovm", "track", "initVideoCodec", "playbackPauseOrResume", "Decoder error", "initAudio", "durationchange", "nakedFlow", "updateConfig", "The user aborted a request", "memory access out of bounds", "multiDragstart", "playbackJustTime", "sourceclose", "zoomExpand", "webcodecs", "player", "right-down", "left-up", "WebTransport", "AAC", "302490WjufeE", "downloadMemoryLog", "mseSourceClose", "videoPayload", "播放超时重播失败", "image/png", "left", "loading", "webcodecsAudioNoDataTimeout", "缓冲区已满", "zoomNarrow", "播放地址不能为空", "webTransport", "resize", "isWebrtcForOthers", "isDropping", "hls-manifest-loaded", "playbackPreRateChange", "1-1", "videoInfo", "progress", "audioCode", "videoWaiting", "subtitle-segments", "MediaSource解码使用canvas渲染失败", "playFailedAndPaused", "simdH264DecodeVideoWidthIsTooLarge", "active", "simple", "talkStreamInactive", "key", "image/jpeg", "webFullscreen", "关闭电子放大", "isNakedFlow", "base64", "start", "recordEnd", "Decoding error", "load", "fmp4", "Jb%20pro%20%E4%BD%93%E9%AA%8C%E7%BB%93%E6%9D%9F%2C%E8%AF%B7%E5%88%B7%E6%96%B0%E9%A1%B5%E9%9D%A2%E5%86%8D%E6%AC%A1%E4%BD%93%E9%AA%8C(wasm%E5%86%85%E9%83%A8%E4%BC%9A%E6%9A%82%E5%81%9C%E8%A7%A3%E7%A0%81)%EF%BC%8C%E5%A6%82%E9%9C%80%E8%A6%81%E8%B4%AD%E4%B9%B0%E5%95%86%E4%B8%9A%E6%8E%88%E6%9D%83%EF%BC%8C%E5%8F%AF%E4%BB%A5%E8%81%94%E7%B3%BB%E5%BE%AE%E4%BF%A1%EF%BC%9Abosswancheng", "warn", "codec", "video decoder initialization failed", "timeUpdate", "decoder-pro-audio.js", "recordBlob", "audioResumeState", "complete", "hls", "streamStats", "zooming", "audioDecode", "websocketClose", "streamEndInner", "videoSmooth", "MEDIA_ERR_SRC_NOT_SUPPORTED", "playbackTimestamp", "stream-parsed", "tenMin", "playback", "load-response-headers", "isTs", "occlusionDetectActive", "videoSyncAudio", "playVodSeekEnd", "0000000000", "apertureNear", "mseDecodeError", "mseHandle", "ten-min", "right-up", "loadstart", "fetchErrorWorker", "mute", "aiObjectDetectorInfo", "JessibucaPro", "stop", "AbortError", "subtitle-playlist", "decoder-pro-hard.js", "supportHls265", "ceil", "audioChannelError", "请求失败", "stalled", "video", "sei-in-time", "playVodCanplay", "buffereos", "workerEnd", "playing", "webrtc不支持H265", "mseSourceBufferBusy", "halfHour", "Invalid NAL unit size", "性能面板激活", "3405YxiDcM", "g711a", "cruiseStop", "decoder-pro-hard-not-wasm.js", "empty", "videoElementPlayingFailed", "ptz", "recordCancel", "64252MpfdPp", "opus", "resetVideoDecode", "visibilityHiddenTimeout", "isWebrtcForZLM", "multiMousemove", "closeEnd", "保存日志", "videoSEI", "ended", "widthOrHeightChangeReplayDelayTime", "1016GVKAad", "talk", "multiIndex", "679839WGoNJa", "flvDemuxBufferSizeTooLarge", "volumechange", "#FFCC00", "mediaSourceDecoderConfigurationError", "jessibuca-pro-mp4-recorder-decoder.js", "remove-buffer", "3321620DyMJeG", "流异常", "csrc", "playVodEnded", "H264(AVC)", "976110FRFpvu", "canvas", "created", "script", "#000", "cruiseStart", "电子放大", "videoDecode", "global", "FLV", "full", "sender", "render", "drawingRectInfo", "加载超时重播失败", "talkGetUserMediaSuccess", "avc", "init", "offscreen", "rtp", "初始化解码器失败", "MPEG4", "initDecoderWorkerTimeout", "isAliyunRtc", "stats", "webrtcStreamH265", "recordingTimestamp", "iframeIntervalTs", "alaw", "wasmDecodeError", "metadata", "videoSEISyncPts", "peer-connection", "120biVkNf", "操作盘", "self", "play() failed because the user didn't interact with the document first", "不支持硬解码H265", "inView", "left-down", "mseSourceBufferFull", "ws1006", "outbound-rtp", "leftUp", "AliyunRtc", "mseSourceEndStreaming", "webrtcChecking", "playbackShowPrecisionChange", "play", "isWebrtc", 'video/mp4;codecs="hev1.3.E.L120.90"', "webcodecsDecodeError", "performance", "netBuf", "MEDIA_ERR_ABORTED", "right", "网络超时重播失败", "webrtc-failed", "streamError", "Cannot call 'decode' on a closed codec", "3-1", "streamDts", "networkDelay", "playbackForwardMaxRateDecodeIFrame", "multiDrop", "webrtcError", "download", "g711", "down", "遮挡物检查激活", "5min", 'video/mp4; codecs="hev1.1.6.L123.b0"', "notConnect", "one-hour", "clickInitPlayer", "playbackRateChange", "pcm", "初始化worker失败", "playVodPauseOrResume", "multiDragenter", "性能面板", "websocketError", "red", "44WFhaKB", "mseSourceBufferError", "streamEnd", "playVodWaiting", "oneHour", "talkStreamStart", "checkFirstIFrame", "canplaythrough", "resetAudioDecode", "playerAudio", "playbackTimeScroll", "networkDelayTimeout", "playVodVideoSeekingByMse", "workerFetch", "停止录制", "playbackStats", "aiFaceDetectorInfo", "flvMetaData", "decode", "playType", "candidate-pair", "hls-level-loaded", "error", "20240910", "playVod", "HEVC decoding is not supported", "rightUp", "webrtc-checking", "mediaSourceAudioInitTimeout", "3-4", "loadingTimeoutRetryEnd", "webcodecsDecodeConfigureError", "destroy", "playAudio", "videoFrame", "退出全屏", "mseError", "offline", "no-audio-track", "m7s", "g711u", "mseWaitVideoCanplayTimeout", "mp4Error", "webrtc", "sourceopen", "webrtcDisconnect", "multiMousedown", "playbackStreamVideoFps", "videoTimeUpdate", "controlSize", "NotAllowedError", "isM7sCrypto", "videoCodec", "wasm", "simd", "crashLog", "white", "websocket", "M7S", "mseWidthOrHeightChange", "fetch", "certificate", "mediaSourceAppendBufferError", "blur", "av1", "wasmError", "4-1", "mouseDownAndUp", "mseAddSourceBufferError", "isFlv", "webm", "loadingTimeout", "解码失败", "load-retry", "ttfb", "videoNalu", "videoInfoError", "fullscreen$2", "window", "talkFailedAndStop", "destroyEnd", "removeLoadingBgImage", "hlsError", "sendMessage", "beforeDestroy", "leftDown", "aliyunRtcError", "FMP4", "webrtcOnConnectionStateChange", "Webrtc", "/crypto/", "mp4", "streamSuccess", "fiStop", "endstreaming", "blob", "物品识别", "webrtcOnIceConnectionStateChange", "volume", "fetchSuccess", "isFmp4", "fetchError", "streamQualityChange", "focus", "talkGetUserMediaTimeout", "mseFirstRenderTime", "websocketMessage", "close", "fetchStream", "playIsNotPauseOrUrlIsNull", "switch-url-success", "fiveMin", "media-source", "message", "suspend", "multiMouseup", "talkStreamClose", "playToRenderTimes", "remote-outbound-rtp", "初始化音频解码器失败", "talkGetUserMediaFail", "default", "webgpu", "decoderWorkerInit", "wcsWidthOrHeightChange", "startstreaming", "notSupportNewWasm", "mediaSourceBufferedIsZeroError", "flv", "websocketOpen", "delta", "networkState", "memoryLog", "udp", "wiperOpen", "jbprov", "videoElementPlayingFailedForWebrtc", "wcs", "MP3", "aacSequenceHeader", "delayTimeoutRetryEnd", "seeking", "wasmWidthOrHeightChange", "4086852kyPVvt", "canplay", "talkStreamError", "playbackSeek", "webglContextLostError", "streamRate", "ratechange", "delayTimeout"];
        return (E = function () {
            return e
        })()
    }

    i(335), i(176), i(375), i(431), i(268), i(543), i(543), i(364), i(182), i(386), i(364), i(232), i(206), i(190), i(118), i(232), i(170), i(118), i(227), i(186), i(138), i(102), i(383), i(211), i(283), i(491), i(263), i(110), i(134), i(436), i(415), i(133), i(348), i(163), Math[i(316)](40);
    i(452), i(328), i(293), i(559), i(438), i(178), i(304), i(146), i(435), i(328), i(293), i(559), i(433), i(239), i(230), i(305), i(404), i(229), i(420), i(433), i(239), i(408), i(533), i(474), i(311), i(541), i(226), i(243), i(301), i(144), i(370), i(333), i(114);
    const S = {g711a: i(332), g711u: i(488), pcm: "pcm", opus: i(340)};
    i(238), i(264), i(225), i(492), i(105), i(542), i(124), i(136), i(129), i(319), i(253), i(381), i(512), i(200), i(424), i(181), i(175), i(271), i(216), i(473), i(221), i(312), i(312), i(240), i(325), i(480);
    const I = {pcma: 8, g711a: 8, pcmu: 0, g711u: 0, jpeg: 26, h264: 96, h265: 97, opus: 98, aac: 99},
        T = {empty: i(335), rtp: i(384), jtt: "jtt"};

    function v(e, t) {
        const r = E();
        return (v = function (e, t) {
            return r[e -= 100]
        })(e, t)
    }

    const A = {tcp: i(201), udp: i(113)};
    i(203), i(294);
    const C = {open: "open", close: i(555), error: i(470), message: i(561)}, B = {worklet: "worklet", script: i(368)},
        _ = {
            encType: S[i(332)],
            packetType: T[i(384)],
            packetTcpSendType: A[i(201)],
            rtpSsrc: i(300),
            numberChannels: 1,
            sampleRate: 8e3,
            sampleBitsWidth: 16,
            frameDuration: 20,
            debug: !1,
            debugLevel: h[i(275)],
            testMicrophone: !1,
            saveToTempFile: !1,
            audioBufferLength: 160,
            engine: B[i(189)],
            checkGetUserMediaTimeout: !1,
            getUserMediaTimeout: 1e4,
            audioConstraints: {
                latency: !0,
                noiseSuppression: !0,
                autoGainControl: !0,
                echoCancellation: !0,
                sampleRate: 48e3,
                channelCount: 1
            },
            isG711a: !1,
            isG711u: !1,
            jttSimNumber: null,
            jttChannelNumber: 1,
            openWebsocketHeart: !1,
            websocketHeartInterval: 15,
            websocketHeartContent: "",
            websocketOpenTimeout: 3
        }, M = {worklet: "worklet", script: i(368), active: i(260)};
    i(356), i(369), i(159), i(563), i(494), i(344), i(223), i(444), i(429), i(425), i(514), i(251), i(477), i(320), i(275), i(508), i(108), i(197), i(256), i(249), i(469), i(522), i(521), i(295), i(321), i(486), i(359), i(323), i(180), i(558), i(313), i(292), i(470), i(176), i(282), i(419), i(191), i(290), i(277), i(401), i(498), i(222), i(468), i(509), i(276), i(362), i(195), i(407), i(397), i(204), i(566), i(376), i(213), i(560), i(406), i(302), i(150), i(423), i(393), i(199), i(475), i(422), i(367), i(240), i(325), typeof window === i(187) && window[i(526)] === window ? window : typeof self === i(187) && self[i(400)] === self ? self : typeof global === i(187) && global[i(373)] === global && global, i(124), i(455), i(282), i(218), i(348), i(306), i(413), i(253), i(129), i(209), i(121), i(562), i(136), i(355), i(470), i(483), i(462), i(399), i(371), i(266), i(445), i(330), i(544), i(156), i(434), i(346);
    const F = "Jb%20pro%20%E4%BD%93%E9%AA%8C%E7%BB%93%E6%9D%9F%2C%E8%AF%B7%E5%88%B7%E6%96%B0%E9%A1%B5%E9%9D%A2%E5%86%8D%E6%AC%A1%E4%BD%93%E9%AA%8C%EF%BC%8C%E5%A6%82%E9%9C%80%E8%A6%81%E8%B4%AD%E4%B9%B0%E5%95%86%E4%B8%9A%E6%8E%88%E6%9D%83%EF%BC%8C%E5%8F%AF%E4%BB%A5%E8%81%94%E7%B3%BB%E5%BE%AE%E4%BF%A1%EF%BC%9Abosswancheng";

    function U(e, t) {
        var r = D();
        return (U = function (e, t) {
            return r[e -= 145]
        })(e, t)
    }

    function D() {
        var e = ["1394239fMZOZR", "webkitfullscreenerror", "18VJOrpF", "exports", "mozFullScreenElement", "msFullscreenEnabled", "webkitCancelFullScreen", "exitFullscreen", "webkitFullscreenEnabled", "removeEventListener", "msExitFullscreen", "addEventListener", "requestFullscreen", "244781lOFCjH", "7839696mcVbzi", "then", "331755vdTnas", "screenfull", "MSFullscreenChange", "webkitCurrentFullScreenElement", "mozfullscreenerror", "mozRequestFullScreen", "14049xPOaeg", "isFullscreen", "off", "msRequestFullscreen", "MSFullscreenError", "fullscreenerror", "100AEaFQH", "mozCancelFullScreen", "6qFbXFc", "documentElement", "webkitExitFullscreen", "10246160kuaPwU", "mozFullScreenEnabled", "11POXMQo", "object", "bind", "12GROiYe", "catch", "webkitfullscreenchange", "undefined", "fullscreenElement", "webkitFullscreenElement", "msFullscreenElement", "fullscreenchange", "4331661oxiXCi", "document", "fullscreenEnabled", "webkitRequestFullScreen", "change"];
        return (D = function () {
            return e
        })()
    }

    i(274), function (e, t) {
        for (var r = U, s = e(); ;) try {
            if (810446 === parseInt(r(188)) / 1 * (parseInt(r(154)) / 2) + -parseInt(r(170)) / 3 + parseInt(r(162)) / 4 * (parseInt(r(191)) / 5) + parseInt(r(177)) / 6 * (-parseInt(r(175)) / 7) + parseInt(r(157)) / 8 + parseInt(r(146)) / 9 * (-parseInt(r(152)) / 10) + parseInt(r(159)) / 11 * (parseInt(r(189)) / 12)) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(D), function (e, t) {
        return e(t = {exports: {}}, t.exports), t.exports
    }((function (e) {
        var t, r, s, i, o, n;
        t = U, r = typeof window !== t(165) && typeof window[t(171)] !== t(165) ? window[t(171)] : {}, s = t(160) !== t(165) && e[t(178)], i = function () {
            for (var e, s = t, i = [[s(187), "exitFullscreen", s(166), s(172), s(169), s(151)], ["webkitRequestFullscreen", s(156), s(167), s(183), s(164), s(176)], [s(173), s(181), s(194), s(181), s(164), s(176)], [s(145), s(153), s(179), s(158), "mozfullscreenchange", s(195)], [s(149), s(185), s(168), s(180), s(193), s(150)]], o = 0, n = i.length, a = {}; o < n; o++) if ((e = i[o]) && e[1] in r) {
                for (o = 0; o < e.length; o++) a[i[0][o]] = e[o];
                return a
            }
            return !1
        }(), o = {change: i[t(169)], error: i[t(151)]}, n = {
            request: function (e, t) {
                return new Promise(function (s, o) {
                    var n = U, a = function () {
                        var e = U;
                        this.off(e(174), a), s()
                    }[n(161)](this);
                    this.on(n(174), a);
                    var c = (e = e || r[n(155)])[i[n(187)]](t);
                    c instanceof Promise && c[n(190)](a)[n(163)](o)
                }.bind(this))
            }, exit: function () {
                return new Promise(function (e, t) {
                    var s = U;
                    if (this[s(147)]) {
                        var o = function () {
                            var t = s;
                            this[t(148)](t(174), o), e()
                        }.bind(this);
                        this.on("change", o);
                        var n = r[i[s(182)]]();
                        n instanceof Promise && n[s(190)](o)[s(163)](t)
                    } else e()
                }[t(161)](this))
            }, toggle: function (e, t) {
                return this.isFullscreen ? this.exit() : this.request(e, t)
            }, onchange: function (e) {
                this.on("change", e)
            }, onerror: function (e) {
                this.on("error", e)
            }, on: function (e, s) {
                var i = o[e];
                i && r[t(186)](i, s, !1)
            }, off: function (e, s) {
                var i = o[e];
                i && r[t(184)](i, s, !1)
            }, raw: i
        }, i ? (Object.defineProperties(n, {
            isFullscreen: {
                get: function () {
                    return Boolean(r[i[t(166)]])
                }
            }, element: {
                enumerable: !0, get: function () {
                    return r[i[t(166)]]
                }
            }, isEnabled: {
                enumerable: !0, get: function () {
                    return Boolean(r[i[t(172)]])
                }
            }
        }), s ? e[t(178)] = n : window[t(192)] = n) : s ? e[t(178)] = {isEnabled: !1} : window[t(192)] = {isEnabled: !1}
    })).isEnabled;
    const R = W;

    function W(e, t) {
        const r = L();
        return (W = function (e, t) {
            return r[e -= 193]
        })(e, t)
    }

    function L() {
        const e = ["error", "ExpGolomb: _fillCurrentWord() but no bytes available", "readByte", "3KDHcEJ", "778852lwObim", "readBits", "_current_word_bits_left", "readUEG", "_skipLeadingZero", "ExpGolomb: readBits() bits exceeded max 32bits!", "_total_bytes", "_total_bits", "1339130JxmGad", "getUint32", "6135615YTBdiR", "byteLength", "_fillCurrentWord", "_current_word", "_buffer", "buffer", "2915214PhAYie", "destroy", "2Ktrthd", "307727XjcEbW", "subarray", "set", "12HXkzya", "2582396GTXeSI", "3148387hBMuQJ", "5EujVuN", "readBool", "32xVmFEF", "_buffer_index", "min"];
        return (L = function () {
            return e
        })()
    }

    !function (e, t) {
        const r = W, s = e();
        for (; ;) try {
            if (438377 === parseInt(r(216)) / 1 * (-parseInt(r(198)) / 2) + parseInt(r(197)) / 3 * (parseInt(r(221)) / 4) + -parseInt(r(223)) / 5 * (-parseInt(r(214)) / 6) + -parseInt(r(217)) / 7 * (parseInt(r(225)) / 8) + parseInt(r(208)) / 9 + -parseInt(r(206)) / 10 + parseInt(r(222)) / 11 * (-parseInt(r(220)) / 12)) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(L);

    class x {
        constructor(e) {
            const t = W;
            this[t(212)] = e, this._buffer_index = 0, this._total_bytes = e[t(209)], this[t(205)] = 8 * e.byteLength, this._current_word = 0, this[t(200)] = 0
        }

        [R(215)]() {
            this[R(212)] = null
        }

        [R(210)]() {
            const e = R;
            let t = this[e(204)] - this[e(226)];
            if (t <= 0) return void console[e(194)](e(195), this[e(204)], this[e(226)]);
            let r = Math[e(193)](4, t), s = new Uint8Array(4);
            s[e(219)](this._buffer[e(218)](this[e(226)], this[e(226)] + r)), this[e(211)] = new DataView(s[e(213)])[e(207)](0, !1), this[e(226)] += r, this._current_word_bits_left = 8 * r
        }

        [R(199)](e) {
            const t = R;
            if (e > 32 && console[t(194)](t(203)), e <= this[t(200)]) {
                let r = this[t(211)] >>> 32 - e;
                return this._current_word <<= e, this[t(200)] -= e, r
            }
            let r = this[t(200)] ? this._current_word : 0;
            r >>>= 32 - this._current_word_bits_left;
            let s = e - this[t(200)];
            this[t(210)]();
            let i = Math[t(193)](s, this[t(200)]), o = this._current_word >>> 32 - i;
            return this[t(211)] <<= i, this._current_word_bits_left -= i, r = r << i | o, r
        }

        [R(224)]() {
            return 1 === this[R(199)](1)
        }

        [R(196)]() {
            return this[R(199)](8)
        }

        [R(202)]() {
            const e = R;
            let t;
            for (t = 0; t < this[e(200)]; t++) if (0 != (this[e(211)] & 2147483648 >>> t)) return this[e(211)] <<= t, this[e(200)] -= t, t;
            return this[e(210)](), t + this[e(202)]()
        }

        [R(201)]() {
            const e = R;
            let t = this._skipLeadingZero();
            return this[e(199)](t + 1) - 1
        }

        readSEG() {
            let e = this[R(201)]();
            return 1 & e ? e + 1 >>> 1 : -1 * (e >>> 1)
        }
    }

    const P = G;
    !function (e, t) {
        const r = G, s = e();
        for (; ;) try {
            if (859270 === -parseInt(r(389)) / 1 * (parseInt(r(343)) / 2) + -parseInt(r(409)) / 3 + parseInt(r(398)) / 4 * (-parseInt(r(341)) / 5) + parseInt(r(346)) / 6 + -parseInt(r(379)) / 7 * (parseInt(r(347)) / 8) + parseInt(r(369)) / 9 + parseInt(r(406)) / 10) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(V);
    const O = [96e3, 88200, 64e3, 48e3, 44100, 32e3, 24e3, 22050, 16e3, 12e3, 11025, 8e3, 7350, -1, -1, -1];
    P(357), P(362);

    function V() {
        const e = ["Flv: AAC invalid sampling frequency index!", "readByte", "bufoff", "error", "buffer", "13988890NSPOxT", "channels", "aac", "4627212nUEudn", "readNextAACFrame", "266345sIWkkr", "byteLength", "956374MUxHUK", "has_last_incomplete_data", "sample_rate", "9985734NAlXmE", "99688evCrAj", "allStreamsSameTimeFraming zero is Not Supported", "slice", "Could not found ADTS syncword until payload end", "buflen", "navigator", "firefox", "profile: ", "channel_config", "set", "look", "channel: ", "ext_object_type", "readBits", "log", "read_golomb", ". Only frameLengthType = 0 Supported", "data_", "iserro", "toLowerCase", "sampleRate: ", "length", "1694943DMlCrm", "sequenceHeader", "readBool", "warn", "subarray", "audioMuxVersionA is Not Supported", "read", "getLATMValue", "sampling_index", "sbr", "119IFgdbW", "findNextSyncwordOffset", "eof_flag_", "unshift", "sampling_freq_index", "sampling_frequency", "mp4a.40.", "object_type", "indexOf", "destroy", "1VECxGe", "more than 2 numProgram Not Supported", "more than 2 numSubFrames Not Supported", "push", "current_syncword_offset_", "bufpos", "Invalid sampling index: ", "chan_config", "audio_object_type", "12BFDRNw", "StreamMuxConfig Missing", "AAC"];
        return (V = function () {
            return e
        })()
    }

    function G(e, t) {
        const r = V();
        return (G = function (e, t) {
            return r[e -= 341]
        })(e, t)
    }

    P(380);
    P(380), P(376), P(410);

    function N(e, t) {
        const r = j();
        return (N = function (e, t) {
            return r[e -= 410]
        })(e, t)
    }

    function j() {
        const e = ["450eszHqT", "push", "length", "1750016VkdLGk", "2340HlVXzR", "5027WPgMLU", "buffer", "toString", "byteLength", "2684946uilBEX", "3468630vObXsC", "subarray", "567eyyWcJ", "629391ozLRBf", "7yWBNcK", "8fRCXrn", "set", "70764EuapGu", "21520DRaZgk"];
        return (j = function () {
            return e
        })()
    }

    !function (e, t) {
        const r = N, s = e();
        for (; ;) try {
            if (359576 === parseInt(r(418)) / 1 * (-parseInt(r(410)) / 2) + -parseInt(r(419)) / 3 * (parseInt(r(421)) / 4) + -parseInt(r(416)) / 5 + parseInt(r(415)) / 6 * (-parseInt(r(420)) / 7) + -parseInt(r(428)) / 8 + parseInt(r(425)) / 9 * (parseInt(r(424)) / 10) + -parseInt(r(411)) / 11 * (-parseInt(r(423)) / 12)) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(j);
    const H = Q;

    function z() {
    }

    function q() {
        const e = Q;
        return (new Date)[e(362)]()
    }

    !function (e, t) {
        const r = Q, s = e();
        for (; ;) try {
            if (152091 === parseInt(r(416)) / 1 * (parseInt(r(437)) / 2) + -parseInt(r(442)) / 3 * (parseInt(r(374)) / 4) + -parseInt(r(424)) / 5 * (-parseInt(r(254)) / 6) + -parseInt(r(387)) / 7 * (-parseInt(r(177)) / 8) + parseInt(r(409)) / 9 * (-parseInt(r(172)) / 10) + -parseInt(r(484)) / 11 * (-parseInt(r(451)) / 12) + -parseInt(r(482)) / 13 * (parseInt(r(371)) / 14)) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(X), (() => {
        const e = Q;
        try {
            if ("object" == typeof WebAssembly && typeof WebAssembly[e(333)] === e(471)) {
                const t = new (WebAssembly[e(253)])(Uint8Array.of(0, 97, 115, 109, 1, 0, 0, 0));
                if (t instanceof WebAssembly[e(253)]) return new (WebAssembly[e(199)])(t) instanceof WebAssembly[e(199)]
            }
        } catch (e) {
        }
    })();
    H(303);
    H(214), H(303), H(311), H(191);
    const J = H(474);

    function X() {
        const e = ["removeChild", "pow", "scrollLeft", "display", "open", "isConfigSupported", "keys", "hev", "useMSE", "[object Error]", "MouseEvents", "status", "opera", "innerHTML", "WebGL2RenderingContext", "pathname", "user-select", "fillStyle", "img", "[object Exception]", "data-", "100", "style", "setAttribute", "not compiled with thread support, using thread emulation", "test", "location", "repeat", "360EE", "quality", "block", "navigator", "length", "search", "EB/s", "wcsDecodeAudio", "NaN", "iFrame", "ManagedMediaSource", "noteOn", "copyright", "content", "atob", "gpu", "px 宋体", "paused", "zIndex", "href", '"production"', "getPropertyValue", "memory", "-o-user-select", "application/mozilla-npqihooquicklogin", "setUint32", "VideoEncoder is not supported", "replace", '"delaytime"', "getParameter", "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=", "filter", "getContext", "format", "VideoEncoder.isConfigSupported is not a function", "createTextNode", "QQBrowser", "removeAttribute", "PB/s", "transferControlToOffscreen", "mseDecodeAudio", "Safari", "target", "substring", "revokeObjectURL", "crossOrigin", "height", "dispatchEvent", "toString", "webkit-3d", "instantiate", "Anonymous", "srcElement", "useWCS", "getDay", "audioWorklet", "https:", "byteLength", "dataset", "hev4", "host", "not compiled with thread support", "indexOf", "getSeconds", "webkitGetUserMedia", "isFullscreen", "pps", "backgroundColor", "solid", "application/vnd.chromium.remoting-viewer", "addModule", "event", "createObjectURL", "scrollTop", "chrome", "visibility", "MB/s", "failed to asynchronously prepare wasm", "firefox", "getTime", "hasAudio", "lineWidth", "string", "text", "charCodeAt", "top", "random", "connect", "4718oOXqkG", "vps", "prototype", "376qZKCWd", "set", "VideoDecoder", "overflow", "createImageBitmap", "performance", "hidden", "validate", "getFullYear", "protocol", "absolute", "webgl", "supported", "14NHDXFX", "360SE", "trim", "toLowerCase", "QHBrowser", "fullscreenElement", "-webkit-text-size-adjust", "readAsArrayBuffer", "webgl2", "-webkit-user-select", "Edg", "fillText", "width", "getHours", "getDate", "split", "getMinutes", "GB/s", "pageY", "MediaStreamTrackGenerator", "playbackRate", "TB/s", "1908EzfCTY", "showModalDialog", "setUint8", "message", "right", "URL", "putImageData", "103LNJpBQ", "getImageData", "touchmove", "AAC", "MULAW", "Chrome", "insertBefore", "hash", "55EGFBqE", "bottom", "high", "true", "version", "toFixed", "xxxx", "object", "KB/s", "canvas", "[object Number]", "win64", "hev2", "1840oDEbZp", "font", "data", "numberOfFrames", "imageSmoothingEnabled", "6963nupAqV", "parse", "rgba(0,0,0,0)", "avc1.", "undefined", "application/vnd.apple.mpegurl", "requestVideoFrameCallback", "setHours", "save", "84kixuXo", "clientX", "toDataURL", "WebGLRenderingContext", "wow32", "log", "hostname", "mediaDevices", "color", "getUserMedia", "mousemove", "opacity", "ended", ";base64", "shadowRoot", "PressureObserver", "MediaSource", "ALAW", "Opera", "angle", "function", "{y}-{m}-{d} {h}:{i}:{s}", "fontSize", "1730390400000", "AudioDecoder", "fmt ", "mousedown", "userAgent", "green yuv", "destination", "charAt", "3068RXWjHP", "fromCharCode", "409211OilsQx", "360", "drawImage", "onload", "AudioData", "Reserved SBR extensions is not implemented", "rect", "error", "sequenceHeader", "imageSmoothingQuality", "0 KB/s", "setInt32", "result", "xxxxxxxxxxxx4xxx", "call", "click", "-ms-user-select", "download", "hev5", "-webkit-touch-callout", "div", "strokeRect", "ZB/s", "application/javascript", "demuxUseWorker", "4760GEoyuP", "setInt8", "slice", "none", "container", "724352HmIWIS", "touches", "clearRect", "apply", "QihooBrowser", "getUint8", "borderStyle", "src", "createElement", "then", "WAVE", "px Arial", "moz-webgl", "html", '"2024-11-1"', "documentElement", "body", "image", "mimeTypes", "ActiveXObject", "stringify", "experimental-webgl", "Instance", "localhost", "application/360softmgrplugin", "onerror", "sps", "match", "pageX", "VideoEncoder.isConfigSupported() result is not supported", "contain", "now", "assign", "keyFrame", "type", "setInt16", "round", '"development"', "borderColor", "Bytes", "data:", "port", "max", "min", "floor", "attachShadow", "line", "objectFit", "setUint16", "start", "VideoFrame", "100%", "join", "document", "forEach", "MediaStream", "borderWidth", "touchstart", "canConstructInDedicatedWorker", "#000", "useOffscreen", "hev3", "Worker", "clientY", "left", "restore", "msGetUserMedia", "appendChild", "mouseup", "0 Bytes", "nLp", "subarray", "green", "mozGetUserMedia", "createBuffer", "isTypeSupported", "Module", "7884SMjSuu"];
        return (X = function () {
            return e
        })()
    }

    const K = () => (() => {
        const e = H, t = window.navigator.userAgent;
        return /MicroMessenger/i[e(280)](t)
    })() && function () {
        const e = H, t = window[e(286)][e(478)].toLowerCase();
        return /android/i[e(280)](t)
    }();

    function Q(e, t) {
        const r = X();
        return (Q = function (e, t) {
            return r[e -= 170]
        })(e, t)
    }

    function Y(e) {
        const t = H;
        let r = "";
        if ("object" == typeof e) try {
            r = JSON[t(197)](e), r = JSON[t(443)](r)
        } catch (t) {
            r = e
        } else r = e;
        return r
    }

    function Z(e) {
        return !0 === e || "true" === e
    }

    function $() {
        return q() >= J
    }

    function ee() {
        const e = ["initialize", "initializeBuffers", "ceil", "outputBuffer", "slice", "resample", "ratioWeight", "8697069xxqRsB", "inputBufferSize", "575976prBejc", "8zKcBfB", "1046380nNgoEp", "outputBufferSize", "length", "tailExists", "destroy", "fromSampleRate", "3JnWynj", "3004764JskPrc", "bufferSlice", "Buffer was of incorrect sample length.", "Invalid settings specified for the resampler.", "channels", "floor", "multiTap", "resampler", "lastOutput", "toSampleRate", "lastWeight", "subarray", "linearInterpolation", "72814iRPaAK", "89000vUMIYa", "99939QYrJak"];
        return (ee = function () {
            return e
        })()
    }

    function te(e, t) {
        const r = ee();
        return (te = function (e, t) {
            return r[e -= 425]
        })(e, t)
    }

    const re = te;
    !function (e, t) {
        const r = te, s = e();
        for (; ;) try {
            if (265213 === -parseInt(r(434)) / 1 + -parseInt(r(432)) / 2 * (parseInt(r(452)) / 3) + -parseInt(r(433)) / 4 + -parseInt(r(446)) / 5 + parseInt(r(444)) / 6 + -parseInt(r(453)) / 7 + -parseInt(r(445)) / 8 * (-parseInt(r(442)) / 9)) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(ee);

    class se {
        constructor(e) {
            const t = te, {fromSampleRate: r, toSampleRate: s, channels: i, inputBufferSize: o} = e;
            if (!r || !s || !i) throw new Error(t(456));
            this[t(426)] = null, this[t(451)] = r, this.toSampleRate = s, this[t(457)] = i || 0, this[t(443)] = o, this[t(435)]()
        }

        [re(450)]() {
            const e = re;
            this.resampler = null, this[e(451)] = null, this[e(428)] = null, this[e(457)] = null, this[e(443)] = null
        }

        [re(435)]() {
            const e = re;
            this[e(451)] == this.toSampleRate ? (this[e(426)] = e => e, this.ratioWeight = 1) : (this.fromSampleRate < this[e(428)] ? (this[e(431)](), this[e(429)] = 1) : (this[e(425)](), this.tailExists = !1, this.lastWeight = 0), this[e(436)](), this[e(441)] = this[e(451)] / this[e(428)])
        }

        [re(454)](e) {
            const t = re;
            try {
                return this.outputBuffer[t(430)](0, e)
            } catch (r) {
                try {
                    return this.outputBuffer[t(448)] = e, this[t(438)]
                } catch (r) {
                    return this.outputBuffer[t(439)](0, e)
                }
            }
        }

        [re(436)]() {
            const e = re;
            this[e(447)] = Math[e(437)](this[e(443)] * this[e(428)] / this[e(451)] / this[e(457)] * 1.0000004768371582) + this[e(457)] + this.channels;
            try {
                this.outputBuffer = new Float32Array(this.outputBufferSize), this[e(427)] = new Float32Array(this[e(457)])
            } catch (t) {
                this[e(438)] = [], this[e(427)] = []
            }
        }

        [re(431)]() {
            const e = re;
            this[e(426)] = t => {
                const r = e;
                let s, i, o, n, a, c, l, u, h, d = t[r(448)], p = this[r(457)];
                if (d % p != 0) throw new Error("Buffer was of incorrect sample length.");
                if (d <= 0) return [];
                for (s = this[r(447)], i = this[r(441)], o = this[r(429)], n = 0, a = 0, c = 0, l = 0, u = this[r(438)]; o < 1; o += i) for (a = o % 1, n = 1 - a, this[r(429)] = o % 1, h = 0; h < this.channels; ++h) u[l++] = this[r(427)][h] * n + t[h] * a;
                for (o -= 1, d -= p, c = Math[r(458)](o) * p; l < s && c < d;) {
                    for (a = o % 1, n = 1 - a, h = 0; h < this[r(457)]; ++h) u[l++] = t[c + (h > 0 ? h : 0)] * n + t[c + (p + h)] * a;
                    o += i, c = Math[r(458)](o) * p
                }
                for (h = 0; h < p; ++h) this[r(427)][h] = t[c++];
                return this[r(454)](l)
            }
        }

        [re(425)]() {
            const e = re;
            this[e(426)] = t => {
                const r = e;
                let s, i, o, n, a, c, l, u, h, d, p, f = t[r(448)], m = this[r(457)];
                if (f % m != 0) throw new Error(r(455));
                if (f <= 0) return [];
                for (s = this.outputBufferSize, i = [], o = this[r(441)], n = 0, c = 0, l = 0, u = !this[r(449)], this[r(449)] = !1, h = this[r(438)], d = 0, p = 0, a = 0; a < m; ++a) i[a] = 0;
                do {
                    if (u) for (n = o, a = 0; a < m; ++a) i[a] = 0; else {
                        for (n = this[r(429)], a = 0; a < m; ++a) i[a] = this.lastOutput[a];
                        u = !0
                    }
                    for (; n > 0 && c < f;) {
                        if (l = 1 + c - p, !(n >= l)) {
                            for (a = 0; a < m; ++a) i[a] += t[c + (a > 0 ? a : 0)] * n;
                            p += n, n = 0;
                            break
                        }
                        for (a = 0; a < m; ++a) i[a] += t[c++] * l;
                        p = c, n -= l
                    }
                    if (0 !== n) {
                        for (this[r(429)] = n, a = 0; a < m; ++a) this.lastOutput[a] = i[a];
                        this[r(449)] = !0;
                        break
                    }
                    for (a = 0; a < m; ++a) h[d++] = i[a] / o
                } while (c < f && d < s);
                return this[r(454)](d)
            }
        }

        [re(440)](e) {
            const t = re;
            return this[t(451)] == this[t(428)] ? this.ratioWeight = 1 : (this[t(451)] < this[t(428)] ? this[t(429)] = 1 : (this[t(449)] = !1, this[t(429)] = 0), this[t(436)](), this[t(441)] = this[t(451)] / this[t(428)]), this[t(426)](e)
        }
    }

    function ie() {
        const e = ["length", "183596OQtMZw", "290lKgGVQ", "270074Zcbytk", "127404QpcXmh", "13956HjgPbz", "min", "64dlQMCQ", "6435BSUdje", "1428mcdcYB", "124qyLlID", "6evFmge", "7227PMQjTd", "6560EaRgHw", "max", "4849581UEsQPg"];
        return (ie = function () {
            return e
        })()
    }

    function oe(e, t) {
        const r = ie();
        return (oe = function (e, t) {
            return r[e -= 410]
        })(e, t)
    }

    !function (e, t) {
        const r = oe, s = e();
        for (; ;) try {
            if (246156 === -parseInt(r(422)) / 1 * (parseInt(r(420)) / 2) + -parseInt(r(421)) / 3 * (-parseInt(r(411)) / 4) + parseInt(r(423)) / 5 * (-parseInt(r(419)) / 6) + parseInt(r(413)) / 7 * (parseInt(r(417)) / 8) + parseInt(r(414)) / 9 * (-parseInt(r(412)) / 10) + parseInt(r(425)) / 11 + parseInt(r(415)) / 12 * (parseInt(r(418)) / 13)) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(ie), function (e, t) {
        const r = de, s = e();
        for (; ;) try {
            if (374644 === -parseInt(r(293)) / 1 * (-parseInt(r(291)) / 2) + -parseInt(r(295)) / 3 + -parseInt(r(298)) / 4 + parseInt(r(294)) / 5 * (parseInt(r(301)) / 6) + -parseInt(r(306)) / 7 + -parseInt(r(307)) / 8 + -parseInt(r(296)) / 9 * (-parseInt(r(300)) / 10)) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(pe);
    const ne = 132, ae = [255, 511, 1023, 2047, 4095, 8191, 16383, 32767];

    function ce(e, t, r) {
        for (let s = 0; s < r; s++) if (e <= t[s]) return s;
        return r
    }

    function le(e) {
        const t = de, r = [];
        return Array[t(299)][t(297)][t(302)](e)[t(304)](((e, t) => {
            r[t] = function (e) {
                let t, r, s;
                return e >= 0 ? t = 213 : (t = 85, (e = -e - 1) < 0 && (e = 32767)), r = ce(e, ae, 8), r >= 8 ? 127 ^ t : (s = r << 4, s |= r < 2 ? e >> 4 & 15 : e >> r + 3 & 15, s ^ t)
            }(e)
        })), r
    }

    function ue(e) {
        const t = de, r = [];
        return Array.prototype[t(297)].call(e).forEach(((e, t) => {
            r[t] = function (e) {
                let t = 0;
                e < 0 ? (e = ne - e, t = 127) : (e += ne, t = 255);
                let r = ce(e, ae, 8);
                return r >= 8 ? 127 ^ t : (r << 4 | e >> r + 3 & 15) ^ t
            }(e)
        })), r
    }

    function he(e) {
        const t = [];
        return e[de(304)]((e => {
            t[e] = function (e) {
                let t = ((15 & (e = ~e)) << 3) + ne;
                return t <<= (112 & e) >> 4, 128 & e ? ne - t : t - ne
            }(e)
        })), t
    }

    function de(e, t) {
        const r = pe();
        return (de = function (e, t) {
            return r[e -= 291]
        })(e, t)
    }

    function pe() {
        const e = ["176656yPfwzL", "774916adfxOW", "length", "1HAPTrC", "2872335qPXgTN", "2078064PCLcoR", "3819537YNuXar", "slice", "604984nsJLZW", "prototype", "20ipCTob", "6CBKgJj", "call", "set", "forEach", "ALAW", "3990357KCsZks"];
        return (pe = function () {
            return e
        })()
    }

    function fe(e, t) {
        const r = ge();
        return (fe = function (e, t) {
            return r[e -= 216]
        })(e, t)
    }

    !function (e, t) {
        const r = fe, s = e();
        for (; ;) try {
            if (192926 === parseInt(r(226)) / 1 + parseInt(r(218)) / 2 + -parseInt(r(222)) / 3 * (-parseInt(r(216)) / 4) + parseInt(r(231)) / 5 + parseInt(r(228)) / 6 * (-parseInt(r(233)) / 7) + -parseInt(r(234)) / 8 + -parseInt(r(227)) / 9) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(ge);

    class me {
        constructor(e) {
            const t = fe;
            this[t(225)] = function (r) {
                const s = t;
                if (e[s(224)].debugLevel == h[s(223)]) {
                    const t = e[s(224)].debugUuid ? "[" + e._opt[s(229)] + "]" : "";
                    for (var i = arguments[s(232)], o = new Array(i > 1 ? i - 1 : 0), n = 1; n < i; n++) o[n - 1] = arguments[n];
                    console.log(s(219) + t + "[✅✅✅][" + r + "]", ...o)
                }
            }, this[t(220)] = function (r) {
                const s = t;
                if (e[s(224)][s(221)] == h[s(223)] || e[s(224)][s(221)] == h.warn) {
                    const t = e[s(224)][s(229)] ? "[" + e._opt.debugUuid + "]" : "";
                    for (var i = arguments[s(232)], o = new Array(i > 1 ? i - 1 : 0), n = 1; n < i; n++) o[n - 1] = arguments[n];
                    console[s(225)](s(219) + t + s(230) + r + "]", ...o)
                }
            }, this[t(235)] = function (r) {
                const s = t, i = e[s(224)][s(229)] ? "[" + e._opt[s(229)] + "]" : "";
                for (var o = arguments[s(232)], n = new Array(o > 1 ? o - 1 : 0), a = 1; a < o; a++) n[a - 1] = arguments[a];
                console[s(235)](s(219) + i + s(217) + r + "]", ...n)
            }
        }
    }

    function ge() {
        const e = ["[❗❗❗][", "1564615fvGVLh", "length", "327138nujJMs", "970008hpYllr", "error", "4JDWDXL", "[❌❌❌][", "465150YJJYwa", "JbPro", "warn", "debugLevel", "60822UAJlYV", "debug", "_opt", "log", "155259zFVlFX", "717444jryuRz", "42BjwIVd", "debugUuid"];
        return (ge = function () {
            return e
        })()
    }

    const be = ye;

    function we() {
        const e = ["length", "destroys", "2CXaTbz", "5607WpZjEM", "map", "removeEventListener", "66eFIBvV", "forEach", "destroy", "431011mdiMTv", "bind", "4990sKxzHT", "master", "addEventListener", "isArray", "605151HgCmIA", "1424204tArdcB", "1249144wJhbZs", "debugLog", "5082756slZWXn", "1014390dVgsUV", "proxy"];
        return (we = function () {
            return e
        })()
    }

    function ye(e, t) {
        const r = we();
        return (ye = function (e, t) {
            return r[e -= 216]
        })(e, t)
    }

    !function (e, t) {
        const r = ye, s = e();
        for (; ;) try {
            if (991969 === -parseInt(r(231)) / 1 + -parseInt(r(217)) / 2 * (parseInt(r(230)) / 3) + parseInt(r(234)) / 4 + parseInt(r(235)) / 5 + parseInt(r(221)) / 6 * (parseInt(r(224)) / 7) + parseInt(r(232)) / 8 + parseInt(r(218)) / 9 * (parseInt(r(226)) / 10)) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(we);

    class ke {
        constructor(e) {
            const t = ye;
            this[t(216)] = [], this[t(236)] = this[t(236)][t(225)](this), this[t(227)] = e
        }

        [be(236)](e, t, r) {
            const s = be;
            let i = arguments[s(237)] > 3 && void 0 !== arguments[3] ? arguments[3] : {};
            if (!e) return;
            if (Array[s(229)](t)) return t[s(219)]((t => this.proxy(e, t, r, i)));
            e[s(228)](t, r, i);
            const o = () => {
                const o = s;
                "function" == typeof e[o(220)] && e[o(220)](t, r, i)
            };
            return this[s(216)].push(o), o
        }

        [be(223)]() {
            const e = be;
            this.master[e(233)]("Events", "destroy"), this[e(216)][e(222)]((e => e())), this.destroys = []
        }
    }

    const Ee = Se;

    function Se(e, t) {
        const r = Ie();
        return (Se = function (e, t) {
            return r[e -= 269]
        })(e, t)
    }

    function Ie() {
        const e = ["_initProcessScriptNode", "removeChild", "play", "_fillScriptNodeOutputBuffer", "duration", "destination", "scriptNode", "1237592wLumTy", "_handleScriptNodeCallback", "destroy", "isG711u", "push", "_initIntervalScriptNode() and interval time is ", "stream", "createMediaStreamDestination", "left", "7637tQGZMU", "outputBuffer", "pause", "audioContext", "1561446SDjsiK", "disconnect", "_isMoreThanMinBufferDuration", "3038060TbqPxH", "stashBuffer", "_initWorkletScriptNode()", "shift", "script start time ", "TAG_NAME", "scriptStartTime", "bufferList", "audioChannel", "$audio", "size", "4008KOleno", "gain", "9YxZRgA", "768430vjAllW", "active", "isG711a", "audioWorklet", "port", "right", "2252247KuXPEC", "buffer", "gainNode", "value", "scriptNodeInterval", "getBufferListDuration", "subarray", "addModule", "close", "_getAutoAudioEngineType", "player", "set", "getChannelData", "connect", "4CqMxJi", "audioBufferSize", "_getAudioBufferSizeByType", "createScriptProcessor", "currentTime", "AudioContext", "createBufferSource", "_initProcessScriptNode()", "debugLog", "_opt", "_getAudioChannel", "webkitAudioContext", "oneBufferDuration", "_initWorkletScriptNode", "script", "_createAudioContext", "parentNode", "isMute", "3933260nriFyZ", "_initScriptNode", "function", "createBuffer", "sampleRate", "engineType", "length", "postMessage", "worklet", "mediaStreamAudioDestinationNode", "_initIntervalScriptNode", "onaudioprocess", "_supportAudioWorklet", "fill"];
        return (Ie = function () {
            return e
        })()
    }

    !function (e, t) {
        const r = Se, s = e();
        for (; ;) try {
            if (564413 === parseInt(r(353)) / 1 + parseInt(r(323)) / 2 + parseInt(r(270)) / 3 * (-parseInt(r(284)) / 4) + parseInt(r(339)) / 5 + parseInt(r(336)) / 6 + -parseInt(r(332)) / 7 * (parseInt(r(350)) / 8) + -parseInt(r(352)) / 9 * (parseInt(r(302)) / 10)) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(Ie);
    Ee(296), Ee(299), Ee(279), Ee(286), Ee(312), Ee(316), Ee(324), Ee(319), Ee(301), Ee(338), Ee(275), Ee(318);

    function Te(e, t) {
        const r = ve();
        return (Te = function (e, t) {
            return r[e -= 203]
        })(e, t)
    }

    function ve() {
        const e = ["8177262FBACzH", "10171080WPSumf", "length", "reduce", "7fLVfdo", "243800FJfglx", "852366srdUIJ", "padStart", "byteLength", "7722165PGVGjK", "set", "6606432RwvahG", "2CFImVq", "toString", "104FPeCFW", "constructor", "1746909QCJPGS", "hexStringToUint8Array: invalid hexString length, must be even and bexString length is: ", "9tJJuKU"];
        return (ve = function () {
            return e
        })()
    }

    function Ae() {
        class e extends AudioWorkletProcessor {
            constructor(e) {
                super(), this._cursor = 0, this._bufferSize = e.processorOptions.bufferSize, this._buffer = new Float32Array(this._bufferSize)
            }

            process(e, t, r) {
                if (!e.length || !e[0].length) return !0;
                for (let t = 0; t < e[0][0].length; t++) this._cursor += 1, this._cursor === this._bufferSize && (this._cursor = 0, this.port.postMessage({
                    eventType: "data",
                    buffer: this._buffer
                })), this._buffer[this._cursor] = e[0][0][t];
                return !0
            }
        }

        registerProcessor("talk-processor", e)
    }

    !function (e, t) {
        const r = Te, s = e();
        for (; ;) try {
            if (775717 === -parseInt(r(221)) / 1 + parseInt(r(208)) / 2 * (parseInt(r(212)) / 3) + parseInt(r(210)) / 4 * (parseInt(r(220)) / 5) + -parseInt(r(215)) / 6 * (parseInt(r(219)) / 7) + parseInt(r(207)) / 8 * (parseInt(r(214)) / 9) + parseInt(r(216)) / 10 + -parseInt(r(205)) / 11) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(ve);
    const Ce = Be;

    function Be(e, t) {
        const r = _e();
        return (Be = function (e, t) {
            return r[e -= 491]
        })(e, t)
    }

    function _e() {
        const e = ["2830729nACPTE", "debugLog", "downloadPcmFile", "talkStreamError", "getUserMedia is not implemented in this browser", "resample", ".ogg", "talkStreamInactive", "numberChannels", "jttChannelNumber", "createMediaStreamSource", "downloadFile", "_doTalk", "_initTalk", " and encType is ", "tempRtpBufferList", "set", "_encodeAudioData", "_createWebSocket", "testMicrophone", "resampler", "destroy", "log", "getUserMedia2 success", ", and ", "createJavaScriptNode", "jttChannelNumber is null or length is not 2", "socketStatusOpen", "wsUrl is null", "632079inRyqS", "_startHeartInterval", "openWebsocketHeart", "tcp", "binaryType", "addJttToBuffer", ", and sampleBitsWidth is ", "worklet", "getOption", "rtp", "URL", "isDestroyed", "getUserMediaTimeout", "10jxGFfM", "createGain", "tempTimestamp", "audioPlayer", "jttPacket", "workletRecorder", "jtt", "checkGetUserMediaTimeout", "2426115qaeoTi", "emit", "assign", "recorder", " length is ", "arraybuffer", "s and status is ", "rtpSsrc", "audioBufferList", "118yAMlVf", "createScriptProcessor", "webkitAudioContext", "mediaStreamSource", "concat", "_opt", "decode", "websocket open -> do talk", "jttSimNumber is null or length is not 12", '" string so return', "send", "AudioContext", "clearWorkletUrlTimeout", "calcAudioBufferLength", "148734rJuxHA", "tempOpusBufferList", "socketStatus", ".wav", "getUserMedia error", "catch", "toString", "toFixed", "stop checkGetUserMediaTimeout", "forEach", "talkStreamMsg", "startTimestamp", "error", "startTalk", "debugError", "bufferSize", "_stopCheckWsOpenTimeout", "_sendClose", "addRtpToBuffer", "_sendTalkMsg", "websocket error -> reject", "oninactive", "audioContext", "addG711ToBuffer", "now", "warn", "tempJttBufferList", "downloadJttFile", "addPcmToBuffer", ".pcm", "_getSequenceId", "close", "_initScriptProcessor", "g711u", "type", "gain", "getUserMedia2 fail", "_getUserMedia3", "heartInterval", "rtpPacket", "download", "userMediaStream", ", rtp only support g711a or g711u or opus so set packetType to empty", "sampleBitsWidth", "open", "volume", "getUserMedia finally", "_initWorklet", "_handleStreamInactive", "empty", "downloadG711File", "packetTcpSendType", "inputBuffer", "pcm", ".rtp", "createElement", "audio/ogg", "talkGetUserMediaSuccess", "connect", "opus", "getUserMedia", "\n            encType is ", "onaudioprocess", "downloadOpusFile", "createBiquadFilter", "navigator", "buffer", "then", "_handleMessage", "downloadRtpFile", "packetType", "setVolume", "data", "packetType is ", "play", "sampleRate", "tempPcmBufferList", "audioWorklet", "_stopHeartInterval", "TAG_NAME", "notConnect", "player", " and length is ", "click", "mediaDevices", "_log", "_getUserMedia", "4hiuWfj", "init", "disconnect", "sequenceId", "7050LqUFLj", "wsUrl", " only support sampleRate 8000 and sampleBitsWidth 16", "script", "tallWebsocketClosedByError", "message", "checkWsOpenTimeout", "g711a", "length", "77KcYTje", "getUserMedia3 fail", "audioBufferLength", "engine", "href", "socket", "stringify", "。\n            ", "addOpusToBuffer_1", "getUserMedia success", "addOpusToBuffer", "tempG711BufferList", "debug", "destination", "stop heart interval", "empty audio data", "770709tYmUPU", "websocketOpenTimeout", "opusDecoder", " and sampleRate is ", "encType", "once", "talkGetUserMediaFail", "getTracks", "opusEncoder", "jttSimNumber", "createObjectURL", "events", "_parseAudioMsg", "talkStreamClose", "byteLength", "onmessage", "getUserMedia() got stream:", "downloadOpusFile_1", "finally", "protocols", "talkStreamStart", "emitError", "_onaudioprocess", "value", "biquadFilter", "call", "init and version is", "reject", "lowpass", "string", "proxy", "_initMethods", "addModule", "frequency", "_destroyed", "debugWarn", "audioContext samplerate", "revokeObjectURL", "websocket Open Timeout form ", "gainNode", "mozGetUserMedia", "_stopCheckGetUserMediaTimeout", "eventType", "heart interval", "1028888vVkVlN", "window.navigator.mediaDevices.getUserMedia is undefined and init function"];
        return (_e = function () {
            return e
        })()
    }

    !function (e, t) {
        const r = Be, s = e();
        for (; ;) try {
            if (313094 === -parseInt(r(551)) / 1 * (-parseInt(r(656)) / 2) + parseInt(r(681)) / 3 * (-parseInt(r(652)) / 4) + parseInt(r(542)) / 5 + parseInt(r(565)) / 6 * (-parseInt(r(665)) / 7) + parseInt(r(725)) / 8 + parseInt(r(521)) / 9 + parseInt(r(534)) / 10 * (-parseInt(r(492)) / 11)) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(_e);

    class Me extends s {
        constructor(e) {
            const t = Be;
            let r = arguments.length > 1 && void 0 !== arguments[1] ? arguments[1] : {};
            super(), this[t(556)] = {}, e && (this[t(646)] = e), this[t(644)] = "talk";
            const s = Y(_);
            this[t(556)] = Object[t(544)]({}, s, r), !this[t(646)] && (this[t(677)] = new me(this)), this[t(514)](this[t(644)], t(707), '"10-15-2024"'), this[t(556)][t(640)] = parseInt(this[t(556)][t(640)], 10), this[t(556)][t(608)] = parseInt(this[t(556)][t(608)], 10), (this._opt[t(685)] === S[t(663)] || this._opt.encType === S[t(598)]) && (8e3 !== this[t(556)][t(640)] || 16 !== this[t(556)][t(608)]) && (this[t(590)](this[t(644)], t(626) + this[t(556)][t(685)] + t(684) + this[t(556)].sampleRate + t(527) + this._opt.sampleBitsWidth + t(672) + this[t(556)][t(685)] + t(658)), this[t(556)][t(640)] = 8e3, this[t(556)][t(608)] = 16), this[t(556)][t(635)] === T.rtp && this[t(556)][t(685)] === S[t(618)] && (this[t(590)](this[t(644)], t(638) + this._opt[t(635)] + t(506) + this._opt[t(685)] + t(607)), this[t(556)].packetType = T[t(614)]), this[t(556)].packetType, t(540), this._opt[t(685)], t(624), this[t(587)] = null, this[t(720)] = null, this[t(545)] = null, this[t(539)] = null, this[t(705)] = null, this[t(606)] = null, this.clearWorkletUrlTimeout = null, this[t(580)] = 512, this[t(556)][t(667)] = this[t(564)](), this.audioBufferList = [], this[t(689)] = null, this[t(683)] = null, this[t(512)] = null, this[t(556)][t(685)], this.socket = null, this[t(567)] = k.notConnect, this[t(554)] = null, this[t(603)] = null, this[t(541)] = null, this.wsUrl = null, this[t(576)] = 0, this[t(662)] = null, this[t(655)] = 0, this.tempTimestamp = null, this[t(715)] = !1, this[t(676)] = new Uint8Array(0), this[t(507)] = new Uint8Array(0), this[t(591)] = new Uint8Array(0), this[t(641)] = new Uint8Array(0), this[t(566)] = new Uint8Array(0), this.events = new ke(this), this[t(537)] = null, this._initTalk();
            try {
                this[t(514)](this[t(644)], t(653), JSON[t(671)](this[t(556)]))
            } catch (e) {
                this[t(514)](this[t(644)], "init", this[t(556)])
            }
        }

        [Ce(513)]() {
            const e = Ce;
            this._destroyed = !0, this.clearWorkletUrlTimeout && (clearTimeout(this[e(563)]), this.clearWorkletUrlTimeout = null), this[e(581)](), this[e(606)] && (this[e(606)][e(688)] && this.userMediaStream[e(688)]()[e(574)]((e => {
                e.stop()
            })), this[e(606)] = null), this[e(554)] && (this.mediaStreamSource[e(654)](), this[e(554)] = null), this[e(545)] && (this[e(545)][e(654)](), this.recorder[e(627)] = null, this.recorder = null), this[e(705)] && (this[e(705)][e(654)](), this[e(705)] = null), this.gainNode && (this[e(720)].disconnect(), this[e(720)] = null), this[e(539)] && (this[e(539)].disconnect(), this[e(539)] = null), this[e(689)] && (this.opusEncoder[e(513)](), this.opusEncoder = null), this[e(683)] && (this[e(683)][e(513)](), this[e(683)] = null), this[e(512)] && (this[e(512)][e(513)](), this[e(512)] = null), this.socket && (this[e(567)] === k.open && this[e(582)](), this[e(670)][e(596)](), this[e(670)] = null), this[e(537)] && (this.audioPlayer[e(513)](), this[e(537)] = null), this[e(643)](), this[e(722)](), this[e(587)] = null, this[e(720)] = null, this[e(545)] = null, this.audioBufferList = [], this[e(655)] = 0, this[e(657)] = null, this.tempTimestamp = null, this[e(507)] = null, this[e(676)] = null, this[e(641)] = null, this[e(566)] = null, this[e(576)] = 0, this.log(this[e(644)], e(513))
        }

        [Ce(581)]() {
            const e = Ce;
            this.checkWsOpenTimeout && (clearTimeout(this[e(662)]), this[e(662)] = null)
        }

        [Ce(532)]() {
            return this[Ce(715)]
        }

        [Ce(583)](e) {
            const t = Ce, r = e[t(664)] + this.tempRtpBufferList[t(664)], s = new Uint8Array(r);
            s.set(this[t(507)], 0), s[t(508)](e, this[t(507)][t(664)]), this[t(507)] = s
        }

        [Ce(588)](e) {
            const t = Ce, r = e[t(664)] + this.tempG711BufferList[t(664)], s = new Uint8Array(r);
            s.set(this[t(676)], 0), s[t(508)](e, this[t(676)][t(664)]), this.tempG711BufferList = s
        }

        [Ce(593)](e) {
            const t = Ce, r = e.length + this[t(641)][t(664)], s = new Uint8Array(r);
            s.set(this[t(641)], 0), s[t(508)](e, this[t(641)][t(664)]), this[t(641)] = s
        }

        addJttToBuffer(e) {
            const t = Ce, r = e.length + this.tempJttBufferList[t(664)], s = new Uint8Array(r);
            s[t(508)](this[t(591)], 0), s.set(e, this[t(591)].length), this[t(591)] = s
        }

        [Ce(673)](e) {
            const t = Ce;
            !this[t(683)] && (this.opusDecoder = new OpusDecoder(this[t(556)][t(640)], this._opt[t(500)]));
            const r = this[t(683)][t(557)](e), s = new Uint8Array(r[t(631)]);
            this[t(593)](s)
        }

        [Ce(675)](e) {
            const t = Ce, r = this[t(566)][t(664)] + 1 + e.length, s = new Uint8Array(r);
            s.set(this[t(566)], 0), s[t(508)]([e[t(664)]], this[t(566)][t(664)]), s[t(508)](e, this[t(566)][t(664)] + 1), this[t(566)] = s
        }

        [Ce(634)]() {
            const e = Ce;
            this.debugLog(this.TAG_NAME, "downloadRtpFile");
            const t = new Blob([this.tempRtpBufferList]);
            try {
                const r = document[e(620)]("a");
                r[e(669)] = window[e(531)].createObjectURL(t), r.download = Date[e(589)]() + e(619), r.click(), this[e(507)] = new Uint8Array(0), window[e(531)][e(718)](r[e(669)])
            } catch (t) {
                console[e(577)](e(634), t)
            }
        }

        [Ce(615)]() {
            const e = Ce;
            this[e(493)](this[e(644)], e(615));
            const t = new Blob([this[e(676)]]);
            try {
                const r = document.createElement("a");
                r[e(669)] = window[e(531)].createObjectURL(t), r.download = Date.now() + "." + this._opt[e(685)], r[e(648)](), this[e(676)] = new Uint8Array(0), window.URL[e(718)](r.href)
            } catch (t) {
                console[e(577)](e(615), t)
            }
        }

        [Ce(698)]() {
            const e = Ce;
            this.debugLog(this[e(644)], e(628)), this.downloadPcmFile()
        }

        [Ce(628)]() {
            const e = Ce;
            this[e(493)](this[e(644)], e(628));
            const t = function (e, t, r) {
                const s = H,
                    i = new Uint8Array([79, 103, 103, 83, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]),
                    o = new Uint8Array([79, 112, 117, 115, 72, 101, 97, 100, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255 & t, t >> 8 & 255, t >> 16 & 255, t >> 24 & 255, 255 & r, r >> 8 & 255, 0, 0, 0, 0]),
                    n = new Uint8Array(i[s(287)] + o.length + e[s(287)]);
                return n[s(375)](i), n[s(375)](o, i[s(287)]), n[s(375)](e, i[s(287)] + o.length), n
            }(this[e(566)], this[e(556)][e(640)], this[e(556)][e(500)]), r = new Blob([t], {type: e(621)});
            try {
                const t = document[e(620)]("a");
                t[e(669)] = window.URL[e(691)](r), t[e(605)] = Date[e(589)]() + e(498), t.click(), this[e(566)] = new Uint8Array(0), window.URL[e(718)](t[e(669)])
            } catch (t) {
                console[e(577)](e(628), t)
            }
        }

        [Ce(494)]() {
            const e = Ce;
            let t = arguments.length > 0 && void 0 !== arguments[0] && arguments[0];
            this.debugLog(this.TAG_NAME, e(494), this._opt[e(640)], this[e(556)][e(500)], this[e(556)][e(608)]);
            let r = new DataView(this.tempPcmBufferList[e(631)]), s = null, i = "";
            t ? (r = function (e, t, r, s) {
                const i = H;
                var o = function (e, t, r) {
                    const s = Q;
                    for (var i = 0; i < r[s(287)]; i++) e[s(411)](t + i, r.charCodeAt(i))
                }, n = t, a = s, c = new ArrayBuffer(44 + e[i(340)]), l = new DataView(c), u = r, h = 0;
                o(l, h, "RIFF"), h += 4, l[i(308)](h, 36 + e[i(340)], !0), o(l, h += 4, i(187)), o(l, h += 4, i(476)), h += 4, l[i(308)](h, 16, !0), h += 4, l[i(225)](h, 1, !0), h += 2, l[i(225)](h, u, !0), h += 2, l[i(308)](h, n, !0), h += 4, l[i(308)](h, u * n * (a / 8), !0), h += 4, l[i(225)](h, u * (a / 8), !0), h += 2, l.setUint16(h, a, !0), o(l, h += 2, i(439)), h += 4, l.setUint32(h, e[i(340)], !0), h += 4;
                for (let t = 0; t < e[i(340)];) l[i(411)](h, e[i(182)](t)), h++, t++;
                return l
            }(r, this[e(556)][e(640)], this[e(556)][e(500)], this[e(556)].sampleBitsWidth), i = e(568), s = new Blob([r], {type: "audio/wav"})) : (i = e(594), s = new Blob([this.tempPcmBufferList]));
            try {
                const t = document[e(620)]("a");
                t[e(669)] = window.URL.createObjectURL(s), t[e(605)] = Date[e(589)]() + i, t.click(), this[e(641)] = new Uint8Array(0), window[e(531)].revokeObjectURL(t[e(669)])
            } catch (t) {
                console.error(e(634), t)
            }
        }

        [Ce(592)]() {
            const e = Ce;
            this[e(493)](this[e(644)], e(592));
            const t = new Blob([this.tempJttBufferList]);
            try {
                const r = document[e(620)]("a");
                r[e(669)] = window.URL.createObjectURL(t), r[e(605)] = Date[e(589)]() + ".jtt", r[e(648)](), this[e(591)] = new Uint8Array(0), window[e(531)][e(718)](r.href)
            } catch (t) {
                console[e(577)](e(592), t)
            }
        }

        [Ce(503)]() {
            const e = Ce;
            this[e(556)][e(635)] === T[e(530)] ? this[e(634)]() : this[e(556)][e(635)] === T[e(540)] ? this.downloadJttFile() : this[e(556)].encType === S.g711a || this[e(556)][e(685)] === S[e(598)] ? this[e(615)]() : this._opt[e(685)] === S[e(624)] ? this.downloadOpusFile() : this[e(556)].encType === S[e(618)] && this[e(494)]()
        }

        [Ce(564)]() {
            const e = Ce, {sampleRate: t, sampleBitsWidth: r, frameDuration: s} = this[e(556)];
            return t * r * (s / 1e3) / 8
        }

        get [Ce(519)]() {
            return this[Ce(567)] === k.open
        }

        [Ce(514)]() {
            const e = Ce;
            for (var t = arguments.length, r = new Array(t), s = 0; s < t; s++) r[s] = arguments[s];
            this[e(650)](e(514), ...r)
        }

        [Ce(590)]() {
            const e = Ce;
            for (var t = arguments[e(664)], r = new Array(t), s = 0; s < t; s++) r[s] = arguments[s];
            this[e(650)]("warn", ...r)
        }

        error() {
            const e = Ce;
            for (var t = arguments[e(664)], r = new Array(t), s = 0; s < t; s++) r[s] = arguments[s];
            this[e(650)](e(577), ...r)
        }

        _log(e) {
            const t = Ce;
            for (var r = arguments.length, s = new Array(r > 1 ? r - 1 : 0), i = 1; i < r; i++) s[i - 1] = arguments[i];
            this[t(646)] ? this[t(646)][t(677)][e](...s) : this.debug ? this[t(677)][e](...s) : console[e](...s)
        }

        [Ce(595)]() {
            return ++this[Ce(655)]
        }

        [Ce(510)](e) {
            return new Promise(((t, r) => {
                const s = Be, i = this[s(692)][s(711)];
                this.socket = new WebSocket(this[s(657)], e[s(700)] || []), this.socket[s(525)] = s(547), this[s(543)](g[s(701)]), this[s(662)] = setTimeout((() => {
                    const e = s;
                    if (this[e(567)] === k[e(645)]) {
                        const t = e(719) + this[e(556)][e(682)] + e(548) + this[e(567)];
                        this[e(579)](this[e(644)], t), this[e(567)] = k.timeout, r(t)
                    }
                }), 1e3 * this._opt[s(682)]), i(this[s(670)], C.open, (() => {
                    const e = s;
                    this[e(567)] = k[e(609)], this[e(581)](), this[e(514)](this[e(644)], e(558)), this[e(543)](g.talkStreamOpen), t(), this[e(504)]()
                })), i(this[s(670)], C[s(661)], (e => {
                    const t = s;
                    typeof e.data !== t(710) ? this[t(633)](e[t(637)]) : this[t(716)](this.TAG_NAME, 'websocket handle message message is "' + e[t(637)] + t(560))
                })), i(this.socket, C[s(596)], (e => {
                    const t = s;
                    this[t(567)] = k[t(596)], this[t(590)](this[t(644)], "websocket close -> reject", e), this[t(702)](g[t(694)]), r(e)
                })), i(this[s(670)], C[s(577)], (e => {
                    const t = s;
                    this[t(567)] = k[t(577)], this[t(577)](this.TAG_NAME, t(585), e), this[t(702)](g[t(495)], e)
                }))
            }))
        }

        [Ce(582)]() {
        }

        [Ce(505)]() {
            const e = Ce;
            this[e(712)](), this[e(556)][e(668)] === B[e(528)] ? this[e(612)]() : this._opt[e(668)] === B[e(659)] && this[e(597)](), this[e(514)](this[e(644)], e(717), this[e(587)][e(640)])
        }

        [Ce(712)]() {
            const e = Ce;
            this[e(587)] = new (window[e(562)] || window[e(553)])({sampleRate: 48e3}), this[e(720)] = this.audioContext[e(535)](), this[e(720)][e(600)].value = 1, this.biquadFilter = this[e(587)][e(629)](), this[e(705)][e(599)] = e(709), this.biquadFilter[e(714)][e(704)] = 3e3, this.resampler = new se({
                fromSampleRate: this.audioContext[e(640)],
                toSampleRate: this[e(556)][e(640)],
                channels: this._opt[e(500)],
                inputBufferSize: this.bufferSize
            })
        }

        [Ce(597)]() {
            const e = Ce, t = this[e(587)][e(552)] || this.audioContext[e(517)];
            this[e(545)] = t.apply(this[e(587)], [this[e(580)], this[e(556)][e(500)], this[e(556)][e(500)]]), this.recorder[e(627)] = t => this[e(703)](t)
        }

        [Ce(612)]() {
            const e = Ce, t = function (e) {
                const t = H, r = function (e) {
                    const t = Q;
                    return e.trim()[t(204)](/^function\s*\w*\s*\([\w\s,]*\)\s*{([\w\W]*?)}$/)[1]
                }(e.toString()), s = new Blob([r], {type: t(170)});
                return URL[t(355)](s)
            }(Ae);
            this.audioContext[e(642)] && this[e(587)][e(642)][e(713)](t)[e(632)]((() => {
                const t = e,
                    r = new AudioWorkletNode(this[t(587)], "talk-processor", {processorOptions: {bufferSize: this[t(580)]}});
                r[t(623)](this.gainNode), r.port[t(696)] = e => {
                    const r = t;
                    e[r(637)][r(723)] === r(637) && this._encodeAudioData(e[r(637)].buffer)
                }, this[t(539)] = r
            })), this[e(563)] = setTimeout((() => {
                URL[e(718)](t), this.clearWorkletUrlTimeout = null
            }), 1e4)
        }

        [Ce(703)](e) {
            const t = Ce, r = e[t(617)].getChannelData(0);
            this[t(509)](new Float32Array(r))
        }

        [Ce(509)](e) {
            const t = Ce;
            if (this.isDestroyed()) return;
            if (0 === e[0] && 0 === e[1]) return void this[t(514)](this[t(644)], t(680));
            const r = this[t(512)][t(497)](e);
            if (this[t(556)][t(685)] === S[t(624)]) ; else {
                let e = r;
                16 === this._opt[t(608)] ? e = function (e) {
                    const t = oe;
                    let r = e[t(410)], s = new Int16Array(r);
                    for (; r--;) {
                        let i = Math[t(424)](-1, Math[t(416)](1, e[r]));
                        s[r] = i < 0 ? 32768 * i : 32767 * i
                    }
                    return s
                }(r) : 8 === this[t(556)][t(608)] ? e = function (e) {
                    const t = oe;
                    let r = e.length, s = new Int8Array(r);
                    for (; r--;) {
                        let i = Math[t(424)](-1, Math[t(416)](1, e[r]));
                        const o = i < 0 ? 32768 * i : 32767 * i;
                        s[r] = parseInt(255 / (65535 / (32768 + o)), 10)
                    }
                    return s
                }(r) : 32 === this._opt.sampleBitsWidth && (e = function (e) {
                    const t = oe;
                    let r = e[t(410)], s = new Int32Array(r);
                    for (; r--;) {
                        let i = Math[t(424)](-1, Math[t(416)](1, e[r]));
                        s[r] = i < 0 ? 2147483648 * i : 2147483647 * i
                    }
                    return s
                }(r));
                let s = null;
                this[t(556)][t(685)] === S[t(663)] ? s = le(e) : this[t(556)][t(685)] === S[t(598)] ? s = ue(e) : this[t(556)][t(685)] === S[t(618)] && (s = e[t(631)]);
                const i = new Uint8Array(s);
                for (let e = 0; e < i.length; e++) {
                    let r = this[t(550)][t(664)];
                    this[t(550)][r++] = i[e], this[t(550)][t(664)] === this[t(556)].audioBufferLength && (this[t(584)](new Uint8Array(this[t(550)])), this.audioBufferList = [])
                }
            }
        }

        [Ce(693)](e) {
            const t = Ce;
            let r = null;
            return this[t(556)][t(635)] !== T[t(530)] || this[t(556)][t(685)] !== S[t(663)] && this._opt[t(685)] !== S.g711u && this._opt.enc !== S[t(624)] ? this[t(556)][t(635)] !== T.jtt || this[t(556)].encType !== S[t(663)] && this[t(556)][t(685)] !== S[t(598)] ? this[t(556)].packetType === T[t(614)] && (r = e) : r = this[t(538)](e) : r = this[t(604)](e), r
        }

        [Ce(604)](e) {
            const t = Ce, r = [];
            let s = 0, i = 0, o = 0;
            const n = this[t(556)][t(549)], a = e[t(664)];
            this[t(556)][t(685)] === S.g711a ? s = I[t(663)] : this[t(556)][t(685)] === S[t(598)] ? s = I[t(598)] : this._opt[t(685)] === S[t(624)] && (s = I[t(624)]), !this.startTimestamp && (this.startTimestamp = q()), o = q() - this[t(576)], i = this[t(595)]();
            let c = 0;
            if (this[t(556)][t(616)] === A[t(524)]) {
                const e = a + 12;
                r[c++] = 255 & e >> 8, r[c++] = 255 & e >> 0
            }
            r[c++] = 128, r[c++] = 128 + s, r[c++] = i / 256, r[c++] = i % 256, r[c++] = o / 65536 / 256, r[c++] = o / 65536 % 256, r[c++] = o % 65536 / 256, r[c++] = o % 65536 % 256, r[c++] = n / 65536 / 256, r[c++] = n / 65536 % 256, r[c++] = n % 65536 / 256, r[c++] = n % 65536 % 256;
            let l = r[t(555)]([...e]), u = new Uint8Array(l.length);
            for (let e = 0; e < l[t(664)]; e++) u[e] = l[e];
            return u
        }

        [Ce(538)](e) {
            const t = Ce, r = [], s = [48, 49, 99, 100], i = [129, 134];
            let o = 0;
            const n = this._opt.jttSimNumber, a = this[t(556)].jttChannelNumber;
            let c = 0;
            const l = e.length;
            !this[t(576)] && (this[t(576)] = q()), c = q() - this.startTimestamp, o = this[t(595)](), r[0] = s[0], r[1] = s[1], r[2] = s[2], r[3] = s[3], r[4] = i[0], r[5] = i[1], r[6] = o / 256, r[7] = o % 256;
            const u = function (e) {
                const t = Te;
                if ((e = e.replace(/\s/g, "")).length % 2 != 0) return console.error(t(213), e[t(217)]), null;
                const r = new Uint8Array(e[t(217)] / 2);
                for (let s = 0; s < e[t(217)]; s += 2) {
                    const t = parseInt(e.substr(s, 2), 16);
                    r[s / 2] = t
                }
                return r
            }(n);
            u && (r[8] = u[0], r[9] = u[1], r[10] = u[2], r[11] = u[3], r[12] = u[4], r[13] = u[5]), r[14] = parseInt(a, 16), r[15] = 48, r[16] = 255 & c >> 56, r[17] = 255 & c >> 48, r[18] = 255 & c >> 40, r[19] = 255 & c >> 32, r[20] = 255 & c >> 24, r[21] = 255 & c >> 16, r[22] = 255 & c >> 8, r[23] = 255 & c >> 0, r[24] = l / 256, r[25] = l % 256;
            let h = r[t(555)]([...e]), d = new Uint8Array(h.length);
            for (let e = 0; e < h[t(664)]; e++) d[e] = h[e];
            return d
        }

        [Ce(584)](e) {
            const t = Ce;
            null === this[t(536)] && (this[t(536)] = q());
            const r = q(), s = r - this[t(536)], i = this[t(693)](e);
            this[t(514)](this[t(644)], "send talk msg and diff is " + s + " and byteLength is " + i[t(695)] + t(647) + i.length + t(516) + this[t(556)][t(685)] + t(546) + e[t(664)]), Z(this[t(556)].saveToTempFile) && Z(this._opt[t(677)]) && (this[t(556)][t(635)] === T.rtp ? this[t(583)](i) : this[t(556)][t(635)] === T[t(540)] ? this[t(526)](i) : this[t(556)][t(685)] === S[t(663)] || this._opt[t(685)] === S.g711u ? this[t(588)](i) : this._opt[t(685)] === S[t(618)] ? this[t(593)](i) : this[t(556)][t(685)] === S[t(624)] && this[t(675)](i)), i && (this[t(556)][t(511)] ? this[t(543)](g[t(575)], i.buffer) : this[t(519)] ? this.socket[t(561)](i[t(631)]) : this[t(702)](y[t(660)])), this[t(536)] = r
        }

        [Ce(504)]() {
            const e = Ce;
            this[e(651)](), this._opt[e(523)] && this._startHeartInterval()
        }

        _getUserMedia() {
            const e = Ce;
            this[e(514)](this[e(644)], e(625)), void 0 === window[e(630)][e(649)] && (window[e(630)][e(649)] = {}), void 0 === window[e(630)][e(649)][e(625)] && (this[e(514)](this[e(644)], e(491)), window[e(630)][e(649)][e(625)] = function (t) {
                const r = e;
                var s = navigator[r(625)] || navigator.webkitGetUserMedia || navigator[r(721)] || navigator.msGetUserMedia;
                return s ? new Promise((function (e, i) {
                    s[r(706)](navigator, t, e, i)
                })) : Promise[r(708)](new Error(r(496)))
            }), this[e(556)][e(541)] && this._startCheckGetUserMediaTimeout(), window[e(630)][e(649)].getUserMedia({
                audio: this[e(556)].audioConstraints,
                video: !1
            })[e(632)]((t => {
                const r = e;
                this[r(514)](this[r(644)], r(674)), this[r(606)] = t, this[r(554)] = this.audioContext[r(502)](t), this[r(554)][r(623)](this[r(705)]), this.recorder ? (this[r(705)].connect(this[r(545)]), this[r(545)][r(623)](this[r(720)])) : this[r(539)] && (this[r(705)][r(623)](this.workletRecorder), this[r(539)][r(623)](this[r(720)])), this.gainNode[r(623)](this[r(587)][r(678)]), this[r(543)](g[r(622)]), null === t[r(586)] && (t[r(586)] = e => {
                    this[r(613)](e)
                })
            })).catch((t => {
                const r = e;
                this[r(577)](this.TAG_NAME, r(569), t[r(571)]()), this[r(543)](g[r(687)], t[r(571)]())
            }))[e(699)]((() => {
                const t = e;
                this[t(514)](this[t(644)], t(611)), this[t(722)]()
            }))
        }

        _getUserMedia2() {
            const e = Ce;
            this[e(514)](this[e(644)], e(625)), navigator[e(649)] ? navigator[e(649)][e(625)]({audio: !0}).then((t => {
                const r = e;
                this[r(514)](this[r(644)], r(515))
            })) : navigator[e(625)]({audio: !0}, this[e(514)](this[e(644)], "getUserMedia2 success"), this[e(514)](this.TAG_NAME, e(601)))
        }

        async [Ce(602)]() {
            const e = Ce;
            this[e(514)](this[e(644)], "getUserMedia3");
            try {
                const t = await navigator[e(649)][e(625)]({
                    audio: {
                        latency: !0,
                        noiseSuppression: !0,
                        autoGainControl: !0,
                        echoCancellation: !0,
                        sampleRate: 48e3,
                        channelCount: 1
                    }, video: !1
                });
                console[e(514)](e(697), t), this[e(514)](this[e(644)], "getUserMedia3 success")
            } catch (t) {
                this.log(this[e(644)], e(666))
            }
        }

        [Ce(613)](e) {
            const t = Ce;
            this[t(606)] && (this[t(590)](this[t(644)], "stream oninactive", e), this.emit(g[t(499)]))
        }

        _startCheckGetUserMediaTimeout() {
            const e = Ce;
            this[e(722)](), this.checkGetUserMediaTimeout = setTimeout((() => {
                const t = e;
                this[t(514)](this.TAG_NAME, "check getUserMedia timeout"), this[t(543)](g.talkGetUserMediaTimeout)
            }), this._opt[e(533)])
        }

        [Ce(722)]() {
            const e = Ce;
            this.checkGetUserMediaTimeout && (this.log(this[e(644)], e(573)), clearTimeout(this[e(541)]), this.checkGetUserMediaTimeout = null)
        }

        [Ce(522)]() {
            const e = Ce;
            this[e(603)] = setInterval((() => {
                const t = e;
                this[t(514)](this[t(644)], t(724));
                let r = this[t(556)].websocketHeartContent;
                r = new Uint8Array(r), this.socket[t(561)](r[t(631)])
            }), 1e3 * this._opt.websocketHeartInterval)
        }

        [Ce(643)]() {
            const e = Ce;
            this[e(603)] && (this[e(514)](this[e(644)], e(679)), clearInterval(this[e(603)]), this[e(603)] = null)
        }

        [Ce(578)](e) {
            const t = Ce;
            let r = arguments[t(664)] > 1 && void 0 !== arguments[1] ? arguments[1] : {};
            return new Promise(((s, i) => {
                const o = t;
                if (!function () {
                    const e = H;
                    let t = !1;
                    const r = window[e(286)];
                    return r && (t = !(!r[e(458)] || !r[e(458)].getUserMedia), !t && (t = !!(r[e(460)] || r[e(347)] || r[e(250)] || r[e(243)]))), t
                }()) return i("not support getUserMedia");
                if (this[o(556)][o(635)] === T[o(540)]) {
                    if (!this._opt[o(690)] || 12 !== ("" + this._opt[o(690)]).length) return i(o(559));
                    if (!this[o(556)][o(501)] || 2 !== ("" + this[o(556)][o(501)])[o(664)]) return i(o(518))
                }
                if (this[o(657)] = e, this[o(556)][o(511)]) this._doTalk(); else {
                    if (!this[o(657)]) return i(o(520));
                    if ($()) return i(decodeURIComponent(F));
                    this[o(510)](r)[o(570)]((e => {
                        i(e)
                    }))
                }
                this.once(g[o(687)], (() => {
                    i("getUserMedia fail")
                })), this[o(686)](g[o(622)], (() => {
                    s()
                }))
            }))
        }

        [Ce(636)](e) {
            const t = Ce;
            e = parseFloat(e).toFixed(2), isNaN(e) || (e = function (e, t, r) {
                const s = Q;
                return Math[s(219)](Math[s(220)](e, Math[s(219)](t, r)), Math.min(t, r))
            }(e, 0, 1), this[t(720)][t(600)][t(704)] = e)
        }

        [Ce(529)]() {
            return this[Ce(556)]
        }

        get [Ce(610)]() {
            const e = Ce;
            return this.gainNode ? parseFloat(100 * this[e(720)].gain[e(704)])[e(572)](0) : null
        }

        [Ce(493)](e) {
            const t = Ce;
            if (this[t(556)][t(677)] && this.debug) {
                for (var r = arguments[t(664)], s = new Array(r > 1 ? r - 1 : 0), i = 1; i < r; i++) s[i - 1] = arguments[i];
                this[t(677)][t(514)](e, ...s)
            }
        }

        [Ce(716)](e) {
            const t = Ce;
            if (this._opt[t(677)] && this[t(677)]) {
                for (var r = arguments[t(664)], s = new Array(r > 1 ? r - 1 : 0), i = 1; i < r; i++) s[i - 1] = arguments[i];
                this[t(677)][t(590)](e, ...s)
            }
        }

        [Ce(579)](e) {
            const t = Ce;
            if (this[t(677)]) {
                for (var r = arguments[t(664)], s = new Array(r > 1 ? r - 1 : 0), i = 1; i < r; i++) s[i - 1] = arguments[i];
                this[t(677)][t(577)](e, ...s)
            }
        }

        [Ce(633)](e) {
            const t = Ce;
            this.debugLog(this[t(644)], t(633), e)
        }

        [Ce(702)](e) {
            const t = Ce;
            let r = arguments[t(664)] > 1 && void 0 !== arguments[1] ? arguments[1] : "";
            this.emit(g[t(577)], e, r), this[t(543)](e, r)
        }
    }

    const Fe = De;

    function Ue() {
        const e = ["stopTalk", "3iayqDS", "talk", "talkStreamError", "talkFailedAndStop", "1021912pxFSyf", "talkStreamError stopTalk", "693090XUVRqM", "bind", "length", "startTalk", "JbProTalk", "8721810vsDtXO", "debugLog", "talkStreamInactive", "destroy()", "_handleTalkGetUserMediaTimeout", "downloadRtpFile", "debug", "talkStreamClose stopTalk", "once", "emit", "tallWebsocketClosedByError", "_handleTalkWebsocketClosedByError", "downloadFile", "error", "volume", "talkStreamClose -> stopTalk", "talkStreamInactive -> stopTalk", "finally", "_initTalk", "talkGetUserMediaTimeout", "setVolume", "talk is not init", "EVENTS", "_initTalk this.talk is not null and destroy", "12353454dcSOGn", "init", "8322114uRZIMf", "_handleTalkStreamClose", "debugError", "downloadTempG711File", "startTalk error", "stringify", "WebPlayerProTalk", "setTalkVolume", "debugWarn", "destroy", "forEach", "downloadTempOpusFile", "2698060QbuDHU", "keys", "5DgKPID", "talkStreamClose", "1642967nDLqiE", "talkGetUserMediaTimeout -> stopTalk", "then", "talkWebsocketClosedByError -> stopTalk", "assign", "77NSYsAY", "talkWebsocketClosedByError stopTalk", "getOption", "_bindTalkEvents", "log", "catch", "toString", "_opt", "warn", "LOG_TAG", "talkStreamError -> stopTalk", "getTalkVolume", "talkStreamInactive stopTalk"];
        return (Ue = function () {
            return e
        })()
    }

    function De(e, t) {
        const r = Ue();
        return (De = function (e, t) {
            return r[e -= 178]
        })(e, t)
    }

    !function (e, t) {
        const r = De, s = e();
        for (; ;) try {
            if (832682 === parseInt(r(180)) / 1 + parseInt(r(205)) / 2 + parseInt(r(199)) / 3 * (-parseInt(r(248)) / 4) + -parseInt(r(178)) / 5 * (parseInt(r(236)) / 6) + parseInt(r(185)) / 7 * (parseInt(r(203)) / 8) + -parseInt(r(234)) / 9 + parseInt(r(210)) / 10) break;
            s.push(s.shift())
        } catch (e) {
            s.push(s.shift())
        }
    }(Ue);

    class Re extends s {
        constructor() {
            const e = De;
            let t = arguments[e(207)] > 0 && void 0 !== arguments[0] ? arguments[0] : {};
            super(), this[e(200)] = null, this._opt = t, this[e(194)] = e(209), this[e(216)] = new me(this);
            try {
                this[e(211)](this[e(194)], e(235), JSON[e(241)](t))
            } catch (r) {
                this[e(211)](this[e(194)], e(235), t)
            }
        }

        [Fe(245)]() {
            const e = Fe;
            this[e(211)](this.LOG_TAG, e(213)), this.off(), this.talk && (this[e(200)][e(245)](), this.talk = null), this.debugLog(this.LOG_TAG, "destroy")
        }

        [Fe(228)]() {
            const e = Fe;
            let t = arguments[e(207)] > 0 && void 0 !== arguments[0] ? arguments[0] : {};
            this.talk && (this.debugLog(this[e(194)], e(233)), this[e(200)][e(245)](), this[e(200)] = null);
            const r = Object[e(184)]({}, Y(this[e(192)]), t);
            this[e(192)] = r, this[e(200)] = new Me(null, r), this[e(211)](this[e(194)], e(228), this[e(200)][e(187)]()), this[e(188)]()
        }

        [Fe(188)]() {
            const e = Fe;
            Object[e(249)](b)[e(246)]((t => {
                this[e(200)].on(b[t], (e => {
                    this.emit(t, e)
                }))
            }))
        }

        [Fe(208)](e) {
            const t = Fe;
            let r = arguments[t(207)] > 1 && void 0 !== arguments[1] ? arguments[1] : {},
                s = arguments.length > 2 && void 0 !== arguments[2] ? arguments[2] : {};
            return new Promise(((i, o) => {
                const n = t;
                try {
                    this[n(211)](this[n(194)], n(208), e, JSON[n(241)](r))
                } catch (t) {
                    this[n(211)](this[n(194)], n(208), e, r)
                }
                if (this[n(228)](r), $()) {
                    const e = decodeURIComponent(F);
                    return this[n(238)](this[n(194)], e), void o(e)
                }
                this[n(200)][n(208)](e, s)[n(182)]((() => {
                    const e = n;
                    i(), this[e(200)].once(g[e(179)], this[e(237)][e(206)](this)), this[e(200)][e(218)](g[e(201)], this._handleTalkStreamError[e(206)](this)), this[e(200)][e(218)](g[e(212)], this._handleTalkStreamInactive[e(206)](this)), this.talk[e(218)](g[e(229)], this[e(214)][e(206)](this)), this[e(200)][e(218)](y[e(220)], this[e(221)][e(206)](this))
                }))[n(190)]((e => {
                    const t = n;
                    this[t(244)](this[t(194)], t(240), e, e[t(191)]()), this[t(198)]()[t(227)]((() => {
                        o(e)
                    }))
                }))
            }))
        }

        [Fe(198)]() {
            return new Promise(((e, t) => {
                const r = De;
                this[r(211)](this.LOG_TAG, "stopTalk()"), !this[r(200)] && t(r(231)), this[r(200)][r(245)](), this.talk = null, e()
            }))
        }

        [Fe(196)]() {
            return new Promise(((e, t) => {
                const r = De;
                !this.talk && t(r(231)), e(this[r(200)][r(224)])
            }))
        }

        [Fe(243)](e) {
            return new Promise(((t, r) => {
                const s = De;
                this[s(211)](this[s(194)], s(243), e), !this[s(200)] && r("talk is not init"), this.talk[s(230)](e / 100), t()
            }))
        }

        downloadTempRtpFile() {
            return new Promise(((e, t) => {
                const r = De;
                this[r(200)] ? (this[r(200)][r(215)](), e()) : t(r(231))
            }))
        }

        [Fe(239)]() {
            return new Promise(((e, t) => {
                const r = De;
                this.talk ? (this[r(200)].downloadG711File(), e()) : t("talk is not init")
            }))
        }

        downloadTempPcmFile(e) {
            return new Promise(((t, r) => {
                const s = De;
                this[s(200)] ? (this[s(200)].downloadPcmFile(e), t()) : r(s(231))
            }))
        }

        [Fe(247)]() {
            return new Promise(((e, t) => {
                const r = De;
                this[r(200)] ? (this[r(200)].downloadOpusFile(), e()) : t(r(231))
            }))
        }

        downloadTempFile() {
            return new Promise(((e, t) => {
                const r = De;
                this.talk ? (this[r(200)][r(222)](), e()) : t(r(231))
            }))
        }

        debugLog(e) {
            const t = Fe;
            if (this._opt[t(216)] && this[t(216)]) {
                for (var r = arguments[t(207)], s = new Array(r > 1 ? r - 1 : 0), i = 1; i < r; i++) s[i - 1] = arguments[i];
                this.debug[t(189)](e, ...s)
            }
        }

        [Fe(244)](e) {
            const t = Fe;
            if (this[t(192)].debug && this.debug) {
                for (var r = arguments[t(207)], s = new Array(r > 1 ? r - 1 : 0), i = 1; i < r; i++) s[i - 1] = arguments[i];
                this[t(216)][t(193)](e, ...s)
            }
        }

        [Fe(238)](e) {
            const t = Fe;
            if (this[t(216)]) {
                for (var r = arguments[t(207)], s = new Array(r > 1 ? r - 1 : 0), i = 1; i < r; i++) s[i - 1] = arguments[i];
                this.debug[t(223)](e, ...s)
            }
        }

        [Fe(237)]() {
            const e = Fe;
            this[e(244)](this[e(194)], e(225)), this[e(198)]()[e(190)]((t => {
                const r = e;
                this[r(244)](this[r(194)], r(217), t)
            }))[e(227)]((() => {
                const t = e;
                this[t(219)](g[t(202)], g[t(179)])
            }))
        }

        _handleTalkStreamError() {
            const e = Fe;
            this[e(238)](this[e(194)], e(195)), this[e(198)]()[e(190)]((t => {
                const r = e;
                this[r(244)](this[r(194)], r(204), t)
            }))[e(227)]((() => {
                const t = e;
                this.emit(g[t(202)], g[t(201)])
            }))
        }

        _handleTalkStreamInactive() {
            const e = Fe;
            this[e(244)](this.LOG_TAG, e(226)), this[e(198)]()[e(190)]((t => {
                const r = e;
                this[r(244)](this[r(194)], r(197), t)
            })).finally((() => {
                const t = e;
                this[t(219)](g[t(202)], g[t(212)])
            }))
        }

        [Fe(214)]() {
            const e = Fe;
            this[e(244)](this[e(194)], e(181)), this[e(198)]()[e(190)]((t => {
                this[e(244)](this.LOG_TAG, "talkGetUserMediaTimeout stopTalk", t)
            }))[e(227)]((() => {
                const t = e;
                this.emit(g[t(202)], g[t(229)])
            }))
        }

        _handleTalkWebsocketClosedByError() {
            const e = Fe;
            this[e(244)](this[e(194)], e(183)), this[e(198)]()[e(190)]((t => {
                const r = e;
                this[r(244)](this[r(194)], r(186), t)
            })).finally((() => {
                const t = e;
                this[t(219)](g[t(202)], y[t(220)])
            }))
        }
    }

    return Re[Fe(232)] = b, window.JessibucaProTalk = Re, window.JbProTalk = Re, window[Fe(242)] = Re, Re
}));
