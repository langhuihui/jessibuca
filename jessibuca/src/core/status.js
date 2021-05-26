export default (jessibuca) => {

    jessibuca._loading = true;
    jessibuca.loading = true;
    jessibuca._recording = false;
    jessibuca.recording = false;
    jessibuca._playing = false;
    jessibuca.playing = false;
    jessibuca._audioPlaying = false;
    jessibuca._quieting = jessibuca._opt.isNotMute ? false : true;
    jessibuca.quieting = jessibuca._opt.isNotMute ? false : true;
    jessibuca._fullscreen = false;
    jessibuca.fullscreen = false;
}