export default (player, control) => {

    Object.defineProperty(control, 'controlsRect', {
        get: () => {
            return control.$controls.getBoundingClientRect();
        },
    });

}
