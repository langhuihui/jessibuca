export default (player)=>{
    Object.defineProperty(player, 'rect', {
        get: () => {
            return player.$container.getBoundingClientRect();
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
