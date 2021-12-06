import {EVENTS} from "../constant";
import {bpsSize, getStyle, setStyle} from "../utils";
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

    try {
        const screenfullChange = () => {
            setStyle(control.$fullscreenExit, 'display', player.fullscreen ? 'flex' : 'none');
            setStyle(control.$fullscreen, 'display', player.fullscreen ? 'none' : 'flex');
            // control.autoSize();
        };
        screenfull.on('change', screenfullChange);
        player.events.destroys.push(() => {
            screenfull.off('change', screenfullChange);
        });
    } catch (error) {
        //
    }

    player.on(EVENTS.recording, () => {
        setStyle(control.$record, 'display', player.recording ? 'none' : 'flex');
        setStyle(control.$recordStop, 'display', player.recording ? 'flex' : 'none');
    })

    //
    player.on(EVENTS.recordingTimestamp, (timestamp) => {
        // console.log(timestamp);
    })

    player.on(EVENTS.playing, (flag) => {
        setStyle(control.$play, 'display', flag ? 'none' : 'flex');
        setStyle(control.$playBig, 'display', flag ? 'none' : 'block');
        setStyle(control.$pause, 'display', flag ? 'flex' : 'none');
        setStyle(control.$screenshot, 'display', flag ? 'flex' : 'none');
        setStyle(control.$record, 'display', flag ? 'flex' : 'none');
        setStyle(control.$fullscreen, 'display', flag ? 'flex' : 'none');

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
