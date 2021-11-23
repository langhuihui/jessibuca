const iconsMap = {
    play: '播放',
    pause: '暂停',
    audio: '',
    mute: '',
    screenshot: '截图',
    loading: '加载',
    fullscreen: '全屏',
    fullscreenExit: '退出全屏',
    record: '录制',
    recordStop: '停止录制',
}

export default Object.keys(iconsMap).reduce((icons, key) => {
    icons[key] = `
    <i class="jessibuca-icon jessibuca-icon-${key}"></i>
    ${iconsMap[key] ? `<span class="icon-title-tips"><span class="icon-title">${iconsMap[key]}</span></span>` : ''}
`;
    return icons;
}, {});
