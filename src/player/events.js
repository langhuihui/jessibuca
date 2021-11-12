import {EVENTS} from "../constant";
import screenfull from "screenfull";

export default (player) => {

//
    player.on(EVENTS.load, () => {
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
                    if (player.$borderSelect) {
                        player.$borderSelect.style.display = 'block';
                    }
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
        Object.keys(EVENTS).forEach((key) => {
            player.on(EVENTS[key], (value) => {
                player.debug.log('player events', EVENTS[key], value);
            })
        })
    }
}
