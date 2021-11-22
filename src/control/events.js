export default (player, control) => {
    const {
        events: {proxy}
    } = player;

    function volumeChangeFromEvent(event) {
        const {bottom: panelBottom, height: panelHeight} = control.$volumePanel.getBoundingClientRect();
        const {height: handleHeight} = control.$volumeHandle.getBoundingClientRect();
        const percentage =
            clamp(panelBottom - event.y - handleHeight / 2, 0, panelHeight - handleHeight / 2) / (panelHeight - handleHeight);
        return percentage;
    }

    //
    proxy(window, ['click', 'contextmenu'], event => {
        if (event.composedPath().indexOf(player.$container) > -1) {
            control.isFocus = true;
        } else {
            control.isFocus = false;
        }
    });

    //
    proxy(window, 'orientationchange', () => {
        setTimeout(() => {
            player.resize();
        }, 300);
    });


    proxy(control.$controls, 'click', (e) => {
        e.stopPropagation();
    })

    control.$ptzArrows.forEach(($ptzArrow) => {
        proxy($ptzArrow, 'click', (e) => {
            e.stopPropagation();
            const target = e.currentTarget;
            const dataset = target.dataset;
            const arrow = dataset.arrow;
            control.$ptzBgActive.classList.add('webmediaplayer-ptz-bg-active-show');
            control.$ptzBgActive.classList.add(`webmediaplayer-ptz-bg-active-${arrow}`);
            control.$ptzControlCircular.classList.add(`webmediaplayer-ptz-control-${arrow}`);
            player.ptzControl.ptzCmd(arrow);
            setTimeout(() => {
                control.$ptzBgActive.classList.remove('webmediaplayer-ptz-bg-active-show');
                PTZ_ARROW.forEach((arrow) => {
                    control.$ptzBgActive.classList.remove(`webmediaplayer-ptz-bg-active-${arrow}`);
                    control.$ptzControlCircular.classList.remove(`webmediaplayer-ptz-control-${arrow}`);
                })
                // player.ptzControl.ptzCmd(PTZ_OBJ.stop);
            }, 300);
        })
    })


    proxy(control.$qualityMenu, 'mouseover', () => {
        control.$qualityMenuList.classList.add('webmediaplayer-quality-menu-shown');
    })

    proxy(control.$qualityMenu, 'mouseout', () => {
        control.$qualityMenuList.classList.remove('webmediaplayer-quality-menu-shown');
    })

    proxy(control.$volume, 'mouseover', () => {
        control.$volumePanelWrap.classList.add('webmediaplayer-volume-panel-wrap-show');
    })

    proxy(control.$volume, 'mouseout', () => {
        control.$volumePanelWrap.classList.remove('webmediaplayer-volume-panel-wrap-show');
    })

    control.$qualityMenuItems.forEach(($qualityMenuItem) => {
        proxy($qualityMenuItem, 'click', (e) => {
            e.stopPropagation();
            control.$qualityMenuItems.forEach(($item) => {
                $item.classList.remove('webmediaplayer-quality-menu-item-active');
            })
            const target = e.currentTarget;
            const dataset = target.dataset;
            player.streamId = '' + dataset.quality;
        })
    })

    proxy(control.$volumeOn, 'click', (e) => {
        e.stopPropagation();
        setStyle(control.$volumeOn, 'display', 'none');
        setStyle(control.$volumeOff, 'display', 'block');
        player.lastVolume = player.volume;
        player.volume = 0;
    })

    proxy(control.$volumeOff, 'click', (e) => {
        e.stopPropagation();
        setStyle(control.$volumeOn, 'display', 'block');
        setStyle(control.$volumeOff, 'display', 'none');
        player.volume = player.lastVolume || 0.5;
    })

    proxy(control.$screenshot, 'click', (e) => {
        e.stopPropagation();
        player.video.screenshot();
    })

    proxy(control.$volumePanel, 'click', event => {
        event.stopPropagation();
        player.volume = volumeChangeFromEvent(event);
    });

    proxy(control.$volumeHandle, 'mousedown', () => {
        control.isVolumeDroging = true;
    });

    proxy(control.$volumeHandle, 'mousemove', event => {
        if (control.isVolumeDroging) {
            player.volume = volumeChangeFromEvent(event);
        }
    });

    proxy(document, 'mouseup', () => {
        if (control.isVolumeDroging) {
            control.isVolumeDroging = false;
        }
    });

    proxy(control.$record, 'click', (e) => {
        e.stopPropagation();
        player.recording = true;
    })

    proxy(control.$recordStop, 'click', (e) => {
        e.stopPropagation();
        player.recording = false;
    })

    proxy(control.$ptz, 'click', (e) => {
        e.stopPropagation();
        setStyle(control.$ptzActive, 'display', 'flex');
        setStyle(control.$ptz, 'display', 'none');
        control.$ptzControl.classList.add('webmediaplayer-ptz-controls-show');
    })

    proxy(control.$ptzActive, 'click', (e) => {
        e.stopPropagation();
        setStyle(control.$ptz, 'display', 'flex');
        setStyle(control.$ptzActive, 'display', 'none');
        control.$ptzControl.classList.remove('webmediaplayer-ptz-controls-show');
    })

    proxy(control.$microphoneOpen, 'click', (e) => {
        e.stopPropagation();
        setStyle(control.$microphoneClose, 'display', 'flex');
        setStyle(control.$microphoneOpen, 'display', 'none');
    })

    proxy(control.$microphoneClose, 'click', (e) => {
        e.stopPropagation();
        setStyle(control.$microphoneOpen, 'display', 'flex');
        setStyle(control.$microphoneClose, 'display', 'none');
    })

    proxy(control.$fullscreen, 'click', (e) => {
        e.stopPropagation();
        player.fullscreen = true;
    })

    proxy(control.$fullscreenExit, 'click', (e) => {
        e.stopPropagation();
        player.fullscreen = false;
    })
}
