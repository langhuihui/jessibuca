import { AFSM } from "afsm";
import { EventEmitter } from 'eventemitter3';
class Jessibuca extends EventEmitter {
  playAFSM = new AFSM({ quickStart: false, name: "play" });
  pauseAFSM = new AFSM({ parent: this.playAFSM, name: "pause" });
  mediaStream: MediaStream = new MediaStream();
  constructor(public options: Options) {
    super();
    // const track = new MediaStreamTrackGenerator({ kind: 'video' });
    // const vwriter_ = track.writable.getWriter();
    if (options.view)
      options.view.srcObject = this.mediaStream;
  }
  async play(url: string) {
    this.playAFSM.stop();
    const { ready } = this.playAFSM;
    this.playAFSM.start(url);
    return ready;
  }
  async pause() {
    const { ready } = this.pauseAFSM;
    this.pauseAFSM.start();
    return ready;
  }
  async resume() {
    const { closed } = this.pauseAFSM;
    this.pauseAFSM.stop();
    return closed;
  }
  async stop() {
    const { closed } = this.playAFSM;
    this.playAFSM.stop();
    return closed;
  }
}
export default Jessibuca;