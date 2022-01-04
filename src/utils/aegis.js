let callbacks = [];
const aegisCDN = 'https://cdn-go.cn/aegis/aegis-sdk/latest/aegis.min.js'

function loadedAegis() {
    return window.Aegis;
}


export function dynamicLoadAegis(callback) {
    const existingScript = document.getElementById(aegisCDN)
    const cb = callback || function () {
    }
    if (!existingScript) {
        try {
            const script = document.createElement('script')
            script.src = aegisCDN // src url for the third-party library being loaded.
            script.id = aegisCDN
            document.body.appendChild(script)
            callbacks.push(cb)
            const onEnd = 'onload' in script ? stdOnEnd : ieOnEnd
            onEnd(script)
        }
        catch (e){
            console.error(e);
        }

    }

    if (existingScript && cb) {
        if (loadedAegis()) {
            cb(null, existingScript)
        } else {
            callbacks.push(cb)
        }
    }

    function stdOnEnd(script) {
        script.onload = function () {
            // this.onload = null here is necessary
            // because even IE9 works not like others
            this.onerror = this.onload = null
            for (const cb of callbacks) {
                cb(null, script)
            }
            callbacks = null
        }
        script.onerror = function () {
            this.onerror = this.onload = null
            cb(new Error('Failed to load ' + aegisCDN), script)
        }
    }

    function ieOnEnd(script) {
        script.onreadystatechange = function () {
            if (this.readyState !== 'complete' && this.readyState !== 'loaded') return
            this.onreadystatechange = null
            for (const cb of callbacks) {
                cb(null, script) // there is no way to catch loading errors in IE8
            }
            callbacks = null
        }
    }


}
