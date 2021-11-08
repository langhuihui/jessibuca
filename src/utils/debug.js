export default class Debug {
    constructor(master, domain) {
        this.log = (name, ...args) => {
            if (master._opt.debug) {
                console.log(`Jessibuca[${domain}]: [${name}]`, ...args);
            }
        };

        this.warn = (name, ...args) => {
            if (master._opt.debug) {
                console.warn(`Jessibuca[${domain}]: [${name}]`, ...args);
            }
        };

        this.error = (name, ...args) => {
            if (master._opt.debug) {
                console.error(`Jessibuca[${domain}]: [${name}]`, ...args);
            }
        };
    }
}
