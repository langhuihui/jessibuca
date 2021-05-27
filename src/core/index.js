import initVariable from "./variable";
import initMethods from "./methods";
import initEventBus from './event';
import initCore from './core';

export default (jessibuca) => {
    //
    initVariable(jessibuca);
    //
    initMethods(jessibuca);
    //
    initEventBus(jessibuca);
    //
    initCore(jessibuca);
}