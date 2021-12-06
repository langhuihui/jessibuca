import icons from './icons';

export default (player, control) => {
    player.$container.classList.add('jessibuca-controls-show');
    const options = player._opt;
    const operateBtns = options.operateBtns;


    player.$container.insertAdjacentHTML(
        'beforeend',
        `
            ${options.background ? `<div class="jessibuca-poster" style="background-image: url(${options.background})"></div>` : ''}
            <div class="jessibuca-loading">
                ${icons.loading}
                ${options.loadingText ? `<div class="jessibuca-loading-text">${options.loadingText}</div>` : ''}
            </div>
            ${options.hasControl && operateBtns.play ? `<div class="jessibuca-play-big"></div>` : ''}
            ${options.hasControl ? `
                <div class="jessibuca-controls">
                    <div class="jessibuca-controls-bottom">
                        <div class="jessibuca-controls-left">
                            ${options.showBandwidth ? `<div class="jessibuca-controls-item jessibuca-speed"></div>` : ''}
                        </div>
                        <div class="jessibuca-controls-right">
                             ${operateBtns.audio ? `
                                 <div class="jessibuca-controls-item jessibuca-volume">
                                     ${icons.audio}
                                     ${icons.mute}
                                     <div class="jessibuca-volume-panel-wrap">
                                          <div class="jessibuca-volume-panel">
                                                 <div class="jessibuca-volume-panel-handle"></div>
                                          </div>
                                          <div class="jessibuca-volume-panel-text"></div>
                                     </div>
                                 </div>
                             ` : ''}
                             ${operateBtns.play ? `<div class="jessibuca-controls-item jessibuca-play">${icons.play}</div><div class="jessibuca-controls-item jessibuca-pause">${icons.pause}</div>` : ''}
                             ${operateBtns.screenshot ? `<div class="jessibuca-controls-item jessibuca-screenshot">${icons.screenshot}</div>` : ''}
                             ${operateBtns.record ? ` <div class="jessibuca-controls-item jessibuca-record">${icons.record}</div><div class="jessibuca-controls-item jessibuca-record-stop">${icons.recordStop}</div>` : ''}
                             ${operateBtns.fullscreen ? `<div class="jessibuca-controls-item jessibuca-fullscreen">${icons.fullscreen}</div><div class="jessibuca-controls-item jessibuca-fullscreen-exit">${icons.fullscreenExit}</div>` : ''}
                        </div>
                    </div>
                </div>
            ` : ''}

        `
    )

    Object.defineProperty(control, '$poster', {
        value: player.$container.querySelector('.jessibuca-poster'),
    });

    Object.defineProperty(control, '$loading', {
        value: player.$container.querySelector('.jessibuca-loading'),
    });

    Object.defineProperty(control, '$play', {
        value: player.$container.querySelector('.jessibuca-play'),
    });

    Object.defineProperty(control, '$playBig', {
        value: player.$container.querySelector('.jessibuca-play-big'),
    });

    Object.defineProperty(control, '$pause', {
        value: player.$container.querySelector('.jessibuca-pause'),
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


    Object.defineProperty(control, '$volume', {
        value: player.$container.querySelector('.jessibuca-volume'),
    });

    Object.defineProperty(control, '$volumePanelWrap', {
        value: player.$container.querySelector('.jessibuca-volume-panel-wrap'),
    });

    Object.defineProperty(control, '$volumePanelText', {
        value: player.$container.querySelector('.jessibuca-volume-panel-text'),
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

    Object.defineProperty(control, '$speed', {
        value: player.$container.querySelector('.jessibuca-speed'),
    });
}
