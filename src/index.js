if (typeof importScripts === "undefined") {
    window.Jessibuca = require("./jessibuca")
} else {
    require('./worker')
}