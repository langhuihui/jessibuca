export default (jessibuca) => {

    jessibuca._on = (events, callback) => {
        let cache, event, list;
        if (!callback) return jessibuca;
        cache = jessibuca.__events || (jessibuca.__events = {});
        events = events.split(/\s+/);
        while (event = events.shift()) {
            list = cache[event] || (cache[event] = []);
            list.push(callback);
        }
        return jessibuca;
    }

    jessibuca._off = () => {
        let cache;
        if (!(cache = jessibuca.__events)) return jessibuca;
        delete jessibuca.__events;
        return jessibuca;
    }

    jessibuca._trigger = (events, ...others) => {
        // Execute callbacks
        function _callEach(list, args) {
            if (list) {
                for (let i = 0, len = list.length; i < len; i += 1) {
                    list[i](...args);
                }
            }
        }

        let cache, event, list;
        if (!(cache = jessibuca.__events)) return jessibuca;
        events = events.split(/\s+/);
        // For each event, walk through the list of callbacks twice, first to
        // trigger the event, then to trigger any `"all"` callbacks.
        while (event = events.shift()) {
            if (list = cache[event]) list = list.slice();
            // Execute event callbacks.
            _callEach(list, others);
        }
        return jessibuca;
    }
}
