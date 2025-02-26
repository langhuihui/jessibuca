import Buffer from './buffer';

interface MediaElement extends HTMLMediaElement {
  seeking: boolean;
  currentTime: number;
}

export default class GapService {
  private _prevCurrentTime: number;

  constructor() {
    this._prevCurrentTime = 0;
  }

  public do(
    media: MediaElement | null,
    maxJumpDistance: number = 3,
    isLive: boolean,
    seekThreshold: number = 1
  ): void {
    if (!media) return;

    const currentTime = media.currentTime;
    let jumpTo = 0;

    if (this._prevCurrentTime === currentTime) {
      const info = Buffer.info(Buffer.get(media), currentTime);
      if (!info.buffers.length) return;

      if (
        (isLive && info.nextStart) ||
        (info.nextStart && info.nextStart - currentTime < maxJumpDistance)
      ) {
        jumpTo = info.nextStart + 0.1;
      } else if (info.end && info.end - currentTime > seekThreshold && !media.seeking) {
        jumpTo = currentTime + 0.1;
      }
    }

    this._prevCurrentTime = currentTime;

    if (jumpTo && currentTime !== jumpTo) {
      media.currentTime = jumpTo;
    }
  }
} 