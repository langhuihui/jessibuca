import Emitter from "../utils/emitter";

export default class AudioLoader extends Emitter {

    constructor(player) {
        super();
        this.player = player;
    }


    destroy(){

    }
}
