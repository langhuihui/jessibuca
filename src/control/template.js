import icons from './icons';

export default (player, control) => {
    player.$container.classList.add('jessibuca-controls-show');
    const options = player._opt;


    player.$container.insertAdjacentHTML(
        'beforeend',
        `
            ${options.poster ? `<div class="jessibuca-poster" style="background-image: url(${options.poster})"></div>` : ''}
            <div class="jessibuca-loading">${icons.loading}</div>
            <div class="jessibuca-controls">
                <div class="jessibuca-controls-bottom">
                    <div class="jessibuca-controls-left">
                        <div class="jessibuca-controls-item"></div>
                    </div>
                    <div class="jessibuca-controls-right">
                         <div class="jessibuca-controls-item jessibuca-volume">
                         ${icons.audio}
                         ${icons.mute}
                         <div class="jessibuca-volume-panel-wrap">
                              <div class="jessibuca-volume-panel">
                                     <div class="jessibuca-volume-panel-handle"></div>
                              </div>
                         </div>
                         </div>
                         <div class="jessibuca-controls-item jessibuca-screenshot">${icons.screenshot}</div>
                         <div class="jessibuca-controls-item jessibuca-record">${icons.record}</div>
                         <div class="jessibuca-controls-item jessibuca-record-stop">${icons.recordStop}</div>
                         <div class="jessibuca-controls-item jessibuca-fullscreen">${icons.fullscreen}</div>
                         <div class="jessibuca-controls-item jessibuca-fullscreen-exit">${icons.fullscreenExit}</div>
                    </div>
                </div>
            </div>
        `
    )

    Object.defineProperty(control, '$poster', {
        value: player.$container.querySelector('.jessibuca-poster'),
    });

    Object.defineProperty(control, '$loading', {
        value: player.$container.querySelector('.jessibuca-loading'),
    });

    Object.defineProperty(control, '$controls', {
        value: player.$container.querySelector('.jessibuca-controls'),
    });

    Object.defineProperty(control, '$fullscreen', {
        value: player.$container.querySelector('.jessibuca-fullscreen'),
    });

    Object.defineProperty(control, '$fullscreen', {
        value: player.$container.querySelector('.jessibuca-fullscreen'),
    });

    Object.defineProperty(control, '$qualityText', {
        value: player.$container.querySelector('.jessibuca-icon-text'),
    });

    Object.defineProperty(control, '$qualityMenu', {
        value: player.$container.querySelector('.jessibuca-quality-menu'),
    });

    Object.defineProperty(control, '$qualityMenuList', {
        value: player.$container.querySelector('.jessibuca-quality-menu-list'),
    });

    Object.defineProperty(control, '$qualityMenuItems', {
        value: player.$container.querySelectorAll('.jessibuca-quality-menu-item'),
    });


    Object.defineProperty(control, '$volume', {
        value: player.$container.querySelector('.jessibuca-volume'),
    });

    Object.defineProperty(control, '$volumePanelWrap', {
        value: player.$container.querySelector('.jessibuca-volume-panel-wrap'),
    });

    Object.defineProperty(control, '$volumePanel', {
        value: player.$container.querySelector('.jessibuca-volume-panel'),
    });

    Object.defineProperty(control, '$volumeHandle', {
        value: player.$container.querySelector('.jessibuca-volume-panel-handle'),
    });

    Object.defineProperty(control, '$volumeOn', {
        value: player.$container.querySelector('.jessibuca-icon-audio'),
    });

    Object.defineProperty(control, '$volumeOff', {
        value: player.$container.querySelector('.jessibuca-icon-mute'),
    });

    Object.defineProperty(control, '$ptz', {
        value: player.$container.querySelector('.jessibuca-ptz'),
    });

    Object.defineProperty(control, '$ptzActive', {
        value: player.$container.querySelector('.jessibuca-ptz-active'),
    });

    Object.defineProperty(control, '$ptzControl', {
        value: player.$container.querySelector('.jessibuca-ptz-controls'),
    });

    Object.defineProperty(control, '$ptzBgActive', {
        value: player.$container.querySelector('.jessibuca-ptz-bg-active'),
    });

    Object.defineProperty(control, '$ptzControlCircular', {
        value: player.$container.querySelector('.jessibuca-ptz-control'),
    });

    Object.defineProperty(control, '$ptzArrows', {
        value: player.$container.querySelectorAll('.jessibuca-ptz-arrow'),
    });

    Object.defineProperty(control, '$microphoneOpen', {
        value: player.$container.querySelector('.jessibuca-microphone-open'),
    });

    Object.defineProperty(control, '$microphoneClose', {
        value: player.$container.querySelector('.jessibuca-microphone-close'),
    });

    Object.defineProperty(control, '$fullscreen', {
        value: player.$container.querySelector('.jessibuca-fullscreen'),
    });

    Object.defineProperty(control, '$fullscreenExit', {
        value: player.$container.querySelector('.jessibuca-fullscreen-exit'),
    });

    Object.defineProperty(control, '$record', {
        value: player.$container.querySelector('.jessibuca-record'),
    });
    Object.defineProperty(control, '$recordStop', {
        value: player.$container.querySelector('.jessibuca-record-stop'),
    });

    Object.defineProperty(control, '$screenshot', {
        value: player.$container.querySelector('.jessibuca-screenshot'),
    });



}
