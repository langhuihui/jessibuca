import Emitter from "../utils/emitter";

export default class FetchLoader extends Emitter {
    constructor(player) {
        super();
        this.player = player;
    }

}
