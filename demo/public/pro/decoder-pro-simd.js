(function (global, factory) {
	typeof exports === 'object' && typeof module !== 'undefined' ? factory(require('path'), require('fs'), require('crypto')) :
	typeof define === 'function' && define.amd ? define(['path', 'fs', 'crypto'], factory) :
	(global = typeof globalThis !== 'undefined' ? globalThis : global || self, factory(global.path, global.fs, global.crypto$1));
})(this, (function (path, fs, crypto$1) { 'use strict';

	function _interopDefaultLegacy (e) { return e && typeof e === 'object' && 'default' in e ? e : { 'default': e }; }

	var path__default = /*#__PURE__*/_interopDefaultLegacy(path);
	var fs__default = /*#__PURE__*/_interopDefaultLegacy(fs);
	var crypto__default = /*#__PURE__*/_interopDefaultLegacy(crypto$1);

	function createCommonjsModule(fn, module) {
		return module = { exports: {} }, fn(module, module.exports), module.exports;
	}

	var decoderProSimd = createCommonjsModule(function (module) {
	  var Module = typeof Module != "undefined" ? Module : {};
	  var Module = typeof Module != "undefined" ? Module : {};

	  Module["locateFile"] = function (url) {
	    if (url == "decoder-pro-simd.wasm" && typeof JESSIBUCA_PRO_SIMD_WASM_URL != "undefined" && JESSIBUCA_PRO_SIMD_WASM_URL != "") return JESSIBUCA_PRO_SIMD_WASM_URL;else return url;
	  };

	  var moduleOverrides = Object.assign({}, Module);
	  var thisProgram = "./this.program";

	  var ENVIRONMENT_IS_WEB = typeof window == "object";
	  var ENVIRONMENT_IS_WORKER = typeof importScripts == "function";
	  var ENVIRONMENT_IS_NODE = typeof process == "object" && typeof process.versions == "object" && typeof process.versions.node == "string";
	  var scriptDirectory = "";

	  function locateFile(path) {
	    if (Module["locateFile"]) {
	      return Module["locateFile"](path, scriptDirectory);
	    }

	    return scriptDirectory + path;
	  }

	  var read_, readAsync, readBinary;

	  var fs;
	  var nodePath;
	  var requireNodeFS;

	  if (ENVIRONMENT_IS_NODE) {
	    if (ENVIRONMENT_IS_WORKER) {
	      scriptDirectory = path__default["default"].dirname(scriptDirectory) + "/";
	    } else {
	      scriptDirectory = __dirname + "/";
	    }

	    requireNodeFS = () => {
	      if (!nodePath) {
	        fs = fs__default["default"];
	        nodePath = path__default["default"];
	      }
	    };

	    read_ = function shell_read(filename, binary) {
	      requireNodeFS();
	      filename = nodePath["normalize"](filename);
	      return fs.readFileSync(filename, binary ? undefined : "utf8");
	    };

	    readBinary = filename => {
	      var ret = read_(filename, true);

	      if (!ret.buffer) {
	        ret = new Uint8Array(ret);
	      }

	      return ret;
	    };

	    readAsync = (filename, onload, onerror) => {
	      requireNodeFS();
	      filename = nodePath["normalize"](filename);
	      fs.readFile(filename, function (err, data) {
	        if (err) onerror(err);else onload(data.buffer);
	      });
	    };

	    if (process["argv"].length > 1) {
	      thisProgram = process["argv"][1].replace(/\\/g, "/");
	    }

	    process["argv"].slice(2);

	    {
	      module["exports"] = Module;
	    }

	    process["on"]("uncaughtException", function (ex) {
	      if (!(ex instanceof ExitStatus)) {
	        throw ex;
	      }
	    });
	    process["on"]("unhandledRejection", function (reason) {
	      throw reason;
	    });

	    Module["inspect"] = function () {
	      return "[Emscripten Module object]";
	    };
	  } else if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
	    if (ENVIRONMENT_IS_WORKER) {
	      scriptDirectory = self.location.href;
	    } else if (typeof document != "undefined" && document.currentScript) {
	      scriptDirectory = document.currentScript.src;
	    }

	    if (scriptDirectory.indexOf("blob:") !== 0) {
	      scriptDirectory = scriptDirectory.substr(0, scriptDirectory.replace(/[?#].*/, "").lastIndexOf("/") + 1);
	    } else {
	      scriptDirectory = "";
	    }

	    {
	      read_ = url => {
	        var xhr = new XMLHttpRequest();
	        xhr.open("GET", url, false);
	        xhr.send(null);
	        return xhr.responseText;
	      };

	      if (ENVIRONMENT_IS_WORKER) {
	        readBinary = url => {
	          var xhr = new XMLHttpRequest();
	          xhr.open("GET", url, false);
	          xhr.responseType = "arraybuffer";
	          xhr.send(null);
	          return new Uint8Array(xhr.response);
	        };
	      }

	      readAsync = (url, onload, onerror) => {
	        var xhr = new XMLHttpRequest();
	        xhr.open("GET", url, true);
	        xhr.responseType = "arraybuffer";

	        xhr.onload = () => {
	          if (xhr.status == 200 || xhr.status == 0 && xhr.response) {
	            onload(xhr.response);
	            return;
	          }

	          onerror();
	        };

	        xhr.onerror = onerror;
	        xhr.send(null);
	      };
	    }
	  } else ;

	  var out = Module["print"] || console.log.bind(console);
	  var err = Module["printErr"] || console.warn.bind(console);
	  Object.assign(Module, moduleOverrides);
	  moduleOverrides = null;
	  if (Module["arguments"]) Module["arguments"];
	  if (Module["thisProgram"]) thisProgram = Module["thisProgram"];
	  if (Module["quit"]) Module["quit"];
	  var POINTER_SIZE = 4;

	  var wasmBinary;
	  if (Module["wasmBinary"]) wasmBinary = Module["wasmBinary"];
	  Module["noExitRuntime"] || true;

	  if (typeof WebAssembly != "object") {
	    abort("no native wasm support detected");
	  }

	  var wasmMemory;
	  var ABORT = false;

	  function assert(condition, text) {
	    if (!condition) {
	      abort(text);
	    }
	  }

	  var UTF8Decoder = typeof TextDecoder != "undefined" ? new TextDecoder("utf8") : undefined;

	  function UTF8ArrayToString(heapOrArray, idx, maxBytesToRead) {
	    var endIdx = idx + maxBytesToRead;
	    var endPtr = idx;

	    while (heapOrArray[endPtr] && !(endPtr >= endIdx)) ++endPtr;

	    if (endPtr - idx > 16 && heapOrArray.buffer && UTF8Decoder) {
	      return UTF8Decoder.decode(heapOrArray.subarray(idx, endPtr));
	    }

	    var str = "";

	    while (idx < endPtr) {
	      var u0 = heapOrArray[idx++];

	      if (!(u0 & 128)) {
	        str += String.fromCharCode(u0);
	        continue;
	      }

	      var u1 = heapOrArray[idx++] & 63;

	      if ((u0 & 224) == 192) {
	        str += String.fromCharCode((u0 & 31) << 6 | u1);
	        continue;
	      }

	      var u2 = heapOrArray[idx++] & 63;

	      if ((u0 & 240) == 224) {
	        u0 = (u0 & 15) << 12 | u1 << 6 | u2;
	      } else {
	        u0 = (u0 & 7) << 18 | u1 << 12 | u2 << 6 | heapOrArray[idx++] & 63;
	      }

	      if (u0 < 65536) {
	        str += String.fromCharCode(u0);
	      } else {
	        var ch = u0 - 65536;
	        str += String.fromCharCode(55296 | ch >> 10, 56320 | ch & 1023);
	      }
	    }

	    return str;
	  }

	  function UTF8ToString(ptr, maxBytesToRead) {
	    return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead) : "";
	  }

	  function stringToUTF8Array(str, heap, outIdx, maxBytesToWrite) {
	    if (!(maxBytesToWrite > 0)) return 0;
	    var startIdx = outIdx;
	    var endIdx = outIdx + maxBytesToWrite - 1;

	    for (var i = 0; i < str.length; ++i) {
	      var u = str.charCodeAt(i);

	      if (u >= 55296 && u <= 57343) {
	        var u1 = str.charCodeAt(++i);
	        u = 65536 + ((u & 1023) << 10) | u1 & 1023;
	      }

	      if (u <= 127) {
	        if (outIdx >= endIdx) break;
	        heap[outIdx++] = u;
	      } else if (u <= 2047) {
	        if (outIdx + 1 >= endIdx) break;
	        heap[outIdx++] = 192 | u >> 6;
	        heap[outIdx++] = 128 | u & 63;
	      } else if (u <= 65535) {
	        if (outIdx + 2 >= endIdx) break;
	        heap[outIdx++] = 224 | u >> 12;
	        heap[outIdx++] = 128 | u >> 6 & 63;
	        heap[outIdx++] = 128 | u & 63;
	      } else {
	        if (outIdx + 3 >= endIdx) break;
	        heap[outIdx++] = 240 | u >> 18;
	        heap[outIdx++] = 128 | u >> 12 & 63;
	        heap[outIdx++] = 128 | u >> 6 & 63;
	        heap[outIdx++] = 128 | u & 63;
	      }
	    }

	    heap[outIdx] = 0;
	    return outIdx - startIdx;
	  }

	  function stringToUTF8(str, outPtr, maxBytesToWrite) {
	    return stringToUTF8Array(str, HEAPU8, outPtr, maxBytesToWrite);
	  }

	  function lengthBytesUTF8(str) {
	    var len = 0;

	    for (var i = 0; i < str.length; ++i) {
	      var c = str.charCodeAt(i);

	      if (c <= 127) {
	        len++;
	      } else if (c <= 2047) {
	        len += 2;
	      } else if (c >= 55296 && c <= 57343) {
	        len += 4;
	        ++i;
	      } else {
	        len += 3;
	      }
	    }

	    return len;
	  }

	  var buffer, HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32, HEAPF32, HEAPF64;

	  function updateGlobalBufferAndViews(buf) {
	    buffer = buf;
	    Module["HEAP8"] = HEAP8 = new Int8Array(buf);
	    Module["HEAP16"] = HEAP16 = new Int16Array(buf);
	    Module["HEAP32"] = HEAP32 = new Int32Array(buf);
	    Module["HEAPU8"] = HEAPU8 = new Uint8Array(buf);
	    Module["HEAPU16"] = HEAPU16 = new Uint16Array(buf);
	    Module["HEAPU32"] = HEAPU32 = new Uint32Array(buf);
	    Module["HEAPF32"] = HEAPF32 = new Float32Array(buf);
	    Module["HEAPF64"] = HEAPF64 = new Float64Array(buf);
	  }

	  Module["INITIAL_MEMORY"] || 268435456;
	  var wasmTable;
	  var __ATPRERUN__ = [];
	  var __ATINIT__ = [];
	  var __ATPOSTRUN__ = [];

	  function preRun() {
	    if (Module["preRun"]) {
	      if (typeof Module["preRun"] == "function") Module["preRun"] = [Module["preRun"]];

	      while (Module["preRun"].length) {
	        addOnPreRun(Module["preRun"].shift());
	      }
	    }

	    callRuntimeCallbacks(__ATPRERUN__);
	  }

	  function initRuntime() {
	    if (!Module["noFSInit"] && !FS.init.initialized) FS.init();
	    FS.ignorePermissions = false;
	    callRuntimeCallbacks(__ATINIT__);
	  }

	  function postRun() {
	    if (Module["postRun"]) {
	      if (typeof Module["postRun"] == "function") Module["postRun"] = [Module["postRun"]];

	      while (Module["postRun"].length) {
	        addOnPostRun(Module["postRun"].shift());
	      }
	    }

	    callRuntimeCallbacks(__ATPOSTRUN__);
	  }

	  function addOnPreRun(cb) {
	    __ATPRERUN__.unshift(cb);
	  }

	  function addOnInit(cb) {
	    __ATINIT__.unshift(cb);
	  }

	  function addOnPostRun(cb) {
	    __ATPOSTRUN__.unshift(cb);
	  }

	  var runDependencies = 0;
	  var dependenciesFulfilled = null;

	  function getUniqueRunDependency(id) {
	    return id;
	  }

	  function addRunDependency(id) {
	    runDependencies++;

	    if (Module["monitorRunDependencies"]) {
	      Module["monitorRunDependencies"](runDependencies);
	    }
	  }

	  function removeRunDependency(id) {
	    runDependencies--;

	    if (Module["monitorRunDependencies"]) {
	      Module["monitorRunDependencies"](runDependencies);
	    }

	    if (runDependencies == 0) {

	      if (dependenciesFulfilled) {
	        var callback = dependenciesFulfilled;
	        dependenciesFulfilled = null;
	        callback();
	      }
	    }
	  }

	  function abort(what) {
	    {
	      if (Module["onAbort"]) {
	        Module["onAbort"](what);
	      }
	    }
	    what = "Aborted(" + what + ")";
	    err(what);
	    ABORT = true;
	    what += ". Build with -sASSERTIONS for more info.";
	    var e = new WebAssembly.RuntimeError(what);
	    throw e;
	  }

	  var dataURIPrefix = "data:application/octet-stream;base64,";

	  function isDataURI(filename) {
	    return filename.startsWith(dataURIPrefix);
	  }

	  function isFileURI(filename) {
	    return filename.startsWith("file://");
	  }

	  var wasmBinaryFile;
	  wasmBinaryFile = "decoder-pro-simd.wasm";

	  if (!isDataURI(wasmBinaryFile)) {
	    wasmBinaryFile = locateFile(wasmBinaryFile);
	  }

	  function getBinary(file) {
	    try {
	      if (file == wasmBinaryFile && wasmBinary) {
	        return new Uint8Array(wasmBinary);
	      }

	      if (readBinary) {
	        return readBinary(file);
	      }

	      throw "both async and sync fetching of the wasm failed";
	    } catch (err) {
	      abort(err);
	    }
	  }

	  function getBinaryPromise() {
	    if (!wasmBinary && (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER)) {
	      if (typeof fetch == "function" && !isFileURI(wasmBinaryFile)) {
	        return fetch(wasmBinaryFile, {
	          credentials: "same-origin"
	        }).then(function (response) {
	          if (!response["ok"]) {
	            throw "failed to load wasm binary file at '" + wasmBinaryFile + "'";
	          }

	          return response["arrayBuffer"]();
	        }).catch(function () {
	          return getBinary(wasmBinaryFile);
	        });
	      } else {
	        if (readAsync) {
	          return new Promise(function (resolve, reject) {
	            readAsync(wasmBinaryFile, function (response) {
	              resolve(new Uint8Array(response));
	            }, reject);
	          });
	        }
	      }
	    }

	    return Promise.resolve().then(function () {
	      return getBinary(wasmBinaryFile);
	    });
	  }

	  function createWasm() {
	    var info = {
	      "a": asmLibraryArg
	    };

	    function receiveInstance(instance, module) {
	      var exports = instance.exports;
	      Module["asm"] = exports;
	      wasmMemory = Module["asm"]["F"];
	      updateGlobalBufferAndViews(wasmMemory.buffer);
	      wasmTable = Module["asm"]["J"];
	      addOnInit(Module["asm"]["G"]);
	      removeRunDependency();
	    }

	    addRunDependency();

	    function receiveInstantiationResult(result) {
	      receiveInstance(result["instance"]);
	    }

	    function instantiateArrayBuffer(receiver) {
	      return getBinaryPromise().then(function (binary) {
	        return WebAssembly.instantiate(binary, info);
	      }).then(function (instance) {
	        return instance;
	      }).then(receiver, function (reason) {
	        err("failed to asynchronously prepare wasm: " + reason);
	        abort(reason);
	      });
	    }

	    function instantiateAsync() {
	      if (!wasmBinary && typeof WebAssembly.instantiateStreaming == "function" && !isDataURI(wasmBinaryFile) && !isFileURI(wasmBinaryFile) && !ENVIRONMENT_IS_NODE && typeof fetch == "function") {
	        return fetch(wasmBinaryFile, {
	          credentials: "same-origin"
	        }).then(function (response) {
	          var result = WebAssembly.instantiateStreaming(response, info);
	          return result.then(receiveInstantiationResult, function (reason) {
	            err("wasm streaming compile failed: " + reason);
	            err("falling back to ArrayBuffer instantiation");
	            return instantiateArrayBuffer(receiveInstantiationResult);
	          });
	        });
	      } else {
	        return instantiateArrayBuffer(receiveInstantiationResult);
	      }
	    }

	    if (Module["instantiateWasm"]) {
	      try {
	        var exports = Module["instantiateWasm"](info, receiveInstance);
	        return exports;
	      } catch (e) {
	        err("Module.instantiateWasm callback failed with error: " + e);
	        return false;
	      }
	    }

	    instantiateAsync();
	    return {};
	  }

	  var tempDouble;
	  var tempI64;

	  function ExitStatus(status) {
	    this.name = "ExitStatus";
	    this.message = "Program terminated with exit(" + status + ")";
	    this.status = status;
	  }

	  function callRuntimeCallbacks(callbacks) {
	    while (callbacks.length > 0) {
	      callbacks.shift()(Module);
	    }
	  }

	  function ___cxa_allocate_exception(size) {
	    return _malloc(size + 24) + 24;
	  }

	  function ExceptionInfo(excPtr) {
	    this.excPtr = excPtr;
	    this.ptr = excPtr - 24;

	    this.set_type = function (type) {
	      HEAPU32[this.ptr + 4 >> 2] = type;
	    };

	    this.get_type = function () {
	      return HEAPU32[this.ptr + 4 >> 2];
	    };

	    this.set_destructor = function (destructor) {
	      HEAPU32[this.ptr + 8 >> 2] = destructor;
	    };

	    this.get_destructor = function () {
	      return HEAPU32[this.ptr + 8 >> 2];
	    };

	    this.set_refcount = function (refcount) {
	      HEAP32[this.ptr >> 2] = refcount;
	    };

	    this.set_caught = function (caught) {
	      caught = caught ? 1 : 0;
	      HEAP8[this.ptr + 12 >> 0] = caught;
	    };

	    this.get_caught = function () {
	      return HEAP8[this.ptr + 12 >> 0] != 0;
	    };

	    this.set_rethrown = function (rethrown) {
	      rethrown = rethrown ? 1 : 0;
	      HEAP8[this.ptr + 13 >> 0] = rethrown;
	    };

	    this.get_rethrown = function () {
	      return HEAP8[this.ptr + 13 >> 0] != 0;
	    };

	    this.init = function (type, destructor) {
	      this.set_adjusted_ptr(0);
	      this.set_type(type);
	      this.set_destructor(destructor);
	      this.set_refcount(0);
	      this.set_caught(false);
	      this.set_rethrown(false);
	    };

	    this.add_ref = function () {
	      var value = HEAP32[this.ptr >> 2];
	      HEAP32[this.ptr >> 2] = value + 1;
	    };

	    this.release_ref = function () {
	      var prev = HEAP32[this.ptr >> 2];
	      HEAP32[this.ptr >> 2] = prev - 1;
	      return prev === 1;
	    };

	    this.set_adjusted_ptr = function (adjustedPtr) {
	      HEAPU32[this.ptr + 16 >> 2] = adjustedPtr;
	    };

	    this.get_adjusted_ptr = function () {
	      return HEAPU32[this.ptr + 16 >> 2];
	    };

	    this.get_exception_ptr = function () {
	      var isPointer = ___cxa_is_pointer_type(this.get_type());

	      if (isPointer) {
	        return HEAPU32[this.excPtr >> 2];
	      }

	      var adjusted = this.get_adjusted_ptr();
	      if (adjusted !== 0) return adjusted;
	      return this.excPtr;
	    };
	  }

	  function ___cxa_throw(ptr, type, destructor) {
	    var info = new ExceptionInfo(ptr);
	    info.init(type, destructor);
	    throw ptr;
	  }

	  function setErrNo(value) {
	    HEAP32[___errno_location() >> 2] = value;
	    return value;
	  }

	  var PATH = {
	    isAbs: path => path.charAt(0) === "/",
	    splitPath: filename => {
	      var splitPathRe = /^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/;
	      return splitPathRe.exec(filename).slice(1);
	    },
	    normalizeArray: (parts, allowAboveRoot) => {
	      var up = 0;

	      for (var i = parts.length - 1; i >= 0; i--) {
	        var last = parts[i];

	        if (last === ".") {
	          parts.splice(i, 1);
	        } else if (last === "..") {
	          parts.splice(i, 1);
	          up++;
	        } else if (up) {
	          parts.splice(i, 1);
	          up--;
	        }
	      }

	      if (allowAboveRoot) {
	        for (; up; up--) {
	          parts.unshift("..");
	        }
	      }

	      return parts;
	    },
	    normalize: path => {
	      var isAbsolute = PATH.isAbs(path),
	          trailingSlash = path.substr(-1) === "/";
	      path = PATH.normalizeArray(path.split("/").filter(p => !!p), !isAbsolute).join("/");

	      if (!path && !isAbsolute) {
	        path = ".";
	      }

	      if (path && trailingSlash) {
	        path += "/";
	      }

	      return (isAbsolute ? "/" : "") + path;
	    },
	    dirname: path => {
	      var result = PATH.splitPath(path),
	          root = result[0],
	          dir = result[1];

	      if (!root && !dir) {
	        return ".";
	      }

	      if (dir) {
	        dir = dir.substr(0, dir.length - 1);
	      }

	      return root + dir;
	    },
	    basename: path => {
	      if (path === "/") return "/";
	      path = PATH.normalize(path);
	      path = path.replace(/\/$/, "");
	      var lastSlash = path.lastIndexOf("/");
	      if (lastSlash === -1) return path;
	      return path.substr(lastSlash + 1);
	    },
	    join: function () {
	      var paths = Array.prototype.slice.call(arguments, 0);
	      return PATH.normalize(paths.join("/"));
	    },
	    join2: (l, r) => {
	      return PATH.normalize(l + "/" + r);
	    }
	  };

	  function getRandomDevice() {
	    if (typeof crypto == "object" && typeof crypto["getRandomValues"] == "function") {
	      var randomBuffer = new Uint8Array(1);
	      return () => {
	        crypto.getRandomValues(randomBuffer);
	        return randomBuffer[0];
	      };
	    } else if (ENVIRONMENT_IS_NODE) {
	      try {
	        var crypto_module = crypto__default["default"];
	        return () => crypto_module["randomBytes"](1)[0];
	      } catch (e) {}
	    }

	    return () => abort("randomDevice");
	  }

	  var PATH_FS = {
	    resolve: function () {
	      var resolvedPath = "",
	          resolvedAbsolute = false;

	      for (var i = arguments.length - 1; i >= -1 && !resolvedAbsolute; i--) {
	        var path = i >= 0 ? arguments[i] : FS.cwd();

	        if (typeof path != "string") {
	          throw new TypeError("Arguments to path.resolve must be strings");
	        } else if (!path) {
	          return "";
	        }

	        resolvedPath = path + "/" + resolvedPath;
	        resolvedAbsolute = PATH.isAbs(path);
	      }

	      resolvedPath = PATH.normalizeArray(resolvedPath.split("/").filter(p => !!p), !resolvedAbsolute).join("/");
	      return (resolvedAbsolute ? "/" : "") + resolvedPath || ".";
	    },
	    relative: (from, to) => {
	      from = PATH_FS.resolve(from).substr(1);
	      to = PATH_FS.resolve(to).substr(1);

	      function trim(arr) {
	        var start = 0;

	        for (; start < arr.length; start++) {
	          if (arr[start] !== "") break;
	        }

	        var end = arr.length - 1;

	        for (; end >= 0; end--) {
	          if (arr[end] !== "") break;
	        }

	        if (start > end) return [];
	        return arr.slice(start, end - start + 1);
	      }

	      var fromParts = trim(from.split("/"));
	      var toParts = trim(to.split("/"));
	      var length = Math.min(fromParts.length, toParts.length);
	      var samePartsLength = length;

	      for (var i = 0; i < length; i++) {
	        if (fromParts[i] !== toParts[i]) {
	          samePartsLength = i;
	          break;
	        }
	      }

	      var outputParts = [];

	      for (var i = samePartsLength; i < fromParts.length; i++) {
	        outputParts.push("..");
	      }

	      outputParts = outputParts.concat(toParts.slice(samePartsLength));
	      return outputParts.join("/");
	    }
	  };

	  function intArrayFromString(stringy, dontAddNull, length) {
	    var len = length > 0 ? length : lengthBytesUTF8(stringy) + 1;
	    var u8array = new Array(len);
	    var numBytesWritten = stringToUTF8Array(stringy, u8array, 0, u8array.length);
	    if (dontAddNull) u8array.length = numBytesWritten;
	    return u8array;
	  }

	  var TTY = {
	    ttys: [],
	    init: function () {},
	    shutdown: function () {},
	    register: function (dev, ops) {
	      TTY.ttys[dev] = {
	        input: [],
	        output: [],
	        ops: ops
	      };
	      FS.registerDevice(dev, TTY.stream_ops);
	    },
	    stream_ops: {
	      open: function (stream) {
	        var tty = TTY.ttys[stream.node.rdev];

	        if (!tty) {
	          throw new FS.ErrnoError(43);
	        }

	        stream.tty = tty;
	        stream.seekable = false;
	      },
	      close: function (stream) {
	        stream.tty.ops.flush(stream.tty);
	      },
	      flush: function (stream) {
	        stream.tty.ops.flush(stream.tty);
	      },
	      read: function (stream, buffer, offset, length, pos) {
	        if (!stream.tty || !stream.tty.ops.get_char) {
	          throw new FS.ErrnoError(60);
	        }

	        var bytesRead = 0;

	        for (var i = 0; i < length; i++) {
	          var result;

	          try {
	            result = stream.tty.ops.get_char(stream.tty);
	          } catch (e) {
	            throw new FS.ErrnoError(29);
	          }

	          if (result === undefined && bytesRead === 0) {
	            throw new FS.ErrnoError(6);
	          }

	          if (result === null || result === undefined) break;
	          bytesRead++;
	          buffer[offset + i] = result;
	        }

	        if (bytesRead) {
	          stream.node.timestamp = Date.now();
	        }

	        return bytesRead;
	      },
	      write: function (stream, buffer, offset, length, pos) {
	        if (!stream.tty || !stream.tty.ops.put_char) {
	          throw new FS.ErrnoError(60);
	        }

	        try {
	          for (var i = 0; i < length; i++) {
	            stream.tty.ops.put_char(stream.tty, buffer[offset + i]);
	          }
	        } catch (e) {
	          throw new FS.ErrnoError(29);
	        }

	        if (length) {
	          stream.node.timestamp = Date.now();
	        }

	        return i;
	      }
	    },
	    default_tty_ops: {
	      get_char: function (tty) {
	        if (!tty.input.length) {
	          var result = null;

	          if (ENVIRONMENT_IS_NODE) {
	            var BUFSIZE = 256;
	            var buf = Buffer.alloc(BUFSIZE);
	            var bytesRead = 0;

	            try {
	              bytesRead = fs.readSync(process.stdin.fd, buf, 0, BUFSIZE, -1);
	            } catch (e) {
	              if (e.toString().includes("EOF")) bytesRead = 0;else throw e;
	            }

	            if (bytesRead > 0) {
	              result = buf.slice(0, bytesRead).toString("utf-8");
	            } else {
	              result = null;
	            }
	          } else if (typeof window != "undefined" && typeof window.prompt == "function") {
	            result = window.prompt("Input: ");

	            if (result !== null) {
	              result += "\n";
	            }
	          } else if (typeof readline == "function") {
	            result = readline();

	            if (result !== null) {
	              result += "\n";
	            }
	          }

	          if (!result) {
	            return null;
	          }

	          tty.input = intArrayFromString(result, true);
	        }

	        return tty.input.shift();
	      },
	      put_char: function (tty, val) {
	        if (val === null || val === 10) {
	          out(UTF8ArrayToString(tty.output, 0));
	          tty.output = [];
	        } else {
	          if (val != 0) tty.output.push(val);
	        }
	      },
	      flush: function (tty) {
	        if (tty.output && tty.output.length > 0) {
	          out(UTF8ArrayToString(tty.output, 0));
	          tty.output = [];
	        }
	      }
	    },
	    default_tty1_ops: {
	      put_char: function (tty, val) {
	        if (val === null || val === 10) {
	          err(UTF8ArrayToString(tty.output, 0));
	          tty.output = [];
	        } else {
	          if (val != 0) tty.output.push(val);
	        }
	      },
	      flush: function (tty) {
	        if (tty.output && tty.output.length > 0) {
	          err(UTF8ArrayToString(tty.output, 0));
	          tty.output = [];
	        }
	      }
	    }
	  };

	  function zeroMemory(address, size) {
	    HEAPU8.fill(0, address, address + size);
	  }

	  function alignMemory(size, alignment) {
	    return Math.ceil(size / alignment) * alignment;
	  }

	  function mmapAlloc(size) {
	    size = alignMemory(size, 65536);

	    var ptr = _emscripten_builtin_memalign(65536, size);

	    if (!ptr) return 0;
	    zeroMemory(ptr, size);
	    return ptr;
	  }

	  var MEMFS = {
	    ops_table: null,
	    mount: function (mount) {
	      return MEMFS.createNode(null, "/", 16384 | 511, 0);
	    },
	    createNode: function (parent, name, mode, dev) {
	      if (FS.isBlkdev(mode) || FS.isFIFO(mode)) {
	        throw new FS.ErrnoError(63);
	      }

	      if (!MEMFS.ops_table) {
	        MEMFS.ops_table = {
	          dir: {
	            node: {
	              getattr: MEMFS.node_ops.getattr,
	              setattr: MEMFS.node_ops.setattr,
	              lookup: MEMFS.node_ops.lookup,
	              mknod: MEMFS.node_ops.mknod,
	              rename: MEMFS.node_ops.rename,
	              unlink: MEMFS.node_ops.unlink,
	              rmdir: MEMFS.node_ops.rmdir,
	              readdir: MEMFS.node_ops.readdir,
	              symlink: MEMFS.node_ops.symlink
	            },
	            stream: {
	              llseek: MEMFS.stream_ops.llseek
	            }
	          },
	          file: {
	            node: {
	              getattr: MEMFS.node_ops.getattr,
	              setattr: MEMFS.node_ops.setattr
	            },
	            stream: {
	              llseek: MEMFS.stream_ops.llseek,
	              read: MEMFS.stream_ops.read,
	              write: MEMFS.stream_ops.write,
	              allocate: MEMFS.stream_ops.allocate,
	              mmap: MEMFS.stream_ops.mmap,
	              msync: MEMFS.stream_ops.msync
	            }
	          },
	          link: {
	            node: {
	              getattr: MEMFS.node_ops.getattr,
	              setattr: MEMFS.node_ops.setattr,
	              readlink: MEMFS.node_ops.readlink
	            },
	            stream: {}
	          },
	          chrdev: {
	            node: {
	              getattr: MEMFS.node_ops.getattr,
	              setattr: MEMFS.node_ops.setattr
	            },
	            stream: FS.chrdev_stream_ops
	          }
	        };
	      }

	      var node = FS.createNode(parent, name, mode, dev);

	      if (FS.isDir(node.mode)) {
	        node.node_ops = MEMFS.ops_table.dir.node;
	        node.stream_ops = MEMFS.ops_table.dir.stream;
	        node.contents = {};
	      } else if (FS.isFile(node.mode)) {
	        node.node_ops = MEMFS.ops_table.file.node;
	        node.stream_ops = MEMFS.ops_table.file.stream;
	        node.usedBytes = 0;
	        node.contents = null;
	      } else if (FS.isLink(node.mode)) {
	        node.node_ops = MEMFS.ops_table.link.node;
	        node.stream_ops = MEMFS.ops_table.link.stream;
	      } else if (FS.isChrdev(node.mode)) {
	        node.node_ops = MEMFS.ops_table.chrdev.node;
	        node.stream_ops = MEMFS.ops_table.chrdev.stream;
	      }

	      node.timestamp = Date.now();

	      if (parent) {
	        parent.contents[name] = node;
	        parent.timestamp = node.timestamp;
	      }

	      return node;
	    },
	    getFileDataAsTypedArray: function (node) {
	      if (!node.contents) return new Uint8Array(0);
	      if (node.contents.subarray) return node.contents.subarray(0, node.usedBytes);
	      return new Uint8Array(node.contents);
	    },
	    expandFileStorage: function (node, newCapacity) {
	      var prevCapacity = node.contents ? node.contents.length : 0;
	      if (prevCapacity >= newCapacity) return;
	      var CAPACITY_DOUBLING_MAX = 1024 * 1024;
	      newCapacity = Math.max(newCapacity, prevCapacity * (prevCapacity < CAPACITY_DOUBLING_MAX ? 2 : 1.125) >>> 0);
	      if (prevCapacity != 0) newCapacity = Math.max(newCapacity, 256);
	      var oldContents = node.contents;
	      node.contents = new Uint8Array(newCapacity);
	      if (node.usedBytes > 0) node.contents.set(oldContents.subarray(0, node.usedBytes), 0);
	    },
	    resizeFileStorage: function (node, newSize) {
	      if (node.usedBytes == newSize) return;

	      if (newSize == 0) {
	        node.contents = null;
	        node.usedBytes = 0;
	      } else {
	        var oldContents = node.contents;
	        node.contents = new Uint8Array(newSize);

	        if (oldContents) {
	          node.contents.set(oldContents.subarray(0, Math.min(newSize, node.usedBytes)));
	        }

	        node.usedBytes = newSize;
	      }
	    },
	    node_ops: {
	      getattr: function (node) {
	        var attr = {};
	        attr.dev = FS.isChrdev(node.mode) ? node.id : 1;
	        attr.ino = node.id;
	        attr.mode = node.mode;
	        attr.nlink = 1;
	        attr.uid = 0;
	        attr.gid = 0;
	        attr.rdev = node.rdev;

	        if (FS.isDir(node.mode)) {
	          attr.size = 4096;
	        } else if (FS.isFile(node.mode)) {
	          attr.size = node.usedBytes;
	        } else if (FS.isLink(node.mode)) {
	          attr.size = node.link.length;
	        } else {
	          attr.size = 0;
	        }

	        attr.atime = new Date(node.timestamp);
	        attr.mtime = new Date(node.timestamp);
	        attr.ctime = new Date(node.timestamp);
	        attr.blksize = 4096;
	        attr.blocks = Math.ceil(attr.size / attr.blksize);
	        return attr;
	      },
	      setattr: function (node, attr) {
	        if (attr.mode !== undefined) {
	          node.mode = attr.mode;
	        }

	        if (attr.timestamp !== undefined) {
	          node.timestamp = attr.timestamp;
	        }

	        if (attr.size !== undefined) {
	          MEMFS.resizeFileStorage(node, attr.size);
	        }
	      },
	      lookup: function (parent, name) {
	        throw FS.genericErrors[44];
	      },
	      mknod: function (parent, name, mode, dev) {
	        return MEMFS.createNode(parent, name, mode, dev);
	      },
	      rename: function (old_node, new_dir, new_name) {
	        if (FS.isDir(old_node.mode)) {
	          var new_node;

	          try {
	            new_node = FS.lookupNode(new_dir, new_name);
	          } catch (e) {}

	          if (new_node) {
	            for (var i in new_node.contents) {
	              throw new FS.ErrnoError(55);
	            }
	          }
	        }

	        delete old_node.parent.contents[old_node.name];
	        old_node.parent.timestamp = Date.now();
	        old_node.name = new_name;
	        new_dir.contents[new_name] = old_node;
	        new_dir.timestamp = old_node.parent.timestamp;
	        old_node.parent = new_dir;
	      },
	      unlink: function (parent, name) {
	        delete parent.contents[name];
	        parent.timestamp = Date.now();
	      },
	      rmdir: function (parent, name) {
	        var node = FS.lookupNode(parent, name);

	        for (var i in node.contents) {
	          throw new FS.ErrnoError(55);
	        }

	        delete parent.contents[name];
	        parent.timestamp = Date.now();
	      },
	      readdir: function (node) {
	        var entries = [".", ".."];

	        for (var key in node.contents) {
	          if (!node.contents.hasOwnProperty(key)) {
	            continue;
	          }

	          entries.push(key);
	        }

	        return entries;
	      },
	      symlink: function (parent, newname, oldpath) {
	        var node = MEMFS.createNode(parent, newname, 511 | 40960, 0);
	        node.link = oldpath;
	        return node;
	      },
	      readlink: function (node) {
	        if (!FS.isLink(node.mode)) {
	          throw new FS.ErrnoError(28);
	        }

	        return node.link;
	      }
	    },
	    stream_ops: {
	      read: function (stream, buffer, offset, length, position) {
	        var contents = stream.node.contents;
	        if (position >= stream.node.usedBytes) return 0;
	        var size = Math.min(stream.node.usedBytes - position, length);

	        if (size > 8 && contents.subarray) {
	          buffer.set(contents.subarray(position, position + size), offset);
	        } else {
	          for (var i = 0; i < size; i++) buffer[offset + i] = contents[position + i];
	        }

	        return size;
	      },
	      write: function (stream, buffer, offset, length, position, canOwn) {
	        if (!length) return 0;
	        var node = stream.node;
	        node.timestamp = Date.now();

	        if (buffer.subarray && (!node.contents || node.contents.subarray)) {
	          if (canOwn) {
	            node.contents = buffer.subarray(offset, offset + length);
	            node.usedBytes = length;
	            return length;
	          } else if (node.usedBytes === 0 && position === 0) {
	            node.contents = buffer.slice(offset, offset + length);
	            node.usedBytes = length;
	            return length;
	          } else if (position + length <= node.usedBytes) {
	            node.contents.set(buffer.subarray(offset, offset + length), position);
	            return length;
	          }
	        }

	        MEMFS.expandFileStorage(node, position + length);

	        if (node.contents.subarray && buffer.subarray) {
	          node.contents.set(buffer.subarray(offset, offset + length), position);
	        } else {
	          for (var i = 0; i < length; i++) {
	            node.contents[position + i] = buffer[offset + i];
	          }
	        }

	        node.usedBytes = Math.max(node.usedBytes, position + length);
	        return length;
	      },
	      llseek: function (stream, offset, whence) {
	        var position = offset;

	        if (whence === 1) {
	          position += stream.position;
	        } else if (whence === 2) {
	          if (FS.isFile(stream.node.mode)) {
	            position += stream.node.usedBytes;
	          }
	        }

	        if (position < 0) {
	          throw new FS.ErrnoError(28);
	        }

	        return position;
	      },
	      allocate: function (stream, offset, length) {
	        MEMFS.expandFileStorage(stream.node, offset + length);
	        stream.node.usedBytes = Math.max(stream.node.usedBytes, offset + length);
	      },
	      mmap: function (stream, length, position, prot, flags) {
	        if (!FS.isFile(stream.node.mode)) {
	          throw new FS.ErrnoError(43);
	        }

	        var ptr;
	        var allocated;
	        var contents = stream.node.contents;

	        if (!(flags & 2) && contents.buffer === buffer) {
	          allocated = false;
	          ptr = contents.byteOffset;
	        } else {
	          if (position > 0 || position + length < contents.length) {
	            if (contents.subarray) {
	              contents = contents.subarray(position, position + length);
	            } else {
	              contents = Array.prototype.slice.call(contents, position, position + length);
	            }
	          }

	          allocated = true;
	          ptr = mmapAlloc(length);

	          if (!ptr) {
	            throw new FS.ErrnoError(48);
	          }

	          HEAP8.set(contents, ptr);
	        }

	        return {
	          ptr: ptr,
	          allocated: allocated
	        };
	      },
	      msync: function (stream, buffer, offset, length, mmapFlags) {
	        if (!FS.isFile(stream.node.mode)) {
	          throw new FS.ErrnoError(43);
	        }

	        if (mmapFlags & 2) {
	          return 0;
	        }

	        MEMFS.stream_ops.write(stream, buffer, 0, length, offset, false);
	        return 0;
	      }
	    }
	  };

	  function asyncLoad(url, onload, onerror, noRunDep) {
	    var dep = !noRunDep ? getUniqueRunDependency("al " + url) : "";
	    readAsync(url, arrayBuffer => {
	      assert(arrayBuffer, 'Loading data file "' + url + '" failed (no arrayBuffer).');
	      onload(new Uint8Array(arrayBuffer));
	      if (dep) removeRunDependency();
	    }, event => {
	      if (onerror) {
	        onerror();
	      } else {
	        throw 'Loading data file "' + url + '" failed.';
	      }
	    });
	    if (dep) addRunDependency();
	  }

	  var FS = {
	    root: null,
	    mounts: [],
	    devices: {},
	    streams: [],
	    nextInode: 1,
	    nameTable: null,
	    currentPath: "/",
	    initialized: false,
	    ignorePermissions: true,
	    ErrnoError: null,
	    genericErrors: {},
	    filesystems: null,
	    syncFSRequests: 0,
	    lookupPath: function (path) {
	      let opts = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
	      path = PATH_FS.resolve(FS.cwd(), path);
	      if (!path) return {
	        path: "",
	        node: null
	      };
	      var defaults = {
	        follow_mount: true,
	        recurse_count: 0
	      };
	      opts = Object.assign(defaults, opts);

	      if (opts.recurse_count > 8) {
	        throw new FS.ErrnoError(32);
	      }

	      var parts = PATH.normalizeArray(path.split("/").filter(p => !!p), false);
	      var current = FS.root;
	      var current_path = "/";

	      for (var i = 0; i < parts.length; i++) {
	        var islast = i === parts.length - 1;

	        if (islast && opts.parent) {
	          break;
	        }

	        current = FS.lookupNode(current, parts[i]);
	        current_path = PATH.join2(current_path, parts[i]);

	        if (FS.isMountpoint(current)) {
	          if (!islast || islast && opts.follow_mount) {
	            current = current.mounted.root;
	          }
	        }

	        if (!islast || opts.follow) {
	          var count = 0;

	          while (FS.isLink(current.mode)) {
	            var link = FS.readlink(current_path);
	            current_path = PATH_FS.resolve(PATH.dirname(current_path), link);
	            var lookup = FS.lookupPath(current_path, {
	              recurse_count: opts.recurse_count + 1
	            });
	            current = lookup.node;

	            if (count++ > 40) {
	              throw new FS.ErrnoError(32);
	            }
	          }
	        }
	      }

	      return {
	        path: current_path,
	        node: current
	      };
	    },
	    getPath: node => {
	      var path;

	      while (true) {
	        if (FS.isRoot(node)) {
	          var mount = node.mount.mountpoint;
	          if (!path) return mount;
	          return mount[mount.length - 1] !== "/" ? mount + "/" + path : mount + path;
	        }

	        path = path ? node.name + "/" + path : node.name;
	        node = node.parent;
	      }
	    },
	    hashName: (parentid, name) => {
	      var hash = 0;

	      for (var i = 0; i < name.length; i++) {
	        hash = (hash << 5) - hash + name.charCodeAt(i) | 0;
	      }

	      return (parentid + hash >>> 0) % FS.nameTable.length;
	    },
	    hashAddNode: node => {
	      var hash = FS.hashName(node.parent.id, node.name);
	      node.name_next = FS.nameTable[hash];
	      FS.nameTable[hash] = node;
	    },
	    hashRemoveNode: node => {
	      var hash = FS.hashName(node.parent.id, node.name);

	      if (FS.nameTable[hash] === node) {
	        FS.nameTable[hash] = node.name_next;
	      } else {
	        var current = FS.nameTable[hash];

	        while (current) {
	          if (current.name_next === node) {
	            current.name_next = node.name_next;
	            break;
	          }

	          current = current.name_next;
	        }
	      }
	    },
	    lookupNode: (parent, name) => {
	      var errCode = FS.mayLookup(parent);

	      if (errCode) {
	        throw new FS.ErrnoError(errCode, parent);
	      }

	      var hash = FS.hashName(parent.id, name);

	      for (var node = FS.nameTable[hash]; node; node = node.name_next) {
	        var nodeName = node.name;

	        if (node.parent.id === parent.id && nodeName === name) {
	          return node;
	        }
	      }

	      return FS.lookup(parent, name);
	    },
	    createNode: (parent, name, mode, rdev) => {
	      var node = new FS.FSNode(parent, name, mode, rdev);
	      FS.hashAddNode(node);
	      return node;
	    },
	    destroyNode: node => {
	      FS.hashRemoveNode(node);
	    },
	    isRoot: node => {
	      return node === node.parent;
	    },
	    isMountpoint: node => {
	      return !!node.mounted;
	    },
	    isFile: mode => {
	      return (mode & 61440) === 32768;
	    },
	    isDir: mode => {
	      return (mode & 61440) === 16384;
	    },
	    isLink: mode => {
	      return (mode & 61440) === 40960;
	    },
	    isChrdev: mode => {
	      return (mode & 61440) === 8192;
	    },
	    isBlkdev: mode => {
	      return (mode & 61440) === 24576;
	    },
	    isFIFO: mode => {
	      return (mode & 61440) === 4096;
	    },
	    isSocket: mode => {
	      return (mode & 49152) === 49152;
	    },
	    flagModes: {
	      "r": 0,
	      "r+": 2,
	      "w": 577,
	      "w+": 578,
	      "a": 1089,
	      "a+": 1090
	    },
	    modeStringToFlags: str => {
	      var flags = FS.flagModes[str];

	      if (typeof flags == "undefined") {
	        throw new Error("Unknown file open mode: " + str);
	      }

	      return flags;
	    },
	    flagsToPermissionString: flag => {
	      var perms = ["r", "w", "rw"][flag & 3];

	      if (flag & 512) {
	        perms += "w";
	      }

	      return perms;
	    },
	    nodePermissions: (node, perms) => {
	      if (FS.ignorePermissions) {
	        return 0;
	      }

	      if (perms.includes("r") && !(node.mode & 292)) {
	        return 2;
	      } else if (perms.includes("w") && !(node.mode & 146)) {
	        return 2;
	      } else if (perms.includes("x") && !(node.mode & 73)) {
	        return 2;
	      }

	      return 0;
	    },
	    mayLookup: dir => {
	      var errCode = FS.nodePermissions(dir, "x");
	      if (errCode) return errCode;
	      if (!dir.node_ops.lookup) return 2;
	      return 0;
	    },
	    mayCreate: (dir, name) => {
	      try {
	        var node = FS.lookupNode(dir, name);
	        return 20;
	      } catch (e) {}

	      return FS.nodePermissions(dir, "wx");
	    },
	    mayDelete: (dir, name, isdir) => {
	      var node;

	      try {
	        node = FS.lookupNode(dir, name);
	      } catch (e) {
	        return e.errno;
	      }

	      var errCode = FS.nodePermissions(dir, "wx");

	      if (errCode) {
	        return errCode;
	      }

	      if (isdir) {
	        if (!FS.isDir(node.mode)) {
	          return 54;
	        }

	        if (FS.isRoot(node) || FS.getPath(node) === FS.cwd()) {
	          return 10;
	        }
	      } else {
	        if (FS.isDir(node.mode)) {
	          return 31;
	        }
	      }

	      return 0;
	    },
	    mayOpen: (node, flags) => {
	      if (!node) {
	        return 44;
	      }

	      if (FS.isLink(node.mode)) {
	        return 32;
	      } else if (FS.isDir(node.mode)) {
	        if (FS.flagsToPermissionString(flags) !== "r" || flags & 512) {
	          return 31;
	        }
	      }

	      return FS.nodePermissions(node, FS.flagsToPermissionString(flags));
	    },
	    MAX_OPEN_FDS: 4096,
	    nextfd: function () {
	      let fd_start = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
	      let fd_end = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : FS.MAX_OPEN_FDS;

	      for (var fd = fd_start; fd <= fd_end; fd++) {
	        if (!FS.streams[fd]) {
	          return fd;
	        }
	      }

	      throw new FS.ErrnoError(33);
	    },
	    getStream: fd => FS.streams[fd],
	    createStream: (stream, fd_start, fd_end) => {
	      if (!FS.FSStream) {
	        FS.FSStream = function () {
	          this.shared = {};
	        };

	        FS.FSStream.prototype = {};
	        Object.defineProperties(FS.FSStream.prototype, {
	          object: {
	            get: function () {
	              return this.node;
	            },
	            set: function (val) {
	              this.node = val;
	            }
	          },
	          isRead: {
	            get: function () {
	              return (this.flags & 2097155) !== 1;
	            }
	          },
	          isWrite: {
	            get: function () {
	              return (this.flags & 2097155) !== 0;
	            }
	          },
	          isAppend: {
	            get: function () {
	              return this.flags & 1024;
	            }
	          },
	          flags: {
	            get: function () {
	              return this.shared.flags;
	            },
	            set: function (val) {
	              this.shared.flags = val;
	            }
	          },
	          position: {
	            get: function () {
	              return this.shared.position;
	            },
	            set: function (val) {
	              this.shared.position = val;
	            }
	          }
	        });
	      }

	      stream = Object.assign(new FS.FSStream(), stream);
	      var fd = FS.nextfd(fd_start, fd_end);
	      stream.fd = fd;
	      FS.streams[fd] = stream;
	      return stream;
	    },
	    closeStream: fd => {
	      FS.streams[fd] = null;
	    },
	    chrdev_stream_ops: {
	      open: stream => {
	        var device = FS.getDevice(stream.node.rdev);
	        stream.stream_ops = device.stream_ops;

	        if (stream.stream_ops.open) {
	          stream.stream_ops.open(stream);
	        }
	      },
	      llseek: () => {
	        throw new FS.ErrnoError(70);
	      }
	    },
	    major: dev => dev >> 8,
	    minor: dev => dev & 255,
	    makedev: (ma, mi) => ma << 8 | mi,
	    registerDevice: (dev, ops) => {
	      FS.devices[dev] = {
	        stream_ops: ops
	      };
	    },
	    getDevice: dev => FS.devices[dev],
	    getMounts: mount => {
	      var mounts = [];
	      var check = [mount];

	      while (check.length) {
	        var m = check.pop();
	        mounts.push(m);
	        check.push.apply(check, m.mounts);
	      }

	      return mounts;
	    },
	    syncfs: (populate, callback) => {
	      if (typeof populate == "function") {
	        callback = populate;
	        populate = false;
	      }

	      FS.syncFSRequests++;

	      if (FS.syncFSRequests > 1) {
	        err("warning: " + FS.syncFSRequests + " FS.syncfs operations in flight at once, probably just doing extra work");
	      }

	      var mounts = FS.getMounts(FS.root.mount);
	      var completed = 0;

	      function doCallback(errCode) {
	        FS.syncFSRequests--;
	        return callback(errCode);
	      }

	      function done(errCode) {
	        if (errCode) {
	          if (!done.errored) {
	            done.errored = true;
	            return doCallback(errCode);
	          }

	          return;
	        }

	        if (++completed >= mounts.length) {
	          doCallback(null);
	        }
	      }

	      mounts.forEach(mount => {
	        if (!mount.type.syncfs) {
	          return done(null);
	        }

	        mount.type.syncfs(mount, populate, done);
	      });
	    },
	    mount: (type, opts, mountpoint) => {
	      var root = mountpoint === "/";
	      var pseudo = !mountpoint;
	      var node;

	      if (root && FS.root) {
	        throw new FS.ErrnoError(10);
	      } else if (!root && !pseudo) {
	        var lookup = FS.lookupPath(mountpoint, {
	          follow_mount: false
	        });
	        mountpoint = lookup.path;
	        node = lookup.node;

	        if (FS.isMountpoint(node)) {
	          throw new FS.ErrnoError(10);
	        }

	        if (!FS.isDir(node.mode)) {
	          throw new FS.ErrnoError(54);
	        }
	      }

	      var mount = {
	        type: type,
	        opts: opts,
	        mountpoint: mountpoint,
	        mounts: []
	      };
	      var mountRoot = type.mount(mount);
	      mountRoot.mount = mount;
	      mount.root = mountRoot;

	      if (root) {
	        FS.root = mountRoot;
	      } else if (node) {
	        node.mounted = mount;

	        if (node.mount) {
	          node.mount.mounts.push(mount);
	        }
	      }

	      return mountRoot;
	    },
	    unmount: mountpoint => {
	      var lookup = FS.lookupPath(mountpoint, {
	        follow_mount: false
	      });

	      if (!FS.isMountpoint(lookup.node)) {
	        throw new FS.ErrnoError(28);
	      }

	      var node = lookup.node;
	      var mount = node.mounted;
	      var mounts = FS.getMounts(mount);
	      Object.keys(FS.nameTable).forEach(hash => {
	        var current = FS.nameTable[hash];

	        while (current) {
	          var next = current.name_next;

	          if (mounts.includes(current.mount)) {
	            FS.destroyNode(current);
	          }

	          current = next;
	        }
	      });
	      node.mounted = null;
	      var idx = node.mount.mounts.indexOf(mount);
	      node.mount.mounts.splice(idx, 1);
	    },
	    lookup: (parent, name) => {
	      return parent.node_ops.lookup(parent, name);
	    },
	    mknod: (path, mode, dev) => {
	      var lookup = FS.lookupPath(path, {
	        parent: true
	      });
	      var parent = lookup.node;
	      var name = PATH.basename(path);

	      if (!name || name === "." || name === "..") {
	        throw new FS.ErrnoError(28);
	      }

	      var errCode = FS.mayCreate(parent, name);

	      if (errCode) {
	        throw new FS.ErrnoError(errCode);
	      }

	      if (!parent.node_ops.mknod) {
	        throw new FS.ErrnoError(63);
	      }

	      return parent.node_ops.mknod(parent, name, mode, dev);
	    },
	    create: (path, mode) => {
	      mode = mode !== undefined ? mode : 438;
	      mode &= 4095;
	      mode |= 32768;
	      return FS.mknod(path, mode, 0);
	    },
	    mkdir: (path, mode) => {
	      mode = mode !== undefined ? mode : 511;
	      mode &= 511 | 512;
	      mode |= 16384;
	      return FS.mknod(path, mode, 0);
	    },
	    mkdirTree: (path, mode) => {
	      var dirs = path.split("/");
	      var d = "";

	      for (var i = 0; i < dirs.length; ++i) {
	        if (!dirs[i]) continue;
	        d += "/" + dirs[i];

	        try {
	          FS.mkdir(d, mode);
	        } catch (e) {
	          if (e.errno != 20) throw e;
	        }
	      }
	    },
	    mkdev: (path, mode, dev) => {
	      if (typeof dev == "undefined") {
	        dev = mode;
	        mode = 438;
	      }

	      mode |= 8192;
	      return FS.mknod(path, mode, dev);
	    },
	    symlink: (oldpath, newpath) => {
	      if (!PATH_FS.resolve(oldpath)) {
	        throw new FS.ErrnoError(44);
	      }

	      var lookup = FS.lookupPath(newpath, {
	        parent: true
	      });
	      var parent = lookup.node;

	      if (!parent) {
	        throw new FS.ErrnoError(44);
	      }

	      var newname = PATH.basename(newpath);
	      var errCode = FS.mayCreate(parent, newname);

	      if (errCode) {
	        throw new FS.ErrnoError(errCode);
	      }

	      if (!parent.node_ops.symlink) {
	        throw new FS.ErrnoError(63);
	      }

	      return parent.node_ops.symlink(parent, newname, oldpath);
	    },
	    rename: (old_path, new_path) => {
	      var old_dirname = PATH.dirname(old_path);
	      var new_dirname = PATH.dirname(new_path);
	      var old_name = PATH.basename(old_path);
	      var new_name = PATH.basename(new_path);
	      var lookup, old_dir, new_dir;
	      lookup = FS.lookupPath(old_path, {
	        parent: true
	      });
	      old_dir = lookup.node;
	      lookup = FS.lookupPath(new_path, {
	        parent: true
	      });
	      new_dir = lookup.node;
	      if (!old_dir || !new_dir) throw new FS.ErrnoError(44);

	      if (old_dir.mount !== new_dir.mount) {
	        throw new FS.ErrnoError(75);
	      }

	      var old_node = FS.lookupNode(old_dir, old_name);
	      var relative = PATH_FS.relative(old_path, new_dirname);

	      if (relative.charAt(0) !== ".") {
	        throw new FS.ErrnoError(28);
	      }

	      relative = PATH_FS.relative(new_path, old_dirname);

	      if (relative.charAt(0) !== ".") {
	        throw new FS.ErrnoError(55);
	      }

	      var new_node;

	      try {
	        new_node = FS.lookupNode(new_dir, new_name);
	      } catch (e) {}

	      if (old_node === new_node) {
	        return;
	      }

	      var isdir = FS.isDir(old_node.mode);
	      var errCode = FS.mayDelete(old_dir, old_name, isdir);

	      if (errCode) {
	        throw new FS.ErrnoError(errCode);
	      }

	      errCode = new_node ? FS.mayDelete(new_dir, new_name, isdir) : FS.mayCreate(new_dir, new_name);

	      if (errCode) {
	        throw new FS.ErrnoError(errCode);
	      }

	      if (!old_dir.node_ops.rename) {
	        throw new FS.ErrnoError(63);
	      }

	      if (FS.isMountpoint(old_node) || new_node && FS.isMountpoint(new_node)) {
	        throw new FS.ErrnoError(10);
	      }

	      if (new_dir !== old_dir) {
	        errCode = FS.nodePermissions(old_dir, "w");

	        if (errCode) {
	          throw new FS.ErrnoError(errCode);
	        }
	      }

	      FS.hashRemoveNode(old_node);

	      try {
	        old_dir.node_ops.rename(old_node, new_dir, new_name);
	      } catch (e) {
	        throw e;
	      } finally {
	        FS.hashAddNode(old_node);
	      }
	    },
	    rmdir: path => {
	      var lookup = FS.lookupPath(path, {
	        parent: true
	      });
	      var parent = lookup.node;
	      var name = PATH.basename(path);
	      var node = FS.lookupNode(parent, name);
	      var errCode = FS.mayDelete(parent, name, true);

	      if (errCode) {
	        throw new FS.ErrnoError(errCode);
	      }

	      if (!parent.node_ops.rmdir) {
	        throw new FS.ErrnoError(63);
	      }

	      if (FS.isMountpoint(node)) {
	        throw new FS.ErrnoError(10);
	      }

	      parent.node_ops.rmdir(parent, name);
	      FS.destroyNode(node);
	    },
	    readdir: path => {
	      var lookup = FS.lookupPath(path, {
	        follow: true
	      });
	      var node = lookup.node;

	      if (!node.node_ops.readdir) {
	        throw new FS.ErrnoError(54);
	      }

	      return node.node_ops.readdir(node);
	    },
	    unlink: path => {
	      var lookup = FS.lookupPath(path, {
	        parent: true
	      });
	      var parent = lookup.node;

	      if (!parent) {
	        throw new FS.ErrnoError(44);
	      }

	      var name = PATH.basename(path);
	      var node = FS.lookupNode(parent, name);
	      var errCode = FS.mayDelete(parent, name, false);

	      if (errCode) {
	        throw new FS.ErrnoError(errCode);
	      }

	      if (!parent.node_ops.unlink) {
	        throw new FS.ErrnoError(63);
	      }

	      if (FS.isMountpoint(node)) {
	        throw new FS.ErrnoError(10);
	      }

	      parent.node_ops.unlink(parent, name);
	      FS.destroyNode(node);
	    },
	    readlink: path => {
	      var lookup = FS.lookupPath(path);
	      var link = lookup.node;

	      if (!link) {
	        throw new FS.ErrnoError(44);
	      }

	      if (!link.node_ops.readlink) {
	        throw new FS.ErrnoError(28);
	      }

	      return PATH_FS.resolve(FS.getPath(link.parent), link.node_ops.readlink(link));
	    },
	    stat: (path, dontFollow) => {
	      var lookup = FS.lookupPath(path, {
	        follow: !dontFollow
	      });
	      var node = lookup.node;

	      if (!node) {
	        throw new FS.ErrnoError(44);
	      }

	      if (!node.node_ops.getattr) {
	        throw new FS.ErrnoError(63);
	      }

	      return node.node_ops.getattr(node);
	    },
	    lstat: path => {
	      return FS.stat(path, true);
	    },
	    chmod: (path, mode, dontFollow) => {
	      var node;

	      if (typeof path == "string") {
	        var lookup = FS.lookupPath(path, {
	          follow: !dontFollow
	        });
	        node = lookup.node;
	      } else {
	        node = path;
	      }

	      if (!node.node_ops.setattr) {
	        throw new FS.ErrnoError(63);
	      }

	      node.node_ops.setattr(node, {
	        mode: mode & 4095 | node.mode & ~4095,
	        timestamp: Date.now()
	      });
	    },
	    lchmod: (path, mode) => {
	      FS.chmod(path, mode, true);
	    },
	    fchmod: (fd, mode) => {
	      var stream = FS.getStream(fd);

	      if (!stream) {
	        throw new FS.ErrnoError(8);
	      }

	      FS.chmod(stream.node, mode);
	    },
	    chown: (path, uid, gid, dontFollow) => {
	      var node;

	      if (typeof path == "string") {
	        var lookup = FS.lookupPath(path, {
	          follow: !dontFollow
	        });
	        node = lookup.node;
	      } else {
	        node = path;
	      }

	      if (!node.node_ops.setattr) {
	        throw new FS.ErrnoError(63);
	      }

	      node.node_ops.setattr(node, {
	        timestamp: Date.now()
	      });
	    },
	    lchown: (path, uid, gid) => {
	      FS.chown(path, uid, gid, true);
	    },
	    fchown: (fd, uid, gid) => {
	      var stream = FS.getStream(fd);

	      if (!stream) {
	        throw new FS.ErrnoError(8);
	      }

	      FS.chown(stream.node, uid, gid);
	    },
	    truncate: (path, len) => {
	      if (len < 0) {
	        throw new FS.ErrnoError(28);
	      }

	      var node;

	      if (typeof path == "string") {
	        var lookup = FS.lookupPath(path, {
	          follow: true
	        });
	        node = lookup.node;
	      } else {
	        node = path;
	      }

	      if (!node.node_ops.setattr) {
	        throw new FS.ErrnoError(63);
	      }

	      if (FS.isDir(node.mode)) {
	        throw new FS.ErrnoError(31);
	      }

	      if (!FS.isFile(node.mode)) {
	        throw new FS.ErrnoError(28);
	      }

	      var errCode = FS.nodePermissions(node, "w");

	      if (errCode) {
	        throw new FS.ErrnoError(errCode);
	      }

	      node.node_ops.setattr(node, {
	        size: len,
	        timestamp: Date.now()
	      });
	    },
	    ftruncate: (fd, len) => {
	      var stream = FS.getStream(fd);

	      if (!stream) {
	        throw new FS.ErrnoError(8);
	      }

	      if ((stream.flags & 2097155) === 0) {
	        throw new FS.ErrnoError(28);
	      }

	      FS.truncate(stream.node, len);
	    },
	    utime: (path, atime, mtime) => {
	      var lookup = FS.lookupPath(path, {
	        follow: true
	      });
	      var node = lookup.node;
	      node.node_ops.setattr(node, {
	        timestamp: Math.max(atime, mtime)
	      });
	    },
	    open: (path, flags, mode) => {
	      if (path === "") {
	        throw new FS.ErrnoError(44);
	      }

	      flags = typeof flags == "string" ? FS.modeStringToFlags(flags) : flags;
	      mode = typeof mode == "undefined" ? 438 : mode;

	      if (flags & 64) {
	        mode = mode & 4095 | 32768;
	      } else {
	        mode = 0;
	      }

	      var node;

	      if (typeof path == "object") {
	        node = path;
	      } else {
	        path = PATH.normalize(path);

	        try {
	          var lookup = FS.lookupPath(path, {
	            follow: !(flags & 131072)
	          });
	          node = lookup.node;
	        } catch (e) {}
	      }

	      var created = false;

	      if (flags & 64) {
	        if (node) {
	          if (flags & 128) {
	            throw new FS.ErrnoError(20);
	          }
	        } else {
	          node = FS.mknod(path, mode, 0);
	          created = true;
	        }
	      }

	      if (!node) {
	        throw new FS.ErrnoError(44);
	      }

	      if (FS.isChrdev(node.mode)) {
	        flags &= ~512;
	      }

	      if (flags & 65536 && !FS.isDir(node.mode)) {
	        throw new FS.ErrnoError(54);
	      }

	      if (!created) {
	        var errCode = FS.mayOpen(node, flags);

	        if (errCode) {
	          throw new FS.ErrnoError(errCode);
	        }
	      }

	      if (flags & 512 && !created) {
	        FS.truncate(node, 0);
	      }

	      flags &= ~(128 | 512 | 131072);
	      var stream = FS.createStream({
	        node: node,
	        path: FS.getPath(node),
	        flags: flags,
	        seekable: true,
	        position: 0,
	        stream_ops: node.stream_ops,
	        ungotten: [],
	        error: false
	      });

	      if (stream.stream_ops.open) {
	        stream.stream_ops.open(stream);
	      }

	      if (Module["logReadFiles"] && !(flags & 1)) {
	        if (!FS.readFiles) FS.readFiles = {};

	        if (!(path in FS.readFiles)) {
	          FS.readFiles[path] = 1;
	        }
	      }

	      return stream;
	    },
	    close: stream => {
	      if (FS.isClosed(stream)) {
	        throw new FS.ErrnoError(8);
	      }

	      if (stream.getdents) stream.getdents = null;

	      try {
	        if (stream.stream_ops.close) {
	          stream.stream_ops.close(stream);
	        }
	      } catch (e) {
	        throw e;
	      } finally {
	        FS.closeStream(stream.fd);
	      }

	      stream.fd = null;
	    },
	    isClosed: stream => {
	      return stream.fd === null;
	    },
	    llseek: (stream, offset, whence) => {
	      if (FS.isClosed(stream)) {
	        throw new FS.ErrnoError(8);
	      }

	      if (!stream.seekable || !stream.stream_ops.llseek) {
	        throw new FS.ErrnoError(70);
	      }

	      if (whence != 0 && whence != 1 && whence != 2) {
	        throw new FS.ErrnoError(28);
	      }

	      stream.position = stream.stream_ops.llseek(stream, offset, whence);
	      stream.ungotten = [];
	      return stream.position;
	    },
	    read: (stream, buffer, offset, length, position) => {
	      if (length < 0 || position < 0) {
	        throw new FS.ErrnoError(28);
	      }

	      if (FS.isClosed(stream)) {
	        throw new FS.ErrnoError(8);
	      }

	      if ((stream.flags & 2097155) === 1) {
	        throw new FS.ErrnoError(8);
	      }

	      if (FS.isDir(stream.node.mode)) {
	        throw new FS.ErrnoError(31);
	      }

	      if (!stream.stream_ops.read) {
	        throw new FS.ErrnoError(28);
	      }

	      var seeking = typeof position != "undefined";

	      if (!seeking) {
	        position = stream.position;
	      } else if (!stream.seekable) {
	        throw new FS.ErrnoError(70);
	      }

	      var bytesRead = stream.stream_ops.read(stream, buffer, offset, length, position);
	      if (!seeking) stream.position += bytesRead;
	      return bytesRead;
	    },
	    write: (stream, buffer, offset, length, position, canOwn) => {
	      if (length < 0 || position < 0) {
	        throw new FS.ErrnoError(28);
	      }

	      if (FS.isClosed(stream)) {
	        throw new FS.ErrnoError(8);
	      }

	      if ((stream.flags & 2097155) === 0) {
	        throw new FS.ErrnoError(8);
	      }

	      if (FS.isDir(stream.node.mode)) {
	        throw new FS.ErrnoError(31);
	      }

	      if (!stream.stream_ops.write) {
	        throw new FS.ErrnoError(28);
	      }

	      if (stream.seekable && stream.flags & 1024) {
	        FS.llseek(stream, 0, 2);
	      }

	      var seeking = typeof position != "undefined";

	      if (!seeking) {
	        position = stream.position;
	      } else if (!stream.seekable) {
	        throw new FS.ErrnoError(70);
	      }

	      var bytesWritten = stream.stream_ops.write(stream, buffer, offset, length, position, canOwn);
	      if (!seeking) stream.position += bytesWritten;
	      return bytesWritten;
	    },
	    allocate: (stream, offset, length) => {
	      if (FS.isClosed(stream)) {
	        throw new FS.ErrnoError(8);
	      }

	      if (offset < 0 || length <= 0) {
	        throw new FS.ErrnoError(28);
	      }

	      if ((stream.flags & 2097155) === 0) {
	        throw new FS.ErrnoError(8);
	      }

	      if (!FS.isFile(stream.node.mode) && !FS.isDir(stream.node.mode)) {
	        throw new FS.ErrnoError(43);
	      }

	      if (!stream.stream_ops.allocate) {
	        throw new FS.ErrnoError(138);
	      }

	      stream.stream_ops.allocate(stream, offset, length);
	    },
	    mmap: (stream, length, position, prot, flags) => {
	      if ((prot & 2) !== 0 && (flags & 2) === 0 && (stream.flags & 2097155) !== 2) {
	        throw new FS.ErrnoError(2);
	      }

	      if ((stream.flags & 2097155) === 1) {
	        throw new FS.ErrnoError(2);
	      }

	      if (!stream.stream_ops.mmap) {
	        throw new FS.ErrnoError(43);
	      }

	      return stream.stream_ops.mmap(stream, length, position, prot, flags);
	    },
	    msync: (stream, buffer, offset, length, mmapFlags) => {
	      if (!stream || !stream.stream_ops.msync) {
	        return 0;
	      }

	      return stream.stream_ops.msync(stream, buffer, offset, length, mmapFlags);
	    },
	    munmap: stream => 0,
	    ioctl: (stream, cmd, arg) => {
	      if (!stream.stream_ops.ioctl) {
	        throw new FS.ErrnoError(59);
	      }

	      return stream.stream_ops.ioctl(stream, cmd, arg);
	    },
	    readFile: function (path) {
	      let opts = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
	      opts.flags = opts.flags || 0;
	      opts.encoding = opts.encoding || "binary";

	      if (opts.encoding !== "utf8" && opts.encoding !== "binary") {
	        throw new Error('Invalid encoding type "' + opts.encoding + '"');
	      }

	      var ret;
	      var stream = FS.open(path, opts.flags);
	      var stat = FS.stat(path);
	      var length = stat.size;
	      var buf = new Uint8Array(length);
	      FS.read(stream, buf, 0, length, 0);

	      if (opts.encoding === "utf8") {
	        ret = UTF8ArrayToString(buf, 0);
	      } else if (opts.encoding === "binary") {
	        ret = buf;
	      }

	      FS.close(stream);
	      return ret;
	    },
	    writeFile: function (path, data) {
	      let opts = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : {};
	      opts.flags = opts.flags || 577;
	      var stream = FS.open(path, opts.flags, opts.mode);

	      if (typeof data == "string") {
	        var buf = new Uint8Array(lengthBytesUTF8(data) + 1);
	        var actualNumBytes = stringToUTF8Array(data, buf, 0, buf.length);
	        FS.write(stream, buf, 0, actualNumBytes, undefined, opts.canOwn);
	      } else if (ArrayBuffer.isView(data)) {
	        FS.write(stream, data, 0, data.byteLength, undefined, opts.canOwn);
	      } else {
	        throw new Error("Unsupported data type");
	      }

	      FS.close(stream);
	    },
	    cwd: () => FS.currentPath,
	    chdir: path => {
	      var lookup = FS.lookupPath(path, {
	        follow: true
	      });

	      if (lookup.node === null) {
	        throw new FS.ErrnoError(44);
	      }

	      if (!FS.isDir(lookup.node.mode)) {
	        throw new FS.ErrnoError(54);
	      }

	      var errCode = FS.nodePermissions(lookup.node, "x");

	      if (errCode) {
	        throw new FS.ErrnoError(errCode);
	      }

	      FS.currentPath = lookup.path;
	    },
	    createDefaultDirectories: () => {
	      FS.mkdir("/tmp");
	      FS.mkdir("/home");
	      FS.mkdir("/home/web_user");
	    },
	    createDefaultDevices: () => {
	      FS.mkdir("/dev");
	      FS.registerDevice(FS.makedev(1, 3), {
	        read: () => 0,
	        write: (stream, buffer, offset, length, pos) => length
	      });
	      FS.mkdev("/dev/null", FS.makedev(1, 3));
	      TTY.register(FS.makedev(5, 0), TTY.default_tty_ops);
	      TTY.register(FS.makedev(6, 0), TTY.default_tty1_ops);
	      FS.mkdev("/dev/tty", FS.makedev(5, 0));
	      FS.mkdev("/dev/tty1", FS.makedev(6, 0));
	      var random_device = getRandomDevice();
	      FS.createDevice("/dev", "random", random_device);
	      FS.createDevice("/dev", "urandom", random_device);
	      FS.mkdir("/dev/shm");
	      FS.mkdir("/dev/shm/tmp");
	    },
	    createSpecialDirectories: () => {
	      FS.mkdir("/proc");
	      var proc_self = FS.mkdir("/proc/self");
	      FS.mkdir("/proc/self/fd");
	      FS.mount({
	        mount: () => {
	          var node = FS.createNode(proc_self, "fd", 16384 | 511, 73);
	          node.node_ops = {
	            lookup: (parent, name) => {
	              var fd = +name;
	              var stream = FS.getStream(fd);
	              if (!stream) throw new FS.ErrnoError(8);
	              var ret = {
	                parent: null,
	                mount: {
	                  mountpoint: "fake"
	                },
	                node_ops: {
	                  readlink: () => stream.path
	                }
	              };
	              ret.parent = ret;
	              return ret;
	            }
	          };
	          return node;
	        }
	      }, {}, "/proc/self/fd");
	    },
	    createStandardStreams: () => {
	      if (Module["stdin"]) {
	        FS.createDevice("/dev", "stdin", Module["stdin"]);
	      } else {
	        FS.symlink("/dev/tty", "/dev/stdin");
	      }

	      if (Module["stdout"]) {
	        FS.createDevice("/dev", "stdout", null, Module["stdout"]);
	      } else {
	        FS.symlink("/dev/tty", "/dev/stdout");
	      }

	      if (Module["stderr"]) {
	        FS.createDevice("/dev", "stderr", null, Module["stderr"]);
	      } else {
	        FS.symlink("/dev/tty1", "/dev/stderr");
	      }

	      FS.open("/dev/stdin", 0);
	      FS.open("/dev/stdout", 1);
	      FS.open("/dev/stderr", 1);
	    },
	    ensureErrnoError: () => {
	      if (FS.ErrnoError) return;

	      FS.ErrnoError = function ErrnoError(errno, node) {
	        this.node = node;

	        this.setErrno = function (errno) {
	          this.errno = errno;
	        };

	        this.setErrno(errno);
	        this.message = "FS error";
	      };

	      FS.ErrnoError.prototype = new Error();
	      FS.ErrnoError.prototype.constructor = FS.ErrnoError;
	      [44].forEach(code => {
	        FS.genericErrors[code] = new FS.ErrnoError(code);
	        FS.genericErrors[code].stack = "<generic error, no stack>";
	      });
	    },
	    staticInit: () => {
	      FS.ensureErrnoError();
	      FS.nameTable = new Array(4096);
	      FS.mount(MEMFS, {}, "/");
	      FS.createDefaultDirectories();
	      FS.createDefaultDevices();
	      FS.createSpecialDirectories();
	      FS.filesystems = {
	        "MEMFS": MEMFS
	      };
	    },
	    init: (input, output, error) => {
	      FS.init.initialized = true;
	      FS.ensureErrnoError();
	      Module["stdin"] = input || Module["stdin"];
	      Module["stdout"] = output || Module["stdout"];
	      Module["stderr"] = error || Module["stderr"];
	      FS.createStandardStreams();
	    },
	    quit: () => {
	      FS.init.initialized = false;

	      for (var i = 0; i < FS.streams.length; i++) {
	        var stream = FS.streams[i];

	        if (!stream) {
	          continue;
	        }

	        FS.close(stream);
	      }
	    },
	    getMode: (canRead, canWrite) => {
	      var mode = 0;
	      if (canRead) mode |= 292 | 73;
	      if (canWrite) mode |= 146;
	      return mode;
	    },
	    findObject: (path, dontResolveLastLink) => {
	      var ret = FS.analyzePath(path, dontResolveLastLink);

	      if (!ret.exists) {
	        return null;
	      }

	      return ret.object;
	    },
	    analyzePath: (path, dontResolveLastLink) => {
	      try {
	        var lookup = FS.lookupPath(path, {
	          follow: !dontResolveLastLink
	        });
	        path = lookup.path;
	      } catch (e) {}

	      var ret = {
	        isRoot: false,
	        exists: false,
	        error: 0,
	        name: null,
	        path: null,
	        object: null,
	        parentExists: false,
	        parentPath: null,
	        parentObject: null
	      };

	      try {
	        var lookup = FS.lookupPath(path, {
	          parent: true
	        });
	        ret.parentExists = true;
	        ret.parentPath = lookup.path;
	        ret.parentObject = lookup.node;
	        ret.name = PATH.basename(path);
	        lookup = FS.lookupPath(path, {
	          follow: !dontResolveLastLink
	        });
	        ret.exists = true;
	        ret.path = lookup.path;
	        ret.object = lookup.node;
	        ret.name = lookup.node.name;
	        ret.isRoot = lookup.path === "/";
	      } catch (e) {
	        ret.error = e.errno;
	      }

	      return ret;
	    },
	    createPath: (parent, path, canRead, canWrite) => {
	      parent = typeof parent == "string" ? parent : FS.getPath(parent);
	      var parts = path.split("/").reverse();

	      while (parts.length) {
	        var part = parts.pop();
	        if (!part) continue;
	        var current = PATH.join2(parent, part);

	        try {
	          FS.mkdir(current);
	        } catch (e) {}

	        parent = current;
	      }

	      return current;
	    },
	    createFile: (parent, name, properties, canRead, canWrite) => {
	      var path = PATH.join2(typeof parent == "string" ? parent : FS.getPath(parent), name);
	      var mode = FS.getMode(canRead, canWrite);
	      return FS.create(path, mode);
	    },
	    createDataFile: (parent, name, data, canRead, canWrite, canOwn) => {
	      var path = name;

	      if (parent) {
	        parent = typeof parent == "string" ? parent : FS.getPath(parent);
	        path = name ? PATH.join2(parent, name) : parent;
	      }

	      var mode = FS.getMode(canRead, canWrite);
	      var node = FS.create(path, mode);

	      if (data) {
	        if (typeof data == "string") {
	          var arr = new Array(data.length);

	          for (var i = 0, len = data.length; i < len; ++i) arr[i] = data.charCodeAt(i);

	          data = arr;
	        }

	        FS.chmod(node, mode | 146);
	        var stream = FS.open(node, 577);
	        FS.write(stream, data, 0, data.length, 0, canOwn);
	        FS.close(stream);
	        FS.chmod(node, mode);
	      }

	      return node;
	    },
	    createDevice: (parent, name, input, output) => {
	      var path = PATH.join2(typeof parent == "string" ? parent : FS.getPath(parent), name);
	      var mode = FS.getMode(!!input, !!output);
	      if (!FS.createDevice.major) FS.createDevice.major = 64;
	      var dev = FS.makedev(FS.createDevice.major++, 0);
	      FS.registerDevice(dev, {
	        open: stream => {
	          stream.seekable = false;
	        },
	        close: stream => {
	          if (output && output.buffer && output.buffer.length) {
	            output(10);
	          }
	        },
	        read: (stream, buffer, offset, length, pos) => {
	          var bytesRead = 0;

	          for (var i = 0; i < length; i++) {
	            var result;

	            try {
	              result = input();
	            } catch (e) {
	              throw new FS.ErrnoError(29);
	            }

	            if (result === undefined && bytesRead === 0) {
	              throw new FS.ErrnoError(6);
	            }

	            if (result === null || result === undefined) break;
	            bytesRead++;
	            buffer[offset + i] = result;
	          }

	          if (bytesRead) {
	            stream.node.timestamp = Date.now();
	          }

	          return bytesRead;
	        },
	        write: (stream, buffer, offset, length, pos) => {
	          for (var i = 0; i < length; i++) {
	            try {
	              output(buffer[offset + i]);
	            } catch (e) {
	              throw new FS.ErrnoError(29);
	            }
	          }

	          if (length) {
	            stream.node.timestamp = Date.now();
	          }

	          return i;
	        }
	      });
	      return FS.mkdev(path, mode, dev);
	    },
	    forceLoadFile: obj => {
	      if (obj.isDevice || obj.isFolder || obj.link || obj.contents) return true;

	      if (typeof XMLHttpRequest != "undefined") {
	        throw new Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.");
	      } else if (read_) {
	        try {
	          obj.contents = intArrayFromString(read_(obj.url), true);
	          obj.usedBytes = obj.contents.length;
	        } catch (e) {
	          throw new FS.ErrnoError(29);
	        }
	      } else {
	        throw new Error("Cannot load without read() or XMLHttpRequest.");
	      }
	    },
	    createLazyFile: (parent, name, url, canRead, canWrite) => {
	      function LazyUint8Array() {
	        this.lengthKnown = false;
	        this.chunks = [];
	      }

	      LazyUint8Array.prototype.get = function LazyUint8Array_get(idx) {
	        if (idx > this.length - 1 || idx < 0) {
	          return undefined;
	        }

	        var chunkOffset = idx % this.chunkSize;
	        var chunkNum = idx / this.chunkSize | 0;
	        return this.getter(chunkNum)[chunkOffset];
	      };

	      LazyUint8Array.prototype.setDataGetter = function LazyUint8Array_setDataGetter(getter) {
	        this.getter = getter;
	      };

	      LazyUint8Array.prototype.cacheLength = function LazyUint8Array_cacheLength() {
	        var xhr = new XMLHttpRequest();
	        xhr.open("HEAD", url, false);
	        xhr.send(null);
	        if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
	        var datalength = Number(xhr.getResponseHeader("Content-length"));
	        var header;
	        var hasByteServing = (header = xhr.getResponseHeader("Accept-Ranges")) && header === "bytes";
	        var usesGzip = (header = xhr.getResponseHeader("Content-Encoding")) && header === "gzip";
	        var chunkSize = 1024 * 1024;
	        if (!hasByteServing) chunkSize = datalength;

	        var doXHR = (from, to) => {
	          if (from > to) throw new Error("invalid range (" + from + ", " + to + ") or no bytes requested!");
	          if (to > datalength - 1) throw new Error("only " + datalength + " bytes available! programmer error!");
	          var xhr = new XMLHttpRequest();
	          xhr.open("GET", url, false);
	          if (datalength !== chunkSize) xhr.setRequestHeader("Range", "bytes=" + from + "-" + to);
	          xhr.responseType = "arraybuffer";

	          if (xhr.overrideMimeType) {
	            xhr.overrideMimeType("text/plain; charset=x-user-defined");
	          }

	          xhr.send(null);
	          if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);

	          if (xhr.response !== undefined) {
	            return new Uint8Array(xhr.response || []);
	          }

	          return intArrayFromString(xhr.responseText || "", true);
	        };

	        var lazyArray = this;
	        lazyArray.setDataGetter(chunkNum => {
	          var start = chunkNum * chunkSize;
	          var end = (chunkNum + 1) * chunkSize - 1;
	          end = Math.min(end, datalength - 1);

	          if (typeof lazyArray.chunks[chunkNum] == "undefined") {
	            lazyArray.chunks[chunkNum] = doXHR(start, end);
	          }

	          if (typeof lazyArray.chunks[chunkNum] == "undefined") throw new Error("doXHR failed!");
	          return lazyArray.chunks[chunkNum];
	        });

	        if (usesGzip || !datalength) {
	          chunkSize = datalength = 1;
	          datalength = this.getter(0).length;
	          chunkSize = datalength;
	          out("LazyFiles on gzip forces download of the whole file when length is accessed");
	        }

	        this._length = datalength;
	        this._chunkSize = chunkSize;
	        this.lengthKnown = true;
	      };

	      if (typeof XMLHttpRequest != "undefined") {
	        if (!ENVIRONMENT_IS_WORKER) throw "Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc";
	        var lazyArray = new LazyUint8Array();
	        Object.defineProperties(lazyArray, {
	          length: {
	            get: function () {
	              if (!this.lengthKnown) {
	                this.cacheLength();
	              }

	              return this._length;
	            }
	          },
	          chunkSize: {
	            get: function () {
	              if (!this.lengthKnown) {
	                this.cacheLength();
	              }

	              return this._chunkSize;
	            }
	          }
	        });
	        var properties = {
	          isDevice: false,
	          contents: lazyArray
	        };
	      } else {
	        var properties = {
	          isDevice: false,
	          url: url
	        };
	      }

	      var node = FS.createFile(parent, name, properties, canRead, canWrite);

	      if (properties.contents) {
	        node.contents = properties.contents;
	      } else if (properties.url) {
	        node.contents = null;
	        node.url = properties.url;
	      }

	      Object.defineProperties(node, {
	        usedBytes: {
	          get: function () {
	            return this.contents.length;
	          }
	        }
	      });
	      var stream_ops = {};
	      var keys = Object.keys(node.stream_ops);
	      keys.forEach(key => {
	        var fn = node.stream_ops[key];

	        stream_ops[key] = function forceLoadLazyFile() {
	          FS.forceLoadFile(node);
	          return fn.apply(null, arguments);
	        };
	      });

	      function writeChunks(stream, buffer, offset, length, position) {
	        var contents = stream.node.contents;
	        if (position >= contents.length) return 0;
	        var size = Math.min(contents.length - position, length);

	        if (contents.slice) {
	          for (var i = 0; i < size; i++) {
	            buffer[offset + i] = contents[position + i];
	          }
	        } else {
	          for (var i = 0; i < size; i++) {
	            buffer[offset + i] = contents.get(position + i);
	          }
	        }

	        return size;
	      }

	      stream_ops.read = (stream, buffer, offset, length, position) => {
	        FS.forceLoadFile(node);
	        return writeChunks(stream, buffer, offset, length, position);
	      };

	      stream_ops.mmap = (stream, length, position, prot, flags) => {
	        FS.forceLoadFile(node);
	        var ptr = mmapAlloc(length);

	        if (!ptr) {
	          throw new FS.ErrnoError(48);
	        }

	        writeChunks(stream, HEAP8, ptr, length, position);
	        return {
	          ptr: ptr,
	          allocated: true
	        };
	      };

	      node.stream_ops = stream_ops;
	      return node;
	    },
	    createPreloadedFile: (parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) => {
	      var fullname = name ? PATH_FS.resolve(PATH.join2(parent, name)) : parent;

	      function processData(byteArray) {
	        function finish(byteArray) {
	          if (preFinish) preFinish();

	          if (!dontCreateFile) {
	            FS.createDataFile(parent, name, byteArray, canRead, canWrite, canOwn);
	          }

	          if (onload) onload();
	          removeRunDependency();
	        }

	        if (Browser.handledByPreloadPlugin(byteArray, fullname, finish, () => {
	          if (onerror) onerror();
	          removeRunDependency();
	        })) {
	          return;
	        }

	        finish(byteArray);
	      }

	      addRunDependency();

	      if (typeof url == "string") {
	        asyncLoad(url, byteArray => processData(byteArray), onerror);
	      } else {
	        processData(url);
	      }
	    },
	    indexedDB: () => {
	      return window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
	    },
	    DB_NAME: () => {
	      return "EM_FS_" + window.location.pathname;
	    },
	    DB_VERSION: 20,
	    DB_STORE_NAME: "FILE_DATA",
	    saveFilesToDB: (paths, onload, onerror) => {
	      onload = onload || (() => {});

	      onerror = onerror || (() => {});

	      var indexedDB = FS.indexedDB();

	      try {
	        var openRequest = indexedDB.open(FS.DB_NAME(), FS.DB_VERSION);
	      } catch (e) {
	        return onerror(e);
	      }

	      openRequest.onupgradeneeded = () => {
	        out("creating db");
	        var db = openRequest.result;
	        db.createObjectStore(FS.DB_STORE_NAME);
	      };

	      openRequest.onsuccess = () => {
	        var db = openRequest.result;
	        var transaction = db.transaction([FS.DB_STORE_NAME], "readwrite");
	        var files = transaction.objectStore(FS.DB_STORE_NAME);
	        var ok = 0,
	            fail = 0,
	            total = paths.length;

	        function finish() {
	          if (fail == 0) onload();else onerror();
	        }

	        paths.forEach(path => {
	          var putRequest = files.put(FS.analyzePath(path).object.contents, path);

	          putRequest.onsuccess = () => {
	            ok++;
	            if (ok + fail == total) finish();
	          };

	          putRequest.onerror = () => {
	            fail++;
	            if (ok + fail == total) finish();
	          };
	        });
	        transaction.onerror = onerror;
	      };

	      openRequest.onerror = onerror;
	    },
	    loadFilesFromDB: (paths, onload, onerror) => {
	      onload = onload || (() => {});

	      onerror = onerror || (() => {});

	      var indexedDB = FS.indexedDB();

	      try {
	        var openRequest = indexedDB.open(FS.DB_NAME(), FS.DB_VERSION);
	      } catch (e) {
	        return onerror(e);
	      }

	      openRequest.onupgradeneeded = onerror;

	      openRequest.onsuccess = () => {
	        var db = openRequest.result;

	        try {
	          var transaction = db.transaction([FS.DB_STORE_NAME], "readonly");
	        } catch (e) {
	          onerror(e);
	          return;
	        }

	        var files = transaction.objectStore(FS.DB_STORE_NAME);
	        var ok = 0,
	            fail = 0,
	            total = paths.length;

	        function finish() {
	          if (fail == 0) onload();else onerror();
	        }

	        paths.forEach(path => {
	          var getRequest = files.get(path);

	          getRequest.onsuccess = () => {
	            if (FS.analyzePath(path).exists) {
	              FS.unlink(path);
	            }

	            FS.createDataFile(PATH.dirname(path), PATH.basename(path), getRequest.result, true, true, true);
	            ok++;
	            if (ok + fail == total) finish();
	          };

	          getRequest.onerror = () => {
	            fail++;
	            if (ok + fail == total) finish();
	          };
	        });
	        transaction.onerror = onerror;
	      };

	      openRequest.onerror = onerror;
	    }
	  };
	  var SYSCALLS = {
	    DEFAULT_POLLMASK: 5,
	    calculateAt: function (dirfd, path, allowEmpty) {
	      if (PATH.isAbs(path)) {
	        return path;
	      }

	      var dir;

	      if (dirfd === -100) {
	        dir = FS.cwd();
	      } else {
	        var dirstream = FS.getStream(dirfd);
	        if (!dirstream) throw new FS.ErrnoError(8);
	        dir = dirstream.path;
	      }

	      if (path.length == 0) {
	        if (!allowEmpty) {
	          throw new FS.ErrnoError(44);
	        }

	        return dir;
	      }

	      return PATH.join2(dir, path);
	    },
	    doStat: function (func, path, buf) {
	      try {
	        var stat = func(path);
	      } catch (e) {
	        if (e && e.node && PATH.normalize(path) !== PATH.normalize(FS.getPath(e.node))) {
	          return -54;
	        }

	        throw e;
	      }

	      HEAP32[buf >> 2] = stat.dev;
	      HEAP32[buf + 4 >> 2] = 0;
	      HEAP32[buf + 8 >> 2] = stat.ino;
	      HEAP32[buf + 12 >> 2] = stat.mode;
	      HEAP32[buf + 16 >> 2] = stat.nlink;
	      HEAP32[buf + 20 >> 2] = stat.uid;
	      HEAP32[buf + 24 >> 2] = stat.gid;
	      HEAP32[buf + 28 >> 2] = stat.rdev;
	      HEAP32[buf + 32 >> 2] = 0;
	      tempI64 = [stat.size >>> 0, (tempDouble = stat.size, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)], HEAP32[buf + 40 >> 2] = tempI64[0], HEAP32[buf + 44 >> 2] = tempI64[1];
	      HEAP32[buf + 48 >> 2] = 4096;
	      HEAP32[buf + 52 >> 2] = stat.blocks;
	      tempI64 = [Math.floor(stat.atime.getTime() / 1e3) >>> 0, (tempDouble = Math.floor(stat.atime.getTime() / 1e3), +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)], HEAP32[buf + 56 >> 2] = tempI64[0], HEAP32[buf + 60 >> 2] = tempI64[1];
	      HEAP32[buf + 64 >> 2] = 0;
	      tempI64 = [Math.floor(stat.mtime.getTime() / 1e3) >>> 0, (tempDouble = Math.floor(stat.mtime.getTime() / 1e3), +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)], HEAP32[buf + 72 >> 2] = tempI64[0], HEAP32[buf + 76 >> 2] = tempI64[1];
	      HEAP32[buf + 80 >> 2] = 0;
	      tempI64 = [Math.floor(stat.ctime.getTime() / 1e3) >>> 0, (tempDouble = Math.floor(stat.ctime.getTime() / 1e3), +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)], HEAP32[buf + 88 >> 2] = tempI64[0], HEAP32[buf + 92 >> 2] = tempI64[1];
	      HEAP32[buf + 96 >> 2] = 0;
	      tempI64 = [stat.ino >>> 0, (tempDouble = stat.ino, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)], HEAP32[buf + 104 >> 2] = tempI64[0], HEAP32[buf + 108 >> 2] = tempI64[1];
	      return 0;
	    },
	    doMsync: function (addr, stream, len, flags, offset) {
	      var buffer = HEAPU8.slice(addr, addr + len);
	      FS.msync(stream, buffer, offset, len, flags);
	    },
	    varargs: undefined,
	    get: function () {
	      SYSCALLS.varargs += 4;
	      var ret = HEAP32[SYSCALLS.varargs - 4 >> 2];
	      return ret;
	    },
	    getStr: function (ptr) {
	      var ret = UTF8ToString(ptr);
	      return ret;
	    },
	    getStreamFromFD: function (fd) {
	      var stream = FS.getStream(fd);
	      if (!stream) throw new FS.ErrnoError(8);
	      return stream;
	    }
	  };

	  function ___syscall_fcntl64(fd, cmd, varargs) {
	    SYSCALLS.varargs = varargs;

	    try {
	      var stream = SYSCALLS.getStreamFromFD(fd);

	      switch (cmd) {
	        case 0:
	          {
	            var arg = SYSCALLS.get();

	            if (arg < 0) {
	              return -28;
	            }

	            var newStream;
	            newStream = FS.createStream(stream, arg);
	            return newStream.fd;
	          }

	        case 1:
	        case 2:
	          return 0;

	        case 3:
	          return stream.flags;

	        case 4:
	          {
	            var arg = SYSCALLS.get();
	            stream.flags |= arg;
	            return 0;
	          }

	        case 5:
	          {
	            var arg = SYSCALLS.get();
	            var offset = 0;
	            HEAP16[arg + offset >> 1] = 2;
	            return 0;
	          }

	        case 6:
	        case 7:
	          return 0;

	        case 16:
	        case 8:
	          return -28;

	        case 9:
	          setErrNo(28);
	          return -1;

	        default:
	          {
	            return -28;
	          }
	      }
	    } catch (e) {
	      if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError)) throw e;
	      return -e.errno;
	    }
	  }

	  function ___syscall_openat(dirfd, path, flags, varargs) {
	    SYSCALLS.varargs = varargs;

	    try {
	      path = SYSCALLS.getStr(path);
	      path = SYSCALLS.calculateAt(dirfd, path);
	      var mode = varargs ? SYSCALLS.get() : 0;
	      return FS.open(path, flags, mode).fd;
	    } catch (e) {
	      if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError)) throw e;
	      return -e.errno;
	    }
	  }

	  function __embind_register_bigint(primitiveType, name, size, minRange, maxRange) {}

	  function getShiftFromSize(size) {
	    switch (size) {
	      case 1:
	        return 0;

	      case 2:
	        return 1;

	      case 4:
	        return 2;

	      case 8:
	        return 3;

	      default:
	        throw new TypeError("Unknown type size: " + size);
	    }
	  }

	  function embind_init_charCodes() {
	    var codes = new Array(256);

	    for (var i = 0; i < 256; ++i) {
	      codes[i] = String.fromCharCode(i);
	    }

	    embind_charCodes = codes;
	  }

	  var embind_charCodes = undefined;

	  function readLatin1String(ptr) {
	    var ret = "";
	    var c = ptr;

	    while (HEAPU8[c]) {
	      ret += embind_charCodes[HEAPU8[c++]];
	    }

	    return ret;
	  }

	  var awaitingDependencies = {};
	  var registeredTypes = {};
	  var typeDependencies = {};
	  var char_0 = 48;
	  var char_9 = 57;

	  function makeLegalFunctionName(name) {
	    if (undefined === name) {
	      return "_unknown";
	    }

	    name = name.replace(/[^a-zA-Z0-9_]/g, "$");
	    var f = name.charCodeAt(0);

	    if (f >= char_0 && f <= char_9) {
	      return "_" + name;
	    }

	    return name;
	  }

	  function createNamedFunction(name, body) {
	    name = makeLegalFunctionName(name);
	    return new Function("body", "return function " + name + "() {\n" + '    "use strict";' + "    return body.apply(this, arguments);\n" + "};\n")(body);
	  }

	  function extendError(baseErrorType, errorName) {
	    var errorClass = createNamedFunction(errorName, function (message) {
	      this.name = errorName;
	      this.message = message;
	      var stack = new Error(message).stack;

	      if (stack !== undefined) {
	        this.stack = this.toString() + "\n" + stack.replace(/^Error(:[^\n]*)?\n/, "");
	      }
	    });
	    errorClass.prototype = Object.create(baseErrorType.prototype);
	    errorClass.prototype.constructor = errorClass;

	    errorClass.prototype.toString = function () {
	      if (this.message === undefined) {
	        return this.name;
	      } else {
	        return this.name + ": " + this.message;
	      }
	    };

	    return errorClass;
	  }

	  var BindingError = undefined;

	  function throwBindingError(message) {
	    throw new BindingError(message);
	  }

	  var InternalError = undefined;

	  function throwInternalError(message) {
	    throw new InternalError(message);
	  }

	  function whenDependentTypesAreResolved(myTypes, dependentTypes, getTypeConverters) {
	    myTypes.forEach(function (type) {
	      typeDependencies[type] = dependentTypes;
	    });

	    function onComplete(typeConverters) {
	      var myTypeConverters = getTypeConverters(typeConverters);

	      if (myTypeConverters.length !== myTypes.length) {
	        throwInternalError("Mismatched type converter count");
	      }

	      for (var i = 0; i < myTypes.length; ++i) {
	        registerType(myTypes[i], myTypeConverters[i]);
	      }
	    }

	    var typeConverters = new Array(dependentTypes.length);
	    var unregisteredTypes = [];
	    var registered = 0;
	    dependentTypes.forEach((dt, i) => {
	      if (registeredTypes.hasOwnProperty(dt)) {
	        typeConverters[i] = registeredTypes[dt];
	      } else {
	        unregisteredTypes.push(dt);

	        if (!awaitingDependencies.hasOwnProperty(dt)) {
	          awaitingDependencies[dt] = [];
	        }

	        awaitingDependencies[dt].push(() => {
	          typeConverters[i] = registeredTypes[dt];
	          ++registered;

	          if (registered === unregisteredTypes.length) {
	            onComplete(typeConverters);
	          }
	        });
	      }
	    });

	    if (0 === unregisteredTypes.length) {
	      onComplete(typeConverters);
	    }
	  }

	  function registerType(rawType, registeredInstance) {
	    let options = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : {};

	    if (!("argPackAdvance" in registeredInstance)) {
	      throw new TypeError("registerType registeredInstance requires argPackAdvance");
	    }

	    var name = registeredInstance.name;

	    if (!rawType) {
	      throwBindingError('type "' + name + '" must have a positive integer typeid pointer');
	    }

	    if (registeredTypes.hasOwnProperty(rawType)) {
	      if (options.ignoreDuplicateRegistrations) {
	        return;
	      } else {
	        throwBindingError("Cannot register type '" + name + "' twice");
	      }
	    }

	    registeredTypes[rawType] = registeredInstance;
	    delete typeDependencies[rawType];

	    if (awaitingDependencies.hasOwnProperty(rawType)) {
	      var callbacks = awaitingDependencies[rawType];
	      delete awaitingDependencies[rawType];
	      callbacks.forEach(cb => cb());
	    }
	  }

	  function __embind_register_bool(rawType, name, size, trueValue, falseValue) {
	    var shift = getShiftFromSize(size);
	    name = readLatin1String(name);
	    registerType(rawType, {
	      name: name,
	      "fromWireType": function (wt) {
	        return !!wt;
	      },
	      "toWireType": function (destructors, o) {
	        return o ? trueValue : falseValue;
	      },
	      "argPackAdvance": 8,
	      "readValueFromPointer": function (pointer) {
	        var heap;

	        if (size === 1) {
	          heap = HEAP8;
	        } else if (size === 2) {
	          heap = HEAP16;
	        } else if (size === 4) {
	          heap = HEAP32;
	        } else {
	          throw new TypeError("Unknown boolean type size: " + name);
	        }

	        return this["fromWireType"](heap[pointer >> shift]);
	      },
	      destructorFunction: null
	    });
	  }

	  function ClassHandle_isAliasOf(other) {
	    if (!(this instanceof ClassHandle)) {
	      return false;
	    }

	    if (!(other instanceof ClassHandle)) {
	      return false;
	    }

	    var leftClass = this.$$.ptrType.registeredClass;
	    var left = this.$$.ptr;
	    var rightClass = other.$$.ptrType.registeredClass;
	    var right = other.$$.ptr;

	    while (leftClass.baseClass) {
	      left = leftClass.upcast(left);
	      leftClass = leftClass.baseClass;
	    }

	    while (rightClass.baseClass) {
	      right = rightClass.upcast(right);
	      rightClass = rightClass.baseClass;
	    }

	    return leftClass === rightClass && left === right;
	  }

	  function shallowCopyInternalPointer(o) {
	    return {
	      count: o.count,
	      deleteScheduled: o.deleteScheduled,
	      preservePointerOnDelete: o.preservePointerOnDelete,
	      ptr: o.ptr,
	      ptrType: o.ptrType,
	      smartPtr: o.smartPtr,
	      smartPtrType: o.smartPtrType
	    };
	  }

	  function throwInstanceAlreadyDeleted(obj) {
	    function getInstanceTypeName(handle) {
	      return handle.$$.ptrType.registeredClass.name;
	    }

	    throwBindingError(getInstanceTypeName(obj) + " instance already deleted");
	  }

	  var finalizationRegistry = false;

	  function detachFinalizer(handle) {}

	  function runDestructor($$) {
	    if ($$.smartPtr) {
	      $$.smartPtrType.rawDestructor($$.smartPtr);
	    } else {
	      $$.ptrType.registeredClass.rawDestructor($$.ptr);
	    }
	  }

	  function releaseClassHandle($$) {
	    $$.count.value -= 1;
	    var toDelete = 0 === $$.count.value;

	    if (toDelete) {
	      runDestructor($$);
	    }
	  }

	  function downcastPointer(ptr, ptrClass, desiredClass) {
	    if (ptrClass === desiredClass) {
	      return ptr;
	    }

	    if (undefined === desiredClass.baseClass) {
	      return null;
	    }

	    var rv = downcastPointer(ptr, ptrClass, desiredClass.baseClass);

	    if (rv === null) {
	      return null;
	    }

	    return desiredClass.downcast(rv);
	  }

	  var registeredPointers = {};

	  function getInheritedInstanceCount() {
	    return Object.keys(registeredInstances).length;
	  }

	  function getLiveInheritedInstances() {
	    var rv = [];

	    for (var k in registeredInstances) {
	      if (registeredInstances.hasOwnProperty(k)) {
	        rv.push(registeredInstances[k]);
	      }
	    }

	    return rv;
	  }

	  var deletionQueue = [];

	  function flushPendingDeletes() {
	    while (deletionQueue.length) {
	      var obj = deletionQueue.pop();
	      obj.$$.deleteScheduled = false;
	      obj["delete"]();
	    }
	  }

	  var delayFunction = undefined;

	  function setDelayFunction(fn) {
	    delayFunction = fn;

	    if (deletionQueue.length && delayFunction) {
	      delayFunction(flushPendingDeletes);
	    }
	  }

	  function init_embind() {
	    Module["getInheritedInstanceCount"] = getInheritedInstanceCount;
	    Module["getLiveInheritedInstances"] = getLiveInheritedInstances;
	    Module["flushPendingDeletes"] = flushPendingDeletes;
	    Module["setDelayFunction"] = setDelayFunction;
	  }

	  var registeredInstances = {};

	  function getBasestPointer(class_, ptr) {
	    if (ptr === undefined) {
	      throwBindingError("ptr should not be undefined");
	    }

	    while (class_.baseClass) {
	      ptr = class_.upcast(ptr);
	      class_ = class_.baseClass;
	    }

	    return ptr;
	  }

	  function getInheritedInstance(class_, ptr) {
	    ptr = getBasestPointer(class_, ptr);
	    return registeredInstances[ptr];
	  }

	  function makeClassHandle(prototype, record) {
	    if (!record.ptrType || !record.ptr) {
	      throwInternalError("makeClassHandle requires ptr and ptrType");
	    }

	    var hasSmartPtrType = !!record.smartPtrType;
	    var hasSmartPtr = !!record.smartPtr;

	    if (hasSmartPtrType !== hasSmartPtr) {
	      throwInternalError("Both smartPtrType and smartPtr must be specified");
	    }

	    record.count = {
	      value: 1
	    };
	    return attachFinalizer(Object.create(prototype, {
	      $$: {
	        value: record
	      }
	    }));
	  }

	  function RegisteredPointer_fromWireType(ptr) {
	    var rawPointer = this.getPointee(ptr);

	    if (!rawPointer) {
	      this.destructor(ptr);
	      return null;
	    }

	    var registeredInstance = getInheritedInstance(this.registeredClass, rawPointer);

	    if (undefined !== registeredInstance) {
	      if (0 === registeredInstance.$$.count.value) {
	        registeredInstance.$$.ptr = rawPointer;
	        registeredInstance.$$.smartPtr = ptr;
	        return registeredInstance["clone"]();
	      } else {
	        var rv = registeredInstance["clone"]();
	        this.destructor(ptr);
	        return rv;
	      }
	    }

	    function makeDefaultHandle() {
	      if (this.isSmartPointer) {
	        return makeClassHandle(this.registeredClass.instancePrototype, {
	          ptrType: this.pointeeType,
	          ptr: rawPointer,
	          smartPtrType: this,
	          smartPtr: ptr
	        });
	      } else {
	        return makeClassHandle(this.registeredClass.instancePrototype, {
	          ptrType: this,
	          ptr: ptr
	        });
	      }
	    }

	    var actualType = this.registeredClass.getActualType(rawPointer);
	    var registeredPointerRecord = registeredPointers[actualType];

	    if (!registeredPointerRecord) {
	      return makeDefaultHandle.call(this);
	    }

	    var toType;

	    if (this.isConst) {
	      toType = registeredPointerRecord.constPointerType;
	    } else {
	      toType = registeredPointerRecord.pointerType;
	    }

	    var dp = downcastPointer(rawPointer, this.registeredClass, toType.registeredClass);

	    if (dp === null) {
	      return makeDefaultHandle.call(this);
	    }

	    if (this.isSmartPointer) {
	      return makeClassHandle(toType.registeredClass.instancePrototype, {
	        ptrType: toType,
	        ptr: dp,
	        smartPtrType: this,
	        smartPtr: ptr
	      });
	    } else {
	      return makeClassHandle(toType.registeredClass.instancePrototype, {
	        ptrType: toType,
	        ptr: dp
	      });
	    }
	  }

	  function attachFinalizer(handle) {
	    if ("undefined" === typeof FinalizationRegistry) {
	      attachFinalizer = handle => handle;

	      return handle;
	    }

	    finalizationRegistry = new FinalizationRegistry(info => {
	      releaseClassHandle(info.$$);
	    });

	    attachFinalizer = handle => {
	      var $$ = handle.$$;
	      var hasSmartPtr = !!$$.smartPtr;

	      if (hasSmartPtr) {
	        var info = {
	          $$: $$
	        };
	        finalizationRegistry.register(handle, info, handle);
	      }

	      return handle;
	    };

	    detachFinalizer = handle => finalizationRegistry.unregister(handle);

	    return attachFinalizer(handle);
	  }

	  function ClassHandle_clone() {
	    if (!this.$$.ptr) {
	      throwInstanceAlreadyDeleted(this);
	    }

	    if (this.$$.preservePointerOnDelete) {
	      this.$$.count.value += 1;
	      return this;
	    } else {
	      var clone = attachFinalizer(Object.create(Object.getPrototypeOf(this), {
	        $$: {
	          value: shallowCopyInternalPointer(this.$$)
	        }
	      }));
	      clone.$$.count.value += 1;
	      clone.$$.deleteScheduled = false;
	      return clone;
	    }
	  }

	  function ClassHandle_delete() {
	    if (!this.$$.ptr) {
	      throwInstanceAlreadyDeleted(this);
	    }

	    if (this.$$.deleteScheduled && !this.$$.preservePointerOnDelete) {
	      throwBindingError("Object already scheduled for deletion");
	    }

	    detachFinalizer(this);
	    releaseClassHandle(this.$$);

	    if (!this.$$.preservePointerOnDelete) {
	      this.$$.smartPtr = undefined;
	      this.$$.ptr = undefined;
	    }
	  }

	  function ClassHandle_isDeleted() {
	    return !this.$$.ptr;
	  }

	  function ClassHandle_deleteLater() {
	    if (!this.$$.ptr) {
	      throwInstanceAlreadyDeleted(this);
	    }

	    if (this.$$.deleteScheduled && !this.$$.preservePointerOnDelete) {
	      throwBindingError("Object already scheduled for deletion");
	    }

	    deletionQueue.push(this);

	    if (deletionQueue.length === 1 && delayFunction) {
	      delayFunction(flushPendingDeletes);
	    }

	    this.$$.deleteScheduled = true;
	    return this;
	  }

	  function init_ClassHandle() {
	    ClassHandle.prototype["isAliasOf"] = ClassHandle_isAliasOf;
	    ClassHandle.prototype["clone"] = ClassHandle_clone;
	    ClassHandle.prototype["delete"] = ClassHandle_delete;
	    ClassHandle.prototype["isDeleted"] = ClassHandle_isDeleted;
	    ClassHandle.prototype["deleteLater"] = ClassHandle_deleteLater;
	  }

	  function ClassHandle() {}

	  function ensureOverloadTable(proto, methodName, humanName) {
	    if (undefined === proto[methodName].overloadTable) {
	      var prevFunc = proto[methodName];

	      proto[methodName] = function () {
	        if (!proto[methodName].overloadTable.hasOwnProperty(arguments.length)) {
	          throwBindingError("Function '" + humanName + "' called with an invalid number of arguments (" + arguments.length + ") - expects one of (" + proto[methodName].overloadTable + ")!");
	        }

	        return proto[methodName].overloadTable[arguments.length].apply(this, arguments);
	      };

	      proto[methodName].overloadTable = [];
	      proto[methodName].overloadTable[prevFunc.argCount] = prevFunc;
	    }
	  }

	  function exposePublicSymbol(name, value, numArguments) {
	    if (Module.hasOwnProperty(name)) {
	      if (undefined === numArguments || undefined !== Module[name].overloadTable && undefined !== Module[name].overloadTable[numArguments]) {
	        throwBindingError("Cannot register public name '" + name + "' twice");
	      }

	      ensureOverloadTable(Module, name, name);

	      if (Module.hasOwnProperty(numArguments)) {
	        throwBindingError("Cannot register multiple overloads of a function with the same number of arguments (" + numArguments + ")!");
	      }

	      Module[name].overloadTable[numArguments] = value;
	    } else {
	      Module[name] = value;

	      if (undefined !== numArguments) {
	        Module[name].numArguments = numArguments;
	      }
	    }
	  }

	  function RegisteredClass(name, constructor, instancePrototype, rawDestructor, baseClass, getActualType, upcast, downcast) {
	    this.name = name;
	    this.constructor = constructor;
	    this.instancePrototype = instancePrototype;
	    this.rawDestructor = rawDestructor;
	    this.baseClass = baseClass;
	    this.getActualType = getActualType;
	    this.upcast = upcast;
	    this.downcast = downcast;
	    this.pureVirtualFunctions = [];
	  }

	  function upcastPointer(ptr, ptrClass, desiredClass) {
	    while (ptrClass !== desiredClass) {
	      if (!ptrClass.upcast) {
	        throwBindingError("Expected null or instance of " + desiredClass.name + ", got an instance of " + ptrClass.name);
	      }

	      ptr = ptrClass.upcast(ptr);
	      ptrClass = ptrClass.baseClass;
	    }

	    return ptr;
	  }

	  function constNoSmartPtrRawPointerToWireType(destructors, handle) {
	    if (handle === null) {
	      if (this.isReference) {
	        throwBindingError("null is not a valid " + this.name);
	      }

	      return 0;
	    }

	    if (!handle.$$) {
	      throwBindingError('Cannot pass "' + embindRepr(handle) + '" as a ' + this.name);
	    }

	    if (!handle.$$.ptr) {
	      throwBindingError("Cannot pass deleted object as a pointer of type " + this.name);
	    }

	    var handleClass = handle.$$.ptrType.registeredClass;
	    var ptr = upcastPointer(handle.$$.ptr, handleClass, this.registeredClass);
	    return ptr;
	  }

	  function genericPointerToWireType(destructors, handle) {
	    var ptr;

	    if (handle === null) {
	      if (this.isReference) {
	        throwBindingError("null is not a valid " + this.name);
	      }

	      if (this.isSmartPointer) {
	        ptr = this.rawConstructor();

	        if (destructors !== null) {
	          destructors.push(this.rawDestructor, ptr);
	        }

	        return ptr;
	      } else {
	        return 0;
	      }
	    }

	    if (!handle.$$) {
	      throwBindingError('Cannot pass "' + embindRepr(handle) + '" as a ' + this.name);
	    }

	    if (!handle.$$.ptr) {
	      throwBindingError("Cannot pass deleted object as a pointer of type " + this.name);
	    }

	    if (!this.isConst && handle.$$.ptrType.isConst) {
	      throwBindingError("Cannot convert argument of type " + (handle.$$.smartPtrType ? handle.$$.smartPtrType.name : handle.$$.ptrType.name) + " to parameter type " + this.name);
	    }

	    var handleClass = handle.$$.ptrType.registeredClass;
	    ptr = upcastPointer(handle.$$.ptr, handleClass, this.registeredClass);

	    if (this.isSmartPointer) {
	      if (undefined === handle.$$.smartPtr) {
	        throwBindingError("Passing raw pointer to smart pointer is illegal");
	      }

	      switch (this.sharingPolicy) {
	        case 0:
	          if (handle.$$.smartPtrType === this) {
	            ptr = handle.$$.smartPtr;
	          } else {
	            throwBindingError("Cannot convert argument of type " + (handle.$$.smartPtrType ? handle.$$.smartPtrType.name : handle.$$.ptrType.name) + " to parameter type " + this.name);
	          }

	          break;

	        case 1:
	          ptr = handle.$$.smartPtr;
	          break;

	        case 2:
	          if (handle.$$.smartPtrType === this) {
	            ptr = handle.$$.smartPtr;
	          } else {
	            var clonedHandle = handle["clone"]();
	            ptr = this.rawShare(ptr, Emval.toHandle(function () {
	              clonedHandle["delete"]();
	            }));

	            if (destructors !== null) {
	              destructors.push(this.rawDestructor, ptr);
	            }
	          }

	          break;

	        default:
	          throwBindingError("Unsupporting sharing policy");
	      }
	    }

	    return ptr;
	  }

	  function nonConstNoSmartPtrRawPointerToWireType(destructors, handle) {
	    if (handle === null) {
	      if (this.isReference) {
	        throwBindingError("null is not a valid " + this.name);
	      }

	      return 0;
	    }

	    if (!handle.$$) {
	      throwBindingError('Cannot pass "' + embindRepr(handle) + '" as a ' + this.name);
	    }

	    if (!handle.$$.ptr) {
	      throwBindingError("Cannot pass deleted object as a pointer of type " + this.name);
	    }

	    if (handle.$$.ptrType.isConst) {
	      throwBindingError("Cannot convert argument of type " + handle.$$.ptrType.name + " to parameter type " + this.name);
	    }

	    var handleClass = handle.$$.ptrType.registeredClass;
	    var ptr = upcastPointer(handle.$$.ptr, handleClass, this.registeredClass);
	    return ptr;
	  }

	  function simpleReadValueFromPointer(pointer) {
	    return this["fromWireType"](HEAP32[pointer >> 2]);
	  }

	  function RegisteredPointer_getPointee(ptr) {
	    if (this.rawGetPointee) {
	      ptr = this.rawGetPointee(ptr);
	    }

	    return ptr;
	  }

	  function RegisteredPointer_destructor(ptr) {
	    if (this.rawDestructor) {
	      this.rawDestructor(ptr);
	    }
	  }

	  function RegisteredPointer_deleteObject(handle) {
	    if (handle !== null) {
	      handle["delete"]();
	    }
	  }

	  function init_RegisteredPointer() {
	    RegisteredPointer.prototype.getPointee = RegisteredPointer_getPointee;
	    RegisteredPointer.prototype.destructor = RegisteredPointer_destructor;
	    RegisteredPointer.prototype["argPackAdvance"] = 8;
	    RegisteredPointer.prototype["readValueFromPointer"] = simpleReadValueFromPointer;
	    RegisteredPointer.prototype["deleteObject"] = RegisteredPointer_deleteObject;
	    RegisteredPointer.prototype["fromWireType"] = RegisteredPointer_fromWireType;
	  }

	  function RegisteredPointer(name, registeredClass, isReference, isConst, isSmartPointer, pointeeType, sharingPolicy, rawGetPointee, rawConstructor, rawShare, rawDestructor) {
	    this.name = name;
	    this.registeredClass = registeredClass;
	    this.isReference = isReference;
	    this.isConst = isConst;
	    this.isSmartPointer = isSmartPointer;
	    this.pointeeType = pointeeType;
	    this.sharingPolicy = sharingPolicy;
	    this.rawGetPointee = rawGetPointee;
	    this.rawConstructor = rawConstructor;
	    this.rawShare = rawShare;
	    this.rawDestructor = rawDestructor;

	    if (!isSmartPointer && registeredClass.baseClass === undefined) {
	      if (isConst) {
	        this["toWireType"] = constNoSmartPtrRawPointerToWireType;
	        this.destructorFunction = null;
	      } else {
	        this["toWireType"] = nonConstNoSmartPtrRawPointerToWireType;
	        this.destructorFunction = null;
	      }
	    } else {
	      this["toWireType"] = genericPointerToWireType;
	    }
	  }

	  function replacePublicSymbol(name, value, numArguments) {
	    if (!Module.hasOwnProperty(name)) {
	      throwInternalError("Replacing nonexistant public symbol");
	    }

	    if (undefined !== Module[name].overloadTable && undefined !== numArguments) {
	      Module[name].overloadTable[numArguments] = value;
	    } else {
	      Module[name] = value;
	      Module[name].argCount = numArguments;
	    }
	  }

	  function dynCallLegacy(sig, ptr, args) {
	    var f = Module["dynCall_" + sig];
	    return args && args.length ? f.apply(null, [ptr].concat(args)) : f.call(null, ptr);
	  }

	  var wasmTableMirror = [];

	  function getWasmTableEntry(funcPtr) {
	    var func = wasmTableMirror[funcPtr];

	    if (!func) {
	      if (funcPtr >= wasmTableMirror.length) wasmTableMirror.length = funcPtr + 1;
	      wasmTableMirror[funcPtr] = func = wasmTable.get(funcPtr);
	    }

	    return func;
	  }

	  function dynCall(sig, ptr, args) {
	    if (sig.includes("j")) {
	      return dynCallLegacy(sig, ptr, args);
	    }

	    var rtn = getWasmTableEntry(ptr).apply(null, args);
	    return rtn;
	  }

	  function getDynCaller(sig, ptr) {
	    var argCache = [];
	    return function () {
	      argCache.length = 0;
	      Object.assign(argCache, arguments);
	      return dynCall(sig, ptr, argCache);
	    };
	  }

	  function embind__requireFunction(signature, rawFunction) {
	    signature = readLatin1String(signature);

	    function makeDynCaller() {
	      if (signature.includes("j")) {
	        return getDynCaller(signature, rawFunction);
	      }

	      return getWasmTableEntry(rawFunction);
	    }

	    var fp = makeDynCaller();

	    if (typeof fp != "function") {
	      throwBindingError("unknown function pointer with signature " + signature + ": " + rawFunction);
	    }

	    return fp;
	  }

	  var UnboundTypeError = undefined;

	  function getTypeName(type) {
	    var ptr = ___getTypeName(type);

	    var rv = readLatin1String(ptr);

	    _free(ptr);

	    return rv;
	  }

	  function throwUnboundTypeError(message, types) {
	    var unboundTypes = [];
	    var seen = {};

	    function visit(type) {
	      if (seen[type]) {
	        return;
	      }

	      if (registeredTypes[type]) {
	        return;
	      }

	      if (typeDependencies[type]) {
	        typeDependencies[type].forEach(visit);
	        return;
	      }

	      unboundTypes.push(type);
	      seen[type] = true;
	    }

	    types.forEach(visit);
	    throw new UnboundTypeError(message + ": " + unboundTypes.map(getTypeName).join([", "]));
	  }

	  function __embind_register_class(rawType, rawPointerType, rawConstPointerType, baseClassRawType, getActualTypeSignature, getActualType, upcastSignature, upcast, downcastSignature, downcast, name, destructorSignature, rawDestructor) {
	    name = readLatin1String(name);
	    getActualType = embind__requireFunction(getActualTypeSignature, getActualType);

	    if (upcast) {
	      upcast = embind__requireFunction(upcastSignature, upcast);
	    }

	    if (downcast) {
	      downcast = embind__requireFunction(downcastSignature, downcast);
	    }

	    rawDestructor = embind__requireFunction(destructorSignature, rawDestructor);
	    var legalFunctionName = makeLegalFunctionName(name);
	    exposePublicSymbol(legalFunctionName, function () {
	      throwUnboundTypeError("Cannot construct " + name + " due to unbound types", [baseClassRawType]);
	    });
	    whenDependentTypesAreResolved([rawType, rawPointerType, rawConstPointerType], baseClassRawType ? [baseClassRawType] : [], function (base) {
	      base = base[0];
	      var baseClass;
	      var basePrototype;

	      if (baseClassRawType) {
	        baseClass = base.registeredClass;
	        basePrototype = baseClass.instancePrototype;
	      } else {
	        basePrototype = ClassHandle.prototype;
	      }

	      var constructor = createNamedFunction(legalFunctionName, function () {
	        if (Object.getPrototypeOf(this) !== instancePrototype) {
	          throw new BindingError("Use 'new' to construct " + name);
	        }

	        if (undefined === registeredClass.constructor_body) {
	          throw new BindingError(name + " has no accessible constructor");
	        }

	        var body = registeredClass.constructor_body[arguments.length];

	        if (undefined === body) {
	          throw new BindingError("Tried to invoke ctor of " + name + " with invalid number of parameters (" + arguments.length + ") - expected (" + Object.keys(registeredClass.constructor_body).toString() + ") parameters instead!");
	        }

	        return body.apply(this, arguments);
	      });
	      var instancePrototype = Object.create(basePrototype, {
	        constructor: {
	          value: constructor
	        }
	      });
	      constructor.prototype = instancePrototype;
	      var registeredClass = new RegisteredClass(name, constructor, instancePrototype, rawDestructor, baseClass, getActualType, upcast, downcast);
	      var referenceConverter = new RegisteredPointer(name, registeredClass, true, false, false);
	      var pointerConverter = new RegisteredPointer(name + "*", registeredClass, false, false, false);
	      var constPointerConverter = new RegisteredPointer(name + " const*", registeredClass, false, true, false);
	      registeredPointers[rawType] = {
	        pointerType: pointerConverter,
	        constPointerType: constPointerConverter
	      };
	      replacePublicSymbol(legalFunctionName, constructor);
	      return [referenceConverter, pointerConverter, constPointerConverter];
	    });
	  }

	  function heap32VectorToArray(count, firstElement) {
	    var array = [];

	    for (var i = 0; i < count; i++) {
	      array.push(HEAPU32[firstElement + i * 4 >> 2]);
	    }

	    return array;
	  }

	  function runDestructors(destructors) {
	    while (destructors.length) {
	      var ptr = destructors.pop();
	      var del = destructors.pop();
	      del(ptr);
	    }
	  }

	  function new_(constructor, argumentList) {
	    if (!(constructor instanceof Function)) {
	      throw new TypeError("new_ called with constructor type " + typeof constructor + " which is not a function");
	    }

	    var dummy = createNamedFunction(constructor.name || "unknownFunctionName", function () {});
	    dummy.prototype = constructor.prototype;
	    var obj = new dummy();
	    var r = constructor.apply(obj, argumentList);
	    return r instanceof Object ? r : obj;
	  }

	  function craftInvokerFunction(humanName, argTypes, classType, cppInvokerFunc, cppTargetFunc) {
	    var argCount = argTypes.length;

	    if (argCount < 2) {
	      throwBindingError("argTypes array size mismatch! Must at least get return value and 'this' types!");
	    }

	    var isClassMethodFunc = argTypes[1] !== null && classType !== null;
	    var needsDestructorStack = false;

	    for (var i = 1; i < argTypes.length; ++i) {
	      if (argTypes[i] !== null && argTypes[i].destructorFunction === undefined) {
	        needsDestructorStack = true;
	        break;
	      }
	    }

	    var returns = argTypes[0].name !== "void";
	    var argsList = "";
	    var argsListWired = "";

	    for (var i = 0; i < argCount - 2; ++i) {
	      argsList += (i !== 0 ? ", " : "") + "arg" + i;
	      argsListWired += (i !== 0 ? ", " : "") + "arg" + i + "Wired";
	    }

	    var invokerFnBody = "return function " + makeLegalFunctionName(humanName) + "(" + argsList + ") {\n" + "if (arguments.length !== " + (argCount - 2) + ") {\n" + "throwBindingError('function " + humanName + " called with ' + arguments.length + ' arguments, expected " + (argCount - 2) + " args!');\n" + "}\n";

	    if (needsDestructorStack) {
	      invokerFnBody += "var destructors = [];\n";
	    }

	    var dtorStack = needsDestructorStack ? "destructors" : "null";
	    var args1 = ["throwBindingError", "invoker", "fn", "runDestructors", "retType", "classParam"];
	    var args2 = [throwBindingError, cppInvokerFunc, cppTargetFunc, runDestructors, argTypes[0], argTypes[1]];

	    if (isClassMethodFunc) {
	      invokerFnBody += "var thisWired = classParam.toWireType(" + dtorStack + ", this);\n";
	    }

	    for (var i = 0; i < argCount - 2; ++i) {
	      invokerFnBody += "var arg" + i + "Wired = argType" + i + ".toWireType(" + dtorStack + ", arg" + i + "); // " + argTypes[i + 2].name + "\n";
	      args1.push("argType" + i);
	      args2.push(argTypes[i + 2]);
	    }

	    if (isClassMethodFunc) {
	      argsListWired = "thisWired" + (argsListWired.length > 0 ? ", " : "") + argsListWired;
	    }

	    invokerFnBody += (returns ? "var rv = " : "") + "invoker(fn" + (argsListWired.length > 0 ? ", " : "") + argsListWired + ");\n";

	    if (needsDestructorStack) {
	      invokerFnBody += "runDestructors(destructors);\n";
	    } else {
	      for (var i = isClassMethodFunc ? 1 : 2; i < argTypes.length; ++i) {
	        var paramName = i === 1 ? "thisWired" : "arg" + (i - 2) + "Wired";

	        if (argTypes[i].destructorFunction !== null) {
	          invokerFnBody += paramName + "_dtor(" + paramName + "); // " + argTypes[i].name + "\n";
	          args1.push(paramName + "_dtor");
	          args2.push(argTypes[i].destructorFunction);
	        }
	      }
	    }

	    if (returns) {
	      invokerFnBody += "var ret = retType.fromWireType(rv);\n" + "return ret;\n";
	    }

	    invokerFnBody += "}\n";
	    args1.push(invokerFnBody);
	    var invokerFunction = new_(Function, args1).apply(null, args2);
	    return invokerFunction;
	  }

	  function __embind_register_class_constructor(rawClassType, argCount, rawArgTypesAddr, invokerSignature, invoker, rawConstructor) {
	    assert(argCount > 0);
	    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
	    invoker = embind__requireFunction(invokerSignature, invoker);
	    whenDependentTypesAreResolved([], [rawClassType], function (classType) {
	      classType = classType[0];
	      var humanName = "constructor " + classType.name;

	      if (undefined === classType.registeredClass.constructor_body) {
	        classType.registeredClass.constructor_body = [];
	      }

	      if (undefined !== classType.registeredClass.constructor_body[argCount - 1]) {
	        throw new BindingError("Cannot register multiple constructors with identical number of parameters (" + (argCount - 1) + ") for class '" + classType.name + "'! Overload resolution is currently only performed using the parameter count, not actual type info!");
	      }

	      classType.registeredClass.constructor_body[argCount - 1] = () => {
	        throwUnboundTypeError("Cannot construct " + classType.name + " due to unbound types", rawArgTypes);
	      };

	      whenDependentTypesAreResolved([], rawArgTypes, function (argTypes) {
	        argTypes.splice(1, 0, null);
	        classType.registeredClass.constructor_body[argCount - 1] = craftInvokerFunction(humanName, argTypes, null, invoker, rawConstructor);
	        return [];
	      });
	      return [];
	    });
	  }

	  function __embind_register_class_function(rawClassType, methodName, argCount, rawArgTypesAddr, invokerSignature, rawInvoker, context, isPureVirtual) {
	    var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
	    methodName = readLatin1String(methodName);
	    rawInvoker = embind__requireFunction(invokerSignature, rawInvoker);
	    whenDependentTypesAreResolved([], [rawClassType], function (classType) {
	      classType = classType[0];
	      var humanName = classType.name + "." + methodName;

	      if (methodName.startsWith("@@")) {
	        methodName = Symbol[methodName.substring(2)];
	      }

	      if (isPureVirtual) {
	        classType.registeredClass.pureVirtualFunctions.push(methodName);
	      }

	      function unboundTypesHandler() {
	        throwUnboundTypeError("Cannot call " + humanName + " due to unbound types", rawArgTypes);
	      }

	      var proto = classType.registeredClass.instancePrototype;
	      var method = proto[methodName];

	      if (undefined === method || undefined === method.overloadTable && method.className !== classType.name && method.argCount === argCount - 2) {
	        unboundTypesHandler.argCount = argCount - 2;
	        unboundTypesHandler.className = classType.name;
	        proto[methodName] = unboundTypesHandler;
	      } else {
	        ensureOverloadTable(proto, methodName, humanName);
	        proto[methodName].overloadTable[argCount - 2] = unboundTypesHandler;
	      }

	      whenDependentTypesAreResolved([], rawArgTypes, function (argTypes) {
	        var memberFunction = craftInvokerFunction(humanName, argTypes, classType, rawInvoker, context);

	        if (undefined === proto[methodName].overloadTable) {
	          memberFunction.argCount = argCount - 2;
	          proto[methodName] = memberFunction;
	        } else {
	          proto[methodName].overloadTable[argCount - 2] = memberFunction;
	        }

	        return [];
	      });
	      return [];
	    });
	  }

	  var emval_free_list = [];
	  var emval_handle_array = [{}, {
	    value: undefined
	  }, {
	    value: null
	  }, {
	    value: true
	  }, {
	    value: false
	  }];

	  function __emval_decref(handle) {
	    if (handle > 4 && 0 === --emval_handle_array[handle].refcount) {
	      emval_handle_array[handle] = undefined;
	      emval_free_list.push(handle);
	    }
	  }

	  function count_emval_handles() {
	    var count = 0;

	    for (var i = 5; i < emval_handle_array.length; ++i) {
	      if (emval_handle_array[i] !== undefined) {
	        ++count;
	      }
	    }

	    return count;
	  }

	  function get_first_emval() {
	    for (var i = 5; i < emval_handle_array.length; ++i) {
	      if (emval_handle_array[i] !== undefined) {
	        return emval_handle_array[i];
	      }
	    }

	    return null;
	  }

	  function init_emval() {
	    Module["count_emval_handles"] = count_emval_handles;
	    Module["get_first_emval"] = get_first_emval;
	  }

	  var Emval = {
	    toValue: handle => {
	      if (!handle) {
	        throwBindingError("Cannot use deleted val. handle = " + handle);
	      }

	      return emval_handle_array[handle].value;
	    },
	    toHandle: value => {
	      switch (value) {
	        case undefined:
	          return 1;

	        case null:
	          return 2;

	        case true:
	          return 3;

	        case false:
	          return 4;

	        default:
	          {
	            var handle = emval_free_list.length ? emval_free_list.pop() : emval_handle_array.length;
	            emval_handle_array[handle] = {
	              refcount: 1,
	              value: value
	            };
	            return handle;
	          }
	      }
	    }
	  };

	  function __embind_register_emval(rawType, name) {
	    name = readLatin1String(name);
	    registerType(rawType, {
	      name: name,
	      "fromWireType": function (handle) {
	        var rv = Emval.toValue(handle);

	        __emval_decref(handle);

	        return rv;
	      },
	      "toWireType": function (destructors, value) {
	        return Emval.toHandle(value);
	      },
	      "argPackAdvance": 8,
	      "readValueFromPointer": simpleReadValueFromPointer,
	      destructorFunction: null
	    });
	  }

	  function embindRepr(v) {
	    if (v === null) {
	      return "null";
	    }

	    var t = typeof v;

	    if (t === "object" || t === "array" || t === "function") {
	      return v.toString();
	    } else {
	      return "" + v;
	    }
	  }

	  function floatReadValueFromPointer(name, shift) {
	    switch (shift) {
	      case 2:
	        return function (pointer) {
	          return this["fromWireType"](HEAPF32[pointer >> 2]);
	        };

	      case 3:
	        return function (pointer) {
	          return this["fromWireType"](HEAPF64[pointer >> 3]);
	        };

	      default:
	        throw new TypeError("Unknown float type: " + name);
	    }
	  }

	  function __embind_register_float(rawType, name, size) {
	    var shift = getShiftFromSize(size);
	    name = readLatin1String(name);
	    registerType(rawType, {
	      name: name,
	      "fromWireType": function (value) {
	        return value;
	      },
	      "toWireType": function (destructors, value) {
	        return value;
	      },
	      "argPackAdvance": 8,
	      "readValueFromPointer": floatReadValueFromPointer(name, shift),
	      destructorFunction: null
	    });
	  }

	  function integerReadValueFromPointer(name, shift, signed) {
	    switch (shift) {
	      case 0:
	        return signed ? function readS8FromPointer(pointer) {
	          return HEAP8[pointer];
	        } : function readU8FromPointer(pointer) {
	          return HEAPU8[pointer];
	        };

	      case 1:
	        return signed ? function readS16FromPointer(pointer) {
	          return HEAP16[pointer >> 1];
	        } : function readU16FromPointer(pointer) {
	          return HEAPU16[pointer >> 1];
	        };

	      case 2:
	        return signed ? function readS32FromPointer(pointer) {
	          return HEAP32[pointer >> 2];
	        } : function readU32FromPointer(pointer) {
	          return HEAPU32[pointer >> 2];
	        };

	      default:
	        throw new TypeError("Unknown integer type: " + name);
	    }
	  }

	  function __embind_register_integer(primitiveType, name, size, minRange, maxRange) {
	    name = readLatin1String(name);

	    var shift = getShiftFromSize(size);

	    var fromWireType = value => value;

	    if (minRange === 0) {
	      var bitshift = 32 - 8 * size;

	      fromWireType = value => value << bitshift >>> bitshift;
	    }

	    var isUnsignedType = name.includes("unsigned");

	    var checkAssertions = (value, toTypeName) => {};

	    var toWireType;

	    if (isUnsignedType) {
	      toWireType = function (destructors, value) {
	        checkAssertions(value, this.name);
	        return value >>> 0;
	      };
	    } else {
	      toWireType = function (destructors, value) {
	        checkAssertions(value, this.name);
	        return value;
	      };
	    }

	    registerType(primitiveType, {
	      name: name,
	      "fromWireType": fromWireType,
	      "toWireType": toWireType,
	      "argPackAdvance": 8,
	      "readValueFromPointer": integerReadValueFromPointer(name, shift, minRange !== 0),
	      destructorFunction: null
	    });
	  }

	  function __embind_register_memory_view(rawType, dataTypeIndex, name) {
	    var typeMapping = [Int8Array, Uint8Array, Int16Array, Uint16Array, Int32Array, Uint32Array, Float32Array, Float64Array];
	    var TA = typeMapping[dataTypeIndex];

	    function decodeMemoryView(handle) {
	      handle = handle >> 2;
	      var heap = HEAPU32;
	      var size = heap[handle];
	      var data = heap[handle + 1];
	      return new TA(buffer, data, size);
	    }

	    name = readLatin1String(name);
	    registerType(rawType, {
	      name: name,
	      "fromWireType": decodeMemoryView,
	      "argPackAdvance": 8,
	      "readValueFromPointer": decodeMemoryView
	    }, {
	      ignoreDuplicateRegistrations: true
	    });
	  }

	  function __embind_register_std_string(rawType, name) {
	    name = readLatin1String(name);
	    var stdStringIsUTF8 = name === "std::string";
	    registerType(rawType, {
	      name: name,
	      "fromWireType": function (value) {
	        var length = HEAPU32[value >> 2];
	        var payload = value + 4;
	        var str;

	        if (stdStringIsUTF8) {
	          var decodeStartPtr = payload;

	          for (var i = 0; i <= length; ++i) {
	            var currentBytePtr = payload + i;

	            if (i == length || HEAPU8[currentBytePtr] == 0) {
	              var maxRead = currentBytePtr - decodeStartPtr;
	              var stringSegment = UTF8ToString(decodeStartPtr, maxRead);

	              if (str === undefined) {
	                str = stringSegment;
	              } else {
	                str += String.fromCharCode(0);
	                str += stringSegment;
	              }

	              decodeStartPtr = currentBytePtr + 1;
	            }
	          }
	        } else {
	          var a = new Array(length);

	          for (var i = 0; i < length; ++i) {
	            a[i] = String.fromCharCode(HEAPU8[payload + i]);
	          }

	          str = a.join("");
	        }

	        _free(value);

	        return str;
	      },
	      "toWireType": function (destructors, value) {
	        if (value instanceof ArrayBuffer) {
	          value = new Uint8Array(value);
	        }

	        var length;
	        var valueIsOfTypeString = typeof value == "string";

	        if (!(valueIsOfTypeString || value instanceof Uint8Array || value instanceof Uint8ClampedArray || value instanceof Int8Array)) {
	          throwBindingError("Cannot pass non-string to std::string");
	        }

	        if (stdStringIsUTF8 && valueIsOfTypeString) {
	          length = lengthBytesUTF8(value);
	        } else {
	          length = value.length;
	        }

	        var base = _malloc(4 + length + 1);

	        var ptr = base + 4;
	        HEAPU32[base >> 2] = length;

	        if (stdStringIsUTF8 && valueIsOfTypeString) {
	          stringToUTF8(value, ptr, length + 1);
	        } else {
	          if (valueIsOfTypeString) {
	            for (var i = 0; i < length; ++i) {
	              var charCode = value.charCodeAt(i);

	              if (charCode > 255) {
	                _free(ptr);

	                throwBindingError("String has UTF-16 code units that do not fit in 8 bits");
	              }

	              HEAPU8[ptr + i] = charCode;
	            }
	          } else {
	            for (var i = 0; i < length; ++i) {
	              HEAPU8[ptr + i] = value[i];
	            }
	          }
	        }

	        if (destructors !== null) {
	          destructors.push(_free, base);
	        }

	        return base;
	      },
	      "argPackAdvance": 8,
	      "readValueFromPointer": simpleReadValueFromPointer,
	      destructorFunction: function (ptr) {
	        _free(ptr);
	      }
	    });
	  }

	  var UTF16Decoder = typeof TextDecoder != "undefined" ? new TextDecoder("utf-16le") : undefined;

	  function UTF16ToString(ptr, maxBytesToRead) {
	    var endPtr = ptr;
	    var idx = endPtr >> 1;
	    var maxIdx = idx + maxBytesToRead / 2;

	    while (!(idx >= maxIdx) && HEAPU16[idx]) ++idx;

	    endPtr = idx << 1;

	    if (endPtr - ptr > 32 && UTF16Decoder) {
	      return UTF16Decoder.decode(HEAPU8.subarray(ptr, endPtr));
	    } else {
	      var str = "";

	      for (var i = 0; !(i >= maxBytesToRead / 2); ++i) {
	        var codeUnit = HEAP16[ptr + i * 2 >> 1];
	        if (codeUnit == 0) break;
	        str += String.fromCharCode(codeUnit);
	      }

	      return str;
	    }
	  }

	  function stringToUTF16(str, outPtr, maxBytesToWrite) {
	    if (maxBytesToWrite === undefined) {
	      maxBytesToWrite = 2147483647;
	    }

	    if (maxBytesToWrite < 2) return 0;
	    maxBytesToWrite -= 2;
	    var startPtr = outPtr;
	    var numCharsToWrite = maxBytesToWrite < str.length * 2 ? maxBytesToWrite / 2 : str.length;

	    for (var i = 0; i < numCharsToWrite; ++i) {
	      var codeUnit = str.charCodeAt(i);
	      HEAP16[outPtr >> 1] = codeUnit;
	      outPtr += 2;
	    }

	    HEAP16[outPtr >> 1] = 0;
	    return outPtr - startPtr;
	  }

	  function lengthBytesUTF16(str) {
	    return str.length * 2;
	  }

	  function UTF32ToString(ptr, maxBytesToRead) {
	    var i = 0;
	    var str = "";

	    while (!(i >= maxBytesToRead / 4)) {
	      var utf32 = HEAP32[ptr + i * 4 >> 2];
	      if (utf32 == 0) break;
	      ++i;

	      if (utf32 >= 65536) {
	        var ch = utf32 - 65536;
	        str += String.fromCharCode(55296 | ch >> 10, 56320 | ch & 1023);
	      } else {
	        str += String.fromCharCode(utf32);
	      }
	    }

	    return str;
	  }

	  function stringToUTF32(str, outPtr, maxBytesToWrite) {
	    if (maxBytesToWrite === undefined) {
	      maxBytesToWrite = 2147483647;
	    }

	    if (maxBytesToWrite < 4) return 0;
	    var startPtr = outPtr;
	    var endPtr = startPtr + maxBytesToWrite - 4;

	    for (var i = 0; i < str.length; ++i) {
	      var codeUnit = str.charCodeAt(i);

	      if (codeUnit >= 55296 && codeUnit <= 57343) {
	        var trailSurrogate = str.charCodeAt(++i);
	        codeUnit = 65536 + ((codeUnit & 1023) << 10) | trailSurrogate & 1023;
	      }

	      HEAP32[outPtr >> 2] = codeUnit;
	      outPtr += 4;
	      if (outPtr + 4 > endPtr) break;
	    }

	    HEAP32[outPtr >> 2] = 0;
	    return outPtr - startPtr;
	  }

	  function lengthBytesUTF32(str) {
	    var len = 0;

	    for (var i = 0; i < str.length; ++i) {
	      var codeUnit = str.charCodeAt(i);
	      if (codeUnit >= 55296 && codeUnit <= 57343) ++i;
	      len += 4;
	    }

	    return len;
	  }

	  function __embind_register_std_wstring(rawType, charSize, name) {
	    name = readLatin1String(name);
	    var decodeString, encodeString, getHeap, lengthBytesUTF, shift;

	    if (charSize === 2) {
	      decodeString = UTF16ToString;
	      encodeString = stringToUTF16;
	      lengthBytesUTF = lengthBytesUTF16;

	      getHeap = () => HEAPU16;

	      shift = 1;
	    } else if (charSize === 4) {
	      decodeString = UTF32ToString;
	      encodeString = stringToUTF32;
	      lengthBytesUTF = lengthBytesUTF32;

	      getHeap = () => HEAPU32;

	      shift = 2;
	    }

	    registerType(rawType, {
	      name: name,
	      "fromWireType": function (value) {
	        var length = HEAPU32[value >> 2];
	        var HEAP = getHeap();
	        var str;
	        var decodeStartPtr = value + 4;

	        for (var i = 0; i <= length; ++i) {
	          var currentBytePtr = value + 4 + i * charSize;

	          if (i == length || HEAP[currentBytePtr >> shift] == 0) {
	            var maxReadBytes = currentBytePtr - decodeStartPtr;
	            var stringSegment = decodeString(decodeStartPtr, maxReadBytes);

	            if (str === undefined) {
	              str = stringSegment;
	            } else {
	              str += String.fromCharCode(0);
	              str += stringSegment;
	            }

	            decodeStartPtr = currentBytePtr + charSize;
	          }
	        }

	        _free(value);

	        return str;
	      },
	      "toWireType": function (destructors, value) {
	        if (!(typeof value == "string")) {
	          throwBindingError("Cannot pass non-string to C++ string type " + name);
	        }

	        var length = lengthBytesUTF(value);

	        var ptr = _malloc(4 + length + charSize);

	        HEAPU32[ptr >> 2] = length >> shift;
	        encodeString(value, ptr + 4, length + charSize);

	        if (destructors !== null) {
	          destructors.push(_free, ptr);
	        }

	        return ptr;
	      },
	      "argPackAdvance": 8,
	      "readValueFromPointer": simpleReadValueFromPointer,
	      destructorFunction: function (ptr) {
	        _free(ptr);
	      }
	    });
	  }

	  function __embind_register_void(rawType, name) {
	    name = readLatin1String(name);
	    registerType(rawType, {
	      isVoid: true,
	      name: name,
	      "argPackAdvance": 0,
	      "fromWireType": function () {
	        return undefined;
	      },
	      "toWireType": function (destructors, o) {
	        return undefined;
	      }
	    });
	  }

	  function __emscripten_date_now() {
	    return Date.now();
	  }

	  var emval_symbols = {};

	  function getStringOrSymbol(address) {
	    var symbol = emval_symbols[address];

	    if (symbol === undefined) {
	      return readLatin1String(address);
	    }

	    return symbol;
	  }

	  var emval_methodCallers = [];

	  function __emval_call_void_method(caller, handle, methodName, args) {
	    caller = emval_methodCallers[caller];
	    handle = Emval.toValue(handle);
	    methodName = getStringOrSymbol(methodName);
	    caller(handle, methodName, null, args);
	  }

	  function emval_addMethodCaller(caller) {
	    var id = emval_methodCallers.length;
	    emval_methodCallers.push(caller);
	    return id;
	  }

	  function requireRegisteredType(rawType, humanName) {
	    var impl = registeredTypes[rawType];

	    if (undefined === impl) {
	      throwBindingError(humanName + " has unknown type " + getTypeName(rawType));
	    }

	    return impl;
	  }

	  function emval_lookupTypes(argCount, argTypes) {
	    var a = new Array(argCount);

	    for (var i = 0; i < argCount; ++i) {
	      a[i] = requireRegisteredType(HEAPU32[argTypes + i * POINTER_SIZE >> 2], "parameter " + i);
	    }

	    return a;
	  }

	  var emval_registeredMethods = [];

	  function __emval_get_method_caller(argCount, argTypes) {
	    var types = emval_lookupTypes(argCount, argTypes);
	    var retType = types[0];
	    var signatureName = retType.name + "_$" + types.slice(1).map(function (t) {
	      return t.name;
	    }).join("_") + "$";
	    var returnId = emval_registeredMethods[signatureName];

	    if (returnId !== undefined) {
	      return returnId;
	    }

	    var params = ["retType"];
	    var args = [retType];
	    var argsList = "";

	    for (var i = 0; i < argCount - 1; ++i) {
	      argsList += (i !== 0 ? ", " : "") + "arg" + i;
	      params.push("argType" + i);
	      args.push(types[1 + i]);
	    }

	    var functionName = makeLegalFunctionName("methodCaller_" + signatureName);
	    var functionBody = "return function " + functionName + "(handle, name, destructors, args) {\n";
	    var offset = 0;

	    for (var i = 0; i < argCount - 1; ++i) {
	      functionBody += "    var arg" + i + " = argType" + i + ".readValueFromPointer(args" + (offset ? "+" + offset : "") + ");\n";
	      offset += types[i + 1]["argPackAdvance"];
	    }

	    functionBody += "    var rv = handle[name](" + argsList + ");\n";

	    for (var i = 0; i < argCount - 1; ++i) {
	      if (types[i + 1]["deleteObject"]) {
	        functionBody += "    argType" + i + ".deleteObject(arg" + i + ");\n";
	      }
	    }

	    if (!retType.isVoid) {
	      functionBody += "    return retType.toWireType(destructors, rv);\n";
	    }

	    functionBody += "};\n";
	    params.push(functionBody);
	    var invokerFunction = new_(Function, params).apply(null, args);
	    returnId = emval_addMethodCaller(invokerFunction);
	    emval_registeredMethods[signatureName] = returnId;
	    return returnId;
	  }

	  function _abort() {
	    abort("");
	  }

	  function _emscripten_memcpy_big(dest, src, num) {
	    HEAPU8.copyWithin(dest, src, src + num);
	  }

	  function abortOnCannotGrowMemory(requestedSize) {
	    abort("OOM");
	  }

	  function _emscripten_resize_heap(requestedSize) {
	    HEAPU8.length;
	    abortOnCannotGrowMemory();
	  }

	  var ENV = {};

	  function getExecutableName() {
	    return thisProgram || "./this.program";
	  }

	  function getEnvStrings() {
	    if (!getEnvStrings.strings) {
	      var lang = (typeof navigator == "object" && navigator.languages && navigator.languages[0] || "C").replace("-", "_") + ".UTF-8";
	      var env = {
	        "USER": "web_user",
	        "LOGNAME": "web_user",
	        "PATH": "/",
	        "PWD": "/",
	        "HOME": "/home/web_user",
	        "LANG": lang,
	        "_": getExecutableName()
	      };

	      for (var x in ENV) {
	        if (ENV[x] === undefined) delete env[x];else env[x] = ENV[x];
	      }

	      var strings = [];

	      for (var x in env) {
	        strings.push(x + "=" + env[x]);
	      }

	      getEnvStrings.strings = strings;
	    }

	    return getEnvStrings.strings;
	  }

	  function writeAsciiToMemory(str, buffer, dontAddNull) {
	    for (var i = 0; i < str.length; ++i) {
	      HEAP8[buffer++ >> 0] = str.charCodeAt(i);
	    }

	    if (!dontAddNull) HEAP8[buffer >> 0] = 0;
	  }

	  function _environ_get(__environ, environ_buf) {
	    var bufSize = 0;
	    getEnvStrings().forEach(function (string, i) {
	      var ptr = environ_buf + bufSize;
	      HEAPU32[__environ + i * 4 >> 2] = ptr;
	      writeAsciiToMemory(string, ptr);
	      bufSize += string.length + 1;
	    });
	    return 0;
	  }

	  function _environ_sizes_get(penviron_count, penviron_buf_size) {
	    var strings = getEnvStrings();
	    HEAPU32[penviron_count >> 2] = strings.length;
	    var bufSize = 0;
	    strings.forEach(function (string) {
	      bufSize += string.length + 1;
	    });
	    HEAPU32[penviron_buf_size >> 2] = bufSize;
	    return 0;
	  }

	  function _fd_close(fd) {
	    try {
	      var stream = SYSCALLS.getStreamFromFD(fd);
	      FS.close(stream);
	      return 0;
	    } catch (e) {
	      if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError)) throw e;
	      return e.errno;
	    }
	  }

	  function _fd_fdstat_get(fd, pbuf) {
	    try {
	      var stream = SYSCALLS.getStreamFromFD(fd);
	      var type = stream.tty ? 2 : FS.isDir(stream.mode) ? 3 : FS.isLink(stream.mode) ? 7 : 4;
	      HEAP8[pbuf >> 0] = type;
	      return 0;
	    } catch (e) {
	      if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError)) throw e;
	      return e.errno;
	    }
	  }

	  function doReadv(stream, iov, iovcnt, offset) {
	    var ret = 0;

	    for (var i = 0; i < iovcnt; i++) {
	      var ptr = HEAPU32[iov >> 2];
	      var len = HEAPU32[iov + 4 >> 2];
	      iov += 8;
	      var curr = FS.read(stream, HEAP8, ptr, len, offset);
	      if (curr < 0) return -1;
	      ret += curr;
	      if (curr < len) break;
	    }

	    return ret;
	  }

	  function _fd_read(fd, iov, iovcnt, pnum) {
	    try {
	      var stream = SYSCALLS.getStreamFromFD(fd);
	      var num = doReadv(stream, iov, iovcnt);
	      HEAP32[pnum >> 2] = num;
	      return 0;
	    } catch (e) {
	      if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError)) throw e;
	      return e.errno;
	    }
	  }

	  function convertI32PairToI53Checked(lo, hi) {
	    return hi + 2097152 >>> 0 < 4194305 - !!lo ? (lo >>> 0) + hi * 4294967296 : NaN;
	  }

	  function _fd_seek(fd, offset_low, offset_high, whence, newOffset) {
	    try {
	      var offset = convertI32PairToI53Checked(offset_low, offset_high);
	      if (isNaN(offset)) return 61;
	      var stream = SYSCALLS.getStreamFromFD(fd);
	      FS.llseek(stream, offset, whence);
	      tempI64 = [stream.position >>> 0, (tempDouble = stream.position, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble - +(~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)], HEAP32[newOffset >> 2] = tempI64[0], HEAP32[newOffset + 4 >> 2] = tempI64[1];
	      if (stream.getdents && offset === 0 && whence === 0) stream.getdents = null;
	      return 0;
	    } catch (e) {
	      if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError)) throw e;
	      return e.errno;
	    }
	  }

	  function doWritev(stream, iov, iovcnt, offset) {
	    var ret = 0;

	    for (var i = 0; i < iovcnt; i++) {
	      var ptr = HEAPU32[iov >> 2];
	      var len = HEAPU32[iov + 4 >> 2];
	      iov += 8;
	      var curr = FS.write(stream, HEAP8, ptr, len, offset);
	      if (curr < 0) return -1;
	      ret += curr;
	    }

	    return ret;
	  }

	  function _fd_write(fd, iov, iovcnt, pnum) {
	    try {
	      var stream = SYSCALLS.getStreamFromFD(fd);
	      var num = doWritev(stream, iov, iovcnt);
	      HEAPU32[pnum >> 2] = num;
	      return 0;
	    } catch (e) {
	      if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError)) throw e;
	      return e.errno;
	    }
	  }

	  function _setTempRet0(val) {
	  }

	  var FSNode = function (parent, name, mode, rdev) {
	    if (!parent) {
	      parent = this;
	    }

	    this.parent = parent;
	    this.mount = parent.mount;
	    this.mounted = null;
	    this.id = FS.nextInode++;
	    this.name = name;
	    this.mode = mode;
	    this.node_ops = {};
	    this.stream_ops = {};
	    this.rdev = rdev;
	  };

	  var readMode = 292 | 73;
	  var writeMode = 146;
	  Object.defineProperties(FSNode.prototype, {
	    read: {
	      get: function () {
	        return (this.mode & readMode) === readMode;
	      },
	      set: function (val) {
	        val ? this.mode |= readMode : this.mode &= ~readMode;
	      }
	    },
	    write: {
	      get: function () {
	        return (this.mode & writeMode) === writeMode;
	      },
	      set: function (val) {
	        val ? this.mode |= writeMode : this.mode &= ~writeMode;
	      }
	    },
	    isFolder: {
	      get: function () {
	        return FS.isDir(this.mode);
	      }
	    },
	    isDevice: {
	      get: function () {
	        return FS.isChrdev(this.mode);
	      }
	    }
	  });
	  FS.FSNode = FSNode;
	  FS.staticInit();
	  embind_init_charCodes();
	  BindingError = Module["BindingError"] = extendError(Error, "BindingError");
	  InternalError = Module["InternalError"] = extendError(Error, "InternalError");
	  init_ClassHandle();
	  init_embind();
	  init_RegisteredPointer();
	  UnboundTypeError = Module["UnboundTypeError"] = extendError(Error, "UnboundTypeError");
	  init_emval();
	  var asmLibraryArg = {
	    "s": ___cxa_allocate_exception,
	    "r": ___cxa_throw,
	    "C": ___syscall_fcntl64,
	    "v": ___syscall_openat,
	    "u": __embind_register_bigint,
	    "E": __embind_register_bool,
	    "m": __embind_register_class,
	    "l": __embind_register_class_constructor,
	    "d": __embind_register_class_function,
	    "D": __embind_register_emval,
	    "o": __embind_register_float,
	    "c": __embind_register_integer,
	    "b": __embind_register_memory_view,
	    "p": __embind_register_std_string,
	    "k": __embind_register_std_wstring,
	    "q": __embind_register_void,
	    "j": __emscripten_date_now,
	    "f": __emval_call_void_method,
	    "g": __emval_decref,
	    "e": __emval_get_method_caller,
	    "a": _abort,
	    "z": _emscripten_memcpy_big,
	    "i": _emscripten_resize_heap,
	    "x": _environ_get,
	    "y": _environ_sizes_get,
	    "n": _fd_close,
	    "w": _fd_fdstat_get,
	    "B": _fd_read,
	    "t": _fd_seek,
	    "A": _fd_write,
	    "h": _setTempRet0
	  };
	  createWasm();

	  Module["___wasm_call_ctors"] = function () {
	    return (Module["___wasm_call_ctors"] = Module["asm"]["G"]).apply(null, arguments);
	  };

	  var _free = Module["_free"] = function () {
	    return (_free = Module["_free"] = Module["asm"]["H"]).apply(null, arguments);
	  };

	  var _malloc = Module["_malloc"] = function () {
	    return (_malloc = Module["_malloc"] = Module["asm"]["I"]).apply(null, arguments);
	  };

	  var ___errno_location = Module["___errno_location"] = function () {
	    return (___errno_location = Module["___errno_location"] = Module["asm"]["K"]).apply(null, arguments);
	  };

	  var ___getTypeName = Module["___getTypeName"] = function () {
	    return (___getTypeName = Module["___getTypeName"] = Module["asm"]["L"]).apply(null, arguments);
	  };

	  Module["___embind_register_native_and_builtin_types"] = function () {
	    return (Module["___embind_register_native_and_builtin_types"] = Module["asm"]["M"]).apply(null, arguments);
	  };

	  var _emscripten_builtin_memalign = Module["_emscripten_builtin_memalign"] = function () {
	    return (_emscripten_builtin_memalign = Module["_emscripten_builtin_memalign"] = Module["asm"]["N"]).apply(null, arguments);
	  };

	  var ___cxa_is_pointer_type = Module["___cxa_is_pointer_type"] = function () {
	    return (___cxa_is_pointer_type = Module["___cxa_is_pointer_type"] = Module["asm"]["O"]).apply(null, arguments);
	  };

	  Module["dynCall_viiijj"] = function () {
	    return (Module["dynCall_viiijj"] = Module["asm"]["P"]).apply(null, arguments);
	  };

	  Module["dynCall_jij"] = function () {
	    return (Module["dynCall_jij"] = Module["asm"]["Q"]).apply(null, arguments);
	  };

	  Module["dynCall_jii"] = function () {
	    return (Module["dynCall_jii"] = Module["asm"]["R"]).apply(null, arguments);
	  };

	  Module["dynCall_jiji"] = function () {
	    return (Module["dynCall_jiji"] = Module["asm"]["S"]).apply(null, arguments);
	  };

	  Module["_ff_h264_cabac_tables"] = 215452;

	  var calledRun;

	  dependenciesFulfilled = function runCaller() {
	    if (!calledRun) run();
	    if (!calledRun) dependenciesFulfilled = runCaller;
	  };

	  function run(args) {

	    if (runDependencies > 0) {
	      return;
	    }

	    preRun();

	    if (runDependencies > 0) {
	      return;
	    }

	    function doRun() {
	      if (calledRun) return;
	      calledRun = true;
	      Module["calledRun"] = true;
	      if (ABORT) return;
	      initRuntime();
	      if (Module["onRuntimeInitialized"]) Module["onRuntimeInitialized"]();
	      postRun();
	    }

	    if (Module["setStatus"]) {
	      Module["setStatus"]("Running...");
	      setTimeout(function () {
	        setTimeout(function () {
	          Module["setStatus"]("");
	        }, 1);
	        doRun();
	      }, 1);
	    } else {
	      doRun();
	    }
	  }

	  if (Module["preInit"]) {
	    if (typeof Module["preInit"] == "function") Module["preInit"] = [Module["preInit"]];

	    while (Module["preInit"].length > 0) {
	      Module["preInit"].pop()();
	    }
	  }

	  run();
	  module.exports = Module;
	});

	/**
	 * Common utilities
	 * @module glMatrix
	 */
	// Configuration Constants
	var EPSILON = 0.000001;
	var ARRAY_TYPE = typeof Float32Array !== 'undefined' ? Float32Array : Array;
	if (!Math.hypot) Math.hypot = function () {
	  var y = 0,
	      i = arguments.length;

	  while (i--) {
	    y += arguments[i] * arguments[i];
	  }

	  return Math.sqrt(y);
	};

	/**
	 * 4x4 Matrix<br>Format: column-major, when typed out it looks like row-major<br>The matrices are being post multiplied.
	 * @module mat4
	 */

	/**
	 * Creates a new identity mat4
	 *
	 * @returns {mat4} a new 4x4 matrix
	 */

	function create$1() {
	  var out = new ARRAY_TYPE(16);

	  if (ARRAY_TYPE != Float32Array) {
	    out[1] = 0;
	    out[2] = 0;
	    out[3] = 0;
	    out[4] = 0;
	    out[6] = 0;
	    out[7] = 0;
	    out[8] = 0;
	    out[9] = 0;
	    out[11] = 0;
	    out[12] = 0;
	    out[13] = 0;
	    out[14] = 0;
	  }

	  out[0] = 1;
	  out[5] = 1;
	  out[10] = 1;
	  out[15] = 1;
	  return out;
	}
	/**
	 * Set a mat4 to the identity matrix
	 *
	 * @param {mat4} out the receiving matrix
	 * @returns {mat4} out
	 */

	function identity(out) {
	  out[0] = 1;
	  out[1] = 0;
	  out[2] = 0;
	  out[3] = 0;
	  out[4] = 0;
	  out[5] = 1;
	  out[6] = 0;
	  out[7] = 0;
	  out[8] = 0;
	  out[9] = 0;
	  out[10] = 1;
	  out[11] = 0;
	  out[12] = 0;
	  out[13] = 0;
	  out[14] = 0;
	  out[15] = 1;
	  return out;
	}
	/**
	 * Generates a orthogonal projection matrix with the given bounds.
	 * The near/far clip planes correspond to a normalized device coordinate Z range of [-1, 1],
	 * which matches WebGL/OpenGL's clip volume.
	 *
	 * @param {mat4} out mat4 frustum matrix will be written into
	 * @param {number} left Left bound of the frustum
	 * @param {number} right Right bound of the frustum
	 * @param {number} bottom Bottom bound of the frustum
	 * @param {number} top Top bound of the frustum
	 * @param {number} near Near bound of the frustum
	 * @param {number} far Far bound of the frustum
	 * @returns {mat4} out
	 */

	function orthoNO(out, left, right, bottom, top, near, far) {
	  var lr = 1 / (left - right);
	  var bt = 1 / (bottom - top);
	  var nf = 1 / (near - far);
	  out[0] = -2 * lr;
	  out[1] = 0;
	  out[2] = 0;
	  out[3] = 0;
	  out[4] = 0;
	  out[5] = -2 * bt;
	  out[6] = 0;
	  out[7] = 0;
	  out[8] = 0;
	  out[9] = 0;
	  out[10] = 2 * nf;
	  out[11] = 0;
	  out[12] = (left + right) * lr;
	  out[13] = (top + bottom) * bt;
	  out[14] = (far + near) * nf;
	  out[15] = 1;
	  return out;
	}
	/**
	 * Alias for {@link mat4.orthoNO}
	 * @function
	 */

	var ortho = orthoNO;
	/**
	 * Generates a look-at matrix with the given eye position, focal point, and up axis.
	 * If you want a matrix that actually makes an object look at another object, you should use targetTo instead.
	 *
	 * @param {mat4} out mat4 frustum matrix will be written into
	 * @param {ReadonlyVec3} eye Position of the viewer
	 * @param {ReadonlyVec3} center Point the viewer is looking at
	 * @param {ReadonlyVec3} up vec3 pointing up
	 * @returns {mat4} out
	 */

	function lookAt(out, eye, center, up) {
	  var x0, x1, x2, y0, y1, y2, z0, z1, z2, len;
	  var eyex = eye[0];
	  var eyey = eye[1];
	  var eyez = eye[2];
	  var upx = up[0];
	  var upy = up[1];
	  var upz = up[2];
	  var centerx = center[0];
	  var centery = center[1];
	  var centerz = center[2];

	  if (Math.abs(eyex - centerx) < EPSILON && Math.abs(eyey - centery) < EPSILON && Math.abs(eyez - centerz) < EPSILON) {
	    return identity(out);
	  }

	  z0 = eyex - centerx;
	  z1 = eyey - centery;
	  z2 = eyez - centerz;
	  len = 1 / Math.hypot(z0, z1, z2);
	  z0 *= len;
	  z1 *= len;
	  z2 *= len;
	  x0 = upy * z2 - upz * z1;
	  x1 = upz * z0 - upx * z2;
	  x2 = upx * z1 - upy * z0;
	  len = Math.hypot(x0, x1, x2);

	  if (!len) {
	    x0 = 0;
	    x1 = 0;
	    x2 = 0;
	  } else {
	    len = 1 / len;
	    x0 *= len;
	    x1 *= len;
	    x2 *= len;
	  }

	  y0 = z1 * x2 - z2 * x1;
	  y1 = z2 * x0 - z0 * x2;
	  y2 = z0 * x1 - z1 * x0;
	  len = Math.hypot(y0, y1, y2);

	  if (!len) {
	    y0 = 0;
	    y1 = 0;
	    y2 = 0;
	  } else {
	    len = 1 / len;
	    y0 *= len;
	    y1 *= len;
	    y2 *= len;
	  }

	  out[0] = x0;
	  out[1] = y0;
	  out[2] = z0;
	  out[3] = 0;
	  out[4] = x1;
	  out[5] = y1;
	  out[6] = z1;
	  out[7] = 0;
	  out[8] = x2;
	  out[9] = y2;
	  out[10] = z2;
	  out[11] = 0;
	  out[12] = -(x0 * eyex + x1 * eyey + x2 * eyez);
	  out[13] = -(y0 * eyex + y1 * eyey + y2 * eyez);
	  out[14] = -(z0 * eyex + z1 * eyey + z2 * eyez);
	  out[15] = 1;
	  return out;
	}

	/**
	 * 3 Dimensional Vector
	 * @module vec3
	 */

	/**
	 * Creates a new, empty vec3
	 *
	 * @returns {vec3} a new 3D vector
	 */

	function create() {
	  var out = new ARRAY_TYPE(3);

	  if (ARRAY_TYPE != Float32Array) {
	    out[0] = 0;
	    out[1] = 0;
	    out[2] = 0;
	  }

	  return out;
	}
	/**
	 * Creates a new vec3 initialized with the given values
	 *
	 * @param {Number} x X component
	 * @param {Number} y Y component
	 * @param {Number} z Z component
	 * @returns {vec3} a new 3D vector
	 */

	function fromValues(x, y, z) {
	  var out = new ARRAY_TYPE(3);
	  out[0] = x;
	  out[1] = y;
	  out[2] = z;
	  return out;
	}
	/**
	 * Perform some operation over an array of vec3s.
	 *
	 * @param {Array} a the array of vectors to iterate over
	 * @param {Number} stride Number of elements between the start of each vec3. If 0 assumes tightly packed
	 * @param {Number} offset Number of elements to skip at the beginning of the array
	 * @param {Number} count Number of vec3s to iterate over. If 0 iterates over entire array
	 * @param {Function} fn Function to call for each vector in the array
	 * @param {Object} [arg] additional argument to pass to fn
	 * @returns {Array} a
	 * @function
	 */

	(function () {
	  var vec = create();
	  return function (a, stride, offset, count, fn, arg) {
	    var i, l;

	    if (!stride) {
	      stride = 3;
	    }

	    if (!offset) {
	      offset = 0;
	    }

	    if (count) {
	      l = Math.min(count * stride + offset, a.length);
	    } else {
	      l = a.length;
	    }

	    for (i = offset; i < l; i += stride) {
	      vec[0] = a[i];
	      vec[1] = a[i + 1];
	      vec[2] = a[i + 2];
	      fn(vec, vec, arg);
	      a[i] = vec[0];
	      a[i + 1] = vec[1];
	      a[i + 2] = vec[2];
	    }

	    return a;
	  };
	})();

	var createWebGL = ((gl, openWebglAlignment) => {
	  const vertexShaderScript = `
            attribute vec4 aVertexPosition;
            attribute vec2 aTexturePosition;
            uniform mat4 uModelMatrix;
            uniform mat4 uViewMatrix;
            uniform mat4 uProjectionMatrix;
            varying lowp vec2 vTexturePosition;
            void main(void) {
              gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aVertexPosition;
              vTexturePosition = aTexturePosition;
            }
        `;
	  const fragmentShaderScript = `
            precision highp float;
            varying highp vec2 vTexturePosition;
            uniform int isyuv;
            uniform sampler2D rgbaTexture;
            uniform sampler2D yTexture;
            uniform sampler2D uTexture;
            uniform sampler2D vTexture;

            const mat4 YUV2RGB = mat4( 1.1643828125, 0, 1.59602734375, -.87078515625,
                                       1.1643828125, -.39176171875, -.81296875, .52959375,
                                       1.1643828125, 2.017234375, 0, -1.081390625,
                                       0, 0, 0, 1);


            void main(void) {

                if (isyuv>0) {

                    highp float y = texture2D(yTexture,  vTexturePosition).r;
                    highp float u = texture2D(uTexture,  vTexturePosition).r;
                    highp float v = texture2D(vTexture,  vTexturePosition).r;
                    gl_FragColor = vec4(y, u, v, 1) * YUV2RGB;

                } else {
                    gl_FragColor =  texture2D(rgbaTexture, vTexturePosition);
                }
            }
        `;

	  if (openWebglAlignment) {
	    gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);
	  }

	  const shaderProgram = _initShaderProgram();

	  const _programInfo = {
	    program: shaderProgram,
	    attribLocations: {
	      vertexPosition: gl.getAttribLocation(shaderProgram, 'aVertexPosition'),
	      texturePosition: gl.getAttribLocation(shaderProgram, 'aTexturePosition')
	    },
	    uniformLocations: {
	      projectionMatrix: gl.getUniformLocation(shaderProgram, 'uProjectionMatrix'),
	      modelMatrix: gl.getUniformLocation(shaderProgram, 'uModelMatrix'),
	      viewMatrix: gl.getUniformLocation(shaderProgram, 'uViewMatrix'),
	      rgbatexture: gl.getUniformLocation(shaderProgram, 'rgbaTexture'),
	      ytexture: gl.getUniformLocation(shaderProgram, 'yTexture'),
	      utexture: gl.getUniformLocation(shaderProgram, 'uTexture'),
	      vtexture: gl.getUniformLocation(shaderProgram, 'vTexture'),
	      isyuv: gl.getUniformLocation(shaderProgram, 'isyuv')
	    }
	  };

	  const _buffers = _initBuffers();

	  const _rgbatexture = _createTexture();

	  const _ytexture = _createTexture();

	  const _utexture = _createTexture();

	  const _vtexture = _createTexture();

	  function _initBuffers() {
	    // Create a buffer for the cube's vertex positions.
	    const positionBuffer = gl.createBuffer(); // Select the positionBuffer as the one to apply buffer
	    // operations to from here out.

	    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer); // Now create an array of positions for the cube.

	    const positions = [// Front face
	    -1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0]; // Now pass the list of positions into WebGL to build the
	    // shape. We do this by creating a Float32Array from the
	    // JavaScript array, then use it to fill the current buffer.

	    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(positions), gl.STATIC_DRAW); // Now set up the colors for the faces. We'll use solid colors
	    // for each face.
	    //   const facePos = [
	    //     [0.0,  0.0],
	    //     [1.0,  0.0],
	    //     [1.0,  1.0],
	    //     [0.0,  1.0]
	    //   ];

	    const facePos = [[0.0, 1.0], [1.0, 1.0], [1.0, 0.0], [0.0, 0.0]]; // Convert the array of colors into a table for all the vertices.

	    var texturePos = [];
	    texturePos = texturePos.concat(...facePos);
	    const texpositionBuffer = gl.createBuffer();
	    gl.bindBuffer(gl.ARRAY_BUFFER, texpositionBuffer);
	    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(texturePos), gl.STATIC_DRAW); // Build the element array buffer; this specifies the indices
	    // into the vertex arrays for each face's vertices.

	    const indexBuffer = gl.createBuffer();
	    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer); // This array defines each face as two triangles, using the
	    // indices into the vertex array to specify each triangle's
	    // position.

	    const indices = [0, 1, 2, 0, 2, 3]; // Now send the element array to GL

	    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(indices), gl.STATIC_DRAW);
	    return {
	      position: positionBuffer,
	      texPosition: texpositionBuffer,
	      indices: indexBuffer
	    };
	  }

	  function _createTexture() {
	    let texture = gl.createTexture();
	    gl.bindTexture(gl.TEXTURE_2D, texture);
	    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
	    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
	    return texture;
	  }

	  function _loadShader(type, source) {
	    const shader = gl.createShader(type); // Send the source to the shader object

	    gl.shaderSource(shader, source); // Compile the shader program

	    gl.compileShader(shader); // See if it compiled successfully

	    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
	      console.log('An error occurred compiling the shaders: ' + gl.getShaderInfoLog(shader));
	      gl.deleteShader(shader);
	      return null;
	    }

	    return shader;
	  }

	  function _initShaderProgram() {
	    const vertexShader = _loadShader(gl.VERTEX_SHADER, vertexShaderScript);

	    const fragmentShader = _loadShader(gl.FRAGMENT_SHADER, fragmentShaderScript); // Create the shader program


	    const shaderProgram = gl.createProgram();
	    gl.attachShader(shaderProgram, vertexShader);
	    gl.attachShader(shaderProgram, fragmentShader);
	    gl.linkProgram(shaderProgram); // If creating the shader program failed, alert

	    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
	      console.log('Unable to initialize the shader program: ' + gl.getProgramInfoLog(shaderProgram));
	      return null;
	    }

	    return shaderProgram;
	  }

	  function _drawScene(w, h) {
	    gl.viewport(0, 0, w, h);
	    gl.clearColor(0.0, 0.0, 0.0, 0.0); // Clear to black, fully opaque

	    gl.clearDepth(1.0); // Clear everything

	    gl.enable(gl.DEPTH_TEST); // Enable depth testing

	    gl.depthFunc(gl.LEQUAL); // Near things obscure far things
	    // Clear the canvas before we start drawing on it.

	    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	    const zNear = 0.1;
	    const zFar = 100.0;
	    const projectionMatrix = create$1();
	    ortho(projectionMatrix, -1, 1, -1, 1, zNear, zFar); // Set the drawing position to the "identity" point, which is
	    // the center of the scene.

	    const modelMatrix = create$1();
	    identity(modelMatrix);
	    const viewMatrix = create$1();
	    lookAt(viewMatrix, fromValues(0, 0, 0), fromValues(0, 0, -1), fromValues(0, 1, 0)); // Tell WebGL how to pull out the positions from the position
	    // buffer into the vertexPosition attribute

	    {
	      const numComponents = 3;
	      const type = gl.FLOAT;
	      const normalize = false;
	      const stride = 0;
	      const offset = 0;
	      gl.bindBuffer(gl.ARRAY_BUFFER, _buffers.position);
	      gl.vertexAttribPointer(_programInfo.attribLocations.vertexPosition, numComponents, type, normalize, stride, offset);
	      gl.enableVertexAttribArray(_programInfo.attribLocations.vertexPosition);
	    } // Tell WebGL how to pull out the colors from the color buffer
	    // into the vertexColor attribute.

	    {
	      const numComponents = 2;
	      const type = gl.FLOAT;
	      const normalize = false;
	      const stride = 0;
	      const offset = 0;
	      gl.bindBuffer(gl.ARRAY_BUFFER, _buffers.texPosition);
	      gl.vertexAttribPointer(_programInfo.attribLocations.texturePosition, numComponents, type, normalize, stride, offset);
	      gl.enableVertexAttribArray(_programInfo.attribLocations.texturePosition);
	    }
	    let rgbatextunit = 2;
	    let ytextunit = rgbatextunit + 1;
	    let utextunit = rgbatextunit + 2;
	    let vtextunit = rgbatextunit + 3;
	    gl.activeTexture(gl.TEXTURE0 + ytextunit);
	    gl.bindTexture(gl.TEXTURE_2D, _ytexture);
	    gl.activeTexture(gl.TEXTURE0 + utextunit);
	    gl.bindTexture(gl.TEXTURE_2D, _utexture);
	    gl.activeTexture(gl.TEXTURE0 + vtextunit);
	    gl.bindTexture(gl.TEXTURE_2D, _vtexture); // Tell WebGL which indices to use to index the vertices

	    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, _buffers.indices); // Tell WebGL to use our program when drawing

	    gl.useProgram(_programInfo.program); // Set the shader uniforms

	    gl.uniformMatrix4fv(_programInfo.uniformLocations.projectionMatrix, false, projectionMatrix);
	    gl.uniformMatrix4fv(_programInfo.uniformLocations.modelMatrix, false, modelMatrix);
	    gl.uniformMatrix4fv(_programInfo.uniformLocations.viewMatrix, false, viewMatrix);
	    gl.uniform1i(_programInfo.uniformLocations.rgbatexture, rgbatextunit);
	    gl.uniform1i(_programInfo.uniformLocations.ytexture, ytextunit);
	    gl.uniform1i(_programInfo.uniformLocations.utexture, utextunit);
	    gl.uniform1i(_programInfo.uniformLocations.vtexture, vtextunit);
	    gl.uniform1i(_programInfo.uniformLocations.isyuv, 1);
	    {
	      const vertexCount = 6;
	      const type = gl.UNSIGNED_SHORT;
	      const offset = 0;
	      gl.drawElements(gl.TRIANGLES, vertexCount, type, offset);
	    } // Update the rotation for the next draw
	  }

	  return {
	    render: function (width, height, y, u, v) {
	      gl.activeTexture(gl.TEXTURE0);
	      gl.bindTexture(gl.TEXTURE_2D, _ytexture);
	      gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, width, height, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, y);
	      gl.activeTexture(gl.TEXTURE1);
	      gl.bindTexture(gl.TEXTURE_2D, _utexture);
	      gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, width / 2, height / 2, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, u);
	      gl.activeTexture(gl.TEXTURE2);
	      gl.bindTexture(gl.TEXTURE_2D, _vtexture);
	      gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, width / 2, height / 2, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, v);

	      _drawScene(width, height);
	    },
	    renderYUV: function (width, height, data) {
	      let y = data.slice(0, width * height);
	      let u = data.slice(width * height, width * height * 5 / 4);
	      let v = data.slice(width * height * 5 / 4, width * height * 3 / 2);
	      gl.activeTexture(gl.TEXTURE0);
	      gl.bindTexture(gl.TEXTURE_2D, _ytexture);
	      gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, width, height, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, y);
	      gl.activeTexture(gl.TEXTURE1);
	      gl.bindTexture(gl.TEXTURE_2D, _utexture);
	      gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, width / 2, height / 2, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, u);
	      gl.activeTexture(gl.TEXTURE2);
	      gl.bindTexture(gl.TEXTURE_2D, _vtexture);
	      gl.texImage2D(gl.TEXTURE_2D, 0, gl.LUMINANCE, width / 2, height / 2, 0, gl.LUMINANCE, gl.UNSIGNED_BYTE, v);

	      _drawScene(width, height);
	    },
	    destroy: function () {
	      gl.deleteProgram(_programInfo.program);
	      gl.deleteBuffer(_buffers.position);
	      gl.deleteBuffer(_buffers.texPosition);
	      gl.deleteBuffer(_buffers.indices);
	      gl.deleteTexture(_rgbatexture);
	      gl.deleteTexture(_ytexture);
	      gl.deleteTexture(_utexture);
	      gl.deleteTexture(_vtexture);
	    }
	  };
	});

	// 播放协议
	const PLAYER_PLAY_PROTOCOL = {
	  websocket: 1,
	  fetch: 2,
	  hls: 3,
	  webrtc: 4,
	  webTransport: 5
	}; // 播放

	const PLAY_TYPE = {
	  player: "player",
	  playbackTF: 'playbackTF'
	};
	const FILE_SUFFIX = {
	  mp4: 'mp4',
	  webm: 'webm'
	};
	const DEMUX_TYPE = {
	  flv: 'flv',
	  m7s: 'm7s',
	  hls: 'hls',
	  webrtc: 'webrtc',
	  webTransport: 'webTransport',
	  nakedFlow: 'nakedFlow'
	};
	const DEFAULT_PLAYBACK_FORWARD_MAX_RATE_DECODE_IFRAME = 8;
	const DOCUMENT_ONE_SECOND_BUFFER_LENGTH = 60; // default one second buffer length (fps is 25 - 30)

	const DEFAULT_PLAYER_OPTIONS = {
	  playType: PLAY_TYPE.player,
	  //
	  container: '',
	  //
	  videoBuffer: 1 * 1000,
	  // 1* 1000ms == 1 second
	  videoBufferDelay: 1 * 1000,
	  // 1 * 1000ms
	  networkDelay: 10 * 1000,
	  //  10 * 1000ms
	  isResize: true,
	  //
	  isFullResize: false,
	  // full resize
	  isFlv: false,
	  // flv
	  isHls: false,
	  // hls
	  isWebrtc: false,
	  // webrtc
	  isWebrtcForZLM: false,
	  // webrtc for ZLM
	  isNakedFlow: false,
	  // 是否是裸流（264、265）
	  debug: false,
	  hotKey: false,
	  // 快捷键
	  loadingTimeout: 10,
	  // loading timeout
	  heartTimeout: 10,
	  // heart timeout
	  timeout: 10,
	  // second
	  loadingTimeoutReplay: true,
	  // loading timeout replay
	  heartTimeoutReplay: true,
	  // heart timeout replay。
	  loadingTimeoutReplayTimes: 3,
	  // loading timeout replay fail times
	  heartTimeoutReplayTimes: 3,
	  // heart timeout replay fail times
	  supportDblclickFullscreen: false,
	  showBandwidth: false,
	  //
	  showPerformance: false,
	  // 是否显示性能面板
	  keepScreenOn: false,
	  isNotMute: false,
	  hasAudio: true,
	  hasVideo: true,
	  operateBtns: {
	    fullscreen: false,
	    screenshot: false,
	    play: false,
	    audio: false,
	    record: false,
	    ptz: false,
	    quality: false,
	    zoom: false,
	    close: false,
	    scale: false,
	    performance: false,
	    face: false,
	    fullscreenFn: null,
	    fullscreenExitFn: null,
	    screenshotFn: null,
	    playFn: null,
	    pauseFn: null,
	    recordFn: null,
	    recordStopFn: null
	  },
	  watermarkConfig: {},
	  controlAutoHide: false,
	  hasControl: false,
	  loadingIcon: true,
	  loadingText: '',
	  background: '',
	  decoder: 'decoder-pro.js',
	  decoderWASM: '',
	  url: '',
	  //
	  rotate: 0,
	  mirrorRotate: 'none',
	  // 镜像xx
	  playbackConfig: {
	    playList: [],
	    // {start:xx,end:xx}
	    fps: '',
	    // fps值
	    showControl: true,
	    isCacheBeforeDecodeForFpsRender: false,
	    // rfs渲染时，是否在解码前缓存数据
	    uiUsePlaybackPause: false,
	    // ui上面是否使用 playbackPause 方法
	    isUseFpsRender: false,
	    // 是否使用固定的fps渲染，如果设置的fps小于流推过来的，会造成内存堆积甚至溢出
	    isUseLocalCalculateTime: false,
	    // 是否使用本地时间来计算playback时间
	    localOneFrameTimestamp: 40 // 一帧 40ms, isUseLocalCalculateTime 为 true 生效。

	  },
	  qualityConfig: [],
	  // 支持 ['高清','超清','4K']
	  defaultStreamQuality: '',
	  scaleConfig: ['拉伸', '缩放', '正常'],
	  // text: '',
	  forceNoOffscreen: true,
	  // 默认是不采用
	  hiddenAutoPause: false,
	  protocol: PLAYER_PLAY_PROTOCOL.fetch,
	  demuxType: DEMUX_TYPE.flv,
	  //
	  useWasm: false,
	  //wasm 解码 默认
	  useWCS: false,
	  //
	  useSIMD: false,
	  wcsUseVideoRender: true,
	  // wcs 是否使用 video 渲染
	  wasmUseVideoRender: true,
	  // wasm 用video标签渲染
	  mseUseCanvasRender: false,
	  //mse 用canvas标签渲染
	  hlsUseCanvasRender: false,
	  // hls 用canvas标签渲染
	  useMSE: false,
	  //
	  useOffscreen: false,
	  //
	  autoWasm: true,
	  // 自动降级到 wasm 模式
	  wasmDecodeErrorReplay: true,
	  // 解码失败重新播放。
	  openWebglAlignment: false,
	  //  https://github.com/langhuihui/jessibuca/issues/152
	  wasmDecodeAudioSyncVideo: true,
	  // wasm 解码之后音视频同步
	  syncAudioAndVideo: false,
	  // 音视频同步
	  // playback config
	  playbackDelayTime: 1000,
	  // TF卡流播放延迟时间
	  playbackFps: 25,
	  playbackForwardMaxRateDecodeIFrame: DEFAULT_PLAYBACK_FORWARD_MAX_RATE_DECODE_IFRAME,
	  // max rate render i frame
	  playbackCurrentTimeMove: true,
	  useVideoRender: true,
	  // 使用video标签渲染
	  useCanvasRender: false,
	  // 使用canvas渲染
	  networkDelayTimeoutReplay: false,
	  // 网络延迟重连
	  recordType: FILE_SUFFIX.mp4,
	  checkFirstIFrame: true,
	  // 检查第一帧是否是I帧
	  nakedFlowFps: 25,
	  // 裸流fps
	  audioEngine: null,
	  // 音频引擎
	  isShowRecordingUI: true,
	  // 是否显示录制中
	  isShowZoomingUI: true,
	  // 是否显示缩放中
	  useFaceDetector: true //

	};
	const WORKER_CMD_TYPE = {
	  init: 'init',
	  initVideo: 'initVideo',
	  render: 'render',
	  playAudio: 'playAudio',
	  initAudio: 'initAudio',
	  kBps: 'kBps',
	  decode: 'decode',
	  audioCode: 'audioCode',
	  videoCode: 'videoCode',
	  videoCodec: 'videoCodec',
	  videoNalu: 'videoNalu',
	  wasmError: 'wasmError',
	  workerFetch: 'workerFetch',
	  iframeIntervalTs: 'iframeIntervalTs',
	  workerEnd: 'workerEnd',
	  networkDelay: 'networkDelay',
	  playbackStreamVideoFps: 'playbackStreamVideoFps'
	};
	const MEDIA_TYPE = {
	  audio: 1,
	  video: 2
	};
	const FLV_MEDIA_TYPE = {
	  audio: 8,
	  video: 9,
	  scriptData: 18
	};
	const WORKER_SEND_TYPE = {
	  init: 'init',
	  decode: 'decode',
	  audioDecode: 'audioDecode',
	  videoDecode: 'videoDecode',
	  initAudioCodec: 'initAudioCodec',
	  initVideoCodec: 'initVideoCodec',
	  close: 'close',
	  updateConfig: 'updateConfig',
	  resetDecode: 'resetDecode',
	  clearBuffer: 'clearBuffer',
	  resetAudioDecode: 'resetAudioDecode',
	  resetVideoDecode: 'resetVideoDecode',
	  fetchStream: 'fetchStream'
	}; // inner events

	const EVENTS = {
	  fullscreen: 'fullscreen$2',
	  webFullscreen: 'webFullscreen',
	  decoderWorkerInit: 'decoderWorkerInit',
	  play: 'play',
	  playing: 'playing',
	  pause: 'pause',
	  mute: 'mute',
	  load: 'load',
	  loading: 'loading',
	  zooming: 'zooming',
	  videoInfo: 'videoInfo',
	  timeUpdate: 'timeUpdate',
	  audioInfo: "audioInfo",
	  log: 'log',
	  error: "error",
	  kBps: 'kBps',
	  timeout: 'timeout',
	  delayTimeout: 'delayTimeout',
	  loadingTimeout: 'loadingTimeout',
	  stats: 'stats',
	  performance: "performance",
	  faceDetectActive: 'faceDetectActive',
	  // record
	  record: 'record',
	  recording: 'recording',
	  recordingTimestamp: 'recordingTimestamp',
	  recordStart: 'recordStart',
	  recordEnd: 'recordEnd',
	  recordCreateError: 'recordCreateError',
	  recordBlob: 'recordBlob',
	  buffer: 'buffer',
	  videoFrame: 'videoFrame',
	  start: 'start',
	  metadata: 'metadata',
	  resize: 'resize',
	  volumechange: 'volumechange',
	  destroy: 'destroy',
	  beforeDestroy: 'beforeDestroy',
	  // stream
	  streamEnd: 'streamEnd',
	  streamRate: 'streamRate',
	  streamAbps: 'streamAbps',
	  streamVbps: 'streamVbps',
	  streamDts: 'streamDts',
	  streamSuccess: 'streamSuccess',
	  streamMessage: 'streamMessage',
	  streamError: 'streamError',
	  streamStats: 'streamStats',
	  // MSE
	  mseSourceOpen: 'mseSourceOpen',
	  mseSourceClose: 'mseSourceClose',
	  mseSourceended: 'mseSourceended',
	  mseSourceBufferError: 'mseSourceBufferError',
	  mseSourceBufferBusy: 'mseSourceBufferBusy',
	  mseSourceBufferFull: 'mseSourceBufferFull',
	  // VIDEO
	  videoWaiting: 'videoWaiting',
	  videoTimeUpdate: 'videoTimeUpdate',
	  videoSyncAudio: 'videoSyncAudio',
	  //
	  playToRenderTimes: 'playToRenderTimes',
	  playbackTime: 'playbackTime',
	  playbackTimestamp: 'playbackTimestamp',
	  playbackPrecision: 'playbackPrecision',
	  playbackJustTime: 'playbackJustTime',
	  playbackStats: 'playbackStats',
	  playbackSeek: 'playbackSeek',
	  playbackPause: 'playbackPause',
	  playbackPauseOrResume: 'playbackPauseOrResume',
	  ptz: 'ptz',
	  streamQualityChange: 'streamQualityChange',
	  visibilityChange: "visibilityChange",
	  netBuf: 'netBuf',
	  close: 'close',
	  networkDelayTimeout: 'networkDelayTimeout',
	  togglePerformancePanel: 'togglePerformancePanel',
	  viewResizeChange: 'viewResizeChange',
	  // talk
	  talkGetUserMediaSuccess: 'talkGetUserMediaSuccess',
	  talkGetUserMediaFail: 'talkGetUserMediaFail',
	  talkGetUserMediaTimeout: 'talkGetUserMediaTimeout',
	  talkStreamStart: 'talkStreamStart',
	  talkStreamOpen: 'talkStreamOpen',
	  talkStreamClose: 'talkStreamClose',
	  talkStreamError: 'talkStreamError',
	  talkStreamInactive: 'talkStreamInactive',
	  webrtcDisconnect: 'webrtcDisconnect',
	  webrtcFailed: 'webrtcFailed',
	  webrtcClosed: 'webrtcClosed',
	  // crash
	  crashLog: 'crashLog'
	};
	const EVENTS_ERROR = {
	  playError: 'playIsNotPauseOrUrlIsNull',
	  fetchError: "fetchError",
	  websocketError: 'websocketError',
	  webcodecsH265NotSupport: 'webcodecsH265NotSupport',
	  webcodecsDecodeError: 'webcodecsDecodeError',
	  mediaSourceH265NotSupport: 'mediaSourceH265NotSupport',
	  mediaSourceFull: EVENTS.mseSourceBufferFull,
	  mseSourceBufferError: EVENTS.mseSourceBufferError,
	  mediaSourceAppendBufferError: 'mediaSourceAppendBufferError',
	  mediaSourceBufferListLarge: 'mediaSourceBufferListLarge',
	  mediaSourceAppendBufferEndTimeout: 'mediaSourceAppendBufferEndTimeout',
	  wasmDecodeError: 'wasmDecodeError',
	  hlsError: 'hlsError',
	  webrtcError: 'webrtcError',
	  webglAlignmentError: 'webglAlignmentError',
	  webcodecsWidthOrHeightChange: 'webcodecsWidthOrHeightChange',
	  tallWebsocketClosedByError: 'tallWebsocketClosedByError'
	};
	const VIDEO_ENC_CODE = {
	  h264: 7,
	  h265: 12
	};
	const VIDEO_ENC_TYPE_SHOW = {
	  h264: 'H264(AVC)',
	  h265: 'H265(HEVC)'
	};
	const AUDIO_ENC_CODE = {
	  AAC: 10,
	  ALAW: 7,
	  MULAW: 8
	};
	const H264_NAL_TYPE = {
	  sps: 7,
	  pps: 8,
	  iFrame: 5,
	  kUnspecified: 0,
	  kSliceNonIDR: 1,
	  kSliceDPA: 2,
	  kSliceDPB: 3,
	  kSliceDPC: 4,
	  kSliceIDR: 5,
	  kSliceSEI: 6,
	  kSliceSPS: 7,
	  kSlicePPS: 8,
	  kSliceAUD: 9,
	  kEndOfSequence: 10,
	  kEndOfStream: 11,
	  kFiller: 12,
	  kSPSExt: 13,
	  kReserved0: 14
	};
	const H265_NAL_TYPE = {
	  vps: 32,
	  // 语义为视频参数集
	  sps: 33,
	  // 语义为序列参数集
	  pps: 34,
	  // 语义为图像参数集
	  sei: 39 //SEI 语义为补充增强信息
	  //iFrame: 19, // 语义为可能有RADL图像的IDR图像的SS编码数据 IDR
	  //pFrame: 1, //  语义为被参考的后置图像，且非TSA、非STSA的SS编码数据
	  //nLp: 20, // kSliceIDR_N_LP

	};
	const ENCODED_VIDEO_TYPE = {
	  key: 'key',
	  delta: 'delta'
	};
	const FETCH_ERROR = {
	  abortError: 'The user aborted a request',
	  abortError2: 'AbortError',
	  abort: 'AbortError'
	};
	const AVC_PACKET_TYPE = {
	  sequenceHeader: 0,
	  nalu: 1
	};
	const FRAME_TYPE = {
	  keyFrame: 1,
	  interFrame: 2
	}; //     RTP_PAYLOAD_TYPE_PCMU    = 0,   // g711u

	var screenfull = createCommonjsModule(function (module) {
	/*!
	* screenfull
	* v5.1.0 - 2020-12-24
	* (c) Sindre Sorhus; MIT License
	*/
	(function () {

		var document = typeof window !== 'undefined' && typeof window.document !== 'undefined' ? window.document : {};
		var isCommonjs = module.exports;

		var fn = (function () {
			var val;

			var fnMap = [
				[
					'requestFullscreen',
					'exitFullscreen',
					'fullscreenElement',
					'fullscreenEnabled',
					'fullscreenchange',
					'fullscreenerror'
				],
				// New WebKit
				[
					'webkitRequestFullscreen',
					'webkitExitFullscreen',
					'webkitFullscreenElement',
					'webkitFullscreenEnabled',
					'webkitfullscreenchange',
					'webkitfullscreenerror'

				],
				// Old WebKit
				[
					'webkitRequestFullScreen',
					'webkitCancelFullScreen',
					'webkitCurrentFullScreenElement',
					'webkitCancelFullScreen',
					'webkitfullscreenchange',
					'webkitfullscreenerror'

				],
				[
					'mozRequestFullScreen',
					'mozCancelFullScreen',
					'mozFullScreenElement',
					'mozFullScreenEnabled',
					'mozfullscreenchange',
					'mozfullscreenerror'
				],
				[
					'msRequestFullscreen',
					'msExitFullscreen',
					'msFullscreenElement',
					'msFullscreenEnabled',
					'MSFullscreenChange',
					'MSFullscreenError'
				]
			];

			var i = 0;
			var l = fnMap.length;
			var ret = {};

			for (; i < l; i++) {
				val = fnMap[i];
				if (val && val[1] in document) {
					for (i = 0; i < val.length; i++) {
						ret[fnMap[0][i]] = val[i];
					}
					return ret;
				}
			}

			return false;
		})();

		var eventNameMap = {
			change: fn.fullscreenchange,
			error: fn.fullscreenerror
		};

		var screenfull = {
			request: function (element, options) {
				return new Promise(function (resolve, reject) {
					var onFullScreenEntered = function () {
						this.off('change', onFullScreenEntered);
						resolve();
					}.bind(this);

					this.on('change', onFullScreenEntered);

					element = element || document.documentElement;

					var returnPromise = element[fn.requestFullscreen](options);

					if (returnPromise instanceof Promise) {
						returnPromise.then(onFullScreenEntered).catch(reject);
					}
				}.bind(this));
			},
			exit: function () {
				return new Promise(function (resolve, reject) {
					if (!this.isFullscreen) {
						resolve();
						return;
					}

					var onFullScreenExit = function () {
						this.off('change', onFullScreenExit);
						resolve();
					}.bind(this);

					this.on('change', onFullScreenExit);

					var returnPromise = document[fn.exitFullscreen]();

					if (returnPromise instanceof Promise) {
						returnPromise.then(onFullScreenExit).catch(reject);
					}
				}.bind(this));
			},
			toggle: function (element, options) {
				return this.isFullscreen ? this.exit() : this.request(element, options);
			},
			onchange: function (callback) {
				this.on('change', callback);
			},
			onerror: function (callback) {
				this.on('error', callback);
			},
			on: function (event, callback) {
				var eventName = eventNameMap[event];
				if (eventName) {
					document.addEventListener(eventName, callback, false);
				}
			},
			off: function (event, callback) {
				var eventName = eventNameMap[event];
				if (eventName) {
					document.removeEventListener(eventName, callback, false);
				}
			},
			raw: fn
		};

		if (!fn) {
			if (isCommonjs) {
				module.exports = {isEnabled: false};
			} else {
				window.screenfull = {isEnabled: false};
			}

			return;
		}

		Object.defineProperties(screenfull, {
			isFullscreen: {
				get: function () {
					return Boolean(document[fn.fullscreenElement]);
				}
			},
			element: {
				enumerable: true,
				get: function () {
					return document[fn.fullscreenElement];
				}
			},
			isEnabled: {
				enumerable: true,
				get: function () {
					// Coerce to boolean in case of old WebKit
					return Boolean(document[fn.fullscreenEnabled]);
				}
			}
		});

		if (isCommonjs) {
			module.exports = screenfull;
		} else {
			window.screenfull = screenfull;
		}
	})();
	});
	screenfull.isEnabled;

	/**
	 *
	 * @param payload
	 * @returns {boolean}
	 */

	function isAacCodecPacket(payload) {
	  return payload[0] >> 4 === AUDIO_ENC_CODE.AAC && payload[1] === AVC_PACKET_TYPE.sequenceHeader;
	}

	function now() {
	  return new Date().getTime();
	}
	(() => {
	  try {
	    if (typeof WebAssembly === "object" && typeof WebAssembly.instantiate === "function") {
	      const module = new WebAssembly.Module(Uint8Array.of(0x0, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00));
	      if (module instanceof WebAssembly.Module) return new WebAssembly.Instance(module) instanceof WebAssembly.Instance;
	    }
	  } catch (e) {}

	  return false;
	})();
	function getNowTime() {
	  if (performance && typeof performance.now === 'function') {
	    return performance.now();
	  }

	  return Date.now();
	}
	function calculationRate(callback) {
	  let totalSize = 0;
	  let lastTime = getNowTime();
	  return size => {
	    if (!isNumber(size)) {
	      return;
	    }

	    totalSize += size;
	    const thisTime = getNowTime();
	    const diffTime = thisTime - lastTime;

	    if (diffTime >= 1000) {
	      callback(totalSize / diffTime * 1000);
	      lastTime = thisTime;
	      totalSize = 0;
	    }
	  };
	}
	function formatVideoDecoderConfigure(avcC) {
	  let codecArray = avcC.subarray(1, 4);
	  let codecString = "avc1.";

	  for (let j = 0; j < 3; j++) {
	    let h = codecArray[j].toString(16);

	    if (h.length < 2) {
	      h = "0" + h;
	    }

	    codecString += h;
	  }

	  return {
	    codec: codecString,
	    description: avcC
	  };
	}
	function isNumber(value) {
	  const toString = Object.prototype.toString;
	  return toString.call(value) === "[object Number]";
	}
	function isEmpty(value) {
	  return value === null || value === undefined;
	}
	function isFunction(fn) {
	  return typeof fn === "function";
	}
	function checkNaluType(naluBuffer) {
	  let result = null;
	  let type = naluBuffer[0] & 0b0001_1111;

	  if (type === H264_NAL_TYPE.sps || type === H264_NAL_TYPE.pps) {
	    result = VIDEO_ENC_TYPE_SHOW.h264;
	  }

	  if (!result) {
	    type = (naluBuffer[0] & 0x7E) >> 1;

	    if (type === H265_NAL_TYPE.vps || type === H265_NAL_TYPE.sps || type === H265_NAL_TYPE.pps) {
	      result = VIDEO_ENC_TYPE_SHOW.h265;
	    }
	  }

	  return result;
	}
	function supportWritableStream() {
	  return typeof WritableStream !== 'undefined';
	}
	function calcStreamFpsByBufferList(bufferList, type) {
	  if (type) {
	    bufferList = bufferList.filter(item => item.type === type);
	  }

	  let firstItem = bufferList[0];
	  let oneSecondLength = null;

	  if (firstItem) {
	    // next start
	    for (let i = 1; i < bufferList.length; i++) {
	      let tempItem = bufferList[i];

	      if (type && tempItem.type !== type) {
	        tempItem = null;
	      }

	      if (tempItem) {
	        const diff = tempItem.ts - firstItem.ts;

	        if (diff >= 1000) {
	          const prevTempItem = bufferList[i - 1];
	          const diff2 = prevTempItem.ts - firstItem.ts;

	          if (diff2 < 1000) {
	            oneSecondLength = i + 1;
	          }
	        }
	      }
	    }
	  }

	  return oneSecondLength;
	}

	var __awaiter = (undefined && undefined.__awaiter) || function (thisArg, _arguments, P, generator) {
	    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
	    return new (P || (P = Promise))(function (resolve, reject) {
	        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
	        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
	        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
	        step((generator = generator.apply(thisArg, _arguments || [])).next());
	    });
	};
	const Types = [[Uint8Array, Int8Array], [Uint16Array, Int16Array], [Uint32Array, Int32Array, Float32Array], [Float64Array]];
	const U32 = Symbol(32);
	const U16 = Symbol(16);
	const U8 = Symbol(8);
	const OPutMap = new Map();
	Types.forEach((t, i) => t.forEach((t) => OPutMap.set(t, i)));
	class OPut {
	    constructor(g) {
	        this.g = g;
	        this.consumed = 0;
	        if (g)
	            this.need = g.next().value;
	    }
	    fillFromReader(source) {
	        return __awaiter(this, void 0, void 0, function* () {
	            const { done, value } = yield source.read();
	            if (done) {
	                this.close();
	                return;
	            }
	            else {
	                this.write(value);
	                return this.fillFromReader(source);
	            }
	        });
	    }
	    ;
	    consume() {
	        if (this.buffer && this.consumed) {
	            this.buffer.copyWithin(0, this.consumed);
	            this.buffer = this.buffer.subarray(0, this.buffer.length - this.consumed);
	            this.consumed = 0;
	        }
	    }
	    demand(n, consume) {
	        if (consume)
	            this.consume();
	        this.need = n;
	        return this.flush();
	    }
	    read(need) {
	        return new Promise((resolve, reject) => {
	            if (this.resolve)
	                return reject("last read not complete yet");
	            this.resolve = (data) => {
	                delete this.resolve;
	                delete this.need;
	                resolve(data);
	            };
	            this.demand(need, true);
	        });
	    }
	    readU32() {
	        return this.read(U32);
	    }
	    readU16() {
	        return this.read(U16);
	    }
	    readU8() {
	        return this.read(U8);
	    }
	    close() {
	        if (this.g)
	            this.g.return();
	    }
	    flush() {
	        if (!this.buffer || !this.need)
	            return;
	        let returnValue = null;
	        const unread = this.buffer.subarray(this.consumed);
	        let n = 0;
	        const notEnough = (x) => unread.length < (n = x);
	        if (typeof this.need === 'number') {
	            if (notEnough(this.need))
	                return;
	            returnValue = unread.subarray(0, n);
	        }
	        else if (this.need instanceof ArrayBuffer) {
	            if (notEnough(this.need.byteLength))
	                return;
	            new Uint8Array(this.need).set(unread.subarray(0, n));
	            returnValue = this.need;
	        }
	        else if (this.need === U32) {
	            if (notEnough(4))
	                return;
	            returnValue = (unread[0] << 24) | (unread[1] << 16) | (unread[2] << 8) | unread[3];
	        }
	        else if (this.need === U16) {
	            if (notEnough(2))
	                return;
	            returnValue = (unread[0] << 8) | unread[1];
	        }
	        else if (this.need === U8) {
	            if (notEnough(1))
	                return;
	            returnValue = unread[0];
	        }
	        else if (OPutMap.has(this.need.constructor)) {
	            if (notEnough(this.need.length << OPutMap.get(this.need.constructor)))
	                return;
	            new Uint8Array(this.need.buffer, this.need.byteOffset).set(unread.subarray(0, n));
	            returnValue = this.need;
	        }
	        else if (this.g) {
	            this.g.throw(new Error('Unsupported type'));
	            return;
	        }
	        this.consumed += n;
	        if (this.g)
	            this.demand(this.g.next(returnValue).value, true);
	        else if (this.resolve)
	            this.resolve(returnValue);
	        return returnValue;
	    }
	    write(value) {
	        if (value instanceof ArrayBuffer) {
	            this.malloc(value.byteLength).set(new Uint8Array(value));
	        }
	        else {
	            this.malloc(value.byteLength).set(new Uint8Array(value.buffer, value.byteOffset, value.byteLength));
	        }
	        if (this.g || this.resolve)
	            this.flush();
	    }
	    writeU32(value) {
	        this.malloc(4).set([(value >> 24) & 0xff, (value >> 16) & 0xff, (value >> 8) & 0xff, value & 0xff]);
	        this.flush();
	    }
	    writeU16(value) {
	        this.malloc(2).set([(value >> 8) & 0xff, value & 0xff]);
	        this.flush();
	    }
	    writeU8(value) {
	        this.malloc(1)[0] = value;
	        this.flush();
	    }
	    malloc(size) {
	        if (this.buffer) {
	            const l = this.buffer.length;
	            const nl = l + size;
	            if (nl <= this.buffer.buffer.byteLength - this.buffer.byteOffset) {
	                this.buffer = new Uint8Array(this.buffer.buffer, this.buffer.byteOffset, nl);
	            }
	            else {
	                const n = new Uint8Array(nl);
	                n.set(this.buffer);
	                this.buffer = n;
	            }
	            return this.buffer.subarray(l, nl);
	        }
	        else {
	            this.buffer = new Uint8Array(size);
	            return this.buffer;
	        }
	    }
	}
	OPut.U32 = U32;
	OPut.U16 = U16;
	OPut.U8 = U8;

	class Debug {
	  constructor(master) {
	    this.log = function (name) {
	      if (master._opt.debug) {
	        for (var _len = arguments.length, args = new Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
	          args[_key - 1] = arguments[_key];
	        }

	        console.log(`JessibucaPro:[log][${name}]`, ...args);
	      }
	    };

	    this.warn = function (name) {
	      if (master._opt.debug) {
	        for (var _len2 = arguments.length, args = new Array(_len2 > 1 ? _len2 - 1 : 0), _key2 = 1; _key2 < _len2; _key2++) {
	          args[_key2 - 1] = arguments[_key2];
	        }

	        console.log(`JessibucaPro:[warn][${name}]`, ...args);
	      }
	    };

	    this.error = function (name) {
	      for (var _len3 = arguments.length, args = new Array(_len3 > 1 ? _len3 - 1 : 0), _key3 = 1; _key3 < _len3; _key3++) {
	        args[_key3 - 1] = arguments[_key3];
	      }

	      console.error(`JessibucaPro:[error][${name}]`, ...args);
	    };
	  }

	}

	// Exponential-Golomb buffer decoder
	class ExpGolomb {
	  constructor(uint8array) {
	    this._buffer = uint8array;
	    this._buffer_index = 0;
	    this._total_bytes = uint8array.byteLength;
	    this._total_bits = uint8array.byteLength * 8;
	    this._current_word = 0;
	    this._current_word_bits_left = 0;
	  }

	  destroy() {
	    this._buffer = null;
	  }

	  _fillCurrentWord() {
	    let buffer_bytes_left = this._total_bytes - this._buffer_index;

	    if (buffer_bytes_left <= 0) {
	      // throw new IllegalStateException('ExpGolomb: _fillCurrentWord() but no bytes available');
	      console.error('ExpGolomb: _fillCurrentWord() but no bytes available', this._total_bytes, this._buffer_index);
	      return;
	    }

	    let bytes_read = Math.min(4, buffer_bytes_left);
	    let word = new Uint8Array(4);
	    word.set(this._buffer.subarray(this._buffer_index, this._buffer_index + bytes_read));
	    this._current_word = new DataView(word.buffer).getUint32(0, false);
	    this._buffer_index += bytes_read;
	    this._current_word_bits_left = bytes_read * 8;
	  }

	  readBits(bits) {
	    if (bits > 32) {
	      // throw new InvalidArgumentException('ExpGolomb: readBits() bits exceeded max 32bits!');
	      console.error('ExpGolomb: readBits() bits exceeded max 32bits!');
	    }

	    if (bits <= this._current_word_bits_left) {
	      let result = this._current_word >>> 32 - bits;
	      this._current_word <<= bits;
	      this._current_word_bits_left -= bits;
	      return result;
	    }

	    let result = this._current_word_bits_left ? this._current_word : 0;
	    result = result >>> 32 - this._current_word_bits_left;
	    let bits_need_left = bits - this._current_word_bits_left;

	    this._fillCurrentWord();

	    let bits_read_next = Math.min(bits_need_left, this._current_word_bits_left);
	    let result2 = this._current_word >>> 32 - bits_read_next;
	    this._current_word <<= bits_read_next;
	    this._current_word_bits_left -= bits_read_next;
	    result = result << bits_read_next | result2;
	    return result;
	  }

	  readBool() {
	    return this.readBits(1) === 1;
	  }

	  readByte() {
	    return this.readBits(8);
	  }

	  _skipLeadingZero() {
	    let zero_count;

	    for (zero_count = 0; zero_count < this._current_word_bits_left; zero_count++) {
	      if (0 !== (this._current_word & 0x80000000 >>> zero_count)) {
	        this._current_word <<= zero_count;
	        this._current_word_bits_left -= zero_count;
	        return zero_count;
	      }
	    }

	    this._fillCurrentWord();

	    return zero_count + this._skipLeadingZero();
	  }

	  readUEG() {
	    // unsigned exponential golomb
	    let leading_zeros = this._skipLeadingZero();

	    return this.readBits(leading_zeros + 1) - 1;
	  }

	  readSEG() {
	    // signed exponential golomb
	    let value = this.readUEG();

	    if (value & 0x01) {
	      return value + 1 >>> 1;
	    } else {
	      return -1 * (value >>> 1);
	    }
	  }

	}

	/**
	 *
	 * @param sps
	 * @param pps
	 * @returns {Uint8Array}
	 */

	function avcEncoderConfigurationRecord(_ref) {
	  let {
	    sps,
	    pps
	  } = _ref;
	  //     todo: 1 是 序列帧 2-4cts帧
	  // 从 0x01 开始 表示version。
	  // RTMP_AVC_HEAD
	  // 0x17 keyframe  7:AVC
	  // 0x00 AVC sequence header
	  // 0x00 0x00 0x00
	  // 0x01 configurationVersion
	  // 0x42 AVCProfileIndication
	  // 0x00 profile_compatibility
	  // 0x1E AVCLevelIndication
	  // 0xFF lengthSizeMinusOne
	  const tmp = [0x17, 0x00, 0x00, 0x00, 0x00, 0x01, 0x42, 0x00, 0x1E, 0xFF]; // 是个是私有协议标识，h264的。
	  // 0x17 :23

	  tmp[0] = 0x17; // 标准的 nale 格式。。。

	  tmp[6] = sps[1]; // 0x42 avc profile ( sps[0][1] )

	  tmp[7] = sps[2]; // 0x00 avc compatibility ( sps[0][2] )

	  tmp[8] = sps[3]; // 0x1E avc level ( sps[0][3] )
	  //tmp[9]  0xFF  6   reserved ( all bits on )
	  // temp 的length 是10  最后的下标就是 9
	  // 0xE1 : 225

	  tmp[10] = 0xE1; //
	  // number of SPS NALUs (usually 1)  repeated once per SPS:
	  //

	  tmp[11] = sps.byteLength >> 8 & 0xff; //
	  // SPS size

	  tmp[12] = sps.byteLength & 0xff; //
	  // variable   SPS NALU data
	  // number of PPS NALUs (usually 1)  　repeated once per PPS
	  // PPS size
	  // variable PPS NALU data

	  tmp.push(...sps, 0x01, pps.byteLength >> 8 & 0xff, pps.byteLength & 0xff, ...pps); //

	  const arrayBuffer = new Uint8Array(tmp);
	  return arrayBuffer;
	}
	/**
	 *
	 * @param oneNALBuffer
	 * @param isIframe
	 * @returns {Uint8Array}
	 */

	function avcEncoderNalePacket(oneNALBuffer, isIframe) {
	  //     正常发送nal
	  const idrBit = 0x10 | 7;
	  const nIdrBit = 0x20 | 7;
	  let tmp = [];

	  if (isIframe) {
	    tmp[0] = idrBit;
	  } else {
	    tmp[0] = nIdrBit;
	  } // compositionTime


	  tmp[1] = 1;
	  tmp[2] = 0;
	  tmp[3] = 0;
	  tmp[4] = 0; //

	  tmp[5] = oneNALBuffer.byteLength >> 24 & 0xff;
	  tmp[6] = oneNALBuffer.byteLength >> 16 & 0xff;
	  tmp[7] = oneNALBuffer.byteLength >> 8 & 0xff;
	  tmp[8] = oneNALBuffer.byteLength & 0xff;
	  const arrayBuffer = new Uint8Array(tmp.length + oneNALBuffer.byteLength);
	  arrayBuffer.set(tmp, 0);
	  arrayBuffer.set(oneNALBuffer, tmp.length);
	  return arrayBuffer;
	}
	/**
	 * (NALU类型  & 0001  1111)
	 * @param nalu
	 * @returns {number}
	 */

	function getAvcSeqHeadType(nalu) {
	  return nalu[0] & 0b0001_1111;
	}
	function isAvcSeqHead(type) {
	  return type === H264_NAL_TYPE.sps || type === H264_NAL_TYPE.pps;
	}
	function isHvcSEIType(type) {
	  return type === H264_NAL_TYPE.kSliceSEI;
	}
	function isNotAvcSeqHead(type) {
	  return !isAvcSeqHead(type);
	}
	function isAvcNaluIFrame(type) {
	  return type === H264_NAL_TYPE.iFrame;
	}

	const _ebsp2rbsp = uint8array => {
	  let src = uint8array;
	  let src_length = src.byteLength;
	  let dst = new Uint8Array(src_length);
	  let dst_idx = 0;

	  for (let i = 0; i < src_length; i++) {
	    if (i >= 2) {
	      // Unescape: Skip 0x03 after 00 00
	      if (src[i] === 0x03 && src[i - 1] === 0x00 && src[i - 2] === 0x00) {
	        continue;
	      }
	    }

	    dst[dst_idx] = src[i];
	    dst_idx++;
	  }

	  return new Uint8Array(dst.buffer, 0, dst_idx);
	};

	const getLevelString = level_idc => {
	  return (level_idc / 30).toFixed(1);
	};

	const getChromaFormatString = chroma_format_idc => {
	  switch (chroma_format_idc) {
	    case 0:
	      return '4:0:0';

	    case 1:
	      return '4:2:0';

	    case 2:
	      return '4:2:2';

	    case 3:
	      return '4:4:4';

	    default:
	      return 'Unknown';
	  }
	};

	const parseHevcSPS = uint8array => {
	  let rbsp = _ebsp2rbsp(uint8array);

	  let gb = new ExpGolomb(rbsp);
	  /* remove NALu Header */

	  gb.readByte();
	  gb.readByte();
	  let left_offset = 0,
	      right_offset = 0,
	      top_offset = 0,
	      bottom_offset = 0; // SPS

	  gb.readBits(4);
	  let max_sub_layers_minus1 = gb.readBits(3);
	  gb.readBool(); // profile_tier_level begin

	  let general_profile_space = gb.readBits(2);
	  let general_tier_flag = gb.readBool();
	  let general_profile_idc = gb.readBits(5);
	  let general_profile_compatibility_flags_1 = gb.readByte();
	  let general_profile_compatibility_flags_2 = gb.readByte();
	  let general_profile_compatibility_flags_3 = gb.readByte();
	  let general_profile_compatibility_flags_4 = gb.readByte();
	  let general_constraint_indicator_flags_1 = gb.readByte();
	  let general_constraint_indicator_flags_2 = gb.readByte();
	  let general_constraint_indicator_flags_3 = gb.readByte();
	  let general_constraint_indicator_flags_4 = gb.readByte();
	  let general_constraint_indicator_flags_5 = gb.readByte();
	  let general_constraint_indicator_flags_6 = gb.readByte();
	  let general_level_idc = gb.readByte();
	  let sub_layer_profile_present_flag = [];
	  let sub_layer_level_present_flag = [];

	  for (let i = 0; i < max_sub_layers_minus1; i++) {
	    sub_layer_profile_present_flag.push(gb.readBool());
	    sub_layer_level_present_flag.push(gb.readBool());
	  }

	  if (max_sub_layers_minus1 > 0) {
	    for (let i = max_sub_layers_minus1; i < 8; i++) {
	      gb.readBits(2);
	    }
	  }

	  for (let i = 0; i < max_sub_layers_minus1; i++) {
	    if (sub_layer_profile_present_flag[i]) {
	      gb.readByte(); // sub_layer_profile_space, sub_layer_tier_flag, sub_layer_profile_idc

	      gb.readByte();
	      gb.readByte();
	      gb.readByte();
	      gb.readByte(); // sub_layer_profile_compatibility_flag

	      gb.readByte();
	      gb.readByte();
	      gb.readByte();
	      gb.readByte();
	      gb.readByte();
	      gb.readByte();
	    }

	    if (sub_layer_profile_present_flag[i]) {
	      gb.readByte();
	    }
	  } // profile_tier_level end


	  gb.readUEG();
	  let chroma_format_idc = gb.readUEG();

	  if (chroma_format_idc == 3) {
	    gb.readBits(1); // separate_colour_plane_flag
	  }

	  let pic_width_in_luma_samples = gb.readUEG();
	  let pic_height_in_luma_samples = gb.readUEG();
	  let conformance_window_flag = gb.readBool();

	  if (conformance_window_flag) {
	    left_offset += gb.readUEG();
	    right_offset += gb.readUEG();
	    top_offset += gb.readUEG();
	    bottom_offset += gb.readUEG();
	  }

	  let bit_depth_luma_minus8 = gb.readUEG();
	  let bit_depth_chroma_minus8 = gb.readUEG();
	  let log2_max_pic_order_cnt_lsb_minus4 = gb.readUEG();
	  let sub_layer_ordering_info_present_flag = gb.readBool();

	  for (let i = sub_layer_ordering_info_present_flag ? 0 : max_sub_layers_minus1; i <= max_sub_layers_minus1; i++) {
	    gb.readUEG(); // max_dec_pic_buffering_minus1[i]

	    gb.readUEG(); // max_num_reorder_pics[i]

	    gb.readUEG(); // max_latency_increase_plus1[i]
	  }

	  gb.readUEG();
	  gb.readUEG();
	  gb.readUEG();
	  gb.readUEG();
	  gb.readUEG();
	  gb.readUEG();
	  let scaling_list_enabled_flag = gb.readBool();

	  if (scaling_list_enabled_flag) {
	    let sps_scaling_list_data_present_flag = gb.readBool();

	    if (sps_scaling_list_data_present_flag) {
	      for (let sizeId = 0; sizeId < 4; sizeId++) {
	        for (let matrixId = 0; matrixId < (sizeId === 3 ? 2 : 6); matrixId++) {
	          let scaling_list_pred_mode_flag = gb.readBool();

	          if (!scaling_list_pred_mode_flag) {
	            gb.readUEG(); // scaling_list_pred_matrix_id_delta
	          } else {
	            let coefNum = Math.min(64, 1 << 4 + (sizeId << 1));

	            if (sizeId > 1) {
	              gb.readSEG();
	            }

	            for (let i = 0; i < coefNum; i++) {
	              gb.readSEG();
	            }
	          }
	        }
	      }
	    }
	  }

	  gb.readBool();
	  gb.readBool();
	  let pcm_enabled_flag = gb.readBool();

	  if (pcm_enabled_flag) {
	    gb.readByte();
	    gb.readUEG();
	    gb.readUEG();
	    gb.readBool();
	  }

	  let num_short_term_ref_pic_sets = gb.readUEG();
	  let num_delta_pocs = 0;

	  for (let i = 0; i < num_short_term_ref_pic_sets; i++) {
	    let inter_ref_pic_set_prediction_flag = false;

	    if (i !== 0) {
	      inter_ref_pic_set_prediction_flag = gb.readBool();
	    }

	    if (inter_ref_pic_set_prediction_flag) {
	      if (i === num_short_term_ref_pic_sets) {
	        gb.readUEG();
	      }

	      gb.readBool();
	      gb.readUEG();
	      let next_num_delta_pocs = 0;

	      for (let j = 0; j <= num_delta_pocs; j++) {
	        let used_by_curr_pic_flag = gb.readBool();
	        let use_delta_flag = false;

	        if (!used_by_curr_pic_flag) {
	          use_delta_flag = gb.readBool();
	        }

	        if (used_by_curr_pic_flag || use_delta_flag) {
	          next_num_delta_pocs++;
	        }
	      }

	      num_delta_pocs = next_num_delta_pocs;
	    } else {
	      let num_negative_pics = gb.readUEG();
	      let num_positive_pics = gb.readUEG();
	      num_delta_pocs = num_negative_pics + num_positive_pics;

	      for (let j = 0; j < num_negative_pics; j++) {
	        gb.readUEG();
	        gb.readBool();
	      }

	      for (let j = 0; j < num_positive_pics; j++) {
	        gb.readUEG();
	        gb.readBool();
	      }
	    }
	  }

	  let long_term_ref_pics_present_flag = gb.readBool();

	  if (long_term_ref_pics_present_flag) {
	    let num_long_term_ref_pics_sps = gb.readUEG();

	    for (let i = 0; i < num_long_term_ref_pics_sps; i++) {
	      for (let j = 0; j < log2_max_pic_order_cnt_lsb_minus4 + 4; j++) {
	        gb.readBits(1);
	      }

	      gb.readBits(1);
	    }
	  } //*


	  let default_display_window_flag = false; // for calc offset

	  let min_spatial_segmentation_idc = 0; // for hvcC

	  let sar_width = 1,
	      sar_height = 1;
	  let fps_fixed = false,
	      fps_den = 1,
	      fps_num = 1; //*/

	  gb.readBool();
	  gb.readBool();
	  let vui_parameters_present_flag = gb.readBool();

	  if (vui_parameters_present_flag) {
	    let aspect_ratio_info_present_flag = gb.readBool();

	    if (aspect_ratio_info_present_flag) {
	      let aspect_ratio_idc = gb.readByte();
	      let sar_w_table = [1, 12, 10, 16, 40, 24, 20, 32, 80, 18, 15, 64, 160, 4, 3, 2];
	      let sar_h_table = [1, 11, 11, 11, 33, 11, 11, 11, 33, 11, 11, 33, 99, 3, 2, 1];

	      if (aspect_ratio_idc > 0 && aspect_ratio_idc < 16) {
	        sar_width = sar_w_table[aspect_ratio_idc - 1];
	        sar_height = sar_h_table[aspect_ratio_idc - 1];
	      } else if (aspect_ratio_idc === 255) {
	        sar_width = gb.readBits(16);
	        sar_height = gb.readBits(16);
	      }
	    }

	    let overscan_info_present_flag = gb.readBool();

	    if (overscan_info_present_flag) {
	      gb.readBool();
	    }

	    let video_signal_type_present_flag = gb.readBool();

	    if (video_signal_type_present_flag) {
	      gb.readBits(3);
	      gb.readBool();
	      let colour_description_present_flag = gb.readBool();

	      if (colour_description_present_flag) {
	        gb.readByte();
	        gb.readByte();
	        gb.readByte();
	      }
	    }

	    let chroma_loc_info_present_flag = gb.readBool();

	    if (chroma_loc_info_present_flag) {
	      gb.readUEG();
	      gb.readUEG();
	    }

	    gb.readBool();
	    gb.readBool();
	    gb.readBool();
	    default_display_window_flag = gb.readBool();

	    if (default_display_window_flag) {
	      left_offset += gb.readUEG();
	      right_offset += gb.readUEG();
	      top_offset += gb.readUEG();
	      bottom_offset += gb.readUEG();
	    }

	    let vui_timing_info_present_flag = gb.readBool();

	    if (vui_timing_info_present_flag) {
	      fps_den = gb.readBits(32);
	      fps_num = gb.readBits(32);
	      let vui_poc_proportional_to_timing_flag = gb.readBool();

	      if (vui_poc_proportional_to_timing_flag) {
	        gb.readUEG();
	        let vui_hrd_parameters_present_flag = gb.readBool();

	        if (vui_hrd_parameters_present_flag) {
	          let nal_hrd_parameters_present_flag = false;
	          let vcl_hrd_parameters_present_flag = false;
	          let sub_pic_hrd_params_present_flag = false;

	          {
	            nal_hrd_parameters_present_flag = gb.readBool();
	            vcl_hrd_parameters_present_flag = gb.readBool();

	            if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag) {
	              sub_pic_hrd_params_present_flag = gb.readBool();

	              if (sub_pic_hrd_params_present_flag) {
	                gb.readByte();
	                gb.readBits(5);
	                gb.readBool();
	                gb.readBits(5);
	              }

	              gb.readBits(4);
	              gb.readBits(4);

	              if (sub_pic_hrd_params_present_flag) {
	                gb.readBits(4);
	              }

	              gb.readBits(5);
	              gb.readBits(5);
	              gb.readBits(5);
	            }
	          }

	          for (let i = 0; i <= max_sub_layers_minus1; i++) {
	            let fixed_pic_rate_general_flag = gb.readBool();
	            fps_fixed = fixed_pic_rate_general_flag;
	            let fixed_pic_rate_within_cvs_flag = false;
	            let cpbCnt = 1;

	            if (!fixed_pic_rate_general_flag) {
	              fixed_pic_rate_within_cvs_flag = gb.readBool();
	            }

	            let low_delay_hrd_flag = false;

	            if (fixed_pic_rate_within_cvs_flag) {
	              gb.readSEG();
	            } else {
	              low_delay_hrd_flag = gb.readBool();
	            }

	            if (!low_delay_hrd_flag) {
	              cpbcnt = gb.readUEG() + 1;
	            }

	            if (nal_hrd_parameters_present_flag) {
	              for (let j = 0; j < cpbCnt; j++) {
	                gb.readUEG();
	                gb.readUEG();

	                if (sub_pic_hrd_params_present_flag) {
	                  gb.readUEG();
	                  gb.readUEG();
	                }
	              }
	            }

	            if (vcl_hrd_parameters_present_flag) {
	              for (let j = 0; j < cpbCnt; j++) {
	                gb.readUEG();
	                gb.readUEG();

	                if (sub_pic_hrd_params_present_flag) {
	                  gb.readUEG();
	                  gb.readUEG();
	                }
	              }
	            }
	          }
	        }
	      }
	    }

	    let bitstream_restriction_flag = gb.readBool();

	    if (bitstream_restriction_flag) {
	      gb.readBool();
	      gb.readBool();
	      gb.readBool();
	      min_spatial_segmentation_idc = gb.readUEG();
	      gb.readUEG();
	      gb.readUEG();
	      gb.readUEG();
	      gb.readUEG();
	    }
	  }

	  gb.readBool(); // ignore...
	  // for meta data

	  let codec_mimetype = `hvc1.${general_profile_idc}.1.L${general_level_idc}.B0`;
	  let codec_width = pic_width_in_luma_samples;
	  let codec_height = pic_height_in_luma_samples;
	  let sar_scale = 1;

	  if (sar_width !== 1 && sar_height !== 1) {
	    sar_scale = sar_width / sar_height;
	  }

	  gb.destroy();
	  gb = null;
	  return {
	    codec_mimetype,
	    level_string: getLevelString(general_level_idc),
	    profile_idc: general_profile_idc,
	    bit_depth: bit_depth_luma_minus8 + 8,
	    ref_frames: 1,
	    // FIXME!!!
	    chroma_format: chroma_format_idc,
	    chroma_format_string: getChromaFormatString(chroma_format_idc),
	    general_level_idc,
	    general_profile_space,
	    general_tier_flag,
	    general_profile_idc,
	    general_profile_compatibility_flags_1,
	    general_profile_compatibility_flags_2,
	    general_profile_compatibility_flags_3,
	    general_profile_compatibility_flags_4,
	    general_constraint_indicator_flags_1,
	    general_constraint_indicator_flags_2,
	    general_constraint_indicator_flags_3,
	    general_constraint_indicator_flags_4,
	    general_constraint_indicator_flags_5,
	    general_constraint_indicator_flags_6,
	    min_spatial_segmentation_idc,
	    constant_frame_rate: 0
	    /* FIXME!! fps_fixed ? 1 : 0? */
	    ,
	    chroma_format_idc,
	    bit_depth_luma_minus8,
	    bit_depth_chroma_minus8,
	    frame_rate: {
	      fixed: fps_fixed,
	      fps: fps_num / fps_den,
	      fps_den: fps_den,
	      fps_num: fps_num
	    },
	    sar_ratio: {
	      width: sar_width,
	      height: sar_height
	    },
	    codec_size: {
	      width: codec_width,
	      height: codec_height
	    },
	    present_size: {
	      width: codec_width * sar_scale,
	      height: codec_height
	    }
	  };
	};
	const parseHevcVPS = uint8array => {
	  let rbsp = _ebsp2rbsp(uint8array);

	  let gb = new ExpGolomb(rbsp);
	  /* remove NALu Header */

	  gb.readByte();
	  gb.readByte(); // VPS

	  gb.readBits(4);
	  gb.readBits(2);
	  gb.readBits(6);
	  let max_sub_layers_minus1 = gb.readBits(3);
	  let temporal_id_nesting_flag = gb.readBool(); // and more ...

	  return {
	    num_temporal_layers: max_sub_layers_minus1 + 1,
	    temporal_id_nested: temporal_id_nesting_flag
	  };
	};
	const parseHevcPPS = uint8array => {
	  let rbsp = _ebsp2rbsp(uint8array);

	  let gb = new ExpGolomb(rbsp);
	  /* remove NALu Header */

	  gb.readByte();
	  gb.readByte();
	  gb.readUEG();
	  gb.readUEG();
	  gb.readBool();
	  gb.readBool();
	  gb.readBits(3);
	  gb.readBool();
	  gb.readBool();
	  gb.readUEG();
	  gb.readUEG();
	  gb.readSEG();
	  gb.readBool();
	  gb.readBool();
	  let cu_qp_delta_enabled_flag = gb.readBool();

	  if (cu_qp_delta_enabled_flag) {
	    gb.readUEG();
	  }

	  gb.readSEG();
	  gb.readSEG();
	  gb.readBool();
	  gb.readBool();
	  gb.readBool();
	  gb.readBool();
	  let tiles_enabled_flag = gb.readBool();
	  let entropy_coding_sync_enabled_flag = gb.readBool(); // and more ...
	  // needs hvcC

	  let parallelismType = 1; // slice-based parallel decoding

	  if (entropy_coding_sync_enabled_flag && tiles_enabled_flag) {
	    parallelismType = 0; // mixed-type parallel decoding
	  } else if (entropy_coding_sync_enabled_flag) {
	    parallelismType = 3; // wavefront-based parallel decoding
	  } else if (tiles_enabled_flag) {
	    parallelismType = 2; // tile-based parallel decoding
	  }

	  return {
	    parallelismType
	  };
	};

	function hevcEncoderConfigurationRecord$2(_ref2) {
	  let {
	    vps,
	    pps,
	    sps
	  } = _ref2;
	  let detail = {
	    configurationVersion: 1
	  };
	  const vpsDetail = parseHevcVPS(vps);
	  const spsDetail = parseHevcSPS(sps);
	  const ppsDetail = parseHevcPPS(pps);
	  detail = Object.assign(detail, vpsDetail, spsDetail, ppsDetail);
	  let length = 23 + (3 + 2 + vps.byteLength) + (3 + 2 + sps.byteLength) + (3 + 2 + pps.byteLength);
	  let data = new Uint8Array(length);
	  data[0] = 0x01; // configurationVersion

	  data[1] = (detail.general_profile_space & 0x03) << 6 | (detail.general_tier_flag ? 1 : 0) << 5 | detail.general_profile_idc & 0x1F;
	  data[2] = detail.general_profile_compatibility_flags_1 || 0;
	  data[3] = detail.general_profile_compatibility_flags_2 || 0;
	  data[4] = detail.general_profile_compatibility_flags_3 || 0;
	  data[5] = detail.general_profile_compatibility_flags_4 || 0;
	  data[6] = detail.general_constraint_indicator_flags_1 || 0;
	  data[7] = detail.general_constraint_indicator_flags_2 || 0;
	  data[8] = detail.general_constraint_indicator_flags_3 || 0;
	  data[9] = detail.general_constraint_indicator_flags_4 || 0;
	  data[10] = detail.general_constraint_indicator_flags_5 || 0;
	  data[11] = detail.general_constraint_indicator_flags_6 || 0;
	  data[12] = 0x3C;
	  data[13] = 0xF0 | (detail.min_spatial_segmentation_idc & 0x0F00) >> 8;
	  data[14] = detail.min_spatial_segmentation_idc & 0xFF;
	  data[15] = 0xFC | detail.parallelismType & 0x03;
	  data[16] = 0xFC | detail.chroma_format_idc & 0x03;
	  data[17] = 0xF8 | detail.bit_depth_luma_minus8 & 0x07;
	  data[18] = 0xF8 | detail.bit_depth_chroma_minus8 & 0x07;
	  data[19] = 0;
	  data[20] = 0;
	  data[21] = (detail.constant_frame_rate & 0x03) << 6 | (detail.num_temporal_layers & 0x07) << 3 | (detail.temporal_id_nested ? 1 : 0) << 2 | 3;
	  data[22] = 3;
	  data[23 + 0 + 0] = 0x80 | H265_NAL_TYPE.vps;
	  data[23 + 0 + 1] = 0;
	  data[23 + 0 + 2] = 1;
	  data[23 + 0 + 3] = (vps.byteLength & 0xFF00) >> 8;
	  data[23 + 0 + 4] = (vps.byteLength & 0x00FF) >> 0;
	  data.set(vps, 23 + 0 + 5);
	  data[23 + (5 + vps.byteLength) + 0] = 0x80 | H265_NAL_TYPE.sps;
	  data[23 + (5 + vps.byteLength) + 1] = 0;
	  data[23 + (5 + vps.byteLength) + 2] = 1;
	  data[23 + (5 + vps.byteLength) + 3] = (sps.byteLength & 0xFF00) >> 8;
	  data[23 + (5 + vps.byteLength) + 4] = (sps.byteLength & 0x00FF) >> 0;
	  data.set(sps, 23 + (5 + vps.byteLength) + 5);
	  data[23 + (5 + vps.byteLength + 5 + sps.byteLength) + 0] = 0x80 | H265_NAL_TYPE.pps;
	  data[23 + (5 + vps.byteLength + 5 + sps.byteLength) + 1] = 0;
	  data[23 + (5 + vps.byteLength + 5 + sps.byteLength) + 2] = 1;
	  data[23 + (5 + vps.byteLength + 5 + sps.byteLength) + 3] = (pps.byteLength & 0xFF00) >> 8;
	  data[23 + (5 + vps.byteLength + 5 + sps.byteLength) + 4] = (pps.byteLength & 0x00FF) >> 0;
	  data.set(pps, 23 + (5 + vps.byteLength + 5 + sps.byteLength) + 5);
	  const prevData = [0x1c, 0, 0, 0, 0];
	  const newData = new Uint8Array(prevData.length + data.byteLength);
	  newData.set(prevData, 0);
	  newData.set(data, prevData.length);
	  return newData;
	}
	/**
	 *
	 * @param oneNALBuffer
	 * @param isIframe
	 * @returns {Uint8Array}
	 */

	function hevcEncoderNalePacket(oneNALBuffer, isIframe) {
	  //     正常发送nal
	  // 这边增加 是否i帧， 然后前面封装了1 + 8 个字节的数据。
	  const idrBit = 0x10 | 12;
	  const nIdrBit = 0x20 | 12;
	  let tmp = [];

	  if (isIframe) {
	    tmp[0] = idrBit;
	  } else {
	    tmp[0] = nIdrBit;
	  }

	  tmp[1] = 1; //

	  tmp[2] = 0;
	  tmp[3] = 0;
	  tmp[4] = 0; // 真正开始的地方。。。

	  tmp[5] = oneNALBuffer.byteLength >> 24 & 0xff;
	  tmp[6] = oneNALBuffer.byteLength >> 16 & 0xff;
	  tmp[7] = oneNALBuffer.byteLength >> 8 & 0xff;
	  tmp[8] = oneNALBuffer.byteLength & 0xff;
	  const arrayBuffer = new Uint8Array(tmp.length + oneNALBuffer.byteLength);
	  arrayBuffer.set(tmp, 0);
	  arrayBuffer.set(oneNALBuffer, tmp.length);
	  return arrayBuffer;
	}
	function getHevcSeqHeadType(nalu) {
	  return (nalu[0] & 0x7E) >> 1;
	}
	function isHevcSEIType(type) {
	  return type === H265_NAL_TYPE.sei;
	} // 32-40是VPS SPS PPS SUFFIX_SEI_NUT等

	function isHevcSeqHead(type) {
	  return type >= 32 && type <= 40;
	}
	function isNotHevcSeqHead(type) {
	  return !isHevcSeqHead(type);
	} // 16-21是关键(I)帧

	function isHevcNalIFrame(type) {
	  return type >= 16 && type <= 21;
	} // 0-9是P帧

	if (!Date.now) Date.now = function () {
	  return new Date().getTime();
	};
	function workerPostRun (Module) {
	  let bufferList = [];
	  let tempAudioBuffer = [];
	  let wcsVideoDecoder = {};
	  let abortController = new AbortController();
	  let socket = null;
	  let streamRate = null;
	  let streamRateAndStatsInterval = null;
	  let input = null;
	  let videoWidth = null;
	  let videoHeight = null;
	  let hasInitVideoCodec = false;
	  let hasInitAudioCodec = false;
	  let isVideoFirstIFrame = false;
	  let bufferStartDts = null;
	  let bufferStartLocalTs = null;
	  let audioOutputArray = [];
	  let audioRemain = 0;
	  let audioChannels = 0;
	  let preIframeTs = null;

	  let getDefaultOpt = () => {
	    return {
	      debug: DEFAULT_PLAYER_OPTIONS.debug,
	      useOffscreen: DEFAULT_PLAYER_OPTIONS.useOffscreen,
	      useWCS: DEFAULT_PLAYER_OPTIONS.useWCS,
	      videoBuffer: DEFAULT_PLAYER_OPTIONS.videoBuffer,
	      videoBufferDelay: DEFAULT_PLAYER_OPTIONS.videoBufferDelay,
	      openWebglAlignment: DEFAULT_PLAYER_OPTIONS.openWebglAlignment,
	      playType: DEFAULT_PLAYER_OPTIONS.playType,
	      hasAudio: DEFAULT_PLAYER_OPTIONS.hasAudio,
	      hasVideo: DEFAULT_PLAYER_OPTIONS.hasVideo,
	      playbackRate: 1,
	      playbackForwardMaxRateDecodeIFrame: DEFAULT_PLAYER_OPTIONS.playbackForwardMaxRateDecodeIFrame,
	      playbackIsCacheBeforeDecodeForFpsRender: DEFAULT_PLAYER_OPTIONS.playbackConfig.isCacheBeforeDecodeForFpsRender,
	      sampleRate: 0,
	      networkDelay: DEFAULT_PLAYER_OPTIONS.networkDelay,
	      visibility: true,
	      useSIMD: DEFAULT_PLAYER_OPTIONS.useSIMD,
	      isRecording: false,
	      recordType: DEFAULT_PLAYER_OPTIONS.recordType,
	      isNakedFlow: DEFAULT_PLAYER_OPTIONS.isNakedFlow,
	      checkFirstIFrame: DEFAULT_PLAYER_OPTIONS.checkFirstIFrame,
	      audioBufferSize: 1024
	    };
	  };

	  if ("VideoEncoder" in self) {
	    wcsVideoDecoder = {
	      hasInit: false,
	      isEmitInfo: false,
	      offscreenCanvas: null,
	      offscreenCanvasCtx: null,
	      decoder: new VideoDecoder({
	        output: function (videoFrame) {
	          if (!wcsVideoDecoder.isEmitInfo) {
	            decoder.debug.log('worker', 'Webcodecs Video Decoder initSize');
	            postMessage({
	              cmd: WORKER_CMD_TYPE.initVideo,
	              w: videoFrame.codedWidth,
	              h: videoFrame.codedHeight
	            });
	            wcsVideoDecoder.isEmitInfo = true;
	            wcsVideoDecoder.offscreenCanvas = new OffscreenCanvas(videoFrame.codedWidth, videoFrame.codedHeight);
	            wcsVideoDecoder.offscreenCanvasCtx = wcsVideoDecoder.offscreenCanvas.getContext("2d");
	          }

	          if (isFunction(videoFrame.createImageBitmap)) {
	            videoFrame.createImageBitmap().then(image => {
	              wcsVideoDecoder.offscreenCanvasCtx.drawImage(image, 0, 0, videoFrame.codedWidth, videoFrame.codedHeight);
	              let image_bitmap = wcsVideoDecoder.offscreenCanvas.transferToImageBitmap();
	              postMessage({
	                cmd: WORKER_CMD_TYPE.render,
	                buffer: image_bitmap,
	                delay: decoder.delay,
	                ts: 0
	              }, [image_bitmap]);
	              setTimeout(function () {
	                if (videoFrame.close) {
	                  videoFrame.close();
	                } else {
	                  videoFrame.destroy();
	                }
	              }, 100);
	            });
	          } else {
	            wcsVideoDecoder.offscreenCanvasCtx.drawImage(videoFrame, 0, 0, videoFrame.codedWidth, videoFrame.codedHeight);
	            let image_bitmap = wcsVideoDecoder.offscreenCanvas.transferToImageBitmap();
	            postMessage({
	              cmd: WORKER_CMD_TYPE.render,
	              buffer: image_bitmap,
	              delay: decoder.delay,
	              ts: 0
	            }, [image_bitmap]);
	            setTimeout(function () {
	              if (videoFrame.close) {
	                videoFrame.close();
	              } else {
	                videoFrame.destroy();
	              }
	            }, 100);
	          }
	        },
	        error: function (error) {
	          decoder.debug.error('worker', 'VideoDecoder error', error);
	        }
	      }),
	      decode: function (payload, ts, cts) {
	        const isIFrame = payload[0] >> 4 === 1;

	        if (!wcsVideoDecoder.hasInit) {
	          if (isIFrame && payload[1] === 0) {
	            const videoCodec = payload[0] & 0x0F;
	            postMessage({
	              cmd: WORKER_CMD_TYPE.videoCode,
	              code: videoCodec
	            });
	            const extraData = payload.slice(5);
	            const config = formatVideoDecoderConfigure(extraData);
	            postMessage({
	              cmd: WORKER_CMD_TYPE.videoCodec,
	              buffer: payload,
	              codecId: videoCodec
	            }, [payload.buffer]);
	            wcsVideoDecoder.decoder.configure(config);
	            wcsVideoDecoder.hasInit = true;
	          }
	        } else {
	          const chunk = new EncodedVideoChunk({
	            data: payload.slice(5),
	            timestamp: ts,
	            type: isIFrame ? ENCODED_VIDEO_TYPE.key : ENCODED_VIDEO_TYPE.delta
	          });
	          wcsVideoDecoder.decoder.decode(chunk);
	        }
	      },

	      reset() {
	        wcsVideoDecoder.hasInit = false;
	        wcsVideoDecoder.isEmitInfo = false;
	        wcsVideoDecoder.offscreenCanvas = null;
	        wcsVideoDecoder.offscreenCanvasCtx = null;
	      }

	    };
	  }

	  let abort = function () {
	    if (abortController) {
	      abortController.abort();
	      abortController = null;
	    }
	  };

	  let nakedFlowDemuxer = {
	    init() {
	      nakedFlowDemuxer.lastBuf = null;
	      nakedFlowDemuxer.vps = null;
	      nakedFlowDemuxer.sps = null;
	      nakedFlowDemuxer.pps = null;
	      nakedFlowDemuxer.streamType = null;
	      nakedFlowDemuxer.localDts = 0;
	      nakedFlowDemuxer.isSendSeqHeader = false;
	    },

	    destroy() {
	      nakedFlowDemuxer.lastBuf = null;
	      nakedFlowDemuxer.vps = null;
	      nakedFlowDemuxer.sps = null;
	      nakedFlowDemuxer.pps = null;
	      nakedFlowDemuxer.streamType = null;
	      nakedFlowDemuxer.localDts = 0;
	      nakedFlowDemuxer.isSendSeqHeader = false;
	    },

	    //
	    dispatch(data) {
	      const uint8Array = new Uint8Array(data); // decoder.debug.log('worker', 'nakedFlowDemuxer dispatch', uint8Array.byteLength);

	      nakedFlowDemuxer.extractNALu$2(uint8Array); // nakedFlowDemuxer.handleNALu(uint8Array);
	    },

	    getNaluDts() {
	      let resul = nakedFlowDemuxer.localDts;
	      nakedFlowDemuxer.localDts = nakedFlowDemuxer.localDts + 1000 / 25;
	      return resul;
	    },

	    extractNALu(buffer) {
	      let i = 0,
	          length = buffer.byteLength,
	          value,
	          state = 0,
	          result = [],
	          lastIndex;

	      while (i < length) {
	        value = buffer[i++]; // Annex-B格式使用start code进行分割，start code为0x000001或0x00000001，SPS/PPS作为一般NALU单元以start code作为分隔符的方式放在文件或者直播流的头部。
	        // finding 3 or 4-byte start codes (00 00 01 OR 00 00 00 01)

	        switch (state) {
	          case 0:
	            if (value === 0) {
	              state = 1;
	            }

	            break;

	          case 1:
	            if (value === 0) {
	              state = 2;
	            } else {
	              state = 0;
	            }

	            break;

	          case 2:
	          case 3:
	            if (value === 0) {
	              state = 3;
	            } else if (value === 1 && i < length) {
	              if (lastIndex) {
	                result.push(buffer.subarray(lastIndex, i - state - 1));
	              }

	              lastIndex = i;
	              state = 0;
	            } else {
	              state = 0;
	            }

	            break;
	        }
	      }

	      if (lastIndex) {
	        result.push(buffer.subarray(lastIndex, length));
	      }

	      return result;
	    },

	    extractNALu$2(buffer) {
	      let typedArray = null;
	      if (!buffer || buffer.byteLength < 1) return;

	      if (nakedFlowDemuxer.lastBuf) {
	        typedArray = new Uint8Array(buffer.byteLength + nakedFlowDemuxer.lastBuf.length);
	        typedArray.set(nakedFlowDemuxer.lastBuf);
	        typedArray.set(new Uint8Array(buffer), nakedFlowDemuxer.lastBuf.length);
	      } else {
	        typedArray = new Uint8Array(buffer);
	      }

	      let lastNalEndPos = 0;
	      let b1 = -1; // byte before one

	      let b2 = -2; // byte before two

	      const nalStartPos = new Array();

	      for (let i = 0; i < typedArray.length; i += 2) {
	        const b_0 = typedArray[i];
	        const b_1 = typedArray[i + 1];

	        if (b1 == 0 && b_0 == 0 && b_1 == 0) {
	          nalStartPos.push(i - 1);
	        } else if (b_1 == 1 && b_0 == 0 && b1 == 0 && b2 == 0) {
	          nalStartPos.push(i - 2);
	        }

	        b2 = b_0;
	        b1 = b_1;
	      }

	      if (nalStartPos.length > 1) {
	        for (let i = 0; i < nalStartPos.length - 1; ++i) {
	          const naluItem = typedArray.subarray(nalStartPos[i], nalStartPos[i + 1] + 1);
	          nakedFlowDemuxer.handleNALu(naluItem); //console.log('nakedFlowDemuxer.lastBuf nalType', nakedFlowDemuxer.lastBuf.byteLength);

	          lastNalEndPos = nalStartPos[i + 1];
	        }
	      } else {
	        lastNalEndPos = nalStartPos[0];
	      }

	      if (lastNalEndPos != 0 && lastNalEndPos < typedArray.length) {
	        nakedFlowDemuxer.lastBuf = typedArray.subarray(lastNalEndPos);
	      } else {
	        if (!!!nakedFlowDemuxer.lastBuf) {
	          nakedFlowDemuxer.lastBuf = typedArray;
	        }

	        const _newBuf = new Uint8Array(nakedFlowDemuxer.lastBuf.length + buffer.byteLength);

	        _newBuf.set(nakedFlowDemuxer.lastBuf);

	        _newBuf.set(new Uint8Array(buffer), nakedFlowDemuxer.lastBuf.length);

	        nakedFlowDemuxer.lastBuf = _newBuf;
	      }
	    },

	    handleNALu(nalu) {
	      if (nalu.length <= 4) {
	        decoder.debug.error('worker', 'handleNALu', 'nalu.length <= 4');
	        return;
	      } // 0001 去掉前4个字节（start code）


	      nalu = nalu.slice(4);
	      nakedFlowDemuxer.handleVideoNalu(nalu);
	    },

	    handleVideoNalu(nalu) {
	      // decoder.debug.log('worker', 'handleVideoNalu', nalu);
	      if (!nakedFlowDemuxer.streamType) {
	        nakedFlowDemuxer.streamType = checkNaluType(nalu);
	      }

	      if (nakedFlowDemuxer.streamType === VIDEO_ENC_TYPE_SHOW.h264) {
	        const nalType = getAvcSeqHeadType(nalu);

	        if (nalType === H264_NAL_TYPE.pps) {
	          nakedFlowDemuxer.extractH264PPS(nalu);
	        } else {
	          nakedFlowDemuxer.handleVideoH264Nalu(nalu);
	        }
	      } else if (nakedFlowDemuxer.streamType === VIDEO_ENC_TYPE_SHOW.h265) {
	        const naluType = getHevcSeqHeadType(nalu);

	        if (naluType === H265_NAL_TYPE.pps) {
	          nakedFlowDemuxer.extractH265PPS(nalu);
	        } else {
	          nakedFlowDemuxer.handleVideoH265Nalu(nalu);
	        }
	      }
	    },

	    extractH264PPS(nalu) {
	      const tempNalu = nakedFlowDemuxer.handleAddNaluStartCode(nalu);
	      const naluList = nakedFlowDemuxer.extractNALu(tempNalu);
	      naluList.forEach(naluItem => {
	        const nalType = getAvcSeqHeadType(naluItem);

	        if (isHvcSEIType(nalType)) {
	          nakedFlowDemuxer.extractH264SEI(naluItem);
	        } else {
	          nakedFlowDemuxer.handleVideoH264Nalu(naluItem);
	        }
	      });
	    },

	    extractH265PPS(nalu) {
	      const tempNalu = nakedFlowDemuxer.handleAddNaluStartCode(nalu);
	      const naluList = nakedFlowDemuxer.extractNALu(tempNalu);
	      naluList.forEach(naluItem => {
	        const nalType = getHevcSeqHeadType(naluItem);

	        if (isHevcSEIType(nalType)) {
	          nakedFlowDemuxer.extractH265SEI(naluItem);
	        } else {
	          nakedFlowDemuxer.handleVideoH265Nalu(naluItem);
	        }
	      });
	    },

	    extractH264SEI(nalu) {
	      const tempNalu = nakedFlowDemuxer.handleAddNaluStartCode(nalu);
	      const naluList = nakedFlowDemuxer.extractNALu(tempNalu);
	      naluList.forEach(naluItem => {
	        nakedFlowDemuxer.handleVideoH264Nalu(naluItem);
	      });
	    },

	    extractH265SEI(nalu) {
	      const tempNalu = nakedFlowDemuxer.handleAddNaluStartCode(nalu);
	      const naluList = nakedFlowDemuxer.extractNALu(tempNalu); //console.log('extractH265SEI', naluList);

	      naluList.forEach(naluItem => {
	        nakedFlowDemuxer.handleVideoH265Nalu(naluItem);
	      });
	    },

	    handleAddNaluStartCode(nalu) {
	      const prefix = [0, 0, 0, 1];
	      const newNalu = new Uint8Array(nalu.length + prefix.length);
	      newNalu.set(prefix);
	      newNalu.set(nalu, prefix.length);
	      return newNalu;
	    },

	    handleVideoH264Nalu(nalu) {
	      const nalType = getAvcSeqHeadType(nalu);
	      decoder.debug.log('worker', `handleVideoH264Nalu nalType is ${nalu[0]} , nalu[0] is ${nalType}`);

	      switch (nalType) {
	        case H264_NAL_TYPE.sps:
	          nakedFlowDemuxer.sps = nalu; // console.log('handleVideoH264Nalu sps', nalu);

	          break;

	        case H264_NAL_TYPE.pps:
	          nakedFlowDemuxer.pps = nalu; // console.log('handleVideoH264Nalu pps', nalu);

	          break;
	      }

	      if (!nakedFlowDemuxer.isSendSeqHeader) {
	        if (nakedFlowDemuxer.sps && nakedFlowDemuxer.pps) {
	          nakedFlowDemuxer.isSendSeqHeader = true;
	          const seqHeader = avcEncoderConfigurationRecord({
	            sps: nakedFlowDemuxer.sps,
	            pps: nakedFlowDemuxer.pps
	          }); // console.log('handleVideoH264Nalu seqHeader', seqHeader);
	          // decoder.decodeVideo(seqHeader, 0, true, 0);

	          decoder.decode(seqHeader, {
	            type: MEDIA_TYPE.video,
	            ts: 0,
	            isIFrame: true,
	            cts: 0
	          });
	        }
	      } else {
	        if (isNotAvcSeqHead(nalType)) {
	          const isIFrame = isAvcNaluIFrame(nalType);
	          const dts = nakedFlowDemuxer.getNaluDts();
	          decoder.calcNetworkDelay(dts);

	          if (isIFrame) {
	            decoder.calcIframeIntervalTimestamp(dts);
	          }

	          const packet = avcEncoderNalePacket(nalu, isIFrame); // decoder.decodeVideo(packet, dts, isIFrame, 0);

	          decoder.decode(packet, {
	            type: MEDIA_TYPE.video,
	            ts: dts,
	            isIFrame: isIFrame,
	            cts: 0
	          });
	        } // console.log('handleVideoH264Nalu packet', packet);

	      }
	    },

	    handleVideoH265Nalu(nalu) {
	      const nalType = getHevcSeqHeadType(nalu);
	      decoder.debug.log('worker', `handleVideoH265Nalu nalType is ${nalType} , isHevcNalIFrame ${isHevcNalIFrame(nalType)} ,nalu[0] is ${nalu[0]} ,nalu.bytelength is ${nalu.byteLength}`);

	      switch (nalType) {
	        case H265_NAL_TYPE.vps:
	          nakedFlowDemuxer.vps = nalu; // console.log('handleVideoH265Nalu vps', nalu);

	          break;

	        case H265_NAL_TYPE.sps:
	          nakedFlowDemuxer.sps = nalu; // console.log('handleVideoH265Nalu sps', nalu);

	          break;

	        case H265_NAL_TYPE.pps:
	          nakedFlowDemuxer.pps = nalu; // console.log('handleVideoH265Nalu pps', nalu);

	          break;
	      }

	      if (!nakedFlowDemuxer.isSendSeqHeader) {
	        if (nakedFlowDemuxer.vps && nakedFlowDemuxer.sps && nakedFlowDemuxer.pps) {
	          nakedFlowDemuxer.isSendSeqHeader = true;
	          const seqHeader = hevcEncoderConfigurationRecord$2({
	            vps: nakedFlowDemuxer.vps,
	            sps: nakedFlowDemuxer.sps,
	            pps: nakedFlowDemuxer.pps
	          }); // console.log('handleVideoH265Nalu seqHeader', seqHeader, seqHeader.byteLength);
	          // decoder.decodeVideo(seqHeader, 0, true, 0);

	          decoder.decode(seqHeader, {
	            type: MEDIA_TYPE.video,
	            ts: 0,
	            isIFrame: true,
	            cts: 0
	          });
	        }
	      } else {
	        if (isNotHevcSeqHead(nalType)) {
	          const isIFrame = isHevcNalIFrame(nalType);
	          const dts = nakedFlowDemuxer.getNaluDts();
	          const packet = hevcEncoderNalePacket(nalu, isIFrame);

	          if (isIFrame) {
	            decoder.calcIframeIntervalTimestamp(dts);
	          } // console.log('handleVideoH265Nalu packet', nalu, packet.byteLength, dts, isIFrame);
	          // decoder.decodeVideo(packet, dts, isIFrame, 0);


	          decoder.decode(packet, {
	            type: MEDIA_TYPE.video,
	            ts: dts,
	            isIFrame: isIFrame,
	            cts: 0
	          });
	        }
	      }
	    }

	  };
	  let decoder = {
	    isPlayer: true,
	    _opt: getDefaultOpt(),
	    startStreamRateAndStatsInterval: function () {
	      decoder.debug.log('worker', 'startStreamRateAndStatsInterval');
	      decoder.stopStreamRateAndStatsInterval();
	      streamRateAndStatsInterval = setInterval(() => {
	        streamRate && streamRate(0);
	        const streamStats = JSON.stringify({
	          demuxBufferDelay: decoder.getVideoBufferLength(),
	          audioDemuxBufferDelay: decoder.getAudioBufferLength(),
	          flvBufferByteLength: input && input.buffer ? input.buffer.byteLength : 0
	        });
	        postMessage({
	          cmd: WORKER_CMD_TYPE.workerFetch,
	          type: EVENTS.streamStats,
	          value: streamStats
	        });
	      }, 1000);
	    },
	    stopStreamRateAndStatsInterval: function () {
	      decoder.debug.log('worker', `stopStreamRateAndStatsInterval`);

	      if (streamRateAndStatsInterval) {
	        clearInterval(streamRateAndStatsInterval);
	        streamRateAndStatsInterval = null;
	      }
	    },
	    useOffscreen: function () {
	      return decoder._opt.useOffscreen && typeof OffscreenCanvas != 'undefined';
	    },
	    getDelay: function (timestamp) {
	      if (!timestamp || decoder._opt.hasVideo && !isVideoFirstIFrame) {
	        return -1;
	      }

	      if (!decoder.firstTimestamp) {
	        decoder.firstTimestamp = timestamp;
	        decoder.startTimestamp = Date.now();
	        decoder.delay = -1;
	      } else {
	        if (timestamp) {
	          const localTimestamp = Date.now() - decoder.startTimestamp;
	          const timeTimestamp = timestamp - decoder.firstTimestamp;

	          if (localTimestamp >= timeTimestamp) {
	            decoder.delay = localTimestamp - timeTimestamp;
	          } else {
	            decoder.delay = timeTimestamp - localTimestamp;
	          }
	        }
	      }

	      return decoder.delay;
	    },
	    resetDelay: function () {
	      decoder.firstTimestamp = null;
	      decoder.startTimestamp = null;
	      decoder.delay = -1;
	    },
	    doDecode: function (data) {
	      if (decoder._opt.useWCS && decoder.useOffscreen() && data.type === MEDIA_TYPE.video && wcsVideoDecoder.decode) {
	        wcsVideoDecoder.decode(data.payload, data.ts, data.cts);
	      } else {
	        data.decoder.decode(data.payload, data.ts, data.isIFrame, data.cts);
	      }
	    },
	    init: function () {
	      decoder.debug.log('worker', 'init and opt is', decoder._opt);
	      const isPlayer = decoder._opt.playType === PLAY_TYPE.player;
	      nakedFlowDemuxer.init();
	      decoder.isPlayer = isPlayer; // player || (playback && not cache before decode for fps render)

	      if (!decoder.isPlaybackCacheBeforeDecodeForFpsRender()) {
	        // default loop
	        const loop = () => {
	          let data = null;

	          if (bufferList.length) {
	            if (decoder.dropping) {
	              if (decoder._opt.useSIMD) {
	                while (bufferList.length > 0) {
	                  let data = bufferList[0];

	                  if (data.isIFrame && bufferList.length < DOCUMENT_ONE_SECOND_BUFFER_LENGTH * (decoder._opt.videoBuffer / 1000)) {
	                    decoder.dropping = false;
	                    break;
	                  } // decoder.debug.log('worker', `dropBuffer is dropping and isIFrame ${data.isIFrame} and delay is ${decoder.delay} and bufferlist is ${bufferList.length}`);
	                  // just for simd decode


	                  if (data.type === MEDIA_TYPE.video) {
	                    decoder.doDecode(data);
	                  }

	                  bufferList.shift();
	                }
	              } else {
	                // decoder.debug.log('worker','loop is dropping');
	                // // dropping
	                data = bufferList.shift(); //

	                while (!data.isIFrame && bufferList.length) {
	                  // decoder.debug.log('worker', 'loop is dropping = true, isIFrame is', data.isIFrame);
	                  // dropping
	                  data = bufferList.shift();
	                }

	                if (data.isIFrame && bufferList.length < DOCUMENT_ONE_SECOND_BUFFER_LENGTH * (decoder._opt.videoBuffer / 1000)) {
	                  // decoder.debug.log('worker','loop is dropping = false, is iFrame');
	                  decoder.dropping = false;
	                  decoder.doDecode(data);
	                }
	              }
	            } else {
	              data = bufferList[0];

	              if (decoder.getDelay(data.ts) === -1) {
	                decoder.debug.log('worker', 'common dumex delay is -1 ,data.ts is', data.ts);
	                bufferList.shift();
	                decoder.doDecode(data);
	              } else if (decoder.delay > decoder._opt.videoBuffer + decoder._opt.videoBufferDelay && isPlayer) {
	                // decoder.debug.log('worker',`delay is ${decoder.delay}, set dropping is true`);
	                decoder.resetDelay();
	                decoder.dropping = true;
	              } else {
	                // 持续解码
	                while (bufferList.length) {
	                  data = bufferList[0];

	                  if (decoder.getDelay(data.ts) > decoder._opt.videoBuffer) {
	                    bufferList.shift();
	                    decoder.doDecode(data);
	                  } else {
	                    if (decoder.delay < 0) {
	                      decoder.debug.warn('worker', `do not decode and delay is ${decoder.delay}, bufferList is ${bufferList.length}`);
	                    }

	                    break;
	                  }
	                }
	              }
	            }
	          }
	        };

	        decoder.stopId = setInterval(loop, 10);
	      } else {
	        decoder.debug.log('worker', 'playback and playbackIsCacheBeforeDecodeForFpsRender is true');
	      } // if not check first frame is iframe, will set isVideoFirstIFrame = true


	      if (!decoder._opt.checkFirstIFrame) {
	        isVideoFirstIFrame = true;
	      }
	    },
	    playbackCacheLoop: function () {
	      if (decoder.stopId) {
	        clearInterval(decoder.stopId);
	        decoder.stopId = null;
	      }

	      const loop = () => {
	        let data = null;

	        if (bufferList.length) {
	          data = bufferList.shift();
	          decoder.doDecode(data);
	        }
	      };

	      loop();
	      const fragDuration = Math.ceil(1000 / (decoder.streamFps * decoder._opt.playbackRate));
	      decoder.debug.log('worker', `playbackCacheLoop fragDuration is ${fragDuration}, streamFps is ${decoder.streamFps}, playbackRate is ${decoder._opt.playbackRate}`);
	      decoder.stopId = setInterval(loop, fragDuration);
	    },
	    close: function () {
	      decoder.debug.log('worker', 'close');
	      decoder.stopStreamRateAndStatsInterval();
	      clearInterval(decoder.stopId);
	      decoder.stopId = null;
	      audioDecoder.clear && audioDecoder.clear();
	      videoDecoder.clear && videoDecoder.clear();
	      audioDecoder = null;
	      videoDecoder = null;
	      wcsVideoDecoder.reset && wcsVideoDecoder.reset();
	      decoder.firstTimestamp = null;
	      decoder.startTimestamp = null;
	      decoder.streamFps = null; // audio + video all fps

	      decoder.streamAudioFps = null; //

	      decoder.streamVideoFps = null;
	      decoder.delay = -1;
	      decoder.dropping = false;
	      decoder.isPlayer = true;
	      decoder._opt = getDefaultOpt();

	      if (decoder.webglObj) {
	        decoder.webglObj.destroy();
	        decoder.offscreenCanvas = null;
	        decoder.offscreenCanvasGL = null;
	        decoder.offscreenCanvasCtx = null;
	      }

	      bufferList = [];
	      tempAudioBuffer = [];
	      abortController = null;
	      input = null;

	      if (socket) {
	        socket.close();
	        socket = null;
	      }

	      videoWidth = null;
	      videoHeight = null;
	      hasInitVideoCodec = false;
	      hasInitAudioCodec = false;
	      isVideoFirstIFrame = false;
	      audioOutputArray = [];
	      audioRemain = 0;
	      audioChannels = 0;
	      bufferStartDts = null;
	      bufferStartLocalTs = null;
	      preIframeTs = null;
	      nakedFlowDemuxer.destroy();
	    },
	    pushBuffer: function (bufferData, options) {
	      if (options.type === MEDIA_TYPE.audio && isAacCodecPacket(bufferData)) {
	        decoder.decodeAudio(bufferData, options.ts);
	        return;
	      }

	      if (decoder.isPlayer && isVideoFirstIFrame) {
	        const maxDelay = decoder._opt.videoBuffer + decoder._opt.videoBufferDelay;

	        if (decoder.getDelay(options.ts) > maxDelay) {
	          decoder.debug.log('worker', `pushBuffer, decoder.delay is ${decoder.delay} more than ${maxDelay} and next decoder.dropBuffer()`);
	          decoder.dropBuffer(true);
	        }
	      } // 音频


	      if (options.type === MEDIA_TYPE.audio) {
	        bufferList.push({
	          ts: options.ts,
	          payload: bufferData,
	          decoder: {
	            decode: decoder.decodeAudio
	          },
	          type: MEDIA_TYPE.audio,
	          isIFrame: false
	        });
	      } else if (options.type === MEDIA_TYPE.video) {
	        bufferList.push({
	          ts: options.ts,
	          cts: options.cts,
	          payload: bufferData,
	          decoder: {
	            decode: decoder.decodeVideo
	          },
	          type: MEDIA_TYPE.video,
	          isIFrame: options.isIFrame
	        });
	      }

	      if (decoder.isPlaybackCacheBeforeDecodeForFpsRender()) {
	        if (isEmpty(decoder.streamVideoFps) || isEmpty(decoder.streamAudioFps)) {
	          let streamVideoFps = decoder.streamVideoFps;
	          let streamAudioFps = decoder.streamAudioFps;

	          if (isEmpty(decoder.streamVideoFps)) {
	            streamVideoFps = calcStreamFpsByBufferList(bufferList, MEDIA_TYPE.video);

	            if (streamVideoFps) {
	              decoder.streamVideoFps = streamVideoFps;
	              postMessage({
	                cmd: WORKER_CMD_TYPE.playbackStreamVideoFps,
	                value: decoder.streamVideoFps
	              });

	              if (streamAudioFps) {
	                decoder.streamFps = streamVideoFps + streamAudioFps;
	              } else {
	                decoder.streamFps = streamVideoFps;
	              }

	              this.playbackCacheLoop();
	            }
	          }

	          if (isEmpty(decoder.streamAudioFps)) {
	            streamAudioFps = calcStreamFpsByBufferList(bufferList, MEDIA_TYPE.audio);

	            if (streamAudioFps) {
	              decoder.streamAudioFps = streamAudioFps;

	              if (streamVideoFps) {
	                decoder.streamFps = streamVideoFps + streamAudioFps;
	              } else {
	                decoder.streamFps = streamAudioFps;
	              }

	              this.playbackCacheLoop();
	            }
	          }

	          decoder.debug.log('worker', `playbackCacheBeforeDecodeForFpsRender, calc streamAudioFps is ${streamAudioFps}, streamVideoFps is ${streamVideoFps}, bufferListLength  is ${bufferList.length}`);
	        }
	      } // just for visibility false trigger


	      if (decoder.isPlayer && isVideoFirstIFrame && !decoder._opt.visibility) {
	        const maxBufferLength = DOCUMENT_ONE_SECOND_BUFFER_LENGTH + DOCUMENT_ONE_SECOND_BUFFER_LENGTH * ((decoder._opt.videoBuffer + decoder._opt.videoBufferDelay) / 1000);

	        if (decoder.getVideoBufferLength() > maxBufferLength) {
	          decoder.debug.warn('worker', `pushBuffer bufferList length is ${bufferList.length} more than ${maxBufferLength}  and delay is ${decoder.getDelay(options.ts)} and next dropBuffer()`);
	          decoder.dropBuffer();
	        }
	      }
	    },

	    // video buffer length
	    getVideoBufferLength() {
	      let result = 0;
	      bufferList.forEach(item => {
	        if (item.type === MEDIA_TYPE.video) {
	          result += 1;
	        }
	      });
	      return result;
	    },

	    getAudioBufferLength() {
	      let result = 0;
	      bufferList.forEach(item => {
	        if (item.type === MEDIA_TYPE.audio) {
	          result += 1;
	        }
	      });
	      return result;
	    },

	    fetchStream: function (url, options) {
	      decoder.debug.log('worker', 'fetchStream, url is ' + url, 'options:', JSON.stringify(options));
	      streamRate = calculationRate(rate => {
	        // decoder.debug.log('worker','calculationRate');
	        postMessage({
	          cmd: WORKER_CMD_TYPE.workerFetch,
	          type: EVENTS.streamRate,
	          value: rate
	        });
	      });
	      decoder.startStreamRateAndStatsInterval();

	      if (options.protocol === PLAYER_PLAY_PROTOCOL.fetch) {
	        input = new OPut(decoder.demuxFlv());
	        fetch(url, {
	          signal: abortController.signal
	        }).then(res => {
	          postMessage({
	            cmd: WORKER_CMD_TYPE.workerFetch,
	            type: EVENTS.streamSuccess
	          });

	          if (supportWritableStream()) {
	            res.body.pipeTo(new WritableStream({
	              write: value => {
	                streamRate(value.byteLength);
	                input.write(value);
	              },
	              close: () => {
	                input = null;
	              },
	              abort: e => {
	                input = null;

	                if (e.name === FETCH_ERROR.abort) {
	                  return;
	                }

	                postMessage({
	                  cmd: WORKER_CMD_TYPE.workerFetch,
	                  type: EVENTS_ERROR.fetchError,
	                  value: e.toString()
	                });
	                abort();
	              }
	            }));
	          } else {
	            const reader = res.body.getReader();

	            const fetchNext = () => {
	              reader.read().then(_ref => {
	                let {
	                  done,
	                  value
	                } = _ref;

	                if (done) {
	                  input = null;
	                  return;
	                }

	                streamRate(value.byteLength);
	                input.write(value);
	                fetchNext();
	              }).catch(e => {
	                input = null;

	                if (e.name === FETCH_ERROR.abort) {
	                  return;
	                }

	                postMessage({
	                  cmd: WORKER_CMD_TYPE.workerFetch,
	                  type: EVENTS_ERROR.fetchError,
	                  value: e.toString()
	                });
	                abort();
	              });
	            };

	            fetchNext();
	          }
	        }).catch(e => {
	          if (e.name === FETCH_ERROR.abort) {
	            return;
	          }

	          postMessage({
	            cmd: WORKER_CMD_TYPE.workerFetch,
	            type: EVENTS_ERROR.fetchError,
	            value: e.toString()
	          });
	          input = null;
	          abort();
	        });
	      } else if (options.protocol === PLAYER_PLAY_PROTOCOL.websocket) {
	        if (options.isFlv) {
	          input = new OPut(decoder.demuxFlv());
	        }

	        socket = new WebSocket(url);
	        socket.binaryType = 'arraybuffer';

	        socket.onopen = () => {
	          decoder.debug.log('worker', 'fetchStream, WebsocketStream  socket open');
	          postMessage({
	            cmd: WORKER_CMD_TYPE.workerFetch,
	            type: EVENTS.streamSuccess
	          });
	        };

	        socket.onclose = () => {
	          decoder.debug.log('worker', 'fetchStream, WebsocketStream  socket close');
	          input = null;
	          postMessage({
	            cmd: WORKER_CMD_TYPE.workerFetch,
	            type: EVENTS.streamEnd
	          });
	        };

	        socket.onerror = error => {
	          decoder.debug.error('worker', 'fetchStream, WebsocketStream  socket error');
	          input = null;
	          postMessage({
	            cmd: WORKER_CMD_TYPE.workerFetch,
	            type: EVENTS_ERROR.websocketError,
	            value: error.toString()
	          });
	        };

	        socket.onmessage = event => {
	          streamRate(event.data.byteLength); // decoder.debug.log('worker',` socket on message isFlv is ${options.isFlv}`);

	          if (options.isFlv) {
	            input.write(event.data); // decoder.debug.log('worker',` input.bufferList.length is ${input.buffer.length},byteLength is ${input.buffer.byteLength}`);
	          } else if (decoder._opt.isNakedFlow) {
	            decoder.demuxNakedFlow(event.data);
	          } else {
	            decoder.demuxM7s(event.data);
	          }
	        };
	      }
	    },
	    demuxFlv: function* () {
	      yield 9;
	      const tmp = new ArrayBuffer(4);
	      const tmp8 = new Uint8Array(tmp);
	      const tmp32 = new Uint32Array(tmp);

	      while (true) {
	        tmp8[3] = 0;
	        const t = yield 15;
	        const type = t[4];
	        tmp8[0] = t[7];
	        tmp8[1] = t[6];
	        tmp8[2] = t[5];
	        const length = tmp32[0];
	        tmp8[0] = t[10];
	        tmp8[1] = t[9];
	        tmp8[2] = t[8];
	        let ts = tmp32[0];

	        if (ts === 0xFFFFFF) {
	          tmp8[3] = t[11];
	          ts = tmp32[0];
	        }

	        const payload = (yield length).slice(); // console.log('demuxFlv type is', type);

	        switch (type) {
	          case FLV_MEDIA_TYPE.audio:
	            decoder.decode(payload, {
	              type: MEDIA_TYPE.audio,
	              ts
	            });
	            break;

	          case FLV_MEDIA_TYPE.video:
	            if (payload.byteLength > 0) {
	              const isIFrame = payload[0] >> 4 === 1;

	              if (decoder.isPlayer) {
	                decoder.calcNetworkDelay(ts);

	                if (isIFrame) {
	                  decoder.calcIframeIntervalTimestamp(ts);
	                }
	              }

	              tmp32[0] = payload[4];
	              tmp32[1] = payload[3];
	              tmp32[2] = payload[2];
	              tmp32[3] = 0;
	              let cts = tmp32[0]; // decoder.debug.log('worker',`demuxFlv dts:${ts},isIFrame:${isIFrame}`);

	              decoder.decode(payload, {
	                type: MEDIA_TYPE.video,
	                ts,
	                isIFrame,
	                cts
	              });
	            }

	            break;
	        }
	      }
	    },
	    decode: function (payload, options) {
	      let playType = decoder._opt.playType;

	      if (options.type === MEDIA_TYPE.audio) {
	        if (decoder._opt.hasAudio) {
	          postMessage({
	            cmd: WORKER_CMD_TYPE.workerFetch,
	            type: EVENTS.streamAbps,
	            value: payload.byteLength
	          });

	          if (playType === PLAY_TYPE.player) {
	            decoder.pushBuffer(payload, {
	              type: options.type,
	              ts: options.ts,
	              cts: options.cts
	            });
	          } else if (playType === PLAY_TYPE.playbackTF) {
	            if (decoder.isPlaybackCacheBeforeDecodeForFpsRender()) {
	              decoder.pushBuffer(payload, {
	                type: options.type,
	                ts: options.ts,
	                cts: options.cts
	              });
	            } else {
	              audioDecoder.decode(payload, options.ts);
	            }
	          }
	        }
	      } else if (options.type === MEDIA_TYPE.video) {
	        if (decoder._opt.hasVideo) {
	          postMessage({
	            cmd: WORKER_CMD_TYPE.workerFetch,
	            type: EVENTS.streamVbps,
	            value: payload.byteLength
	          });
	          postMessage({
	            cmd: WORKER_CMD_TYPE.workerFetch,
	            type: EVENTS.streamDts,
	            value: options.ts
	          });

	          if (playType === PLAY_TYPE.player) {
	            // decoder.debug.log('worker','fetchStream, decode video isIFrame is', options.isIFrame);
	            decoder.pushBuffer(payload, {
	              type: options.type,
	              ts: options.ts,
	              isIFrame: options.isIFrame,
	              cts: options.cts
	            });
	          } else if (playType === PLAY_TYPE.playbackTF) {
	            if (decoder._opt.playbackRate >= decoder._opt.playbackForwardMaxRateDecodeIFrame) {
	              if (options.isIFrame) {
	                decoder.decodeVideo(payload, options.ts, options.isIFrame, options.cts);
	              }
	            } else {
	              if (decoder.isPlaybackCacheBeforeDecodeForFpsRender()) {
	                decoder.pushBuffer(payload, {
	                  type: options.type,
	                  ts: options.ts,
	                  isIFrame: options.isIFrame
	                });
	              } else {
	                if (decoder._opt.playbackRate === 1) {
	                  decoder.pushBuffer(payload, {
	                    type: options.type,
	                    ts: options.ts,
	                    cts: options.cts,
	                    isIFrame: options.isIFrame
	                  });
	                } else {
	                  decoder.decodeVideo(payload, options.ts, options.isIFrame, options.cts);
	                }
	              }
	            }
	          }
	        }
	      }
	    },
	    setCodecAudio: function (payload) {
	      const codecId = payload[0] >> 4;

	      if (isAacCodecPacket(payload) || codecId === AUDIO_ENC_CODE.ALAW || codecId === AUDIO_ENC_CODE.MULAW) {
	        // decoder.debug.log('worker','decodeAudio:hasInitAudioCodec');
	        const extraData = codecId === AUDIO_ENC_CODE.AAC ? payload.slice(2) : payload.slice(1);
	        audioDecoder.setCodec(codecId, decoder._opt.sampleRate, extraData);
	        hasInitAudioCodec = true;
	      } else {
	        decoder.debug.warn('worker', 'decodeVideo: hasInitAudioCodec is false, codecId is ', codecId);
	      }
	    },
	    decodeAudio: function (payload, ts) {
	      const codecId = payload[0] >> 4; // decoder.debug.log('worker', 'decodeAudio:', ts);

	      if (!hasInitAudioCodec) {
	        decoder.setCodecAudio(payload);
	      } else {
	        // decoder.debug.log('worker','decodeAudio and codecId is ');
	        audioDecoder.decode(codecId === AUDIO_ENC_CODE.AAC ? payload.slice(2) : payload.slice(1), ts);
	      }
	    },
	    setCodecVideo: function (payload) {
	      const codecId = payload[0] & 0x0F;

	      if (payload[0] >> 4 === FRAME_TYPE.keyFrame && payload[1] === AVC_PACKET_TYPE.sequenceHeader) {
	        if (codecId === VIDEO_ENC_CODE.h264 || codecId === VIDEO_ENC_CODE.h265) {
	          decoder.debug.log('worker', 'decodeVideo: init video codec');
	          hasInitVideoCodec = true;
	          const extraData = payload.slice(5);
	          videoDecoder.setCodec(codecId, extraData); // todo: 待删除。。。。。。。。。

	          if (decoder._opt.recordType === FILE_SUFFIX.mp4) {
	            postMessage({
	              cmd: WORKER_CMD_TYPE.videoCodec,
	              buffer: payload,
	              codecId
	            }, [payload.buffer]);
	          }
	        }
	      } else {
	        decoder.debug.warn('worker', `decodeVideo: hasInitVideoCodec is false, codecId is ${codecId} and`);
	      }
	    },
	    decodeVideo: function (payload, ts, isIFrame, cts) {
	      if (!hasInitVideoCodec) {
	        decoder.setCodecVideo(payload);
	      } else {
	        if (!isVideoFirstIFrame && isIFrame) {
	          isVideoFirstIFrame = true;
	        }

	        if (isVideoFirstIFrame) {
	          // todo:待删除。。。。。。
	          // decoder.debug.log('worker', 'decodeVideo:', ts, isIFrame);
	          const buffer = payload.slice(5); // todo:待删除。。。。。。
	          // console.log(buffer)

	          videoDecoder.decode(buffer, isIFrame ? 1 : 0, ts);

	          if (decoder._opt.isRecording && decoder._opt.recordType === FILE_SUFFIX.mp4) {
	            postMessage({
	              cmd: WORKER_CMD_TYPE.videoNalu,
	              buffer: buffer,
	              isIFrame,
	              ts,
	              cts
	            }, [payload.buffer]);
	          }
	        } else {
	          decoder.debug.warn('worker', 'decodeVideo: first frame is not iframe');
	        }
	      }
	    },
	    clearBuffer: function () {
	      let needClear = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : false;
	      decoder.debug.log('worker', `clearBuffer,bufferList length is ${bufferList.length}, need clear is ${needClear}`);

	      if (needClear) {
	        bufferList = [];
	      }

	      decoder.resetDelay();
	      decoder.dropping = true;
	    },
	    dropBuffer: function () {
	      let notDropDelay = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : false;
	      let maxBufferLength = DOCUMENT_ONE_SECOND_BUFFER_LENGTH * ((decoder._opt.videoBuffer + decoder._opt.videoBufferDelay) / 1000);

	      if (notDropDelay) {
	        maxBufferLength = DOCUMENT_ONE_SECOND_BUFFER_LENGTH * (decoder._opt.videoBuffer / 1000);
	      }

	      while (bufferList.length > 0) {
	        let data = bufferList[0];

	        if (data.isIFrame && bufferList.length < maxBufferLength) {
	          decoder.debug.log('worker', `dropBuffer end isIFrame ${data.isIFrame} and delay is ${decoder.delay} and bufferlist is ${bufferList.length}`);
	          break;
	        }

	        decoder.debug.warn('worker', `dropBuffer is dropping and isIFrame ${data.isIFrame} and delay is ${decoder.delay} and bufferlist is ${bufferList.length}`); // just for simd decode

	        if (data.type === MEDIA_TYPE.video) {
	          decoder.doDecode(data);
	        }

	        bufferList.shift();
	      }
	    },
	    demuxM7s: function (arrayBuffer) {
	      const dv = new DataView(arrayBuffer);
	      const ts = dv.getUint32(1, false);
	      const type = dv.getUint8(0); // decoder.debug.log('worker', `demuxM7s , type is ${type}`);

	      switch (type) {
	        case MEDIA_TYPE.audio:
	          decoder.decode(new Uint8Array(arrayBuffer, 5), {
	            type: MEDIA_TYPE.audio,
	            ts
	          });
	          break;

	        case MEDIA_TYPE.video:
	          if (dv.byteLength > 5) {
	            const isIFrame = dv.getUint8(5) >> 4 === 1;

	            if (decoder.isPlayer) {
	              decoder.calcNetworkDelay(ts);

	              if (isIFrame) {
	                decoder.calcIframeIntervalTimestamp(ts);
	              }
	            }

	            decoder.decode(new Uint8Array(arrayBuffer, 5), {
	              type: MEDIA_TYPE.video,
	              ts,
	              isIFrame
	            });
	          }

	          break;
	      }
	    },
	    demuxNakedFlow: function (arrayBuffer) {
	      decoder.debug.log('worker', `demuxNakedFlow, arrayBuffer length is ${arrayBuffer.byteLength}`); // console.log('demuxNakedFlow', arrayBuffer);

	      nakedFlowDemuxer.dispatch(arrayBuffer);
	    },
	    calcNetworkDelay: function (dts) {
	      if (!(isVideoFirstIFrame && dts > 0)) {
	        return;
	      }

	      if (bufferStartDts === null) {
	        bufferStartDts = dts;
	        bufferStartLocalTs = now();
	      }

	      const diff1 = dts - bufferStartDts;
	      const localDiff = now() - bufferStartLocalTs;
	      const delay = localDiff > diff1 ? localDiff - diff1 : 0;

	      if (delay > decoder._opt.networkDelay && decoder._opt.playType === PLAY_TYPE.player) {
	        decoder.debug.log('worker', `demuxFlv now dts:${dts}, vs start is ${diff1},local diff is ${localDiff} ,delay is ${delay}`);
	        postMessage({
	          cmd: WORKER_CMD_TYPE.workerFetch,
	          type: EVENTS.networkDelayTimeout,
	          value: delay
	        });
	      }

	      postMessage({
	        cmd: WORKER_CMD_TYPE.workerFetch,
	        type: EVENTS.netBuf,
	        value: delay
	      });
	    },
	    calcIframeIntervalTimestamp: function (ts) {
	      if (preIframeTs === null) {
	        preIframeTs = ts;
	      } else {
	        if (preIframeTs < ts) {
	          const intervalTimestamp = ts - preIframeTs;
	          postMessage({
	            cmd: WORKER_CMD_TYPE.iframeIntervalTs,
	            value: intervalTimestamp
	          }); // post 到主线程里面去。

	          preIframeTs = ts;
	        }
	      }
	    },
	    isPlaybackCacheBeforeDecodeForFpsRender: function () {
	      return !decoder.isPlayer && decoder._opt.playbackIsCacheBeforeDecodeForFpsRender;
	    },
	    videoInfo: function (videoCode, width, height) {
	      postMessage({
	        cmd: WORKER_CMD_TYPE.videoCode,
	        code: videoCode
	      });
	      postMessage({
	        cmd: WORKER_CMD_TYPE.initVideo,
	        w: width,
	        h: height
	      });
	      videoWidth = width;
	      videoHeight = height;

	      if (decoder.useOffscreen()) {
	        decoder.offscreenCanvas = new OffscreenCanvas(width, height);
	        decoder.offscreenCanvasGL = decoder.offscreenCanvas.getContext("webgl");
	        decoder.webglObj = createWebGL(decoder.offscreenCanvasGL, decoder._opt.openWebglAlignment);
	      }
	    },
	    audioInfo: function (audioCode, sampleRate, channels) {
	      postMessage({
	        cmd: WORKER_CMD_TYPE.audioCode,
	        code: audioCode
	      });
	      postMessage({
	        cmd: WORKER_CMD_TYPE.initAudio,
	        sampleRate: sampleRate,
	        channels: channels
	      });
	      audioChannels = channels;
	    },
	    yuvData: function (yuv, ts) {
	      const size = videoWidth * videoHeight * 3 / 2;
	      let out = Module.HEAPU8.subarray(yuv, yuv + size);
	      let data = new Uint8Array(out);

	      if (decoder.useOffscreen()) {
	        decoder.webglObj.renderYUV(videoWidth, videoHeight, data);
	        let image_bitmap = decoder.offscreenCanvas.transferToImageBitmap();
	        postMessage({
	          cmd: WORKER_CMD_TYPE.render,
	          buffer: image_bitmap,
	          delay: decoder.delay,
	          ts
	        }, [image_bitmap]);
	      } else {
	        postMessage({
	          cmd: WORKER_CMD_TYPE.render,
	          output: data,
	          delay: decoder.delay,
	          ts
	        }, [data.buffer]);
	      }
	    },
	    pcmData: function (data, len, ts) {
	      let frameCount = len;
	      let origin = [];
	      let start = 0;
	      let audioBufferSize = decoder._opt.audioBufferSize;

	      for (let channel = 0; channel < 2; channel++) {
	        let fp = Module.HEAPU32[(data >> 2) + channel] >> 2;
	        origin[channel] = Module.HEAPF32.subarray(fp, fp + frameCount);
	      }

	      if (audioRemain) {
	        len = audioBufferSize - audioRemain;

	        if (frameCount >= len) {
	          audioOutputArray[0] = Float32Array.of(...tempAudioBuffer[0], ...origin[0].subarray(0, len));

	          if (audioChannels == 2) {
	            audioOutputArray[1] = Float32Array.of(...tempAudioBuffer[1], ...origin[1].subarray(0, len));
	          }

	          postMessage({
	            cmd: WORKER_CMD_TYPE.playAudio,
	            buffer: audioOutputArray,
	            ts
	          }, audioOutputArray.map(x => x.buffer));
	          start = len;
	          frameCount -= len;
	        } else {
	          audioRemain += frameCount;
	          tempAudioBuffer[0] = Float32Array.of(...tempAudioBuffer[0], ...origin[0]);

	          if (audioChannels == 2) {
	            tempAudioBuffer[1] = Float32Array.of(...tempAudioBuffer[1], ...origin[1]);
	          }

	          return;
	        }
	      }

	      for (audioRemain = frameCount; audioRemain >= audioBufferSize; audioRemain -= audioBufferSize) {
	        audioOutputArray[0] = origin[0].slice(start, start += audioBufferSize);

	        if (audioChannels == 2) {
	          audioOutputArray[1] = origin[1].slice(start - audioBufferSize, start);
	        }

	        postMessage({
	          cmd: WORKER_CMD_TYPE.playAudio,
	          buffer: audioOutputArray,
	          ts
	        }, audioOutputArray.map(x => x.buffer));
	      }

	      if (audioRemain) {
	        tempAudioBuffer[0] = origin[0].slice(start);

	        if (audioChannels == 2) {
	          tempAudioBuffer[1] = origin[1].slice(start);
	        }
	      }
	    },
	    timeEnd: function () {
	      postMessage({
	        cmd: WORKER_CMD_TYPE.workerEnd
	      });
	    }
	  };
	  decoder.debug = new Debug(decoder);
	  let audioDecoder = new Module.AudioDecoder(decoder);
	  let videoDecoder = new Module.VideoDecoder(decoder);
	  postMessage({
	    cmd: WORKER_SEND_TYPE.init
	  });

	  self.onmessage = function (event) {
	    let msg = event.data;

	    switch (msg.cmd) {
	      case WORKER_SEND_TYPE.init:
	        try {
	          decoder._opt = Object.assign(decoder._opt, JSON.parse(msg.opt));
	        } catch (e) {}

	        decoder.init();
	        break;

	      case WORKER_SEND_TYPE.decode:
	        decoder.pushBuffer(msg.buffer, msg.options);
	        break;

	      case WORKER_SEND_TYPE.audioDecode:
	        decoder.decodeAudio(msg.buffer, msg.ts);
	        break;

	      case WORKER_SEND_TYPE.videoDecode:
	        decoder.decodeVideo(msg.buffer, msg.ts, msg.isIFrame);
	        break;

	      case WORKER_SEND_TYPE.clearBuffer:
	        decoder.clearBuffer(msg.needClear);
	        break;

	      case WORKER_SEND_TYPE.fetchStream:
	        decoder.fetchStream(msg.url, JSON.parse(msg.opt));
	        break;

	      case WORKER_SEND_TYPE.close:
	        decoder.close();
	        break;

	      case WORKER_SEND_TYPE.updateConfig:
	        decoder._opt[msg.key] = msg.value;

	        if (msg.key === 'playbackRate') {
	          if (decoder.isPlaybackCacheBeforeDecodeForFpsRender()) {
	            decoder.playbackCacheLoop();
	          }
	        }

	        break;
	    }
	  };
	}

	decoderProSimd.postRun = function () {
	  workerPostRun(decoderProSimd);
	};

}));
//# sourceMappingURL=decoder-pro-simd.js.map
