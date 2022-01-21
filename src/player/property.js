export default (player) => {
    Object.defineProperty(player, 'rect', {
        get: () => {
            const clientRect = player.$container.getBoundingClientRect()
            clientRect.width = Math.max(clientRect.width, player.$container.clientWidth);
            clientRect.height = Math.max(clientRect.height, player.$container.clientHeight);
            return clientRect;
        },
    });

    ['bottom', 'height', 'left', 'right', 'top', 'width'].forEach(key => {
        Object.defineProperty(player, key, {
            get: () => {
                return player.rect[key];
            },
        });
    });
}
