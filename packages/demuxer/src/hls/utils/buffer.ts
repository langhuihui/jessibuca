interface BufferInfo {
  start: number;
  end: number;
  index?: number;
  buffers: [number, number][];
  nextStart?: number;
  nextEnd?: number;
  prevStart?: number;
  prevEnd?: number;
  currentTime?: number;
  behind?: number;
  remaining: number;
  length?: number;
}

interface BufferLike {
  buffered?: TimeRanges;
}

export default class Buffer {
  /**
   * Get the start time of a TimeRanges object
   */
  public static start(buf: TimeRanges | null | undefined): number {
    if (!buf || !buf.length) return 0;

    // Safari bug: https://bit.ly/2trx6O8
    if (buf.length === 1 && buf.end(0) - buf.start(0) < 1e-6) return 0;
    // Edge bug: https://bit.ly/2JYLPeB
    if (buf.length === 1 && buf.start(0) < 0) return 0;

    return buf.start(0);
  }

  /**
   * Get the end time of a TimeRanges object
   */
  public static end(buf: TimeRanges | null | undefined): number {
    if (!buf || !buf.length) return 0;

    // Safari bug: https://bit.ly/2trx6O8
    if (buf.length === 1 && buf.end(0) - buf.start(0) < 1e-6) return 0;

    return buf.end(buf.length - 1);
  }

  /**
   * Get the buffered property from a buffer-like object
   */
  public static get(b: BufferLike | null | undefined): TimeRanges | undefined {
    if (!b) return undefined;
    try {
      return b.buffered;
    } catch (error) {
      // ignore
      return undefined;
    }
  }

  /**
   * Get an array of buffer ranges from a TimeRanges object
   */
  public static buffers(buf: TimeRanges | null | undefined, maxHole?: number): [number, number][] {
    if (!buf || !buf.length) return [];

    const buffers: [number, number][] = [];
    for (let i = 0, l = buf.length; i < l; i++) {
      const bufLen = buffers.length;
      if (!bufLen || !maxHole) {
        buffers.push([buf.start(i), buf.end(i)]);
      } else {
        const last = buffers[bufLen - 1];
        const lastEnd = last[1];
        const start = buf.start(i);
        if (start - lastEnd <= maxHole) {
          const end = buf.end(i);
          if (end > lastEnd) {
            last[1] = end;
          }
        } else {
          buffers.push([buf.start(i), buf.end(i)]);
        }
      }
    }
    return buffers;
  }

  /**
   * Calculate the total length of buffer ranges
   */
  public static totalLength(buffers: [number, number][] | null | undefined): number {
    if (!buffers || !buffers.length) return 0;
    return buffers.reduce((a, c) => (a += (c[1] - c[0])), 0);
  }

  /**
   * Get detailed information about buffer ranges at a specific position
   */
  public static info(
    buf: TimeRanges | null | undefined,
    pos: number = 0,
    maxHole: number = 0
  ): BufferInfo {
    if (!buf || !buf.length) return { start: 0, end: 0, buffers: [], remaining: 0 };

    let start = 0;
    let end = 0;
    let index = 0;
    let nextStart = 0;
    let nextEnd = 0;
    let prevStart = 0;
    let prevEnd = 0;
    const buffers = Buffer.buffers(buf, maxHole);

    for (let i = 0, l = buffers.length; i < l; i++) {
      const item = buffers[i];
      if (pos + maxHole >= item[0] && pos < item[1]) {
        start = item[0];
        end = item[1];
        index = i;
      } else if (pos + maxHole < item[0]) {
        nextStart = item[0];
        nextEnd = item[1];
        break;
      } else if (pos + maxHole > item[1]) {
        prevStart = item[0];
        prevEnd = item[1];
      }
    }

    return {
      start,
      end,
      index,
      buffers,
      nextStart,
      nextEnd,
      prevStart,
      prevEnd,
      currentTime: pos,
      behind: pos - start,
      remaining: end ? end - pos : 0,
      length: Buffer.totalLength(buffers)
    };
  }
} 