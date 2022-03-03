// todo: 待定
import {HOT_KEY} from "../constant";

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

    //
    addHotkey(HOT_KEY.esc, () => {
        if (player.fullscreen) {
            player.fullscreen = false;
        }
    })

    //
    addHotkey(HOT_KEY.arrowUp, () => {
        player.volume += 0.05;
    })

    //
    addHotkey(HOT_KEY.arrowDown, () => {
        player.volume -= 0.05;
    })

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
