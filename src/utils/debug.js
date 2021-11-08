export default class Debug {
    constructor(master) {
        this.log = (name, ...args) => {
            if (master._opt.debug) {
                console.log(`Jessibuca: [${name}]`, ...args);
            }
        };

        this.warn = (name, ...args) => {
            if (master._opt.debug) {
                console.warn(`Jessibuca: [${name}]`, ...args);
            }
        };

        this.error = (name, ...args) => {
            if (master._opt.debug) {
                console.error(`Jessibuca: [${name}]`, ...args);
            }
        };
    }
}
