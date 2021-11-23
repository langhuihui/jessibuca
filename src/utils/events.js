export default class Events {
    constructor(master) {
        this.destroys = [];
        this.proxy = this.proxy.bind(this);
        this.master = master;
    }

    proxy(target, name, callback, option = {}) {
        if (!target) {
            return;
        }
        if (Array.isArray(name)) {
            return name.map(item => this.proxy(target, item, callback, option));
        }
        target.addEventListener(name, callback, option);
        const destroy = () => target.removeEventListener(name, callback, option);
        this.destroys.push(destroy);
        return destroy;
    }

    destroy() {
        this.master.debug.log(`Events`, 'destroy');
        this.destroys.forEach(event => event());
    }
}
