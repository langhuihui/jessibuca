export default class Events {
    constructor(master, domain) {
        this.destroys = [];
        this.domain = domain;
        this.proxy = this.proxy.bind(this);
        this.master = master;
    }

    proxy(target, name, callback, option = {}) {
        if (Array.isArray(name)) {
            return name.map(item => this.proxy(target, item, callback, option));
        }
        target.addEventListener(name, callback, option);
        const destroy = () => target.removeEventListener(name, callback, option);
        this.destroys.push(destroy);
        return destroy;
    }

    destroy() {
        this.master.debug.log(`${this.domain} Events`, 'destroy');
        this.destroys.forEach(event => event());
    }
}
