export default (player, control) => {

    Object.defineProperty(control, 'controls', {
        get: () => {
            return player.$player.classList.contains('jessibuca-controls-show');
        },
        set: (type) => {
            if (type) {
                player.$player.classList.add('jessibuca-controls-show');
            } else {
                player.$player.classList.remove('jessibuca-controls-show');
            }
        },
    });


    Object.defineProperty(control, 'controlsRect', {
        get: () => {
            return control.$controls.getBoundingClientRect();
        },
    });


}
