//手势事件
(function(){
    if(!isAndroid()) return;

    var istouch = false;
    var start = [];
    var gesturestart = new CustomEvent('gesturestart');
    var gesturechange = new CustomEvent('gesturechange');
    var gestureend = new CustomEvent('gestureend');

    document.addEventListener("touchstart", function(e) {
        if(e.touches.length >= 2) { //判断是否有两个点在屏幕上
            istouch = true;
            start = e.touches; //得到第一组两个点
            e.target.dispatchEvent(gesturestart);
        };
    }, false);

    document.addEventListener("touchmove", function(e) {
        e.preventDefault();
        if(e.touches.length >= 2 && istouch) {
            var now = e.touches; //得到第二组两个点
            var scale = getDistance(now[0], now[1]) / getDistance(start[0], start[1]); //得到缩放比例
            var rotation = getAngle(now[0], now[1]) - getAngle(start[0], start[1]); //得到旋转角度差
            gesturechange.scale = scale.toFixed(2);
            gesturechange.rotation = rotation.toFixed(2);
            e.target.dispatchEvent(gesturechange);
        };
    }, false);

    document.addEventListener("touchend", function(e) {
        if(istouch) {
            istouch = false;
            e.target.dispatchEvent(gestureend);
        };
    }, false);

    function getDistance(p1, p2) {
            var x = p2.pageX - p1.pageX,
                y = p2.pageY - p1.pageY;
            return Math.sqrt((x * x) + (y * y));
    };
    function getAngle(p1, p2) {
        var x = p1.pageX - p2.pageX,
            y = p1.pageY - p2.pageY;
        return Math.atan2(y, x) * 180 / Math.PI;
    };

    function isAndroid(p1, p2) {
        var u = navigator.userAgent;
        return u.indexOf('Android') > -1 || u.indexOf('Adr') > -1; //android终端    
   };
})()


window.onload=function(){
    var box = document.querySelector("body");
    var p1 = document.querySelector("#p1");
    var p2 = document.querySelector("#p2");
        
    //事件绑定
    box.addEventListener('gesturestart', gesture, false);
    box.addEventListener('gesturechange', gesture, false);
    box.addEventListener('gestureend', gesture, false);
    box.addEventListener('touchstart', gesture, false);
    

    //
    function gesture(event) {
        switch(event.type) {
            case "touchstart":
                p1.innerHTML= "touch";
                p2.innerHTML= "touch";
                break;
            case "gesturestart":
                p1.innerHTML = "rotation=" + event.rotation ;
                p2.innerHTML = "scale=" + event.scale ;
                break;
            case "gesturechange":
                p1.innerHTML = "rotation=" + event.rotation ;
                p2.innerHTML = "scale=" + event.scale ;
                break;
            case "gestureend":
                p1.innerHTML = "gestureend";
                p2.innerHTML ="gestureend";
                break;
        }
    }
}