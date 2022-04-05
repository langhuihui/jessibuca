
var Module = {}


Module.print = function (text) {
    console.log('Jessibuca: [worker]:',text);
}
Module.printErr = function (text) {
    console.warn('Jessibuca: [worker]:',text);
    postMessage({cmd: 'wasmError', message: text})
}
