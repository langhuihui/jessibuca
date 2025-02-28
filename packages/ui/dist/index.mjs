var K = Object.defineProperty;
var Z = (r, e, t) => e in r ? K(r, e, { enumerable: !0, configurable: !0, writable: !0, value: t }) : r[e] = t;
var w = (r, e, t) => Z(r, typeof e != "symbol" ? e + "" : e, t);
import { defineComponent as N, ref as L, watch as z, onMounted as W, onBeforeUnmount as V, h as S, render as ee, computed as _ } from "vue";
function te(r) {
  return r && r.__esModule && Object.prototype.hasOwnProperty.call(r, "default") ? r.default : r;
}
var Q = { exports: {} };
(function(r) {
  var e = Object.prototype.hasOwnProperty, t = "~";
  function i() {
  }
  Object.create && (i.prototype = /* @__PURE__ */ Object.create(null), new i().__proto__ || (t = !1));
  function n(d, l, g) {
    this.fn = d, this.context = l, this.once = g || !1;
  }
  function s(d, l, g, c, u) {
    if (typeof g != "function")
      throw new TypeError("The listener must be a function");
    var v = new n(g, c || d, u), m = t ? t + l : l;
    return d._events[m] ? d._events[m].fn ? d._events[m] = [d._events[m], v] : d._events[m].push(v) : (d._events[m] = v, d._eventsCount++), d;
  }
  function o(d, l) {
    --d._eventsCount === 0 ? d._events = new i() : delete d._events[l];
  }
  function a() {
    this._events = new i(), this._eventsCount = 0;
  }
  a.prototype.eventNames = function() {
    var l = [], g, c;
    if (this._eventsCount === 0) return l;
    for (c in g = this._events)
      e.call(g, c) && l.push(t ? c.slice(1) : c);
    return Object.getOwnPropertySymbols ? l.concat(Object.getOwnPropertySymbols(g)) : l;
  }, a.prototype.listeners = function(l) {
    var g = t ? t + l : l, c = this._events[g];
    if (!c) return [];
    if (c.fn) return [c.fn];
    for (var u = 0, v = c.length, m = new Array(v); u < v; u++)
      m[u] = c[u].fn;
    return m;
  }, a.prototype.listenerCount = function(l) {
    var g = t ? t + l : l, c = this._events[g];
    return c ? c.fn ? 1 : c.length : 0;
  }, a.prototype.emit = function(l, g, c, u, v, m) {
    var f = t ? t + l : l;
    if (!this._events[f]) return !1;
    var h = this._events[f], x = arguments.length, E, T;
    if (h.fn) {
      switch (h.once && this.removeListener(l, h.fn, void 0, !0), x) {
        case 1:
          return h.fn.call(h.context), !0;
        case 2:
          return h.fn.call(h.context, g), !0;
        case 3:
          return h.fn.call(h.context, g, c), !0;
        case 4:
          return h.fn.call(h.context, g, c, u), !0;
        case 5:
          return h.fn.call(h.context, g, c, u, v), !0;
        case 6:
          return h.fn.call(h.context, g, c, u, v, m), !0;
      }
      for (T = 1, E = new Array(x - 1); T < x; T++)
        E[T - 1] = arguments[T];
      h.fn.apply(h.context, E);
    } else {
      var M = h.length, P;
      for (T = 0; T < M; T++)
        switch (h[T].once && this.removeListener(l, h[T].fn, void 0, !0), x) {
          case 1:
            h[T].fn.call(h[T].context);
            break;
          case 2:
            h[T].fn.call(h[T].context, g);
            break;
          case 3:
            h[T].fn.call(h[T].context, g, c);
            break;
          case 4:
            h[T].fn.call(h[T].context, g, c, u);
            break;
          default:
            if (!E) for (P = 1, E = new Array(x - 1); P < x; P++)
              E[P - 1] = arguments[P];
            h[T].fn.apply(h[T].context, E);
        }
    }
    return !0;
  }, a.prototype.on = function(l, g, c) {
    return s(this, l, g, c, !1);
  }, a.prototype.once = function(l, g, c) {
    return s(this, l, g, c, !0);
  }, a.prototype.removeListener = function(l, g, c, u) {
    var v = t ? t + l : l;
    if (!this._events[v]) return this;
    if (!g)
      return o(this, v), this;
    var m = this._events[v];
    if (m.fn)
      m.fn === g && (!u || m.once) && (!c || m.context === c) && o(this, v);
    else {
      for (var f = 0, h = [], x = m.length; f < x; f++)
        (m[f].fn !== g || u && !m[f].once || c && m[f].context !== c) && h.push(m[f]);
      h.length ? this._events[v] = h.length === 1 ? h[0] : h : o(this, v);
    }
    return this;
  }, a.prototype.removeAllListeners = function(l) {
    var g;
    return l ? (g = t ? t + l : l, this._events[g] && o(this, g)) : (this._events = new i(), this._eventsCount = 0), this;
  }, a.prototype.off = a.prototype.removeListener, a.prototype.addListener = a.prototype.on, a.prefixed = t, a.EventEmitter = a, r.exports = a;
})(Q);
var q = Q.exports;
const j = /* @__PURE__ */ te(q), ie = /#EXTINF:(\d+\.\d+),(.*?),(.*?)\s*$/;
class ne extends j {
  constructor() {
    super(), this.segments = [], this.currentSegmentIndex = -1, this.virtualTotalDuration = 0, this.isPlaying = !1, this.currentVirtualTime = 0, this.currentPhysicalTime = 0, this.bufferedRanges = [], this.loadingDelay = 500, this.playbackRate = 1, this.playbackTimer = null, this.setupEventEmitter();
  }
  /**
   * 设置事件发射器
   */
  setupEventEmitter() {
  }
  /**
   * 加载播放列表
   * @param playlist 播放列表信息
   */
  loadPlaylist(e) {
    this.segments = e.segments, this.virtualTotalDuration = e.totalDuration, this.currentSegmentIndex = -1, this.currentVirtualTime = 0, this.currentPhysicalTime = 0, this.bufferedRanges = [], this.emit("debug", `加载了播放列表，共${this.segments.length}个片段，总时长${this.virtualTotalDuration}秒`), this.emit("bufferUpdate", this.bufferedRanges);
  }
  /**
   * 根据m3u8内容创建播放列表
   * @param m3u8Content m3u8文件内容
   * @param baseUrl 基础URL
   */
  createPlaylistFromM3U8(e, t) {
    const i = e.split(`
`), n = [];
    let s = 0, o = null, a = null, d = 0, l = 0, g = 0;
    for (let u = 0; u < i.length; u++) {
      const v = i[u].trim(), m = ie.exec(v);
      if (m) {
        s = parseFloat(m[1]), o = m[2] || null, a = m[3] || null, this.emit("debug", `解析EXTINF: 时长=${s}秒, 物理时间=${o || "N/A"}, 编解码器=${a || "N/A"}`);
        continue;
      }
      if (v && !v.startsWith("#")) {
        const f = this.resolveUrl(t, v), h = {
          index: d,
          url: f,
          duration: s,
          virtualStartTime: l,
          virtualEndTime: l + s,
          physicalStartTime: g,
          physicalEndTime: g + s,
          physicalTime: o ? new Date(o) : null,
          codec: a,
          isLoaded: !1,
          isLoading: !1,
          isBuffered: !1
        };
        n.push(h), l += s, g += s, d++, s = 0, o = null, a = null;
      }
    }
    const c = n.reduce((u, v) => u + v.duration, 0);
    return this.emit("debug", `从M3U8解析出${n.length}个片段，总时长: ${c}秒`), {
      segments: n,
      totalDuration: c
    };
  }
  /**
   * 解析URL
   * @param baseUrl 基础URL
   * @param relativeUrl 相对URL
   */
  resolveUrl(e, t) {
    return t.startsWith("http://") || t.startsWith("https://") ? t : new URL(t, e).href;
  }
  /**
   * 查找包含指定虚拟时间的片段
   * @param virtualTime 虚拟时间
   * @returns 片段索引和开始时间
   */
  findSegmentForVirtualTime(e) {
    for (const t of this.segments)
      if (e >= t.virtualStartTime && e < t.virtualEndTime)
        return {
          segment: t,
          offsetInSegment: e - t.virtualStartTime
        };
    return { segment: null, offsetInSegment: 0 };
  }
  /**
   * 将虚拟时间转换为物理时间
   * @param virtualTime 虚拟时间
   */
  virtualToPhysicalTime(e) {
    const { segment: t, offsetInSegment: i } = this.findSegmentForVirtualTime(e);
    return t ? t.physicalStartTime + i : 0;
  }
  /**
   * 将物理时间转换为虚拟时间
   * @param physicalTime 物理时间
   */
  physicalToVirtualTime(e) {
    for (const t of this.segments)
      if (e >= t.physicalStartTime && e < t.physicalEndTime) {
        const i = e - t.physicalStartTime;
        return t.virtualStartTime + i;
      }
    return 0;
  }
  /**
   * 获取当前播放时间（虚拟时间）
   */
  getCurrentVirtualTime() {
    return this.currentVirtualTime;
  }
  /**
   * 获取总时长（虚拟时间）
   */
  getTotalDuration() {
    return this.virtualTotalDuration;
  }
  /**
   * 获取当前播放进度（百分比）
   */
  getProgress() {
    return this.virtualTotalDuration <= 0 ? 0 : this.currentVirtualTime / this.virtualTotalDuration * 100;
  }
  /**
   * 开始播放
   */
  play() {
    this.isPlaying || (this.isPlaying = !0, this.emit("playbackStateChange", !0), this.loadSegmentsForTime(this.currentVirtualTime), this.startPlaybackTimer());
  }
  /**
   * 暂停播放
   */
  pause() {
    this.isPlaying && (this.isPlaying = !1, this.emit("playbackStateChange", !1), this.stopPlaybackTimer());
  }
  /**
   * 跳转到指定时间（虚拟时间）
   * @param virtualTime 虚拟时间
   */
  seek(e) {
    e < 0 ? e = 0 : e > this.virtualTotalDuration && (e = this.virtualTotalDuration);
    const t = this.isPlaying;
    t && this.pause(), this.currentVirtualTime = e, this.currentPhysicalTime = this.virtualToPhysicalTime(e), this.emit("seeking", this.currentVirtualTime, this.currentPhysicalTime), this.loadSegmentsForTime(e).then(() => {
      this.emit("seeked", this.currentVirtualTime, this.currentPhysicalTime), t && this.play();
    });
  }
  /**
   * 加载指定时间所需的片段
   * @param virtualTime 虚拟时间
   */
  async loadSegmentsForTime(e) {
    const { segment: t } = this.findSegmentForVirtualTime(e);
    if (!t) {
      this.emit("debug", `找不到时间 ${e} 对应的片段`);
      return;
    }
    const i = [t.index];
    t.virtualEndTime - e < 3 && t.index < this.segments.length - 1 && i.push(t.index + 1), await Promise.all(i.map((s) => this.loadSegmentByIndex(s))), this.currentSegmentIndex = t.index;
  }
  /**
   * 加载指定索引的片段
   * @param index 片段索引
   */
  async loadSegmentByIndex(e) {
    if (e < 0 || e >= this.segments.length)
      return;
    const t = this.segments[e];
    if (!(t.isLoaded || t.isLoading)) {
      t.isLoading = !0, this.emit("segmentLoadStart", t), this.emit("debug", `开始加载片段 ${e}: ${t.url}`);
      try {
        await new Promise((i) => setTimeout(i, this.loadingDelay)), t.isLoaded = !0, t.isLoading = !1, t.isBuffered = !0, this.updateBufferRanges(), this.emit("segmentLoaded", t), this.emit("debug", `片段 ${e} 加载完成: ${t.url}`);
      } catch (i) {
        t.isLoading = !1, this.emit("segmentLoadError", t, i), this.emit("debug", `片段 ${e} 加载失败: ${i}`);
      }
    }
  }
  /**
   * 更新缓冲区范围
   */
  updateBufferRanges() {
    const e = [];
    let t = null;
    const i = [...this.segments].sort((n, s) => n.index - s.index);
    for (const n of i)
      n.isBuffered && (t ? n.virtualStartTime <= t.end ? t.end = Math.max(t.end, n.virtualEndTime) : (e.push(t), t = {
        start: n.virtualStartTime,
        end: n.virtualEndTime
      }) : t = {
        start: n.virtualStartTime,
        end: n.virtualEndTime
      });
    t && e.push(t), this.bufferedRanges = e, this.emit("bufferUpdate", this.bufferedRanges);
  }
  /**
   * 预加载下一个片段
   */
  preloadNextSegment() {
    if (this.currentSegmentIndex < 0 || this.currentSegmentIndex >= this.segments.length - 1)
      return;
    const e = this.currentSegmentIndex + 1;
    this.loadSegmentByIndex(e);
  }
  /**
   * 启动播放计时器
   */
  startPlaybackTimer() {
    if (this.playbackTimer !== null)
      return;
    const e = 50;
    this.playbackTimer = window.setInterval(() => {
      if (!this.isPlaying)
        return;
      this.currentVirtualTime += e / 1e3 * this.playbackRate, this.currentPhysicalTime = this.virtualToPhysicalTime(this.currentVirtualTime);
      const { segment: t } = this.findSegmentForVirtualTime(this.currentVirtualTime);
      t && t.index !== this.currentSegmentIndex && (this.currentSegmentIndex = t.index, this.preloadNextSegment()), this.currentVirtualTime >= this.virtualTotalDuration && (this.currentVirtualTime = this.virtualTotalDuration, this.pause()), this.emit("timeUpdate", this.currentVirtualTime, this.currentPhysicalTime);
    }, e);
  }
  /**
   * 停止播放计时器
   */
  stopPlaybackTimer() {
    this.playbackTimer !== null && (window.clearInterval(this.playbackTimer), this.playbackTimer = null);
  }
  /**
   * 设置播放速率
   * @param rate 播放速率
   */
  setPlaybackRate(e) {
    this.playbackRate = e;
  }
  /**
   * 销毁实例
   */
  destroy() {
    this.stopPlaybackTimer(), this.removeAllListeners(), this.segments = [];
  }
}
class se extends j {
  /**
   * Constructor
   */
  constructor() {
    super(), this.slidingWindowConfig = {
      forward: 2,
      backward: 1,
      enabled: !0
    }, this.segments = [], this.currentSegmentIndex = -1, this.segmentQueue = [], this.pendingSegmentLoads = [], this.isProcessingQueue = !1, this.mediaSource = null, this.sourceBuffer = null, this.isMediaSourceReady = !1, this.videoElement = null, this.fmp4InitSegmentUrl = null, this.fmp4InitSegmentLoaded = !1, this.bufferRanges = [];
  }
  /**
   * Initialize the SegmentLoader with required components
   * @param mediaSource MediaSource instance
   * @param videoElement HTML video element
   * @param segments Array of media segments
   * @param codec MIME type and codec information for creating SourceBuffer
   * @param fmp4InitSegmentUrl Optional URL for initialization segment
   */
  initialize(e, t, i, n, s = null) {
    if (this.mediaSource = e, this.sourceBuffer = null, this.videoElement = t, this.segments = i, this.fmp4InitSegmentUrl = s, this.fmp4InitSegmentLoaded = !1, this.currentSegmentIndex = -1, this.segmentQueue = [], this.pendingSegmentLoads = [], this.isProcessingQueue = !1, this.isMediaSourceReady = (e == null ? void 0 : e.readyState) === "open" || !1, this.bufferRanges = [], e) {
      const o = () => {
        this.log("MediaSource 'sourceopen' event received in SegmentLoader", "info"), this.isMediaSourceReady = !0, this.setMediaSourceReady(!0);
        try {
          e.readyState === "open" && (this.sourceBuffer = e.addSourceBuffer(n), this.log(`SourceBuffer created successfully with codec: ${n}`, "info"), this.setupSourceBufferEvents(), this.processPendingSegmentLoads());
        } catch (a) {
          this.log(`Failed to create SourceBuffer: ${a}`, "error"), this.emit("segmentLoadError", -1, new Error(`Failed to create SourceBuffer: ${a}`));
        }
        e.removeEventListener("sourceopen", o);
      };
      if (e.addEventListener("sourceopen", o), e.readyState === "open") {
        this.log("MediaSource already in 'open' state during initialization", "info"), this.isMediaSourceReady = !0, this.setMediaSourceReady(!0);
        try {
          this.sourceBuffer = e.addSourceBuffer(n), this.log(`SourceBuffer created successfully with codec: ${n}`, "info"), this.setupSourceBufferEvents();
        } catch (a) {
          this.log(`Failed to create SourceBuffer: ${a}`, "error"), this.emit("segmentLoadError", -1, new Error(`Failed to create SourceBuffer: ${a}`));
        }
      } else
        this.log(`MediaSource in '${e.readyState}' state during initialization, waiting for 'sourceopen' event`, "info");
    }
    this.sourceBuffer && this.setupSourceBufferEvents(), this.log(`SegmentLoader initialized with ${i.length} segments`, "info");
  }
  /**
   * Setup source buffer event listeners
   */
  setupSourceBufferEvents() {
    this.sourceBuffer && (this.sourceBuffer.addEventListener("updateend", () => {
      this.fmp4InitSegmentUrl && !this.fmp4InitSegmentLoaded ? this.loadInitSegment() : this.processNextSegment(), this.updateBufferRanges();
    }), this.sourceBuffer.addEventListener("error", (e) => {
      this.log(`SourceBuffer error: ${e}`, "error");
    }));
  }
  /**
   * Set the sliding window configuration
   * @param config Configuration object
   */
  setSlidingWindowConfig(e) {
    e.forward !== void 0 && (this.slidingWindowConfig.forward = e.forward), e.backward !== void 0 && (this.slidingWindowConfig.backward = e.backward), e.enabled !== void 0 && (this.slidingWindowConfig.enabled = e.enabled), this.log(
      `Sliding window configuration updated: forward=${this.slidingWindowConfig.forward}, backward=${this.slidingWindowConfig.backward}, enabled=${this.slidingWindowConfig.enabled}`,
      "info"
    ), this.currentSegmentIndex !== -1 && this.slidingWindowConfig.enabled && this.applySegmentSlidingWindow(this.currentSegmentIndex);
  }
  /**
   * Get the current sliding window configuration
   */
  getSlidingWindowConfig() {
    return { ...this.slidingWindowConfig };
  }
  /**
   * Update the current segment index
   * @param index New segment index
   */
  setCurrentSegmentIndex(e) {
    e !== this.currentSegmentIndex && (this.currentSegmentIndex = e, this.log(`Current segment index updated to #${e}`, "info"), this.slidingWindowConfig.enabled && this.applySegmentSlidingWindow(e));
  }
  /**
   * Load a segment with MSE
   * @param segmentIndex Segment index to load
   */
  loadSegmentWithMSE(e) {
    if (e === void 0 || e < 0 || e >= this.segments.length) {
      this.log(`Invalid segment index: ${e}`, "error");
      return;
    }
    if (this.slidingWindowConfig.enabled && this.currentSegmentIndex !== -1 && !this.isSegmentInSlidingWindow(e, this.currentSegmentIndex)) {
      this.log(`Segment #${e} is outside sliding window, skipping load`, "info");
      return;
    }
    const t = this.segments[e];
    if (t.isLoading) {
      this.log(`Segment #${e} is already loading, skipping duplicate request`, "info");
      return;
    }
    if (t.isLoaded && t.isBuffered) {
      this.log(`Segment #${e} is already loaded, skipping`, "info");
      return;
    }
    if (!this.mediaSource || !this.sourceBuffer || this.mediaSource.readyState !== "open") {
      this.pendingSegmentLoads.includes(e) || (this.pendingSegmentLoads.push(e), this.log(`MediaSource not ready, adding segment #${e} to pending queue`, "info"));
      return;
    }
    this.enqueueSegment(e);
  }
  /**
   * Load all segments
   */
  loadAllSegments() {
    this.log("Starting to load all segments", "info"), this.segmentQueue = [];
    for (let e = 0; e < this.segments.length; e++)
      this.enqueueSegment(e);
  }
  /**
   * Clear all segments from the buffer
   */
  clearAllSegments() {
    if (this.log("Clearing all loaded segments", "info"), this.sourceBuffer && !this.sourceBuffer.updating)
      try {
        this.sourceBuffer.remove(0, 1 / 0), this.segments.forEach((e) => {
          e.isLoaded = !1, e.isBuffered = !1;
        }), this.bufferRanges = [], this.emit("bufferUpdate", this.bufferRanges);
      } catch (e) {
        this.log(`Failed to clear segments: ${e}`, "error");
      }
  }
  /**
   * Check if a segment is within the sliding window
   * @param segmentIndex Segment index to check
   * @param currentIndex Current playback segment index
   */
  isSegmentInSlidingWindow(e, t) {
    if (!this.slidingWindowConfig.enabled) return !0;
    const i = Math.max(0, t - this.slidingWindowConfig.backward), n = Math.min(
      this.segments.length - 1,
      t + this.slidingWindowConfig.forward
    );
    return e >= i && e <= n;
  }
  /**
   * Apply sliding window logic - load needed segments and clean up unneeded ones
   * @param currentIndex Current playback segment index
   */
  applySegmentSlidingWindow(e) {
    if (!this.slidingWindowConfig.enabled) return;
    this.log(`Applying sliding window, current segment index: ${e}`, "debug");
    const t = Math.max(0, e - this.slidingWindowConfig.backward), i = Math.min(
      this.segments.length - 1,
      e + this.slidingWindowConfig.forward
    );
    this.log(`Sliding window range: ${t} to ${i}`, "debug");
    for (let n = e; n <= i; n++) {
      const s = this.segments[n];
      !s.isLoaded && !s.isLoading && (this.log(`Sliding window loading segment #${n}`, "info"), this.loadSegmentWithMSE(n));
    }
    if (this.sourceBuffer && !this.sourceBuffer.updating && t > 1)
      try {
        const n = this.segments[t];
        if (n && n.virtualStartTime !== void 0) {
          const s = n.virtualStartTime;
          if (s > 0.5) {
            this.log(`Trying to clean up segments outside window, time range: 0 to ${s.toFixed(2)}`, "info"), this.sourceBuffer.remove(0, s);
            for (let o = 0; o < t; o++)
              this.segments[o].isBuffered && (this.segments[o].isBuffered = !1, this.log(`Marked segment #${o} as not buffered`, "debug"), this.emit("segmentRemoved", o));
          }
        }
      } catch (n) {
        this.log(`Failed to clean up old segments: ${n}`, "error");
      }
  }
  /**
   * Add a segment to the processing queue
   * @param segmentIndex Segment index to add
   */
  enqueueSegment(e) {
    if (e === void 0 || e < 0 || e >= this.segments.length) {
      this.log(`Invalid segment index: ${e}`, "error");
      return;
    }
    if (this.segmentQueue.some((t) => t === e)) {
      this.log(`Segment #${e} is already in queue, skipping duplicate add`, "info");
      return;
    }
    this.segmentQueue.push(e), this.log(`Added segment #${e} to queue`, "info"), this.emit("segmentQueued", e), this.isProcessingQueue || this.processNextSegment();
  }
  /**
   * Process the next segment in the queue
   */
  processNextSegment() {
    if (this.segmentQueue.length === 0 || this.isProcessingQueue)
      return;
    if (!this.mediaSource || !this.sourceBuffer || this.mediaSource.readyState !== "open") {
      this.log("MediaSource not ready, cannot process queue", "warning");
      return;
    }
    if (this.sourceBuffer.updating) {
      this.log("SourceBuffer is updating, waiting for it to complete before processing queue", "info"), this.sourceBuffer.addEventListener("updateend", () => {
        this.sourceBuffer && this.sourceBuffer.removeEventListener("updateend", () => {
        }), this.processNextSegment();
      });
      return;
    }
    this.isProcessingQueue = !0;
    const e = this.segmentQueue.shift();
    if (e === void 0) {
      this.log("Queue item undefined, skipping processing", "error"), this.isProcessingQueue = !1, this.processNextSegment();
      return;
    }
    const t = this.segments[e];
    if (!t) {
      this.log(`Segment #${e} does not exist`, "error"), this.isProcessingQueue = !1, this.processNextSegment();
      return;
    }
    if (t.isBuffered) {
      this.log(`Segment #${e} is already buffered, skipping load`, "info"), this.isProcessingQueue = !1, this.processNextSegment();
      return;
    }
    t.isLoading = !0, this.emit("segmentLoadStart", e);
    const i = t.url;
    this.log(`Starting to load segment #${e}: ${i}`, "info"), fetch(i).then((n) => {
      if (!n.ok)
        throw new Error(`HTTP error! status: ${n.status}`);
      return n.arrayBuffer();
    }).then((n) => {
      if (!this.sourceBuffer)
        throw new Error("SourceBuffer not available");
      return this.sourceBuffer.updating ? new Promise((s) => {
        const o = () => {
          this.sourceBuffer && this.sourceBuffer.removeEventListener("updateend", o), s(n);
        };
        this.sourceBuffer && this.sourceBuffer.addEventListener("updateend", o);
      }) : n;
    }).then((n) => {
      if (this.sourceBuffer && !this.sourceBuffer.updating)
        try {
          this.sourceBuffer.appendBuffer(n);
          const s = () => {
            this.sourceBuffer && this.sourceBuffer.removeEventListener("updateend", s), t.isLoading = !1, t.isLoaded = !0, t.isBuffered = !0, this.log(`Segment #${e} loaded successfully`, "success"), this.emit("segmentLoaded", e), this.updateBufferRanges(), this.isProcessingQueue = !1, this.emit("segmentLoadCompleted", e), this.processNextSegment();
          };
          this.sourceBuffer && this.sourceBuffer.addEventListener("updateend", s);
        } catch (s) {
          this.log(`Error appending segment #${e} to buffer: ${s}`, "error"), t.isLoading = !1, this.isProcessingQueue = !1, this.emit("segmentLoadError", e, s), this.processNextSegment();
        }
      else
        this.log("SourceBuffer not ready or updating when trying to append", "error"), t.isLoading = !1, this.isProcessingQueue = !1, this.processNextSegment();
    }).catch((n) => {
      this.log(`Failed to load segment #${e}: ${n}`, "error"), t.isLoading = !1, this.isProcessingQueue = !1, this.emit("segmentLoadError", e, n), this.processNextSegment();
    });
  }
  /**
   * Process pending segment loads when MediaSource becomes ready
   */
  processPendingSegmentLoads() {
    if (this.pendingSegmentLoads.length === 0)
      return;
    this.log(`Processing ${this.pendingSegmentLoads.length} pending segment load requests`, "info");
    const e = [...this.pendingSegmentLoads];
    this.pendingSegmentLoads = [];
    for (const t of e)
      this.log(`Loading segment #${t} from pending queue`, "info"), this.enqueueSegment(t);
  }
  /**
   * Set MediaSource readiness state
   * @param isReady Whether the MediaSource is ready
   */
  setMediaSourceReady(e) {
    this.isMediaSourceReady = e, e ? (this.log("MediaSource is now ready", "info"), this.processPendingSegmentLoads()) : this.log("MediaSource is no longer ready", "warning");
  }
  /**
   * Load the initialization segment
   */
  loadInitSegment() {
    !this.fmp4InitSegmentUrl || this.fmp4InitSegmentLoaded || (this.log(`Loading FMP4 initialization segment: ${this.fmp4InitSegmentUrl}`, "info"), fetch(this.fmp4InitSegmentUrl).then((e) => {
      if (!e.ok)
        throw new Error(`HTTP error! status: ${e.status}`);
      return e.arrayBuffer();
    }).then((e) => {
      if (!this.sourceBuffer || this.sourceBuffer.updating) {
        if (this.sourceBuffer) {
          const i = () => {
            this.sourceBuffer && this.sourceBuffer.removeEventListener("updateend", i), this.loadInitSegment();
          };
          this.sourceBuffer.addEventListener("updateend", i);
        }
        return;
      }
      this.sourceBuffer.appendBuffer(e);
      const t = () => {
        this.sourceBuffer && this.sourceBuffer.removeEventListener("updateend", t), this.fmp4InitSegmentLoaded = !0, this.log("FMP4 initialization segment loaded successfully", "success"), this.processNextSegment();
      };
      this.sourceBuffer && this.sourceBuffer.addEventListener("updateend", t);
    }).catch((e) => {
      this.log(`Failed to load FMP4 initialization segment: ${e}`, "error");
    }));
  }
  /**
   * Update buffer ranges from the video element
   */
  updateBufferRanges() {
    if (!this.videoElement) return;
    const e = this.videoElement.buffered, t = [];
    for (let i = 0; i < e.length; i++) {
      const n = e.start(i), s = e.end(i);
      t.push({
        start: n,
        end: s
      });
    }
    this.bufferRanges = t, this.emit("bufferUpdate", t);
  }
  /**
   * Log a message and emit a log event
   * @param message Log message
   * @param type Log type
   */
  log(e, t) {
    switch (this.emit("log", e, t), t) {
      case "info":
        console.info(`[SegmentLoader] ${e}`);
        break;
      case "success":
        console.log(`[SegmentLoader] %c${e}`, "color: green");
        break;
      case "warning":
        console.warn(`[SegmentLoader] ${e}`);
        break;
      case "error":
        console.error(`[SegmentLoader] ${e}`);
        break;
      case "debug":
        console.debug(`[SegmentLoader] ${e}`);
        break;
    }
  }
}
class A extends q.EventEmitter {
  /**
   * 构造函数
   * @param videoElement HTML视频元素
   * @param options 解复用器选项
   */
  constructor(e, t = {}) {
    super(), this.isDestroyed = !1, this.totalDuration = 0, this.currentSegmentIndex = 0, this.mediaSource = null, this.lastTimeUpdateValue = 0, this.lastTimeUpdateTimestamp = 0, this.stuckDuration = 0, this.stuckDetectionThreshold = 2e3, this.handleVideoError = (i) => {
      const n = i.target;
      this.handleError("Video error", n.error);
    }, this.handleSeeking = () => {
      this.log(`Video seeking to ${this.videoElement.currentTime}`, "debug");
    }, this.handleTimeUpdate = () => {
      const i = this.videoElement.currentTime, n = this.virtualTimeline.physicalToVirtualTime(i), s = this.findSegmentForTime(n);
      if (s && s.index !== void 0) {
        const a = s.index;
        this.currentSegmentIndex !== a && (this.log(`Playback entered segment #${a}`, "debug"), this.currentSegmentIndex = a, this.segmentLoader.setCurrentSegmentIndex(a), this.segmentLoader.applySegmentSlidingWindow(a), a < this.getTotalSegments() - 1 && this.segmentLoader.loadSegmentWithMSE(a + 1));
      }
      const o = Date.now();
      Math.abs(this.lastTimeUpdateValue - n) < 0.01 ? (this.stuckDuration += o - this.lastTimeUpdateTimestamp, this.stuckDuration > this.stuckDetectionThreshold && this.recoverFromStuck(n)) : this.stuckDuration = 0, this.lastTimeUpdateValue = n, this.lastTimeUpdateTimestamp = o;
    }, this.videoElement = e, this.codec = t.codec || 'video/mp4; codecs="avc1.42E01E, mp4a.40.2"', this.maxBufferLength = t.maxBufferLength || 30, this.logLevel = t.logLevel || "debug", this.virtualTimeline = new ne(), this.setupVirtualTimelineListeners(), this.segmentLoader = new se(), t.slidingWindow && this.segmentLoader.setSlidingWindowConfig(t.slidingWindow), this.setupSegmentLoaderListeners();
  }
  /**
   * 设置片段加载器事件监听
   */
  setupSegmentLoaderListeners() {
    this.segmentLoader.on("segmentLoaded", (e) => {
      this.emit("segmentLoaded", e), this.log(`Segment ${e} loaded`, "debug");
    }), this.segmentLoader.on("bufferUpdate", (e) => {
      const t = e.map((i) => ({ start: i.start, end: i.end }));
      this.emit("bufferUpdate", t);
    }), this.segmentLoader.on("log", (e, t) => {
      this.log(e, t === "error" ? "error" : "debug");
    }), this.segmentLoader.on("segmentLoadError", (e, t) => {
      this.handleError(`Failed to load segment ${e}`, t);
    });
  }
  /**
   * 设置虚拟时间线事件监听
   */
  setupVirtualTimelineListeners() {
    this.virtualTimeline.on("segmentLoaded", (e) => {
      this.emit("segmentLoaded", e.index), this.log(`Segment ${e.index} loaded (virtual time: ${e.virtualStartTime.toFixed(2)}s-${e.virtualEndTime.toFixed(2)}s)`, "debug");
    }), this.virtualTimeline.on("bufferUpdate", (e) => {
      const t = e.map((i) => ({ start: i.start, end: i.end }));
      this.emit("bufferUpdate", t);
    }), this.virtualTimeline.on("debug", (e) => this.log(e, "debug")), this.virtualTimeline.on("timeUpdate", (e) => {
      this.lastTimeUpdateValue = e, this.lastTimeUpdateTimestamp = Date.now();
    }), this.virtualTimeline.on("seeking", (e, t) => this.log(`Seeking to virtual time: ${e.toFixed(2)}s (physical time: ${t.toFixed(2)}s)`, "debug")), this.virtualTimeline.on("seeked", (e, t) => {
      this.log(`Seeked to virtual time: ${e.toFixed(2)}s (physical time: ${t.toFixed(2)}s)`, "debug"), this.tryResumePlayback();
    });
  }
  // ===== 公共 API =====
  /**
   * 初始化解复用器
   * @param url M3U8播放列表URL
   */
  async init(e) {
    try {
      this.log(`Initializing HLSv7Demuxer with URL: ${e}`, "debug");
      const t = await this.fetchPlaylist(e);
      this.virtualTimeline.loadPlaylist(t), this.totalDuration = t.totalDuration, await this.prepareSegmentLoader(t), this.log(`Playlist loaded with ${t.segments.length} segments, total duration: ${this.totalDuration.toFixed(2)}s`, "debug"), this.log("HLSv7Demuxer initialized successfully", "debug");
    } catch (t) {
      throw this.handleError("Failed to initialize HLSv7Demuxer", t), t;
    }
  }
  /**
   * 获取当前播放时间
   * @returns 当前播放时间(秒)
   */
  getCurrentTime() {
    return this.checkDestroyed(), this.virtualTimeline.getCurrentVirtualTime();
  }
  /**
   * 获取总时长
   * @returns 总时长(秒)
   */
  getTotalDuration() {
    return this.checkDestroyed(), this.totalDuration;
  }
  /**
   * 获取播放进度
   * @returns 播放进度(0-1)
   */
  getProgress() {
    this.checkDestroyed();
    const e = this.getCurrentTime(), t = this.getTotalDuration();
    return t > 0 ? Math.min(1, Math.max(0, e / t)) : 0;
  }
  /**
   * 播放视频
   */
  async play() {
    this.checkDestroyed();
    try {
      this.virtualTimeline.play();
      const e = this.getCurrentTime(), t = this.findSegmentForTime(e);
      t && t.index !== void 0 && (this.currentSegmentIndex = t.index, this.segmentLoader.loadSegmentWithMSE(t.index)), await this.videoElement.play();
    } catch (e) {
      throw this.handleError("Failed to play video", e), e;
    }
  }
  /**
   * 暂停视频
   */
  pause() {
    this.checkDestroyed(), this.videoElement.pause(), this.virtualTimeline.pause();
  }
  /**
   * 跳转到指定时间
   * @param time 目标时间(秒)
   */
  async seek(e) {
    this.checkDestroyed();
    try {
      this.log(`Seeking to ${e.toFixed(2)}s`, "debug");
      const t = this.findSegmentForTime(e);
      if (!t || t.index === void 0)
        throw new Error(`Cannot find segment for time ${e}`);
      const i = this.virtualTimeline.virtualToPhysicalTime(e);
      this.currentSegmentIndex = t.index, this.segmentLoader.setCurrentSegmentIndex(t.index), await this.clearBuffer(), this.segmentLoader.loadSegmentWithMSE(t.index), t.index < this.getTotalSegments() - 1 && this.segmentLoader.loadSegmentWithMSE(t.index + 1), this.videoElement.currentTime = i, this.virtualTimeline.seek(e), this.log(`Completed seek to ${e.toFixed(2)}s (physical: ${i.toFixed(2)}s)`, "debug");
    } catch (t) {
      throw this.handleError(`Failed to seek to ${e}`, t), t;
    }
  }
  /**
   * 设置播放速率
   * @param rate 播放速率
   */
  setPlaybackRate(e) {
    this.checkDestroyed(), this.videoElement.playbackRate = e, this.virtualTimeline.setPlaybackRate(e), this.log(`Set playback rate to ${e}`, "debug");
  }
  /**
   * 清除缓冲区
   */
  async clearBuffer() {
    this.checkDestroyed();
    try {
      this.segmentLoader.clearAllSegments(), this.log("Buffer cleared", "debug");
    } catch (e) {
      throw this.handleError("Failed to clear buffer", e), e;
    }
  }
  /**
   * 获取总片段数
   * @returns 总片段数
   */
  getTotalSegments() {
    var t;
    let e = 0;
    for (; (t = this.virtualTimeline.findSegmentForVirtualTime(e * 10)) != null && t.segment && (e++, !(e > 1e3)); )
      ;
    return e || 0;
  }
  /**
   * 设置滑动窗口配置
   * @param config 滑动窗口配置
   */
  setSlidingWindowConfig(e) {
    this.segmentLoader.setSlidingWindowConfig(e);
  }
  /**
   * 获取滑动窗口配置
   * @returns 滑动窗口配置
   */
  getSlidingWindowConfig() {
    return this.segmentLoader.getSlidingWindowConfig();
  }
  /**
   * 销毁解复用器
   */
  destroy() {
    this.isDestroyed || (this.log("Destroying HLSv7Demuxer", "debug"), this.removeEventListeners(), this.virtualTimeline.destroy(), this.segmentLoader.clearAllSegments(), this.videoElement && (this.videoElement.src = "", this.videoElement.load()), this.isDestroyed = !0);
  }
  /**
   * 获取MediaSource实例
   * @returns MediaSource实例或null
   */
  getMediaSource() {
    return this.mediaSource;
  }
  // ===== 私有方法 =====
  /**
   * 设置事件监听器
   */
  setupEventListeners() {
    this.videoElement.addEventListener("error", this.handleVideoError), this.videoElement.addEventListener("seeking", this.handleSeeking), this.videoElement.addEventListener("timeupdate", this.handleTimeUpdate);
  }
  /**
   * 移除事件监听器
   */
  removeEventListeners() {
    this.videoElement.removeEventListener("error", this.handleVideoError), this.videoElement.removeEventListener("seeking", this.handleSeeking), this.videoElement.removeEventListener("timeupdate", this.handleTimeUpdate);
  }
  /**
   * 获取播放列表
   * @param url 播放列表URL
   * @returns 播放列表信息
   */
  async fetchPlaylist(e) {
    try {
      this.log(`Fetching playlist: ${e}`, "debug");
      const t = await this.parseM3U8Playlist(e), i = t.segments.map((n) => ({
        index: n.index,
        url: n.url,
        duration: n.duration,
        time: n.physicalTime || /* @__PURE__ */ new Date()
      }));
      return this.emit("playlistUpdate", i), t;
    } catch (t) {
      throw this.handleError(`Failed to fetch playlist: ${e}`, t), t;
    }
  }
  /**
   * 准备片段加载器
   * @param playlistInfo 播放列表信息
   */
  async prepareSegmentLoader(e) {
    try {
      const t = new MediaSource();
      this.mediaSource = t;
      const i = URL.createObjectURL(t);
      this.log(`MediaSource created with readyState: ${t.readyState}`, "debug"), this.videoElement.src = i, this.log(`Set video.src to MediaSource URL: ${i}`, "debug"), await new Promise((s, o) => {
        const a = () => {
          this.log(`MediaSource 'sourceopen' event fired, readyState: ${t.readyState}`, "debug"), t.removeEventListener("sourceopen", a), s();
        }, d = () => {
          this.log(`MediaSource 'sourceclose' event fired unexpectedly, readyState: ${t.readyState}`, "warning");
        };
        t.addEventListener("sourceopen", a), t.addEventListener("sourceclose", d);
        const l = setTimeout(() => {
          this.log(`MediaSource 'sourceopen' event timed out after 5s, readyState: ${t.readyState}`, "error"), t.removeEventListener("sourceopen", a), this.log(`Video element readyState: ${this.videoElement.readyState}, networkState: ${this.videoElement.networkState}`, "debug"), this.videoElement.error && this.log(`Video element error: ${this.videoElement.error.message}`, "error"), s();
        }, 5e3);
        t.readyState === "open" && (this.log("MediaSource already in 'open' state", "debug"), clearTimeout(l), s());
      }), t.readyState !== "open" && this.log(`MediaSource is still not 'open' (current state: ${t.readyState}). This may cause playback issues.`, "warning");
      const n = e.initSegmentUrl || null;
      this.segmentLoader.initialize(
        t,
        this.videoElement,
        e.segments,
        this.codec,
        // 传递codec信息，让SegmentLoader内部创建SourceBuffer
        n
      ), e.segments.length > 0 && (this.segmentLoader.loadSegmentWithMSE(0), e.segments.length > 1 && this.segmentLoader.loadSegmentWithMSE(1));
    } catch (t) {
      throw this.log(`Error during prepareSegmentLoader: ${t}`, "error"), this.handleError("Failed to prepare segment loader", t), t;
    }
  }
  /**
   * 查找特定时间对应的片段信息
   * @param time 查询时间(秒)
   * @returns 片段信息 {index, startTime}
   */
  findSegmentForTime(e) {
    const t = this.virtualTimeline.findSegmentForVirtualTime(e);
    return t && t.segment ? {
      index: t.segment.index,
      startTime: t.segment.virtualStartTime
    } : { index: 0, startTime: 0 };
  }
  /**
   * 从卡住状态恢复
   * @param currentTime 当前时间
   */
  recoverFromStuck(e) {
    this.log(`Playback appears stuck at ${e.toFixed(2)}s, attempting recovery`, "debug");
    const t = this.findSegmentForTime(e);
    if (t && t.index !== void 0) {
      const i = t.index;
      i < this.getTotalSegments() - 1 ? (this.log("Stuck at segment boundary, attempting to load next segment", "debug"), this.segmentLoader.loadSegmentWithMSE(i + 1)) : (this.log("Stuck within segment, attempting to reload current segment", "debug"), this.segmentLoader.loadSegmentWithMSE(i));
    }
    this.stuckDuration = 0;
  }
  /**
   * 尝试恢复播放
   */
  tryResumePlayback() {
    this.videoElement.paused && !this.videoElement.ended && (this.videoElement.readyState >= 2 ? (this.log("Attempting to resume playback after seek", "debug"), this.videoElement.play().catch((e) => {
      this.log(`Failed to resume playback: ${e}`, "error");
    })) : this.log("Video not ready to play, waiting for more data", "debug"));
  }
  /**
   * 检查解复用器是否已销毁
   */
  checkDestroyed() {
    if (this.isDestroyed)
      throw new Error("HLSv7Demuxer has been destroyed");
  }
  /**
   * 记录日志
   * @param message 日志消息
   * @param level 日志级别
   */
  log(e, t = "debug") {
    (t === "error" || this.logLevel === t || this.logLevel === "debug") && (t === "error" ? console.error(`[HLSv7Demuxer] ${e}`) : console.log(`[HLSv7Demuxer] ${e}`), t === "debug" && this.emit("debug", e));
  }
  /**
   * 处理错误
   * @param message 错误消息
   * @param error 错误对象
   */
  handleError(e, t) {
    this.log(`${e}: ${t}`, "error"), this.emit("error", new Error(`${e}: ${t}`));
  }
  /**
   * 解析M3U8播放列表
   * @param url M3U8播放列表URL
   * @returns 播放列表信息
   */
  async parseM3U8Playlist(e) {
    try {
      const t = await fetch(e);
      if (!t.ok)
        throw new Error(`HTTP error! status: ${t.status}`);
      const n = (await t.text()).split(`
`), s = [];
      let o = 0, a = 0, d = 0, l = null, g = null, c = /* @__PURE__ */ new Set();
      for (let u = 0; u < n.length; u++) {
        const v = n[u].trim();
        if (v.startsWith("#EXTINF:")) {
          const m = v.match(/#EXTINF:([\d.]+)/);
          m && (d = parseFloat(m[1]));
          const f = v.substring(v.indexOf(":") + 1).split(",");
          if (f.length >= 2)
            try {
              l = new Date(f[1]);
            } catch {
            }
          if (f.length >= 3 ? (g = f[2].trim(), c.add(g), this.log(`解析EXTINF: 时长=${d}秒, 物理时间=${(l == null ? void 0 : l.toISOString()) || "unknown"}, 编解码器=${g}`, "debug")) : this.log(`解析EXTINF: 时长=${d}秒, 物理时间=${(l == null ? void 0 : l.toISOString()) || "unknown"}`, "debug"), !g) {
            const h = v.match(/CODECS="([^"]+)"/);
            h && (g = h[1], c.add(g));
          }
        }
        if (!v.startsWith("#") && v.length > 0) {
          const m = new URL(v, e).href;
          s.push({
            index: s.length,
            url: m,
            duration: d,
            virtualStartTime: a,
            virtualEndTime: a + d,
            physicalStartTime: 0,
            physicalEndTime: d,
            physicalTime: l,
            codec: g,
            isLoaded: !1,
            isLoading: !1,
            isBuffered: !1
          }), a += d, o += d, l = null, g = null;
        }
      }
      if (this.log(`从M3U8解析出${s.length}个片段，总时长: ${o}秒`, "debug"), c.size > 0) {
        const u = Array.from(c);
        let v = u.some((f) => f.startsWith("mp4a"));
        u.some((f) => f.startsWith("avc1") || f.startsWith("hvc1") || f.startsWith("vp9")) ? (this.codec = `video/mp4; codecs="${u.join(", ")}"`, this.log(`从M3U8中检测到编解码器，使用视频编解码器: ${this.codec}`, "info")) : this.log(`未检测到视频编解码器，使用默认值: ${this.codec}`, "warning");
      } else
        this.log(`未从M3U8中检测到编解码器信息，使用默认值: ${this.codec}`, "warning");
      return this.totalDuration = o, { segments: s, totalDuration: o };
    } catch (t) {
      throw this.handleError(`Failed to parse M3U8 playlist: ${e}`, t), t;
    }
  }
}
class oe {
  constructor(e = {}) {
    w(this, "options");
    w(this, "currentTime", 0);
    w(this, "currentMediaTime", 0);
    this.options = {
      timeRanges: [],
      ...e
    };
  }
  setTimeRanges(e) {
    this.options.timeRanges = e, this.initializeTimeRanges(e);
  }
  initializeTimeRanges(e) {
    let t = 0;
    e.forEach((i) => {
      i.mediaStart = t, i.mediaDuration = i.end - i.start, t += i.mediaDuration;
    });
  }
  findTimeRangeForTime(e) {
    var t;
    return (t = this.options.timeRanges) == null ? void 0 : t.find(
      (i) => e >= i.start && e <= i.end
    );
  }
  originalToMediaTime(e) {
    var i;
    const t = this.findTimeRangeForTime(e);
    if (!t) {
      const n = (i = this.options.timeRanges) == null ? void 0 : i.find((o) => e < o.start);
      if (n) return n.mediaStart;
      const s = [...this.options.timeRanges || []].reverse().find((o) => e > o.end);
      return s ? s.mediaStart + s.mediaDuration : 0;
    }
    return t.mediaStart + (e - t.start);
  }
  mediaToOriginalTime(e) {
    var t;
    if (!((t = this.options.timeRanges) != null && t.length)) return e;
    for (const i of this.options.timeRanges)
      if (e >= i.mediaStart && e < i.mediaStart + i.mediaDuration)
        return i.start + (e - i.mediaStart);
    return this.options.timeRanges[0].start;
  }
  getTotalDuration() {
    var e;
    return ((e = this.options.timeRanges) == null ? void 0 : e.reduce((t, i) => t + i.mediaDuration, 0)) || 0;
  }
  getTimeRangeExtent() {
    var e;
    return (e = this.options.timeRanges) != null && e.length ? {
      start: Math.min(...this.options.timeRanges.map((t) => t.start)),
      end: Math.max(...this.options.timeRanges.map((t) => t.end))
    } : { start: 0, end: 0 };
  }
  setCurrentTime(e, t = !1) {
    var i, n;
    t ? (this.currentMediaTime = e, this.currentTime = this.mediaToOriginalTime(e)) : (this.currentTime = e, this.currentMediaTime = this.originalToMediaTime(e)), (n = (i = this.options).onTimeUpdate) == null || n.call(i, this.currentTime);
  }
  getCurrentTime() {
    return this.currentTime;
  }
  getCurrentMediaTime() {
    return this.currentMediaTime;
  }
  seek(e, t = !1) {
    var i, n;
    this.setCurrentTime(e, t), (n = (i = this.options).onSeek) == null || n.call(i, this.currentTime);
  }
  getTimeRanges() {
    return this.options.timeRanges || [];
  }
  getGaps() {
    const e = [], t = this.options.timeRanges || [];
    for (let i = 0; i < t.length - 1; i++)
      t[i + 1].start - t[i].end > 0 && e.push({
        start: t[i].end,
        end: t[i + 1].start
      });
    return e;
  }
}
const re = /* @__PURE__ */ N({
  name: "Timeline",
  props: {
    timeRanges: {
      type: Array,
      default: () => []
    },
    showOriginalTimeline: {
      type: Boolean,
      default: !0
    },
    showMediaTimeline: {
      type: Boolean,
      default: !1
    },
    height: {
      type: Number,
      default: 20
    },
    backgroundColor: {
      type: String,
      default: "#f5f5f5"
    },
    segmentColor: {
      type: String,
      default: "rgba(0, 160, 255, 0.3)"
    },
    gapColor: {
      type: String,
      default: "rgba(0, 0, 0, 0.1)"
    },
    cursorColor: {
      type: String,
      default: "#18a058"
    },
    onTimeUpdate: Function,
    onSeek: Function
  },
  setup(r) {
    const e = L(), t = L(), i = L(), n = L(!1), s = L();
    let o;
    const a = (y) => new Date(y * 1e3).toLocaleString("zh-CN", {
      month: "2-digit",
      day: "2-digit",
      hour: "2-digit",
      minute: "2-digit",
      second: "2-digit",
      hour12: !1
    }), d = (y) => {
      const p = Math.floor(y / 3600), b = Math.floor(y % 3600 / 60), C = Math.floor(y % 60);
      return `${p.toString().padStart(2, "0")}:${b.toString().padStart(2, "0")}:${C.toString().padStart(2, "0")}`;
    }, l = (y) => {
      const p = Math.floor(y / 60), b = Math.floor(y % 60);
      return `${p}:${b.toString().padStart(2, "0")}`;
    }, g = () => {
      e.value = new oe({
        timeRanges: r.timeRanges,
        onTimeUpdate: r.onTimeUpdate,
        onSeek: r.onSeek
      });
    }, c = (y, p, b, C, R, k = !1) => {
      y.save(), y.fillStyle = "#666", y.font = "10px Arial";
      const $ = k ? l(p) : a(p);
      y.textAlign = "left", y.fillText($, 8, R - 3);
      const D = k ? l(b) : a(b);
      y.textAlign = "right", y.fillText(D, C - 8, R - 3), y.restore();
    }, u = (y) => {
      if (!e.value) return;
      const p = e.value.getTimeRanges(), b = p.find((C) => C.start > y);
      return b || p[p.length - 1];
    }, v = (y) => {
      if (!e.value) return;
      const p = u(y);
      p && e.value.seek(p.start);
    }, m = () => {
      if (!t.value || !e.value) return;
      const y = t.value, p = y.getContext("2d");
      if (!p) return;
      y.width = y.offsetWidth * window.devicePixelRatio, y.height = r.height * window.devicePixelRatio, p.scale(window.devicePixelRatio, window.devicePixelRatio), p.clearRect(0, 0, y.width, y.height);
      const {
        start: b,
        end: C
      } = e.value.getTimeRangeExtent(), R = C - b;
      if (R <= 0) return;
      const k = y.offsetWidth, $ = y.offsetHeight;
      p.fillStyle = r.backgroundColor, p.fillRect(0, 0, k, $), p.fillStyle = r.gapColor, e.value.getGaps().forEach((B) => {
        const I = (B.start - b) / R * k, U = (B.end - B.start) / R * k, H = 8, Y = Math.ceil(U / H);
        for (let X = 0; X < Y; X++) {
          const J = I + X * H;
          p.fillRect(J, 0, H / 2, $);
        }
      }), e.value.getTimeRanges().forEach((B) => {
        const I = (B.start - b) / R * k, U = (B.end - B.start) / R * k;
        p.fillStyle = r.segmentColor, p.fillRect(I, 0, U, $), p.strokeStyle = r.segmentColor.replace("0.3)", "0.6)"), p.lineWidth = 1, p.strokeRect(I, 0, U, $);
      }), c(p, b, C, k, $);
      const D = (e.value.getCurrentTime() - b) / R * k;
      p.shadowColor = r.cursorColor, p.shadowBlur = 4, p.fillStyle = r.cursorColor, p.fillRect(D - 1, 0, 2, $), p.beginPath(), p.moveTo(D - 2, $ - 4), p.lineTo(D + 2, $ - 4), p.lineTo(D, $), p.closePath(), p.fill(), p.shadowBlur = 0;
    }, f = () => {
      if (!i.value || !e.value) return;
      const y = i.value, p = y.getContext("2d");
      if (!p) return;
      y.width = y.offsetWidth * window.devicePixelRatio, y.height = r.height * window.devicePixelRatio, p.scale(window.devicePixelRatio, window.devicePixelRatio), p.clearRect(0, 0, y.width, y.height);
      const b = e.value.getTotalDuration();
      if (b <= 0) return;
      const C = y.offsetWidth, R = y.offsetHeight;
      p.fillStyle = r.backgroundColor, p.fillRect(0, 0, C, R), e.value.getTimeRanges().forEach(($) => {
        const D = $.mediaStart / b * C, B = $.mediaDuration / b * C;
        p.fillStyle = r.segmentColor, p.fillRect(D, 0, B, R), p.strokeStyle = r.segmentColor.replace("0.3)", "0.6)"), p.lineWidth = 1, p.strokeRect(D, 0, B, R);
      }), c(p, 0, b, C, R, !0);
      const k = e.value.getCurrentMediaTime() / b * C;
      p.shadowColor = r.cursorColor, p.shadowBlur = 4, p.fillStyle = r.cursorColor, p.fillRect(k - 1, 0, 2, R), p.beginPath(), p.moveTo(k - 2, R - 4), p.lineTo(k + 2, R - 4), p.lineTo(k, R), p.closePath(), p.fill(), p.shadowBlur = 0;
    }, h = () => {
      r.showOriginalTimeline && m(), r.showMediaTimeline && f(), o = requestAnimationFrame(h);
    }, x = (y, p) => {
      if (!e.value || !s.value) return;
      const b = p.getBoundingClientRect(), C = y.clientX - b.left;
      s.value.style.left = `${y.clientX}px`, s.value.style.top = `${y.clientY - 25}px`;
      const {
        start: R,
        end: k
      } = e.value.getTimeRangeExtent(), $ = R + C / b.width * (k - R);
      s.value.textContent = a($), s.value.style.display = "block", n.value && (e.value.findTimeRangeForTime($) ? e.value.seek($) : v($));
    }, E = (y) => {
      n.value = !0, document.body.style.cursor = "grabbing";
    }, T = () => {
      n.value = !1, document.body.style.cursor = "default", s.value && (s.value.style.display = "none");
    }, M = () => {
      s.value && (s.value.style.display = "none"), n.value || (document.body.style.cursor = "default");
    }, P = () => {
      document.body.style.cursor = "grab";
    }, G = (y, p) => {
      if (!e.value || !s.value) return;
      const b = p.getBoundingClientRect(), C = y.clientX - b.left;
      s.value.style.left = `${y.clientX}px`, s.value.style.top = `${y.clientY - 25}px`;
      const R = C / b.width * e.value.getTotalDuration();
      s.value.textContent = d(R), s.value.style.display = "block", n.value && e.value.seek(R, !0);
    };
    return z(() => r.timeRanges, (y) => {
      e.value && e.value.setTimeRanges(y);
    }, {
      deep: !0
    }), W(() => {
      g(), h(), window.addEventListener("resize", h);
      const y = document.createElement("div");
      y.className = "timeline-tooltip", document.body.appendChild(y), s.value = y;
    }), V(() => {
      o && cancelAnimationFrame(o), window.removeEventListener("resize", h), s.value && document.body.removeChild(s.value);
    }), () => S("div", {
      class: "timeline-container"
    }, [r.showOriginalTimeline && S("div", {
      class: "timeline-wrapper"
    }, [S("div", {
      class: "timeline-label"
    }, "Original Timeline"), S("div", {
      class: "canvas-wrapper",
      style: {
        height: `${r.height}px`,
        backgroundColor: r.backgroundColor
      }
    }, [S("canvas", {
      ref: t,
      class: "timeline-canvas",
      onMousedown: E,
      onMouseup: T,
      onMousemove: (y) => x(y, t.value),
      onMouseleave: M,
      onMouseenter: P
    })])]), r.showMediaTimeline && S("div", {
      class: "timeline-wrapper"
    }, [S("div", {
      class: "timeline-label"
    }, "Media Timeline"), S("div", {
      class: "canvas-wrapper",
      style: {
        height: `${r.height}px`,
        backgroundColor: r.backgroundColor
      }
    }, [S("canvas", {
      ref: i,
      class: "timeline-canvas",
      onMousedown: E,
      onMouseup: T,
      onMousemove: (y) => G(y, i.value),
      onMouseleave: M,
      onMouseenter: P
    })])])]);
  }
}), F = class F {
  constructor(e, t = {}) {
    w(this, "demuxer");
    w(this, "options");
    w(this, "videoElement");
    w(this, "controlsElement");
    w(this, "progressInput");
    w(this, "progressCanvas");
    w(this, "timeDisplay");
    w(this, "playButton");
    w(this, "rateButtons", []);
    w(this, "updateProgressInterval");
    w(this, "currentTimeRange");
    w(this, "currentMediaTime", 0);
    w(this, "debugContainer");
    w(this, "debugTimelineCanvas");
    w(this, "debugMediaCanvas");
    w(this, "totalPlaylistDuration", 0);
    w(this, "loadingIndicator");
    w(this, "container");
    w(this, "progressInfoLabel");
    var i, n, s;
    this.videoElement = e, this.options = {
      showPlaybackRate: !0,
      showProgress: !0,
      playbackRates: [0.5, 1, 1.5, 2, 3],
      autoGenerateUI: !0,
      timeRangeMode: !1,
      timeRanges: [],
      ...t
    }, this.demuxer = new A(this.videoElement, {
      logLevel: (i = t.debug) != null && i.enabled ? "debug" : "info",
      // Configure sliding window for efficient segment management
      slidingWindow: {
        enabled: !0,
        forward: 2,
        // Preload 2 segments ahead
        backward: 1
        // Keep 1 segment in history
      }
    }), this.setupDemuxerListeners(), this.options.autoGenerateUI && this.createUI(), (n = this.options.debug) != null && n.enabled && this.createDebugUI(), (s = this.options.timeRanges) != null && s.length && this.initializeTimeRanges(this.options.timeRanges), this.videoElement.addEventListener("timeupdate", this.handleTimeUpdate.bind(this)), this.options.showProgress && this.startProgressUpdate();
  }
  /**
   * Setup event listeners for demuxer to handle various events
   */
  setupDemuxerListeners() {
    this.demuxer.on("playlistUpdate", (e) => {
      this.log(`播放列表更新: 片段数=${e.length}`, "info"), this.totalPlaylistDuration = this.demuxer.getTotalDuration(), this.log(`总时长更新: ${this.totalPlaylistDuration}秒`, "info"), this.updateProgress();
    }), this.demuxer.on("bufferUpdate", (e) => {
      var t;
      (t = this.options.debug) != null && t.enabled && this.updateDebugDisplay();
    }), this.demuxer.on("segmentLoaded", (e) => {
      var t;
      this.log(`片段 #${e} 已加载`, "debug"), (t = this.options.debug) != null && t.enabled && this.updateDebugDisplay();
    }), this.demuxer.on("error", (e) => {
      this.log(`播放器错误: ${e.message}`, "error");
    }), this.demuxer.on("debug", (e) => {
      var t;
      (t = this.options.debug) != null && t.enabled && this.log(e, "debug");
    });
  }
  initializeTimeRanges(e) {
    let t = 0;
    e.forEach((i) => {
      i.mediaStart = t, i.mediaDuration = i.end - i.start, t += i.mediaDuration;
    });
  }
  handleTimeUpdate() {
    if (!this.options.timeRangeMode) return;
    this.currentMediaTime = this.videoElement.currentTime;
    const e = this.mediaToOriginalTime(this.currentMediaTime), t = this.findTimeRangeForTime(e);
    t && t !== this.currentTimeRange && this.switchToTimeRange(t);
  }
  /**
   * 查找指定时间所属的时间范围
   */
  findTimeRangeForTime(e) {
    var n;
    if (!((n = this.options.timeRanges) != null && n.length)) return;
    for (const s of this.options.timeRanges) {
      const o = s.end;
      if (e >= s.start && e <= o)
        return s;
    }
    let t, i = Number.MAX_VALUE;
    for (const s of this.options.timeRanges) {
      const o = Math.abs(e - s.start), a = Math.abs(e - s.end), d = Math.min(o, a);
      d < i && (i = d, t = s);
    }
    return t && this.log(`时间 ${e}秒 不在任何时间范围内，使用最近的范围 [${t.start}-${t.end}]`, "debug"), t;
  }
  async switchToTimeRange(e) {
    this.currentTimeRange = e;
    const t = this.currentMediaTime - e.mediaStart;
    await this.load(e.url), this.seek(e.start + t);
  }
  originalToMediaTime(e) {
    var n;
    if (!((n = this.options.timeRanges) != null && n.length)) return e;
    const t = this.findTimeRangeForTime(e);
    if (!t) return e;
    const i = e - t.start;
    return t.mediaStart + i;
  }
  mediaToOriginalTime(e) {
    var t;
    if (!((t = this.options.timeRanges) != null && t.length)) return e;
    for (const i of this.options.timeRanges)
      if (e >= i.mediaStart && e < i.mediaStart + i.mediaDuration)
        return i.start + (e - i.mediaStart);
    return this.options.timeRanges[0].start;
  }
  createUI() {
    var t, i, n;
    const e = document.createElement("div");
    if (e.className = "hls-player", (t = this.videoElement.parentNode) == null || t.insertBefore(e, this.videoElement), e.appendChild(this.videoElement), this.videoElement.className = "hls-player-video", this.container = e, this.controlsElement = document.createElement("div"), this.controlsElement.className = "hls-player-controls", e.appendChild(this.controlsElement), this.playButton = document.createElement("button"), this.playButton.textContent = "播放", this.playButton.onclick = this.togglePlay.bind(this), this.controlsElement.appendChild(this.playButton), this.options.showProgress) {
      const s = document.createElement("div");
      s.className = "hls-player-progress";
      const o = document.createElement("div");
      o.className = "progress-info-label", o.style.cssText = "font-size: 12px; color: #666; margin-bottom: 4px; white-space: nowrap;", o.textContent = "加载中...", s.appendChild(o), this.progressInfoLabel = o;
      const a = document.createElement("div");
      a.className = "progress-wrapper", this.progressCanvas = document.createElement("canvas"), this.progressCanvas.className = "progress-canvas", a.appendChild(this.progressCanvas), this.progressInput = document.createElement("input"), this.progressInput.type = "range", this.progressInput.min = "0", this.progressInput.max = "100", this.progressInput.value = "0", this.progressInput.setAttribute("aria-label", "视频进度条"), this.progressInput.oninput = this.handleSeek.bind(this), a.appendChild(this.progressInput), s.appendChild(a), this.timeDisplay = document.createElement("span");
      const d = `${this.formatTime(0)} / ${this.formatTime(0)}`;
      console.log(`[HLSPlayer] 初始时间显示: ${d}`), this.timeDisplay.textContent = d, s.appendChild(this.timeDisplay), this.controlsElement.appendChild(s), this.options.timeRangeMode && this.updateTimeRangesDisplay();
    }
    if (this.options.showPlaybackRate) {
      const s = document.createElement("div");
      s.className = "hls-player-rate", (i = this.options.playbackRates) == null || i.forEach((o) => {
        const a = document.createElement("button");
        a.textContent = `${o}x`, a.onclick = () => this.setPlaybackRate(o), a.className = o === 1 ? "rate-active" : "", this.rateButtons.push(a), s.appendChild(a);
      }), this.controlsElement.appendChild(s);
    }
    (n = this.options.debug) != null && n.enabled && this.createDebugUI(), this.startProgressUpdate();
  }
  createDebugUI() {
    var n, s;
    this.debugContainer = document.createElement("div"), this.debugContainer.className = "hls-player-debug";
    const e = S(re, {
      timeRanges: this.options.timeRanges,
      showOriginalTimeline: (n = this.options.debug) == null ? void 0 : n.showTimeRanges,
      showMediaTimeline: (s = this.options.debug) == null ? void 0 : s.showMediaTimeline,
      height: 20,
      onTimeUpdate: (o) => {
        this.seek(o);
      },
      onSeek: (o) => {
        this.seek(o);
      }
    }), t = document.createElement("div");
    ee(e, t), this.debugContainer.appendChild(t);
    const i = this.videoElement.closest(".hls-player");
    i == null || i.appendChild(this.debugContainer);
  }
  updateTimeRangesDisplay() {
    if (!this.progressCanvas || !this.options.timeRanges) return;
    const e = this.progressCanvas.getContext("2d");
    if (!e) return;
    const t = this.progressCanvas;
    if (t.width = t.offsetWidth * window.devicePixelRatio, t.height = t.offsetHeight * window.devicePixelRatio, e.clearRect(0, 0, t.width, t.height), !this.options.timeRanges.length) return;
    const i = Math.min(...this.options.timeRanges.map((o) => o.start)), s = Math.max(...this.options.timeRanges.map((o) => o.end)) - i;
    e.fillStyle = "rgba(0, 160, 255, 0.3)", this.options.timeRanges.forEach((o) => {
      const a = (o.start - i) / s * t.width, d = (o.end - o.start) / s * t.width;
      e.fillRect(a, 0, d, t.height);
    });
  }
  calculateSeekTime(e) {
    var o;
    if (!this.options.timeRangeMode || !((o = this.options.timeRanges) != null && o.length))
      return { time: e, url: "" };
    const t = Math.min(...this.options.timeRanges.map((a) => a.start)), i = Math.max(...this.options.timeRanges.map((a) => a.end)), n = t + e / 100 * (i - t), s = this.findTimeRangeForTime(n);
    if (!s) {
      const a = this.options.timeRanges.find((g) => g.start > n), d = [...this.options.timeRanges].reverse().find((g) => g.end < n), l = a || d;
      return l ? {
        time: a ? a.start : d.end,
        url: l.url
      } : { time: n, url: "" };
    }
    return {
      time: n,
      url: s.url
    };
  }
  handleSeek(e) {
    var s;
    const t = e.target, i = Number(t.value), n = this.demuxer.getTotalDuration();
    if (this.options.timeRangeMode) {
      const { time: o, url: a } = this.calculateSeekTime(i);
      a && a !== ((s = this.currentTimeRange) == null ? void 0 : s.url) ? this.load(a).then(() => this.seek(o)) : this.seek(o);
    } else {
      const o = i / 100 * n;
      console.log(`[HLSPlayer] 跳转: value=${i}%, duration=${n}秒, seekTime=${o}秒`), this.seek(o);
    }
  }
  updateProgress() {
    var o, a;
    if (!this.options.showProgress) return;
    const e = this.videoElement.currentTime, t = this.demuxer.getTotalDuration(), i = this.demuxer.getCurrentTime(), n = Date.now();
    if (n - F.lastProgressLogTime >= 5e3 && (console.log(`[HLSPlayer] 更新进度: currentTime=${e.toFixed(2)}秒, totalDuration=${t.toFixed(2)}秒, demuxerTime=${i.toFixed(2)}秒`), F.lastProgressLogTime = n), this.progressInfoLabel) {
      let d = "";
      if (this.videoElement.buffered.length > 0)
        for (let c = 0; c < this.videoElement.buffered.length; c++) {
          const u = this.videoElement.buffered.start(c), v = this.videoElement.buffered.end(c);
          d += `缓冲区 #${c}: ${u.toFixed(1)}-${v.toFixed(1)}秒 `;
        }
      else
        d = "无缓冲区";
      const l = isNaN(t) || t <= 0 || !isFinite(t) ? 100 : t, g = e / l * 100;
      this.progressInfoLabel.textContent = `当前时间: ${e.toFixed(2)}秒 | Demuxer时间: ${i.toFixed(2)}秒 | 总时长: ${t.toFixed(2)}秒 | 进度: ${g.toFixed(1)}% | ${d} | 播放状态: ${this.isPlaying() ? "播放中" : "已暂停"} | ReadyState: ${this.videoElement.readyState}`;
    }
    const s = isNaN(t) || t <= 0 || !isFinite(t) ? 100 : t;
    if (this.options.timeRangeMode && ((o = this.options.timeRanges) != null && o.length)) {
      const d = Math.min(...this.options.timeRanges.map((c) => c.start)), l = Math.max(...this.options.timeRanges.map((c) => c.end)), g = this.findTimeRangeForTime(e);
      if (g && (this.currentTimeRange = g, this.progressInput && (this.progressInput.value = String((e - d) / (l - d) * 100), this.progressInput.max = "100"), this.timeDisplay)) {
        const c = Number(e - d), u = Number(l - d);
        console.log(`[HLSPlayer] 时间范围模式时间: adjustedCurrentTime=${c}秒, adjustedTotalTime=${u}秒`), this.timeDisplay.textContent = `${this.formatTime(c)} / ${this.formatTime(u)}`;
      }
      this.updateProgressDisplay();
    } else {
      if (this.progressInput) {
        const d = e / s * 100;
        this.progressInput.max = "100", this.progressInput.value = String(Math.min(100, Math.max(0, d)));
      }
      if (this.timeDisplay) {
        const d = Number(e), l = Number(s);
        console.log(`[HLSPlayer] 标准模式时间: numCurrentTime=${d}秒, numValidDuration=${l}秒`);
        const g = this.formatTime(d), c = this.formatTime(l), u = `${g} / ${c}`;
        console.log(`[HLSPlayer] 时间显示: ${u}`), this.timeDisplay.textContent = u;
      }
      this.updateProgressDisplay();
    }
    this.playButton && (this.playButton.textContent = this.isPlaying() ? "暂停" : "播放"), (a = this.options.debug) != null && a.enabled && this.updateDebugDisplay();
  }
  formatTime(e) {
    console.log(`[HLSPlayer] formatTime 输入: ${e}秒, 类型: ${typeof e}, isNaN: ${isNaN(e)}, isFinite: ${isFinite(e)}`), (isNaN(e) || !isFinite(e) || e < 0) && (console.log("[HLSPlayer] formatTime 检测到无效时间，重置为0"), e = 0), e = Number(e);
    const t = Math.floor(e / 3600), i = Math.floor(e % 3600 / 60), n = Math.floor(e % 60);
    let s;
    return t > 0 ? s = `${t.toString().padStart(2, "0")}:${i.toString().padStart(2, "0")}:${n.toString().padStart(2, "0")}` : s = `${i.toString().padStart(2, "0")}:${n.toString().padStart(2, "0")}`, console.log(`[HLSPlayer] formatTime 输出: ${s}, 原始秒数: ${e}秒`), s;
  }
  async togglePlay() {
    this.isPlaying() ? this.pause() : await this.play();
  }
  startProgressUpdate() {
    this.updateProgressInterval = window.setInterval(() => {
      var e;
      this.updateProgress(), (e = this.options.debug) != null && e.enabled && this.updateDebugDisplay();
    }, 1e3 / 30);
  }
  stopProgressUpdate() {
    this.updateProgressInterval && clearInterval(this.updateProgressInterval);
  }
  async load(e) {
    var t, i;
    this.log(`加载HLS地址: ${e}`, "info");
    try {
      this.demuxer && (this.demuxer.destroy(), this.demuxer = new A(this.videoElement, {
        logLevel: (t = this.options.debug) != null && t.enabled ? "debug" : "info",
        slidingWindow: {
          enabled: !0,
          forward: 2,
          backward: 1
        }
      }), this.setupDemuxerListeners()), this.loadingIndicator && (this.loadingIndicator.style.display = "block"), await this.demuxer.init(e), this.options.showProgress && this.updateProgressDisplay(), (i = this.options.debug) != null && i.enabled && this.updateDebugDisplay(), this.loadingIndicator && (this.loadingIndicator.style.display = "none");
    } catch (n) {
      throw console.error("加载媒体失败:", n), this.loadingIndicator && (this.loadingIndicator.style.display = "none"), n;
    }
  }
  async play() {
    this.log("开始播放", "info");
    try {
      await this.demuxer.play(), this.startProgressUpdate(), this.playButton && (this.playButton.textContent = "暂停");
    } catch (e) {
      console.error("播放失败:", e);
      try {
        await this.videoElement.play(), this.startProgressUpdate(), this.playButton && (this.playButton.textContent = "暂停");
      } catch (t) {
        console.error("直接播放也失败:", t);
      }
    }
  }
  pause() {
    this.demuxer.pause(), this.stopProgressUpdate(), this.playButton && (this.playButton.textContent = "播放"), this.updateProgress();
  }
  /**
   * 跳转到指定时间
   */
  seek(e) {
    var n;
    if (!this.demuxer) return;
    const t = performance.now();
    if (this.log(`尝试跳转: ${this.videoElement.currentTime.toFixed(2)}s → ${e.toFixed(2)}s`, "info"), this.options.timeRangeMode && ((n = this.options.timeRanges) != null && n.length)) {
      const s = this.findTimeRangeForTime(e);
      if (s && this.currentTimeRange !== s) {
        this.switchToTimeRange(s);
        return;
      }
      const o = this.originalToMediaTime(e);
      this.demuxer.seek(o);
    } else
      this.demuxer.seek(e);
    this.forceUpdateProgress();
    const i = performance.now() - t;
    this.log(`跳转完成，耗时 ${i.toFixed(2)}ms`, "debug");
  }
  /**
   * 获取当前播放时间
   */
  getCurrentTime() {
    var t;
    if (!this.demuxer) return 0;
    const e = this.demuxer.getCurrentTime();
    return this.options.timeRangeMode && ((t = this.options.timeRanges) != null && t.length) ? this.mediaToOriginalTime(e) : e;
  }
  /**
   * 获取总时长
   */
  getDuration() {
    var e;
    return this.demuxer ? this.options.timeRangeMode && ((e = this.options.timeRanges) != null && e.length) ? this.options.timeRanges.reduce((t, i) => t + (i.end - i.start), 0) : this.demuxer.getTotalDuration() : 0;
  }
  /**
   * 设置播放速率
   */
  setPlaybackRate(e) {
    this.demuxer && (this.demuxer.setPlaybackRate(e), this.videoElement.playbackRate = e, this.rateButtons.forEach((t) => {
      t.classList.toggle("active", parseFloat(t.dataset.rate || "1") === e);
    }));
  }
  /**
   * 销毁播放器实例
   */
  destroy() {
    this.stopProgressUpdate(), this.demuxer && this.demuxer.destroy(), this.videoElement.removeEventListener("timeupdate", this.handleTimeUpdate.bind(this)), this.container && this.container.parentNode && this.container.parentNode.removeChild(this.container);
  }
  getPlaybackRate() {
    return this.videoElement.playbackRate;
  }
  getOptions() {
    return this.options;
  }
  setTimeRanges(e) {
    var t;
    if (this.options.timeRanges = e, this.options.timeRangeMode = !0, this.initializeTimeRanges(e), this.options.autoGenerateUI && (this.updateTimeRangesDisplay(), (t = this.options.debug) != null && t.enabled)) {
      if (this.debugContainer) {
        const i = this.videoElement.closest(".hls-player");
        i == null || i.removeChild(this.debugContainer);
      }
      this.createDebugUI();
    }
  }
  updateProgressDisplay() {
    var i;
    if (!this.progressCanvas) return;
    const e = this.progressCanvas.getContext("2d");
    if (!e) return;
    const t = this.progressCanvas;
    if (t.width = t.offsetWidth * window.devicePixelRatio, t.height = t.offsetHeight * window.devicePixelRatio, e.clearRect(0, 0, t.width, t.height), this.options.timeRangeMode && ((i = this.options.timeRanges) != null && i.length)) {
      const n = this.options.timeRanges.reduce(
        (s, o) => s + o.mediaDuration,
        0
      );
      if (e.fillStyle = "rgba(0, 160, 255, 0.3)", this.options.timeRanges.forEach((s) => {
        const o = s.mediaStart / n * t.width, a = s.mediaDuration / n * t.width;
        e.fillRect(o, 0, a, t.height);
      }), this.currentMediaTime > 0) {
        e.fillStyle = "#18a058";
        const s = this.currentMediaTime / n * t.width;
        e.fillRect(s - 1, 0, 2, t.height);
      }
    } else {
      const n = this.videoElement.currentTime, s = this.demuxer.getTotalDuration(), o = isNaN(s) || s <= 0 || !isFinite(s) ? 100 : s;
      F.lastTimelineLogTime || (F.lastTimelineLogTime = 0);
      const a = Date.now();
      if (a - F.lastTimelineLogTime >= 1e4 && (console.log(`[HLSPlayer] 渲染时间轴: currentTime=${n.toFixed(2)}秒, duration=${o.toFixed(2)}秒, videoCurrentTime=${this.videoElement.currentTime.toFixed(2)}秒`), F.lastTimelineLogTime = a), e.fillStyle = "rgba(0, 0, 0, 0.2)", e.fillRect(0, 0, t.width, t.height), this.videoElement.buffered.length > 0) {
        e.fillStyle = "rgba(0, 160, 255, 0.3)";
        for (let d = 0; d < this.videoElement.buffered.length; d++) {
          const l = this.videoElement.buffered.start(d), g = this.videoElement.buffered.end(d), c = l / o * t.width, u = (g - l) / o * t.width;
          e.fillRect(c, 0, u, t.height);
        }
      }
      if (n > 0) {
        e.fillStyle = "rgba(24, 160, 88, 0.5)";
        const d = n / o * t.width;
        e.fillRect(0, 0, d, t.height), e.fillStyle = "#18a058", e.fillRect(d - 1, 0, 2, t.height);
      }
    }
  }
  updateDebugDisplay() {
    var e, t, i;
    (e = this.options.debug) != null && e.enabled && ((t = this.options.debug) != null && t.showTimeRanges && this.updateTimelineDebugCanvas(), (i = this.options.debug) != null && i.showMediaTimeline && this.updateMediaDebugCanvas());
  }
  updateTimelineDebugCanvas() {
    var d;
    if (!this.debugTimelineCanvas || !((d = this.options.timeRanges) != null && d.length)) return;
    const e = this.debugTimelineCanvas, t = e.getContext("2d");
    if (!t) return;
    e.width = e.offsetWidth * window.devicePixelRatio, e.height = e.offsetHeight * window.devicePixelRatio, t.clearRect(0, 0, e.width, e.height);
    const i = Math.min(...this.options.timeRanges.map((l) => l.start)), s = Math.max(...this.options.timeRanges.map((l) => l.end)) - i;
    t.fillStyle = "rgba(0, 0, 0, 0.1)";
    for (let l = 0; l < this.options.timeRanges.length - 1; l++) {
      const g = this.options.timeRanges[l].end, c = this.options.timeRanges[l + 1].start, u = (g - i) / s * e.width, v = (c - g) / s * e.width;
      t.fillRect(u, 0, v, e.height);
    }
    t.fillStyle = "rgba(0, 160, 255, 0.3)", this.options.timeRanges.forEach((l) => {
      const g = (l.start - i) / s * e.width, c = (l.end - l.start) / s * e.width;
      t.fillRect(g, 0, c, e.height);
    });
    const o = this.getCurrentTime();
    t.fillStyle = "#18a058";
    const a = (o - i) / s * e.width;
    t.fillRect(a - 1, 0, 2, e.height);
  }
  updateMediaDebugCanvas() {
    var o;
    if (!this.debugMediaCanvas || !((o = this.options.timeRanges) != null && o.length)) return;
    const e = this.debugMediaCanvas, t = e.getContext("2d");
    if (!t) return;
    e.width = e.offsetWidth * window.devicePixelRatio, e.height = e.offsetHeight * window.devicePixelRatio, t.clearRect(0, 0, e.width, e.height);
    const i = this.getDuration();
    t.fillStyle = "rgba(0, 160, 255, 0.3)", this.options.timeRanges.forEach((a) => {
      const d = a.mediaStart / i * e.width, l = a.mediaDuration / i * e.width;
      t.fillRect(d, 0, l, e.height);
    });
    const n = this.videoElement.currentTime;
    t.fillStyle = "#18a058";
    const s = n / i * e.width;
    t.fillRect(s - 1, 0, 2, e.height);
  }
  // 添加一个方法，强制更新时间轴
  forceUpdateProgress() {
    if (this.updateProgress(), this.videoElement) {
      if (this.videoElement.dispatchEvent(new Event("timeupdate")), this.progressInfoLabel) {
        const e = this.progressInfoLabel.textContent || "";
        this.progressInfoLabel.textContent = `${e} | [强制更新]`;
        const t = this.progressInfoLabel.style.color;
        this.progressInfoLabel.style.color = "#ff5500", setTimeout(() => {
          this.progressInfoLabel && (this.progressInfoLabel.style.color = t);
        }, 1e3);
      }
      if (this.timeDisplay) {
        const e = Number(this.videoElement.currentTime), t = Number(this.demuxer.getTotalDuration()), i = isNaN(t) || t <= 0 || !isFinite(t) ? 100 : t, n = this.formatTime(e), s = this.formatTime(i), o = `${n} / ${s}`;
        console.log(`[HLSPlayer] 强制更新时间显示: ${o}`), this.timeDisplay.textContent = o;
      }
    }
    console.log(`[HLSPlayer] 强制更新进度: currentTime=${this.videoElement.currentTime.toFixed(2)}秒, demuxerTime=${this.demuxer.getCurrentTime().toFixed(2)}秒`);
  }
  /**
   * 记录日志信息
   */
  log(e, t = "info") {
    var n;
    const i = "[HLSPlayer]";
    t === "error" ? console.error(`${i} ${e}`) : t === "info" ? console.log(`${i} ${e}`) : t === "debug" && ((n = this.options.debug) != null && n.enabled) && console.debug(`${i} ${e}`);
  }
  /**
   * 配置滑动窗口参数
   */
  setSlidingWindowConfig(e) {
    this.demuxer && (this.demuxer.setSlidingWindowConfig(e), this.log(`已更新滑动窗口配置: ${JSON.stringify(e)}`, "debug"));
  }
  /**
   * 获取当前滑动窗口配置
   */
  getSlidingWindowConfig() {
    if (this.demuxer)
      return this.demuxer.getSlidingWindowConfig();
  }
  /**
   * 获取视频片段总数
   */
  getTotalSegments() {
    return this.demuxer ? this.demuxer.getTotalSegments() : 0;
  }
  /**
   * 检查播放器是否正在播放
   */
  isPlaying() {
    return !this.videoElement.paused;
  }
};
// 静态属性，用于记录上次日志输出时间
w(F, "lastProgressLogTime", 0), // 静态属性，用于记录上次时间轴渲染日志输出时间
w(F, "lastTimelineLogTime", 0);
let O = F;
const ae = /* @__PURE__ */ N({
  name: "BasicTimeline",
  props: {
    currentTime: {
      type: Number,
      required: !0
    },
    duration: {
      type: Number,
      required: !0
    },
    height: {
      type: Number,
      default: 20
    },
    backgroundColor: {
      type: String,
      default: "rgba(0, 0, 0, 0.1)"
    },
    progressColor: {
      type: String,
      default: "rgba(24, 160, 88, 0.3)"
    },
    cursorColor: {
      type: String,
      default: "#18a058"
    }
  },
  setup(r, {
    emit: e
  }) {
    const t = L(), i = L(!1), n = L(), s = (u) => {
      console.log(`[BasicTimeline] formatTime 输入: ${u}秒, 类型: ${typeof u}, isNaN: ${isNaN(u)}, isFinite: ${isFinite(u)}`), (isNaN(u) || !isFinite(u) || u < 0) && (console.log("[BasicTimeline] formatTime 检测到无效时间，重置为0"), u = 0), u = Number(u);
      const v = Math.floor(u / 3600), m = Math.floor(u % 3600 / 60), f = Math.floor(u % 60), h = `${v.toString().padStart(2, "0")}:${m.toString().padStart(2, "0")}:${f.toString().padStart(2, "0")}`;
      return console.log(`[BasicTimeline] formatTime 输出: ${h}, 原始秒数: ${u}秒`), h;
    }, o = (u) => {
      if (!t.value || !n.value) return;
      const v = t.value.getBoundingClientRect(), f = (u.clientX - v.left) / v.width * r.duration;
      n.value.style.left = `${u.clientX}px`, n.value.style.top = `${u.clientY - 25}px`, n.value.textContent = s(f), n.value.style.display = "block", i.value && e("timeUpdate", f);
    }, a = (u) => {
      i.value = !0, document.body.style.cursor = "grabbing";
      const v = t.value.getBoundingClientRect(), f = (u.clientX - v.left) / v.width * r.duration;
      e("seek", f);
    }, d = () => {
      i.value = !1, document.body.style.cursor = "default", n.value && (n.value.style.display = "none");
    }, l = () => {
      n.value && (n.value.style.display = "none"), i.value || (document.body.style.cursor = "default");
    }, g = () => {
      document.body.style.cursor = "grab";
    };
    W(() => {
      const u = document.createElement("div");
      u.className = "timeline-tooltip", document.body.appendChild(u), n.value = u;
    }), V(() => {
      n.value && document.body.removeChild(n.value);
    });
    const c = _(() => {
      let u = 0;
      return r.duration > 0 && r.currentTime >= 0 && (u = r.currentTime / r.duration * 100, u = Math.min(100, Math.max(0, u))), console.log(`[BasicTimeline] 计算位置: currentTime=${r.currentTime.toFixed(2)}秒, duration=${r.duration.toFixed(2)}秒, position=${u.toFixed(2)}%`), {
        container: {
          position: "relative",
          width: "100%",
          height: `${r.height + 20}px`,
          userSelect: "none"
        },
        labels: {
          position: "absolute",
          top: 0,
          left: 0,
          right: 0,
          height: "16px"
        },
        startLabel: {
          position: "absolute",
          left: "4px",
          top: 0,
          fontSize: "10px",
          fontFamily: "-apple-system, BlinkMacSystemFont, Segoe UI, Roboto, sans-serif",
          color: "#666666"
        },
        endLabel: {
          position: "absolute",
          right: "4px",
          top: 0,
          fontSize: "10px",
          fontFamily: "-apple-system, BlinkMacSystemFont, Segoe UI, Roboto, sans-serif",
          color: "#666666"
        },
        timeline: {
          position: "absolute",
          top: "20px",
          left: 0,
          right: 0,
          height: `${r.height}px`,
          backgroundColor: r.backgroundColor,
          overflow: "hidden"
        },
        progress: {
          position: "absolute",
          left: 0,
          top: 0,
          width: `${u}%`,
          height: "100%",
          backgroundColor: r.progressColor,
          borderRight: `1px solid ${r.progressColor.replace("0.3)", "0.6)")}`
        },
        cursor: {
          position: "absolute",
          left: `${u}%`,
          top: "20px",
          width: "2px",
          height: `${r.height}px`,
          backgroundColor: r.cursorColor,
          transform: "translateX(-50%)",
          boxShadow: "0 0 4px rgba(24, 160, 88, 0.5)"
        },
        cursorTriangle: {
          position: "absolute",
          left: `${u}%`,
          top: `${20 + r.height - 4}px`,
          width: 0,
          height: 0,
          borderLeft: "4px solid transparent",
          borderRight: "4px solid transparent",
          borderTop: `4px solid ${r.cursorColor}`,
          transform: "translateX(-50%)"
        }
      };
    });
    return () => S("div", {
      ref: t,
      class: "timeline-container",
      style: c.value.container,
      onMousedown: a,
      onMouseup: d,
      onMousemove: o,
      onMouseleave: l,
      onMouseenter: g
    }, [
      // Time labels
      S("div", {
        style: c.value.labels
      }, [S("div", {
        style: c.value.startLabel
      }, s(0)), S("div", {
        style: c.value.endLabel
      }, s(r.duration))]),
      // Timeline background with progress
      S("div", {
        style: c.value.timeline
      }, [S("div", {
        style: c.value.progress
      })]),
      // Cursor
      S("div", {
        style: c.value.cursor
      }),
      S("div", {
        style: c.value.cursorTriangle
      })
    ]);
  }
}), le = /* @__PURE__ */ N({
  name: "RangeTimeline",
  props: {
    timeRanges: {
      type: Array,
      required: !0
    },
    currentTime: {
      type: Number,
      required: !0
    },
    height: {
      type: Number,
      default: 20
    },
    backgroundColor: {
      type: String,
      default: "rgba(255, 255, 255, 0.1)"
    },
    segmentColor: {
      type: String,
      default: "rgba(24, 160, 88, 0.6)"
    },
    gapColor: {
      type: String,
      default: "rgba(0, 0, 0, 0.2)"
    },
    cursorColor: {
      type: String,
      default: "#18a058"
    },
    showTimeLabel: {
      type: Boolean,
      default: !0
    }
  },
  setup(r, {
    emit: e
  }) {
    const t = L(), i = L(!1), n = L(), s = (m) => new Date(m * 1e3).toLocaleString("zh-CN", {
      month: "2-digit",
      day: "2-digit",
      hour: "2-digit",
      minute: "2-digit",
      second: "2-digit",
      hour12: !1
    }), o = () => r.timeRanges.length ? {
      start: Math.min(...r.timeRanges.map((m) => m.start)),
      end: Math.max(...r.timeRanges.map((m) => m.end))
    } : {
      start: 0,
      end: 0
    }, a = (m) => r.timeRanges.find((f) => m >= f.start && m <= f.end), d = (m) => {
      if (!t.value || !n.value) return;
      const f = t.value.getBoundingClientRect(), h = m.clientX - f.left, {
        start: x,
        end: E
      } = o(), T = x + h / f.width * (E - x);
      n.value.style.left = `${m.clientX}px`, n.value.style.top = `${m.clientY - 25}px`, n.value.textContent = s(T), n.value.style.display = "block", i.value && a(T) && e("timeUpdate", T);
    }, l = (m) => {
      i.value = !0, document.body.style.cursor = "grabbing";
      const f = t.value.getBoundingClientRect(), h = m.clientX - f.left, {
        start: x,
        end: E
      } = o(), T = x + h / f.width * (E - x);
      a(T) && e("seek", T);
    }, g = () => {
      i.value = !1, document.body.style.cursor = "default", n.value && (n.value.style.display = "none");
    }, c = () => {
      n.value && (n.value.style.display = "none"), i.value || (document.body.style.cursor = "default");
    }, u = () => {
      document.body.style.cursor = "grab";
    };
    W(() => {
      const m = document.createElement("div");
      m.className = "timeline-tooltip", document.body.appendChild(m), n.value = m;
    }), V(() => {
      n.value && document.body.removeChild(n.value);
    });
    const v = _(() => {
      const {
        start: m,
        end: f
      } = o(), h = f - m, x = (r.currentTime - m) / h * 100;
      return {
        container: {
          position: "relative",
          width: "100%",
          height: `${r.height + (r.showTimeLabel ? 20 : 0)}px`,
          userSelect: "none"
        },
        labels: {
          position: "absolute",
          top: 0,
          left: 0,
          right: 0,
          height: "16px",
          display: r.showTimeLabel ? "block" : "none"
        },
        startLabel: {
          position: "absolute",
          left: "4px",
          top: 0,
          fontSize: "10px",
          fontFamily: "-apple-system, BlinkMacSystemFont, Segoe UI, Roboto, sans-serif",
          color: "#666666"
        },
        endLabel: {
          position: "absolute",
          right: "4px",
          top: 0,
          fontSize: "10px",
          fontFamily: "-apple-system, BlinkMacSystemFont, Segoe UI, Roboto, sans-serif",
          color: "#666666"
        },
        timeline: {
          position: "absolute",
          top: r.showTimeLabel ? "20px" : 0,
          left: 0,
          right: 0,
          height: `${r.height}px`,
          backgroundColor: r.backgroundColor
        },
        cursor: {
          position: "absolute",
          left: `${x}%`,
          top: r.showTimeLabel ? "20px" : 0,
          width: "2px",
          height: `${r.height}px`,
          backgroundColor: r.cursorColor,
          transform: "translateX(-50%)",
          boxShadow: "0 0 4px rgba(24, 160, 88, 0.5)"
        },
        cursorTriangle: {
          position: "absolute",
          left: `${x}%`,
          top: `${r.showTimeLabel ? 20 + r.height - 4 : r.height - 4}px`,
          width: 0,
          height: 0,
          borderLeft: "4px solid transparent",
          borderRight: "4px solid transparent",
          borderTop: `4px solid ${r.cursorColor}`,
          transform: "translateX(-50%)"
        }
      };
    });
    return () => {
      const {
        start: m,
        end: f
      } = o(), h = f - m;
      return h <= 0 ? null : S("div", {
        ref: t,
        class: "timeline-container",
        style: v.value.container,
        onMousedown: l,
        onMouseup: g,
        onMousemove: d,
        onMouseleave: c,
        onMouseenter: u
      }, [
        // Time labels
        r.showTimeLabel && S("div", {
          style: v.value.labels
        }, [S("div", {
          style: v.value.startLabel
        }, s(m)), S("div", {
          style: v.value.endLabel
        }, s(f))]),
        // Timeline background
        S("div", {
          style: v.value.timeline
        }, [
          // Segments
          ...r.timeRanges.map((x, E) => {
            const T = (x.start - m) / h * 100, M = (x.end - x.start) / h * 100;
            return S("div", {
              key: `segment-${E}`,
              style: {
                position: "absolute",
                left: `${T}%`,
                top: 0,
                width: `${M}%`,
                height: "100%",
                backgroundColor: r.segmentColor,
                border: `1px solid ${r.segmentColor.replace("0.6)", "0.8)")}`
              }
            });
          }),
          // Gaps
          ...r.timeRanges.slice(0, -1).map((x, E) => {
            const T = {
              start: r.timeRanges[E].end,
              end: r.timeRanges[E + 1].start
            }, M = (T.start - m) / h * 100, P = (T.end - T.start) / h * 100;
            return S("div", {
              key: `gap-${E}`,
              style: {
                position: "absolute",
                left: `${M}%`,
                top: 0,
                width: `${P}%`,
                height: "100%",
                background: `repeating-linear-gradient(
                  45deg,
                  ${r.gapColor},
                  ${r.gapColor} 4px,
                  transparent 4px,
                  transparent 8px
                )`
              }
            });
          })
        ]),
        // Cursor
        S("div", {
          style: v.value.cursor
        }),
        S("div", {
          style: v.value.cursorTriangle
        })
      ]);
    };
  }
}), he = /* @__PURE__ */ N({
  name: "HLSPlayerVue",
  props: {
    src: {
      type: String,
      required: !1,
      default: ""
    },
    options: {
      type: Object,
      default: () => ({})
    },
    timeRanges: {
      type: Array,
      default: () => []
    }
  },
  setup(r) {
    const e = L(), t = L(), i = L(0), n = L(0), s = L(!1), o = L(1), a = L(!1), d = L("idle");
    let l = null;
    const g = () => {
      if (t.value) {
        const f = t.value.getCurrentTime();
        !isNaN(f) && isFinite(f) && f >= 0 ? (i.value = f, console.log(`[HLSPlayerVue] 当前时间更新: ${i.value.toFixed(2)}秒`)) : console.log(`[HLSPlayerVue] 检测到无效的当前时间: ${f}, 保持原值: ${i.value.toFixed(2)}秒`);
        const h = t.value.getDuration();
        !isNaN(h) && isFinite(h) && h > 0 && Math.abs(h - n.value) > 0.5 && (n.value = h, console.log(`[HLSPlayerVue] 总时长更新: ${n.value.toFixed(2)}秒`), a.value = !0), (i.value > 0 || n.value > 0) && console.log(`[HLSPlayerVue] 时间轴数据: currentTime=${i.value.toFixed(2)}秒, duration=${n.value.toFixed(2)}秒, 进度=${(i.value / n.value * 100).toFixed(2)}%`);
      }
    }, c = () => {
      e.value && (e.value.addEventListener("waiting", () => {
        d.value = "loading";
      }), e.value.addEventListener("canplay", () => {
        d.value = "ready";
      }), e.value.addEventListener("playing", () => {
        s.value = !0, d.value = "ready";
      }), e.value.addEventListener("pause", () => {
        s.value = !1;
      }), e.value.addEventListener("progress", () => {
        if (e.value && e.value.buffered.length > 0) {
          const f = e.value.buffered.end(e.value.buffered.length - 1);
          console.log(`Buffered range: 0 - ${f.toFixed(2)}s`);
        }
      }));
    };
    W(() => {
      e.value && r.src && (console.log(`[HLSPlayerVue] 组件挂载，初始化播放器: ${r.src}`), t.value = new O(e.value, {
        ...r.options,
        autoGenerateUI: !1
      }), t.value.load(r.src).then(() => {
        console.log("[HLSPlayerVue] 视频加载完成，初始化时间数据"), g(), l = window.setInterval(() => {
          g();
        }, 250);
      }), r.options.playbackRates && r.options.playbackRates.length > 0 && (o.value = r.options.playbackRates[0]), c());
    }), z(() => r.src, async (f, h) => {
      f && f !== h && t.value && (s.value = !1, d.value = "loading", a.value = !1, await t.value.load(f), setTimeout(g, 500), setTimeout(g, 1e3), setTimeout(g, 2e3));
    }), V(() => {
      l !== null && (clearInterval(l), l = null), t.value && t.value.destroy();
    });
    const u = async () => {
      t.value && (s.value ? (t.value.pause(), s.value = !1) : (d.value = "loading", await t.value.play(), s.value = !0));
    }, v = (f) => {
      var h;
      (h = t.value) == null || h.seek(f);
    }, m = (f) => {
      t.value && (t.value.setPlaybackRate(f), o.value = f);
    };
    return () => {
      var f;
      return S("div", {
        class: "hls-player"
      }, [S("video", {
        ref: e,
        class: "hls-player-video"
      }), S("div", {
        class: "hls-player-controls"
      }, [S("button", {
        onClick: u
      }, s.value ? "暂停" : "播放"), r.timeRanges.length > 0 ? S(le, {
        timeRanges: r.timeRanges,
        currentTime: i.value,
        height: 20,
        onTimeUpdate: v,
        onSeek: v
      }) : S(ae, {
        currentTime: i.value,
        duration: n.value,
        height: 20,
        backgroundColor: "rgba(255, 255, 255, 0.1)",
        progressColor: "rgba(24, 160, 88, 0.6)",
        cursorColor: "#18a058",
        onTimeUpdate: v,
        onSeek: v
      }), r.options.showPlaybackRate && S("div", {
        class: "hls-player-rate"
      }, (f = r.options.playbackRates) == null ? void 0 : f.map((h) => S("button", {
        key: h,
        onClick: () => m(h),
        class: {
          "rate-active": o.value === h
        }
      }, `${h}x`)))])]);
    };
  }
});
export {
  ae as BasicTimeline,
  O as HLSPlayer,
  he as HLSPlayerVue,
  le as RangeTimeline,
  re as Timeline,
  oe as TimelineBase
};
