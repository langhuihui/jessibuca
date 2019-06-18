cc.Class({
    extends: cc.Component,
    editor: CC_EDITOR && {
        executeInEditMode: true,
        requireComponent: cc.Sprite,
    },
    properties: {
        url: "ws://localhost:8080/live/user1",
        videoBuffer: 1,
        autoPlay: false,
        material: {
            visible: false,
            get() {
                return this._material;
            }
        },
    },

    // LIFE-CYCLE CALLBACKS:
    onLoad() {
    },

    start() {
        this.h5lc = new Module.H5LiveClient(this);
        this.h5lc.videoBuffer = this.videoBuffer;
        if (this.autoPlay) this.h5lc.play(this.url);
    },
    play(url) {
        this.h5lc.play(this.url = url);
    },
    close() {
        this.h5lc.close();
    },
    onDestroy() {
        this.h5lc.close()
    }
});
