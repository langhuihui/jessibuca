// todo: 待定
export default (player, control) => {
    const {
        events: {proxy}
    } = player;

    const keys = {};

    function addHotkey(key, event) {
        if (keys[key]) {
            keys[key].push(event);
        } else {
            keys[key] = [event];
        }
    }


    proxy(window, 'keydown', event => {
        if (control.isFocus) {
            const tag = document.activeElement.tagName.toUpperCase();
            const editable = document.activeElement.getAttribute('contenteditable');
            if (tag !== 'INPUT' && tag !== 'TEXTAREA' && editable !== '' && editable !== 'true') {
                const events = keys[event.keyCode];
                if (events) {
                    event.preventDefault();
                    events.forEach(fn => fn());
                }
            }
        }
    });
}
