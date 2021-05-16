
class JessibucaProcessor extends AudioWorkletProcessor {
    constructor(options) {
        super()
        var buffers = this._audioPlayBuffers = []
        this.port.onmessage = (e) => {
            let count = e.data / 128
            for (let i = 0; i < count; i++) {
                buffers.push(e.data.slice(i * 128, i * 128 + 128))
            }
        }
    }
    process(inputs, outputs, parameters) {
        if (this._audioPlayBuffers.length) {
            var buffer = this._audioPlayBuffers.shift()
            outputs[0].forEach((channel, j) => {
                for (var i = 0; i < 128; i++) {
                    channel[i] = buffer[j][i]
                }
            })
        }
        return true
    }
}

registerProcessor('jessibuca-processor', JessibucaProcessor)