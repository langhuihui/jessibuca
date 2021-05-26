import {audioContextUnlock} from "../utils";

export default (jessibuca) => {
    jessibuca._audioContext = new (window.AudioContext || window.webkitAudioContext)();
    jessibuca._gainNode = this._audioContext.createGain();


    jessibuca._audioEnabled = (flag) => {
        if (flag) {
            audioContextUnlock(jessibuca._audioContext)
            jessibuca._audioEnabled = (flag) => {
                if (flag) {
                    // 恢复
                    jessibuca._audioContext.resume();

                } else {
                    // 暂停
                    jessibuca._audioContext.suspend();
                }
            }
        } else {
            jessibuca._audioContext.suspend();
        }
    }

    jessibuca._audioEnabled(true);


    if (!jessibuca._opt.isNotMute) {
        jessibuca._audioEnabled(false);
    }


}