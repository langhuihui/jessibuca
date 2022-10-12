import {CONTROL_HEIGHT, EVENTS} from "../constant";
import {bpsSize, formatTimeTips, getStyle, isBoolean, isMobile, setStyle} from "../utils";
import screenfull from "screenfull";

export default (player, control) => {
    const {
        events: {proxy},
    } = player;

    const object = document.createElement('object');
    object.setAttribute('aria-hidden', 'true');
    object.setAttribute('tabindex', -1);
    object.type = 'text/html';
    object.data = 'about:blank';

    setStyle(object, {
        display: 'block',
        position: 'absolute',
        top: '0',
        left: '0',
        height: '100%',
        width: '100%',
        overflow: 'hidden',
        pointerEvents: 'none',
        zIndex: '-1',
    });

    let playerWidth = player.width;
    let playerHeight = player.height;

    proxy(object, 'load', () => {
        proxy(object.contentDocument.defaultView, 'resize', () => {
            if (player.width !== playerWidth || player.height !== playerHeight) {
                playerWidth = player.width;
                playerHeight = player.height;
                player.emit(EVENTS.resize);
                screenfullH5Control();
            }
        });
    });
    player.$container.appendChild(object);

    player.on(EVENTS.destroy, () => {
        player.$container.removeChild(object);
    })

    function setVolumeHandle(percentage) {
        if (percentage === 0) {
            setStyle(control.$volumeOn, 'display', 'none');
            setStyle(control.$volumeOff, 'display', 'flex');
            setStyle(control.$volumeHandle, 'top', `${48}px`);
        } else {
            if (control.$volumeHandle && control.$volumePanel) {
                const panelHeight = getStyle(control.$volumePanel, 'height') || 60;
                const handleHeight = getStyle(control.$volumeHandle, 'height');
                const top = panelHeight - (panelHeight - handleHeight) * percentage - handleHeight;
                setStyle(control.$volumeHandle, 'top', `${top}px`);
                setStyle(control.$volumeOn, 'display', 'flex');
                setStyle(control.$volumeOff, 'display', 'none');
            }
        }
        control.$volumePanelText && (control.$volumePanelText.innerHTML = parseInt(percentage * 100))
    }

    player.on(EVENTS.volumechange, () => {
        setVolumeHandle(player.volume);
    });

    player.on(EVENTS.loading, (flag) => {
        setStyle(control.$loading, 'display', flag ? 'flex' : 'none');
        setStyle(control.$poster, 'display', 'none')

        if (flag) {
            setStyle(control.$playBig, 'display', 'none');
        }
    })

    const screenfullChange = (fullscreen) => {
        let isFullScreen = isBoolean(fullscreen) ? fullscreen : player.fullscreen
        setStyle(control.$fullscreenExit, 'display', isFullScreen ? 'flex' : 'none');
        setStyle(control.$fullscreen, 'display', isFullScreen ? 'none' : 'flex');
        // control.autoSize();
    };
    const screenfullH5Control = () => {
        if (isMobile() && control.$controls) {
            setTimeout(() => {
                if (player.fullscreen) {
                    // console.log(player.width, player.height);
                    let translateX = player.height / 2 - player.width + CONTROL_HEIGHT / 2;
                    let translateY = player.height / 2 - CONTROL_HEIGHT / 2;

                    control.$controls.style.transform = `translateX(${-translateX}px) translateY(-${translateY}px) rotate(-90deg)`
                } else {
                    control.$controls.style.transform = `translateX(0) translateY(0) rotate(0)`
                }

            }, 10)
        }
    }

    try {

        screenfull.on('change', screenfullChange);
        player.events.destroys.push(() => {
            screenfull.off('change', screenfullChange);
        });
    } catch (error) {
        //
    }

    //
    player.on(EVENTS.webFullscreen, (value) => {
        screenfullChange(value);
        screenfullH5Control();

    })


    player.on(EVENTS.recording, () => {
        setStyle(control.$record, 'display', player.recording ? 'none' : 'flex');
        setStyle(control.$recordStop, 'display', player.recording ? 'flex' : 'none');
        setStyle(control.$recording, 'display', player.recording ? 'flex' : 'none')
    })

    //
    player.on(EVENTS.recordingTimestamp, (timestamp) => {
        // console.log(timestamp);
        control.$recordingTime && (control.$recordingTime.innerHTML = formatTimeTips(timestamp))
    })

    player.on(EVENTS.playing, (flag) => {
        setStyle(control.$play, 'display', flag ? 'none' : 'flex');
        setStyle(control.$playBig, 'display', flag ? 'none' : 'block');
        setStyle(control.$pause, 'display', flag ? 'flex' : 'none');
        setStyle(control.$screenshot, 'display', flag ? 'flex' : 'none');
        setStyle(control.$record, 'display', flag ? 'flex' : 'none');
        // setStyle(control.$fullscreen, 'display', flag ? 'flex' : 'none');
        screenfullChange();
        // 不在播放
        if (!flag) {
            control.$speed && (control.$speed.innerHTML = bpsSize(''));
        }
    })


    player.on(EVENTS.kBps, (rate) => {
        const bps = bpsSize(rate);
        control.$speed && (control.$speed.innerHTML = bps);
    })
}
