export default class Debug {
    constructor(master) {
        this.log = (name, ...args) => {
            if (master._opt && master._opt.debug) {
                console.log(`Jb: [${name}]`, ...args);
            }
        };

        this.warn = (name, ...args) => {
            if (master._opt && master._opt.debug) {
                console.warn(`Jb: [${name}]`, ...args);
            }
        };

        this.error = (name, ...args) => {
            console.error(`Jb: [${name}]`, ...args);
        };
    }
}
