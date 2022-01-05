import {EVENTS, EVENTS_ERROR, JESSIBUCA_EVENTS} from "../constant";
import screenfull from "screenfull";
import {setStyle} from "../utils";

export default (player) => {


    try {
        const screenfullChange = () => {
            player.emit(JESSIBUCA_EVENTS.fullscreen, player.fullscreen)

            // 如果不是fullscreen,则触发下 resize 方法
            if (!player.fullscreen) {
                player.resize();
            } else {
                if(player._opt.useMSE){
                    player.resize();
                }
            }

        };
        screenfull.on('change', screenfullChange);
        player.events.destroys.push(() => {
            screenfull.off('change', screenfullChange);
        });
    } catch (error) {
        //
    }
    //
    player.on(EVENTS.decoderWorkerInit, () => {
        player.debug.log('player', 'has loaded');
        player._hasLoaded = true;
    })

    //
    player.on(EVENTS.play, () => {
        player.loading = false;
    })

    //
    player.on(EVENTS.fullscreen, (value) => {
        if (value) {
            try {
                screenfull.request(player.$container).then(() => {

                }).catch((e) => {
                    player.webFullscreen = true;
                });

            } catch (e) {
                player.webFullscreen = true;
            }
        } else {
            try {
                screenfull.exit().then(() => {
                }).catch(() => {
                    player.webFullscreen = false;

                });
            } catch (e) {
                player.webFullscreen = false;
            }
        }
    })

    player.on(EVENTS.webFullscreen, (value) => {
        if (value) {
            player.$container.classList.add('webmediaplayer-fullscreen-web')
            const {clientHeight: bodyHeight, clientWidth: bodyWidth} = document.body;
            const {clientHeight: playerHeight, clientWidth: playerWidth} = player.video.$videoElement;
            const bodyRatio = bodyWidth / bodyHeight;
            const playerRatio = playerWidth / playerHeight;
            const needSpin = bodyRatio < playerRatio;
            if (needSpin) {
                const scale = Math.min(bodyHeight / playerWidth, bodyWidth / playerHeight);
                player.video.$videoElement.style.transform = `rotate(90deg) scale(${scale},${scale})`;
            }

        } else {
            player.$container.classList.remove('webmediaplayer-fullscreen-web')
            player.video.$videoElement.style.transform = null;
        }
    })

    //
    player.on(EVENTS.resize, () => {
        player.video.resize();
    })


    if (player._opt.debug) {
        const ignoreList = [EVENTS.timeUpdate];
        Object.keys(EVENTS).forEach((key) => {
            player.on(EVENTS[key], (value) => {
                if (ignoreList.includes(key)) {
                    return;
                }

                player.debug.log('player events', EVENTS[key], value);
            })
        })

        Object.keys(EVENTS_ERROR).forEach((key) => {
            player.on(EVENTS_ERROR[key], (value) => {
                player.debug.log('player event error', EVENTS_ERROR[key], value);
            })
        })
    }
}
