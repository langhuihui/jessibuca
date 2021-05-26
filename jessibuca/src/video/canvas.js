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
}