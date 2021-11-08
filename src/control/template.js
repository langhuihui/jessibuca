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
}
