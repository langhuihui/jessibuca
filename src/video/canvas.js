import createWebGL from "../utils/webgl";

export default (jessibuca) => {
    const $canvasElement = document.createElement("canvas");

    $canvasElement.style.position = "absolute";
    $canvasElement.style.top = 0;
    $canvasElement.style.left = 0;
    jessibuca.$container.appendChild($canvasElement);
    jessibuca.$canvasElement = $canvasElement;
    jessibuca.$container.style.overflow = 'hidden';

    if (jessibuca.$container.style.position !== 'absolute') {
        jessibuca.$container.style.position = 'relative';
    }

    const _initContextGL = () => {
        const canvas = jessibuca.$canvasElement;
        let gl = null;

        const validContextNames = ["webgl", "experimental-webgl", "moz-webgl", "webkit-3d"];
        let nameIndex = 0;

        while (!gl && nameIndex < validContextNames.length) {
            const contextName = validContextNames[nameIndex];

            try {
                let contextOptions = {preserveDrawingBuffer: true};
                if (jessibuca._opt.contextOptions) {
                    contextOptions = Object.assign(contextOptions, jessibuca._opt.contextOptions);
                }

                gl = canvas.getContext(contextName, contextOptions);
            } catch (e) {
                gl = null;
            }

            if (!gl || typeof gl.getParameter !== "function") {
                gl = null;
            }

            ++nameIndex;
        }


        return gl;
    }

    //
    if (!jessibuca._supportOffscreen()) {
        const _contextGL = _initContextGL();
        jessibuca._contextGLRender = createWebGL(_contextGL);
        jessibuca._contextGL = _contextGL;
    }

    jessibuca._destroyContextGL = () => {
        if (jessibuca._contextGL) {
            jessibuca._contextGL = null;
        }
        if (jessibuca._contextGLRender) {
            jessibuca._contextGLRender = null;
        }
        if (jessibuca._bitmaprenderer) {
            jessibuca._bitmaprenderer = null;
        }
    }
}