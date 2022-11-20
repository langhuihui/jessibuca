export class WebCodecsVideoRenderer {
  mediaStream: MediaStream;
  videoWriter: WritableStreamDefaultWriter<VideoFrame>;
  audioWriter: WritableStreamDefaultWriter<AudioData>;
  constructor(display: HTMLVideoElement) {
    const videoTrack = new MediaStreamTrackGenerator({ kind: 'video' });
    this.videoWriter = videoTrack.writable.getWriter();
    const audioTrack = new MediaStreamTrackGenerator({ kind: 'audio' });
    this.audioWriter = audioTrack.writable.getWriter();
    this.mediaStream = display.srcObject = new MediaStream([audioTrack, videoTrack]);
    display.play();
  }
  writeVideo(frame: VideoFrame) {
    return this.videoWriter.write(frame);
  }
  writeAudio(data: AudioData) {
    return this.audioWriter.write(data);
  }
  close() {
    this.mediaStream.getTracks().forEach(t => t.stop());
  }
}
export class MSEVideoRenderer {
  constructor(display: HTMLVideoElement) {

  }
  writeVideo(frame: Uint8Array, init: VideoFrameBufferInit) {

  }
  writeAudio(data: AudioData) {
  }
  close() {

  }
}