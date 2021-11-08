import {EVEMTS} from "../constant";

export default (player) => {

//
    player.on(EVEMTS.load, () => {
        player.debug.log('player', 'has loaded');
        player._hasLoaded = true;
    })

    //
    player.on(EVEMTS.play, () => {
        player.loading = false;
    })

    //
    player.on(EVEMTS.fullscreen, (value) => {
        if (value) {
            try {
                screenfull.request(player.$container).then(() => {
                    if (player.$borderSelect) {
                        player.$borderSelect.style.display = 'none';
                    }
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

    player.on(EVEMTS.webFullscreen, (value) => {
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
    player.on(EVEMTS.resize, () => {
        player.video.resize();
    })


    if (player._opt.debug) {
        Object.keys(EVEMTS).forEach((key) => {
            player.on(EVEMTS[key], (value) => {
                player.debug.log('player events', EVEMTS[key], value);
            })
        })
    }
}
