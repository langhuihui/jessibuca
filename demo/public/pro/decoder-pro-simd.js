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
	    "f": __embind_register_class_function,
	    "D": __embind_register_emval,
	    "o": __embind_register_float,
	    "c": __embind_register_integer,
	    "b": __embind_register_memory_view,
	    "p": __embind_register_std_string,
	    "k": __embind_register_std_wstring,
	    "q": __embind_register_void,
	    "j": __emscripten_date_now,
	    "e": __emval_call_void_method,
	    "g": __emval_decref,
	    "d": __emval_get_method_caller,
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

	  let _programInfo = {
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

	  let _buffers = _initBuffers();

	  let _rgbatexture = _createTexture();

	  let _ytexture = _createTexture();

	  let _utexture = _createTexture();

	  let _vtexture = _createTexture();

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
	      _programInfo = null;
	      _buffers = null;
	      _rgbatexture = null;
	      _ytexture = null;
	      _utexture = null;
	      _vtexture = null;
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
	};
	const PLAYER_STREAM_TYPE = {
	  fetch: 'fetch',
	  hls: 'hls',
	  websocket: 'websocket',
	  webrtc: 'webrtc',
	  webTransport: 'webTransport',
	  worker: 'worker'
	}; // 播放

	const PLAY_TYPE = {
	  player: "player",
	  playerAudio: 'playerAudio',
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
	const DEBUG_LEVEL = {
	  debug: 'debug',
	  warn: 'warn'
	};
	const PTZ_ACTIVE_EVENT_TYPE = {
	  click: 'click',
	  mouseDownAndUp: 'mouseDownAndUp'
	};
	const DEFAULT_PLAYBACK_FORWARD_MAX_RATE_DECODE_IFRAME = 4; // default playback forward max rate decode iframe

	const FRAME_TS_MAX_DIFF = 1000 * 60 * 60; // 1 hour

	const SIMD_H264_DECODE_MAX_WIDTH = 4080;
	const VIDEO_PAYLOAD_MIN_SIZE = 12; // video payload min size: 12 byte

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
	  // hls（inner）
	  isWebrtc: false,
	  // webrtc (inner)
	  isWebrtcForZLM: false,
	  // webrtc for ZLM (inner)
	  isNakedFlow: false,
	  // 是否是裸流（264、265）
	  debug: false,
	  // debug log
	  debugLevel: DEBUG_LEVEL.warn,
	  // log level
	  debugUuid: '',
	  // debug uuid (inner)
	  isMulti: false,
	  // 是否多实例播放
	  hotKey: false,
	  // 快捷键
	  loadingTimeout: 10,
	  // loading timeout
	  heartTimeout: 10,
	  // heart timeout
	  timeout: 10,
	  // second
	  pageVisibilityHiddenTimeout: 5 * 60,
	  //   5 * 60  = 5 minute
	  loadingTimeoutReplay: true,
	  // loading timeout replay
	  heartTimeoutReplay: true,
	  // heart timeout replay。
	  loadingTimeoutReplayTimes: 3,
	  // loading timeout replay fail times
	  heartTimeoutReplayTimes: 3,
	  // heart timeout replay fail times
	  heartTimeoutReplayUseLastFrameShow: false,
	  // heart timeout replay use last frame
	  replayUseLastFrameShow: false,
	  // replay use last frame
	  supportDblclickFullscreen: false,
	  showBandwidth: false,
	  //
	  showPerformance: false,
	  // 是否显示性能面板
	  mseCorrectTimeDuration: 20,
	  // mse correct time duration 20ms
	  keepScreenOn: true,
	  // keep screen on
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
	    aiFace: false,
	    aiObject: false,
	    fullscreenFn: null,
	    fullscreenExitFn: null,
	    screenshotFn: null,
	    playFn: null,
	    pauseFn: null,
	    recordFn: null,
	    recordStopFn: null
	  },
	  extendOperateBtns: [],
	  contextmenuBtns: [],
	  watermarkConfig: {},
	  // 局部水印设置
	  controlAutoHide: false,
	  hasControl: false,
	  // inner params
	  loadingIcon: true,
	  loadingText: '',
	  background: '',
	  backgroundLoadingShow: false,
	  // 加载过程中是否显示背景
	  loadingBackground: '',
	  // 内部参数 重新播放过程中，显示播放失败前的最后一帧数据。
	  decoder: 'decoder-pro.js',
	  decoderAudio: 'decoder-pro-audio.js',
	  decoderWASM: '',
	  url: '',
	  // inner
	  rotate: 0,
	  mirrorRotate: 'none',
	  // 镜像xx
	  playbackConfig: {
	    playList: [],
	    // {start:xx,end:xx}
	    fps: '',
	    // fps值
	    showControl: true,
	    showRateBtn: false,
	    rateConfig: [],
	    // 播放倍率切换，支持[{label:'正常',value:1},{label:'2倍',value:2}]
	    isCacheBeforeDecodeForFpsRender: false,
	    // rfs渲染时，是否在解码前缓存数据
	    uiUsePlaybackPause: false,
	    // ui上面是否使用 playbackPause 方法
	    isPlaybackPauseClearCache: true,
	    // playbackPause是否清除缓存数据
	    isUseFpsRender: false,
	    // 是否使用固定的fps渲染，如果设置的fps小于流推过来的，会造成内存堆积甚至溢出
	    isUseLocalCalculateTime: false,
	    // 是否使用本地时间来计算playback时间
	    localOneFrameTimestamp: 40,
	    // 一帧 40ms, isUseLocalCalculateTime 为 true 生效。
	    supportWheel: false // 是否支持滚动轴切换精度。

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
	  // 内部参数
	  demuxType: DEMUX_TYPE.flv,
	  // 内部参数
	  useWasm: false,
	  //wasm 解码 （inner）默认
	  useWCS: false,
	  //
	  useSIMD: true,
	  // pro 默认优先使用wasm simd解码，不支持则使用wasm解码
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
	  //  内部参数
	  useWebGPU: false,
	  //  是否使用webgpu引擎
	  mseDecodeErrorReplay: true,
	  // mse 解码失败重新播放
	  wcsDecodeErrorReplay: true,
	  // wcs 解码失败重新播放
	  wasmDecodeErrorReplay: true,
	  // 解码失败重新播放。
	  autoWasm: true,
	  // 自动降级到 wasm 模式
	  webglAlignmentErrorReplay: true,
	  // webgl对齐失败重新播放。
	  webglContextLostErrorReplay: true,
	  // webgl context lost 重新播放。
	  openWebglAlignment: false,
	  //  https://github.com/langhuihui/jessibuca/issues/152
	  syncAudioAndVideo: false,
	  // 音视频同步
	  // playback config
	  playbackDelayTime: 1000,
	  // TF卡流播放延迟时间  Inner
	  playbackFps: 25,
	  // Inner
	  playbackForwardMaxRateDecodeIFrame: DEFAULT_PLAYBACK_FORWARD_MAX_RATE_DECODE_IFRAME,
	  // max rate render i frame , Inner
	  playbackCurrentTimeMove: true,
	  // Inner
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
	  useFaceDetector: false,
	  // 使用人脸检测
	  useObjectDetector: false,
	  // 使用物体检测
	  ptzClickType: PTZ_ACTIVE_EVENT_TYPE.click,
	  // PTZ 点击类型
	  ptzStopEmitDelay: 0.3,
	  // ptz停止后，停止发送指令的延迟时间(秒)
	  ptzZoomShow: false,
	  // ptz操作是否显示放大缩小操作
	  ptzApertureShow: false,
	  // ptz操作是否显示光圈操作
	  ptzFocusShow: false,
	  // ptz操作是否显示聚焦操作
	  ptzMoreArrow: false,
	  // ptz操作是否显示更多箭头
	  // 微信安卓音频播放块大小
	  // 计算规则 48000 * ms / 1000 = size,
	  // 48000 * 200 /1000 = 9600  播放时长200ms
	  // 48000 * 175 /1000 = 8400  播放时长175ms
	  // 48000 * 150 /1000 = 7200  播放时长150ms
	  // 48000 * 125 /1000 = 6000  播放时长125ms
	  // 48000 * 100 /1000 = 4800  播放时长100ms default
	  // 48000 * 50 /1000 = 2400  播放时长50ms
	  // 48000 * 25 /1000 = 1200  播放时长25ms
	  // 48000 * 10 /1000 = 480  播放时长10ms
	  weiXinInAndroidAudioBufferSize: 4800,
	  isCrypto: false,
	  // 是否加密
	  cryptoKey: '',
	  // 加密key
	  cryptoIV: '',
	  // 加密iv
	  cryptoKeyUrl: '',
	  // 加密key获取域名
	  autoResize: false,
	  // 自动调整大小 inner
	  useWebFullScreen: false,
	  // 使用web全屏(旋转播放器90度)(只会在移动端生效)
	  ptsMaxDiff: 60 * 60,
	  //单位(秒)，默认值是1H
	  aiFaceDetectWidth: 192,
	  // 人脸检测宽度
	  aiObjectDetectWidth: 192,
	  // 物体检测宽度
	  videoRenderSupportScale: true,
	  // video渲染支持Scale
	  mediaSourceTsIsMaxDiffReplay: true,
	  // 当ts间隔超过最大值之后，重新播放
	  controlHtml: '',
	  // 自定义控制栏Html
	  isH265: false,
	  // 是否是h265,
	  supportLockScreenPlayAudio: true,
	  // 是否支持锁屏播放音频（mobile,ipad 端）
	  supportHls265: false // 是否支持hls265

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
	  playbackStreamVideoFps: 'playbackStreamVideoFps',
	  wasmDecodeVideoNoResponseError: 'wasmDecodeVideoNoResponseError',
	  wasmWidthOrHeightChange: 'wasmWidthOrHeightChange',
	  simdH264DecodeVideoWidthIsTooLarge: 'simdH264DecodeVideoWidthIsTooLarge',
	  websocketOpen: 'websocketOpen',
	  closeEnd: 'closeEnd'
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
	  fetchStream: 'fetchStream',
	  sendWsMessage: 'sendWsMessage'
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
	  delayTimeoutRetryEnd: 'delayTimeoutRetryEnd',
	  loadingTimeout: 'loadingTimeout',
	  loadingTimeoutRetryEnd: 'loadingTimeoutRetryEnd',
	  stats: 'stats',
	  performance: "performance",
	  faceDetectActive: 'faceDetectActive',
	  objectDetectActive: 'objectDetectActive',
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
	  mseAddSourceBufferError: 'mseAddSourceBufferError',
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
	  playbackRateChange: 'playbackRateChange',
	  playbackPreRateChange: 'playbackPreRateChange',
	  ptz: 'ptz',
	  streamQualityChange: 'streamQualityChange',
	  visibilityChange: "visibilityChange",
	  netBuf: 'netBuf',
	  close: 'close',
	  networkDelayTimeout: 'networkDelayTimeout',
	  togglePerformancePanel: 'togglePerformancePanel',
	  viewResizeChange: 'viewResizeChange',
	  flvDemuxBufferSizeTooLarge: 'flvDemuxBufferSizeTooLarge',
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
	  crashLog: 'crashLog',
	  // dom
	  focus: 'focus',
	  blur: 'blur',
	  visibilityHiddenTimeout: 'visibilityHiddenTimeout',
	  // websocket
	  websocketOpen: 'websocketOpen',
	  websocketClose: 'websocketClose',
	  websocketError: 'websocketError',
	  websocketMessage: 'websocketMessage',
	  // ai
	  aiObjectDetectorInfo: 'aiObjectDetectorInfo',
	  aiFaceDetector: 'aiFaceDetector',
	  //
	  playFailedAndPaused: 'playFailedAndPaused',
	  // audio
	  audioResumeState: 'audioResumeState'
	}; // jessibuca events
	const EVENTS_ERROR = {
	  playError: 'playIsNotPauseOrUrlIsNull',
	  fetchError: "fetchError",
	  websocketError: 'websocketError',
	  webcodecsH265NotSupport: 'webcodecsH265NotSupport',
	  webcodecsDecodeError: 'webcodecsDecodeError',
	  webcodecsUnsupportedConfigurationError: 'webcodecsUnsupportedConfigurationError',
	  mediaSourceH265NotSupport: 'mediaSourceH265NotSupport',
	  mediaSourceDecoderConfigurationError: 'mediaSourceDecoderConfigurationError',
	  mediaSourceFull: EVENTS.mseSourceBufferFull,
	  mseSourceBufferError: EVENTS.mseSourceBufferError,
	  mseAddSourceBufferError: EVENTS.mseAddSourceBufferError,
	  mediaSourceAppendBufferError: 'mediaSourceAppendBufferError',
	  mediaSourceBufferListLarge: 'mediaSourceBufferListLarge',
	  mediaSourceAppendBufferEndTimeout: 'mediaSourceAppendBufferEndTimeout',
	  mediaSourceTsIsMaxDiff: 'mediaSourceTsIsMaxDiff',
	  mediaSourceUseCanvasRenderPlayFailed: 'mediaSourceUseCanvasRenderPlayFailed',
	  wasmDecodeError: 'wasmDecodeError',
	  wasmUseVideoRenderError: 'wasmUseVideoRenderError',
	  hlsError: 'hlsError',
	  webrtcError: 'webrtcError',
	  webglAlignmentError: 'webglAlignmentError',
	  wasmWidthOrHeightChange: 'wasmWidthOrHeightChange',
	  mseWidthOrHeightChange: 'mseWidthOrHeightChange',
	  wcsWidthOrHeightChange: 'wcsWidthOrHeightChange',
	  tallWebsocketClosedByError: 'tallWebsocketClosedByError',
	  flvDemuxBufferSizeTooLarge: EVENTS.flvDemuxBufferSizeTooLarge,
	  wasmDecodeVideoNoResponseError: 'wasmDecodeVideoNoResponseError',
	  audioChannelError: 'audioChannelError',
	  simdH264DecodeVideoWidthIsTooLarge: 'simdH264DecodeVideoWidthIsTooLarge',
	  webglContextLostError: 'webglContextLostError'
	};
	const WEBSOCKET_STATUS_CODE = {
	  connecting: 0,
	  open: 1,
	  closing: 2,
	  closed: 3
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
	  MULAW: 8,
	  MP3: 2
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
	const VIDEO_ENCODE_TYPE = {
	  h264: 'avc',
	  h265: 'hevc'
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
	const experienceTimeout = '1';
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
	  let nextIndex = 1;

	  if (bufferList.length > 0) {
	    let nextItem = bufferList[1];

	    if (nextItem && nextItem.ts - firstItem.ts > 100000) {
	      firstItem = nextItem;
	      nextIndex = 2;
	    }
	  }

	  if (firstItem) {
	    // next start
	    for (let i = nextIndex; i < bufferList.length; i++) {
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
	function isFetchSuccess(res) {
	  return res.ok && res.status >= 200 && res.status <= 299;
	} // stroke rect or text in canvas
	function clone(obj) {
	  let result = ''; //

	  if (typeof obj === 'object') {
	    try {
	      result = JSON.stringify(obj);
	      result = JSON.parse(result);
	    } catch (e) {
	      result = obj;
	    }
	  } else {
	    result = obj;
	  }

	  return result;
	}
	/**
	 *
	 * @returns {object:DEFAULT_PLAYER_OPTIONS}
	 */

	function getDefaultPlayerOptions() {
	  return clone(DEFAULT_PLAYER_OPTIONS);
	}
	function isTrue(value) {
	  return value === true || value === 'true';
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
	      if (master._opt.debug && master._opt.debugLevel == DEBUG_LEVEL.debug) {
	        const prefix = master._opt.debugUuid ? `[${master._opt.debugUuid}]` : '';

	        for (var _len = arguments.length, args = new Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
	          args[_key - 1] = arguments[_key];
	        }

	        console.log(`JbPro${prefix}:[\u2705\u2705\u2705][${name}]`, ...args);
	      }
	    };

	    this.warn = function (name) {
	      if (master._opt.debug && (master._opt.debugLevel == DEBUG_LEVEL.debug || master._opt.debugLevel == DEBUG_LEVEL.warn)) {
	        const prefix = master._opt.debugUuid ? `[${master._opt.debugUuid}]` : '';

	        for (var _len2 = arguments.length, args = new Array(_len2 > 1 ? _len2 - 1 : 0), _key2 = 1; _key2 < _len2; _key2++) {
	          args[_key2 - 1] = arguments[_key2];
	        }

	        console.log(`JbPro${prefix}:[\u2757\u2757\u2757][${name}]`, ...args);
	      }
	    };

	    this.error = function (name) {
	      const prefix = master._opt.debugUuid ? `[${master._opt.debugUuid}]` : '';

	      for (var _len3 = arguments.length, args = new Array(_len3 > 1 ? _len3 - 1 : 0), _key3 = 1; _key3 < _len3; _key3++) {
	        args[_key3 - 1] = arguments[_key3];
	      }

	      console.error(`JbPro${prefix}:[\u274C\u274C\u274C][${name}]`, ...args);
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

	/*
	 * Copyright (C) 2016 Bilibili. All Rights Reserved.
	 *
	 * @author zheng qian <xqq@xqq.im>
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
	 * you may not use this file except in compliance with the License.
	 * You may obtain a copy of the License at
	 *
	 *     http://www.apache.org/licenses/LICENSE-2.0
	 *
	 * Unless required by applicable law or agreed to in writing, software
	 * distributed under the License is distributed on an "AS IS" BASIS,
	 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	 * See the License for the specific language governing permissions and
	 * limitations under the License.
	 */

	class SPSParser {
	  static _ebsp2rbsp(uint8array) {
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
	  } // 解析 SPS
	  // https://zhuanlan.zhihu.com/p/27896239


	  static parseSPS(uint8array) {
	    let rbsp = SPSParser._ebsp2rbsp(uint8array);

	    let gb = new ExpGolomb(rbsp);
	    gb.readByte(); // 标识当前H.264码流的profile。
	    // 我们知道，H.264中定义了三种常用的档次profile： 基准档次：baseline profile;主要档次：main profile; 扩展档次：extended profile;

	    let profile_idc = gb.readByte(); // profile_idc

	    gb.readByte(); // constraint_set_flags[5] + reserved_zero[3]
	    // 标识当前码流的Level。编码的Level定义了某种条件下的最大视频分辨率、最大视频帧率等参数，码流所遵从的level由level_idc指定。

	    let level_idc = gb.readByte(); // level_idc
	    // 表示当前的序列参数集的id。通过该id值，图像参数集pps可以引用其代表的sps中的参数。

	    gb.readUEG(); // seq_parameter_set_id

	    let profile_string = SPSParser.getProfileString(profile_idc);
	    let level_string = SPSParser.getLevelString(level_idc);
	    let chroma_format_idc = 1;
	    let chroma_format = 420;
	    let chroma_format_table = [0, 420, 422, 444];
	    let bit_depth = 8; //

	    if (profile_idc === 100 || profile_idc === 110 || profile_idc === 122 || profile_idc === 244 || profile_idc === 44 || profile_idc === 83 || profile_idc === 86 || profile_idc === 118 || profile_idc === 128 || profile_idc === 138 || profile_idc === 144) {
	      //
	      chroma_format_idc = gb.readUEG();

	      if (chroma_format_idc === 3) {
	        gb.readBits(1); // separate_colour_plane_flag
	      }

	      if (chroma_format_idc <= 3) {
	        chroma_format = chroma_format_table[chroma_format_idc];
	      }

	      bit_depth = gb.readUEG() + 8; // bit_depth_luma_minus8

	      gb.readUEG(); // bit_depth_chroma_minus8

	      gb.readBits(1); // qpprime_y_zero_transform_bypass_flag

	      if (gb.readBool()) {
	        // seq_scaling_matrix_present_flag
	        let scaling_list_count = chroma_format_idc !== 3 ? 8 : 12;

	        for (let i = 0; i < scaling_list_count; i++) {
	          if (gb.readBool()) {
	            // seq_scaling_list_present_flag
	            if (i < 6) {
	              SPSParser._skipScalingList(gb, 16);
	            } else {
	              SPSParser._skipScalingList(gb, 64);
	            }
	          }
	        }
	      }
	    } // 用于计算MaxFrameNum的值。计算公式为MaxFrameNum = 2^(log2_max_frame_num_minus4 +


	    gb.readUEG(); // log2_max_frame_num_minus4
	    // 表示解码picture order count(POC)的方法。POC是另一种计量图像序号的方式，与frame_num有着不同的计算方法。该语法元素的取值为0、1或2。

	    let pic_order_cnt_type = gb.readUEG();

	    if (pic_order_cnt_type === 0) {
	      gb.readUEG(); // log2_max_pic_order_cnt_lsb_minus_4
	    } else if (pic_order_cnt_type === 1) {
	      gb.readBits(1); // delta_pic_order_always_zero_flag

	      gb.readSEG(); // offset_for_non_ref_pic

	      gb.readSEG(); // offset_for_top_to_bottom_field

	      let num_ref_frames_in_pic_order_cnt_cycle = gb.readUEG();

	      for (let i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) {
	        gb.readSEG(); // offset_for_ref_frame
	      }
	    } // 用于表示参考帧的最大数目。


	    let ref_frames = gb.readUEG(); // max_num_ref_frames
	    // 标识位，说明frame_num中是否允许不连续的值。

	    gb.readBits(1); // gaps_in_frame_num_value_allowed_flag
	    // 用于计算图像的宽度。单位为宏块个数，因此图像的实际宽度为:

	    let pic_width_in_mbs_minus1 = gb.readUEG(); // 使用PicHeightInMapUnits来度量视频中一帧图像的高度。
	    // PicHeightInMapUnits并非图像明确的以像素或宏块为单位的高度，而需要考虑该宏块是帧编码或场编码。PicHeightInMapUnits的计算方式为：

	    let pic_height_in_map_units_minus1 = gb.readUEG(); // 标识位，说明宏块的编码方式。当该标识位为0时，宏块可能为帧编码或场编码；
	    // 该标识位为1时，所有宏块都采用帧编码。根据该标识位取值不同，PicHeightInMapUnits的含义也不同，
	    // 为0时表示一场数据按宏块计算的高度，为1时表示一帧数据按宏块计算的高度。

	    let frame_mbs_only_flag = gb.readBits(1);

	    if (frame_mbs_only_flag === 0) {
	      // 标识位，说明是否采用了宏块级的帧场自适应编码。当该标识位为0时，不存在帧编码和场编码之间的切换；当标识位为1时，宏块可能在帧编码和场编码模式之间进行选择。
	      gb.readBits(1); // mb_adaptive_frame_field_flag
	    } // 标识位，用于B_Skip、B_Direct模式运动矢量的推导计算。


	    gb.readBits(1); // direct_8x8_inference_flag

	    let frame_crop_left_offset = 0;
	    let frame_crop_right_offset = 0;
	    let frame_crop_top_offset = 0;
	    let frame_crop_bottom_offset = 0;
	    let frame_cropping_flag = gb.readBool();

	    if (frame_cropping_flag) {
	      frame_crop_left_offset = gb.readUEG();
	      frame_crop_right_offset = gb.readUEG();
	      frame_crop_top_offset = gb.readUEG();
	      frame_crop_bottom_offset = gb.readUEG();
	    }

	    let sar_width = 1,
	        sar_height = 1;
	    let fps = 0,
	        fps_fixed = true,
	        fps_num = 0,
	        fps_den = 0; // 标识位，说明SPS中是否存在VUI信息。

	    let vui_parameters_present_flag = gb.readBool();

	    if (vui_parameters_present_flag) {
	      if (gb.readBool()) {
	        // aspect_ratio_info_present_flag
	        let aspect_ratio_idc = gb.readByte();
	        let sar_w_table = [1, 12, 10, 16, 40, 24, 20, 32, 80, 18, 15, 64, 160, 4, 3, 2];
	        let sar_h_table = [1, 11, 11, 11, 33, 11, 11, 11, 33, 11, 11, 33, 99, 3, 2, 1];

	        if (aspect_ratio_idc > 0 && aspect_ratio_idc < 16) {
	          sar_width = sar_w_table[aspect_ratio_idc - 1];
	          sar_height = sar_h_table[aspect_ratio_idc - 1];
	        } else if (aspect_ratio_idc === 255) {
	          sar_width = gb.readByte() << 8 | gb.readByte();
	          sar_height = gb.readByte() << 8 | gb.readByte();
	        }
	      }

	      if (gb.readBool()) {
	        // overscan_info_present_flag
	        gb.readBool(); // overscan_appropriate_flag
	      }

	      if (gb.readBool()) {
	        // video_signal_type_present_flag
	        gb.readBits(4); // video_format & video_full_range_flag

	        if (gb.readBool()) {
	          // colour_description_present_flag
	          gb.readBits(24); // colour_primaries & transfer_characteristics & matrix_coefficients
	        }
	      }

	      if (gb.readBool()) {
	        // chroma_loc_info_present_flag
	        gb.readUEG(); // chroma_sample_loc_type_top_field

	        gb.readUEG(); // chroma_sample_loc_type_bottom_field
	      }

	      if (gb.readBool()) {
	        // timing_info_present_flag
	        let num_units_in_tick = gb.readBits(32);
	        let time_scale = gb.readBits(32);
	        fps_fixed = gb.readBool(); // fixed_frame_rate_flag

	        fps_num = time_scale;
	        fps_den = num_units_in_tick * 2;
	        fps = fps_num / fps_den;
	      }
	    }

	    let sarScale = 1;

	    if (sar_width !== 1 || sar_height !== 1) {
	      sarScale = sar_width / sar_height;
	    }

	    let crop_unit_x = 0,
	        crop_unit_y = 0;

	    if (chroma_format_idc === 0) {
	      crop_unit_x = 1;
	      crop_unit_y = 2 - frame_mbs_only_flag;
	    } else {
	      let sub_wc = chroma_format_idc === 3 ? 1 : 2;
	      let sub_hc = chroma_format_idc === 1 ? 2 : 1;
	      crop_unit_x = sub_wc;
	      crop_unit_y = sub_hc * (2 - frame_mbs_only_flag);
	    }

	    let codec_width = (pic_width_in_mbs_minus1 + 1) * 16;
	    let codec_height = (2 - frame_mbs_only_flag) * ((pic_height_in_map_units_minus1 + 1) * 16);
	    codec_width -= (frame_crop_left_offset + frame_crop_right_offset) * crop_unit_x;
	    codec_height -= (frame_crop_top_offset + frame_crop_bottom_offset) * crop_unit_y;
	    let present_width = Math.ceil(codec_width * sarScale);
	    gb.destroy();
	    gb = null; // 解析出来的SPS 内容。

	    return {
	      profile_string: profile_string,
	      // baseline, high, high10, ...
	      level_string: level_string,
	      // 3, 3.1, 4, 4.1, 5, 5.1, ...
	      bit_depth: bit_depth,
	      // 8bit, 10bit, ...
	      ref_frames: ref_frames,
	      chroma_format: chroma_format,
	      // 4:2:0, 4:2:2, ...
	      chroma_format_string: SPSParser.getChromaFormatString(chroma_format),
	      frame_rate: {
	        fixed: fps_fixed,
	        fps: fps,
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
	        width: present_width,
	        height: codec_height
	      }
	    };
	  }

	  static parseSPS$2(uint8array) {
	    let codec_array = uint8array.subarray(1, 4);
	    let codec_mimetype = 'avc1.';

	    for (let j = 0; j < 3; j++) {
	      let h = codec_array[j].toString(16);

	      if (h.length < 2) {
	        h = '0' + h;
	      }

	      codec_mimetype += h;
	    }

	    let rbsp = SPSParser._ebsp2rbsp(uint8array);

	    let gb = new ExpGolomb(rbsp);
	    gb.readByte();
	    let profile_idc = gb.readByte(); // profile_idc

	    gb.readByte(); // constraint_set_flags[5] + reserved_zero[3]

	    let level_idc = gb.readByte(); // level_idc

	    gb.readUEG(); // seq_parameter_set_id

	    let profile_string = SPSParser.getProfileString(profile_idc);
	    let level_string = SPSParser.getLevelString(level_idc);
	    let chroma_format_idc = 1;
	    let chroma_format = 420;
	    let chroma_format_table = [0, 420, 422, 444];
	    let bit_depth_luma = 8;
	    let bit_depth_chroma = 8;

	    if (profile_idc === 100 || profile_idc === 110 || profile_idc === 122 || profile_idc === 244 || profile_idc === 44 || profile_idc === 83 || profile_idc === 86 || profile_idc === 118 || profile_idc === 128 || profile_idc === 138 || profile_idc === 144) {
	      chroma_format_idc = gb.readUEG();

	      if (chroma_format_idc === 3) {
	        gb.readBits(1); // separate_colour_plane_flag
	      }

	      if (chroma_format_idc <= 3) {
	        chroma_format = chroma_format_table[chroma_format_idc];
	      }

	      bit_depth_luma = gb.readUEG() + 8; // bit_depth_luma_minus8

	      bit_depth_chroma = gb.readUEG() + 8; // bit_depth_chroma_minus8

	      gb.readBits(1); // qpprime_y_zero_transform_bypass_flag

	      if (gb.readBool()) {
	        // seq_scaling_matrix_present_flag
	        let scaling_list_count = chroma_format_idc !== 3 ? 8 : 12;

	        for (let i = 0; i < scaling_list_count; i++) {
	          if (gb.readBool()) {
	            // seq_scaling_list_present_flag
	            if (i < 6) {
	              SPSParser._skipScalingList(gb, 16);
	            } else {
	              SPSParser._skipScalingList(gb, 64);
	            }
	          }
	        }
	      }
	    }

	    gb.readUEG(); // log2_max_frame_num_minus4

	    let pic_order_cnt_type = gb.readUEG();

	    if (pic_order_cnt_type === 0) {
	      gb.readUEG(); // log2_max_pic_order_cnt_lsb_minus_4
	    } else if (pic_order_cnt_type === 1) {
	      gb.readBits(1); // delta_pic_order_always_zero_flag

	      gb.readSEG(); // offset_for_non_ref_pic

	      gb.readSEG(); // offset_for_top_to_bottom_field

	      let num_ref_frames_in_pic_order_cnt_cycle = gb.readUEG();

	      for (let i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) {
	        gb.readSEG(); // offset_for_ref_frame
	      }
	    }

	    let ref_frames = gb.readUEG(); // max_num_ref_frames

	    gb.readBits(1); // gaps_in_frame_num_value_allowed_flag

	    let pic_width_in_mbs_minus1 = gb.readUEG();
	    let pic_height_in_map_units_minus1 = gb.readUEG();
	    let frame_mbs_only_flag = gb.readBits(1);

	    if (frame_mbs_only_flag === 0) {
	      gb.readBits(1); // mb_adaptive_frame_field_flag
	    }

	    gb.readBits(1); // direct_8x8_inference_flag

	    let frame_crop_left_offset = 0;
	    let frame_crop_right_offset = 0;
	    let frame_crop_top_offset = 0;
	    let frame_crop_bottom_offset = 0;
	    let frame_cropping_flag = gb.readBool();

	    if (frame_cropping_flag) {
	      frame_crop_left_offset = gb.readUEG();
	      frame_crop_right_offset = gb.readUEG();
	      frame_crop_top_offset = gb.readUEG();
	      frame_crop_bottom_offset = gb.readUEG();
	    }

	    let sar_width = 1,
	        sar_height = 1;
	    let fps = 0,
	        fps_fixed = true,
	        fps_num = 0,
	        fps_den = 0;
	    let vui_parameters_present_flag = gb.readBool();

	    if (vui_parameters_present_flag) {
	      if (gb.readBool()) {
	        // aspect_ratio_info_present_flag
	        let aspect_ratio_idc = gb.readByte();
	        let sar_w_table = [1, 12, 10, 16, 40, 24, 20, 32, 80, 18, 15, 64, 160, 4, 3, 2];
	        let sar_h_table = [1, 11, 11, 11, 33, 11, 11, 11, 33, 11, 11, 33, 99, 3, 2, 1];

	        if (aspect_ratio_idc > 0 && aspect_ratio_idc < 16) {
	          sar_width = sar_w_table[aspect_ratio_idc - 1];
	          sar_height = sar_h_table[aspect_ratio_idc - 1];
	        } else if (aspect_ratio_idc === 255) {
	          sar_width = gb.readByte() << 8 | gb.readByte();
	          sar_height = gb.readByte() << 8 | gb.readByte();
	        }
	      }

	      if (gb.readBool()) {
	        // overscan_info_present_flag
	        gb.readBool(); // overscan_appropriate_flag
	      }

	      if (gb.readBool()) {
	        // video_signal_type_present_flag
	        gb.readBits(4); // video_format & video_full_range_flag

	        if (gb.readBool()) {
	          // colour_description_present_flag
	          gb.readBits(24); // colour_primaries & transfer_characteristics & matrix_coefficients
	        }
	      }

	      if (gb.readBool()) {
	        // chroma_loc_info_present_flag
	        gb.readUEG(); // chroma_sample_loc_type_top_field

	        gb.readUEG(); // chroma_sample_loc_type_bottom_field
	      }

	      if (gb.readBool()) {
	        // timing_info_present_flag
	        let num_units_in_tick = gb.readBits(32);
	        let time_scale = gb.readBits(32);
	        fps_fixed = gb.readBool(); // fixed_frame_rate_flag

	        fps_num = time_scale;
	        fps_den = num_units_in_tick * 2;
	        fps = fps_num / fps_den;
	      }
	    }

	    let sarScale = 1;

	    if (sar_width !== 1 || sar_height !== 1) {
	      sarScale = sar_width / sar_height;
	    }

	    let crop_unit_x = 0,
	        crop_unit_y = 0;

	    if (chroma_format_idc === 0) {
	      crop_unit_x = 1;
	      crop_unit_y = 2 - frame_mbs_only_flag;
	    } else {
	      let sub_wc = chroma_format_idc === 3 ? 1 : 2;
	      let sub_hc = chroma_format_idc === 1 ? 2 : 1;
	      crop_unit_x = sub_wc;
	      crop_unit_y = sub_hc * (2 - frame_mbs_only_flag);
	    }

	    let codec_width = (pic_width_in_mbs_minus1 + 1) * 16;
	    let codec_height = (2 - frame_mbs_only_flag) * ((pic_height_in_map_units_minus1 + 1) * 16);
	    codec_width -= (frame_crop_left_offset + frame_crop_right_offset) * crop_unit_x;
	    codec_height -= (frame_crop_top_offset + frame_crop_bottom_offset) * crop_unit_y;
	    let present_width = Math.ceil(codec_width * sarScale);
	    gb.destroy();
	    gb = null;
	    return {
	      codec_mimetype,
	      profile_idc,
	      level_idc,
	      profile_string,
	      // baseline, high, high10, ...
	      level_string,
	      // 3, 3.1, 4, 4.1, 5, 5.1, ...
	      chroma_format_idc,
	      bit_depth: bit_depth_luma,
	      // 8bit, 10bit, ...
	      bit_depth_luma,
	      bit_depth_chroma,
	      ref_frames,
	      chroma_format,
	      // 4:2:0, 4:2:2, ...
	      chroma_format_string: SPSParser.getChromaFormatString(chroma_format),
	      frame_rate: {
	        fixed: fps_fixed,
	        fps: fps,
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
	        width: present_width,
	        height: codec_height
	      }
	    };
	  }

	  static _skipScalingList(gb, count) {
	    let last_scale = 8,
	        next_scale = 8;
	    let delta_scale = 0;

	    for (let i = 0; i < count; i++) {
	      if (next_scale !== 0) {
	        delta_scale = gb.readSEG();
	        next_scale = (last_scale + delta_scale + 256) % 256;
	      }

	      last_scale = next_scale === 0 ? last_scale : next_scale;
	    }
	  } // profile_idc = 66 → baseline profile;
	  // profile_idc = 77 → main profile;
	  // profile_idc = 88 → extended profile;
	  // 在新版的标准中，还包括了High、High 10、High 4:2:2、High 4:4:4、High 10 Intra、High
	  // 4:2:2 Intra、High 4:4:4 Intra、CAVLC 4:4:4 Intra


	  static getProfileString(profile_idc) {
	    switch (profile_idc) {
	      case 66:
	        return 'Baseline';

	      case 77:
	        return 'Main';

	      case 88:
	        return 'Extended';

	      case 100:
	        return 'High';

	      case 110:
	        return 'High10';

	      case 122:
	        return 'High422';

	      case 244:
	        return 'High444';

	      default:
	        return 'Unknown';
	    }
	  }

	  static getLevelString(level_idc) {
	    return (level_idc / 10).toFixed(1);
	  }

	  static getChromaFormatString(chroma) {
	    switch (chroma) {
	      case 420:
	        return '4:2:0';

	      case 422:
	        return '4:2:2';

	      case 444:
	        return '4:4:4';

	      default:
	        return 'Unknown';
	    }
	  }

	}

	class Bitop {
	  constructor(buffer) {
	    this.buffer = buffer;
	    this.buflen = buffer.length;
	    this.bufpos = 0;
	    this.bufoff = 0;
	    this.iserro = false;
	  }

	  read(n) {
	    let v = 0;
	    let d = 0;

	    while (n) {
	      if (n < 0 || this.bufpos >= this.buflen) {
	        this.iserro = true;
	        return 0;
	      }

	      this.iserro = false;
	      d = this.bufoff + n > 8 ? 8 - this.bufoff : n;
	      v <<= d;
	      v += this.buffer[this.bufpos] >> 8 - this.bufoff - d & 0xff >> 8 - d;
	      this.bufoff += d;
	      n -= d;

	      if (this.bufoff == 8) {
	        this.bufpos++;
	        this.bufoff = 0;
	      }
	    }

	    return v;
	  }

	  look(n) {
	    let p = this.bufpos;
	    let o = this.bufoff;
	    let v = this.read(n);
	    this.bufpos = p;
	    this.bufoff = o;
	    return v;
	  }

	  read_golomb() {
	    let n;

	    for (n = 0; this.read(1) === 0 && !this.iserro; n++);

	    return (1 << n) + this.read(n) - 1;
	  }

	}

	function parseAVCDecoderConfigurationRecord(arrayBuffer) {
	  const meta = {};
	  const v = new DataView(arrayBuffer.buffer);
	  let version = v.getUint8(0); // configurationVersion

	  let avcProfile = v.getUint8(1); // avcProfileIndication

	  v.getUint8(2); // profile_compatibil

	  v.getUint8(3); // AVCLevelIndication

	  if (version !== 1 || avcProfile === 0) {
	    // this._onError(DemuxErrors.FORMAT_ERROR, 'Flv: Invalid AVCDecoderConfigurationRecord');
	    return {};
	  }

	  const _naluLengthSize = (v.getUint8(4) & 3) + 1; // lengthSizeMinusOne


	  if (_naluLengthSize !== 3 && _naluLengthSize !== 4) {
	    // holy shit!!!
	    // this._onError(DemuxErrors.FORMAT_ERROR, `Flv: Strange NaluLengthSizeMinusOne: ${_naluLengthSize - 1}`);
	    return {};
	  }

	  let spsCount = v.getUint8(5) & 31; // numOfSequenceParameterSets

	  if (spsCount === 0) {
	    // this._onError(DemuxErrors.FORMAT_ERROR, 'Flv: Invalid AVCDecoderConfigurationRecord: No SPS');
	    return {};
	  }

	  let offset = 6;

	  for (let i = 0; i < spsCount; i++) {
	    let len = v.getUint16(offset, false); // sequenceParameterSetLength

	    offset += 2;

	    if (len === 0) {
	      continue;
	    } // Notice: Nalu without startcode header (00 00 00 01)


	    let sps = new Uint8Array(arrayBuffer.buffer, offset, len);
	    offset += len; // flv.js作者选择了自己来解析这个数据结构，也是迫不得已，因为JS环境下没有ffmpeg，解析这个结构主要是为了提取 sps和pps。虽然理论上sps允许有多个，但其实一般就一个。
	    // packetTtype 为 1 表示 NALU，NALU= network abstract layer unit，这是H.264的概念，网络抽象层数据单元，其实简单理解就是一帧视频数据。
	    // pps的信息没什么用，所以作者只实现了sps的分析器，说明作者下了很大功夫去学习264的标准，其中的Golomb解码还是挺复杂的，能解对不容易，我在PC和手机平台都是用ffmpeg去解析的。
	    // SPS里面包括了视频分辨率，帧率，profile level等视频重要信息。

	    let config = SPSParser.parseSPS(sps); // console.log('h264 sps config',config)

	    if (i !== 0) {
	      // ignore other sps's config
	      continue;
	    }

	    meta.sps = sps;
	    meta.timescale = 1000;
	    meta.codecWidth = config.codec_size.width;
	    meta.codecHeight = config.codec_size.height;
	    meta.presentWidth = config.present_size.width;
	    meta.presentHeight = config.present_size.height;
	    meta.profile = config.profile_string;
	    meta.level = config.level_string;
	    meta.bitDepth = config.bit_depth;
	    meta.chromaFormat = config.chroma_format;
	    meta.sarRatio = config.sar_ratio;
	    meta.frameRate = config.frame_rate;

	    if (config.frame_rate.fixed === false || config.frame_rate.fps_num === 0 || config.frame_rate.fps_den === 0) {
	      meta.frameRate = {
	        fixed: true,
	        fps: 23.976,
	        fps_num: 23976,
	        fps_den: 1000
	      };
	    }

	    let fps_den = meta.frameRate.fps_den;
	    let fps_num = meta.frameRate.fps_num;
	    meta.refSampleDuration = meta.timescale * (fps_den / fps_num);
	    let codecArray = sps.subarray(1, 4);
	    let codecString = 'avc1.';

	    for (let j = 0; j < 3; j++) {
	      let h = codecArray[j].toString(16);

	      if (h.length < 2) {
	        h = '0' + h;
	      }

	      codecString += h;
	    } // codec


	    meta.codec = codecString;
	  }

	  let ppsCount = v.getUint8(offset); // numOfPictureParameterSets

	  if (ppsCount === 0) {
	    // this._onError(DemuxErrors.FORMAT_ERROR, 'Flv: Invalid AVCDecoderConfigurationRecord: No PPS');
	    return {};
	  }

	  offset++;

	  for (let i = 0; i < ppsCount; i++) {
	    let len = v.getUint16(offset, false); // pictureParameterSetLength

	    offset += 2;

	    if (len === 0) {
	      continue;
	    }

	    let pps = new Uint8Array(arrayBuffer.buffer, offset, len); // pps is useless for extracting video information

	    offset += len;
	    meta.pps = pps;
	  }

	  meta.videoType = 'avc';

	  if (meta.sps) {
	    const spsLength = meta.sps.byteLength;
	    const spsFlag = new Uint8Array([spsLength >>> 24 & 0xFF, spsLength >>> 16 & 0xFF, spsLength >>> 8 & 0xFF, spsLength & 0xFF]);
	    const sps = new Uint8Array(spsLength + 4);
	    sps.set(spsFlag, 0);
	    sps.set(meta.sps, 4);
	    meta.sps = sps;
	  }

	  if (meta.pps) {
	    const ppsLength = meta.pps.byteLength;
	    const ppsFlag = new Uint8Array([ppsLength >>> 24 & 0xFF, ppsLength >>> 16 & 0xFF, ppsLength >>> 8 & 0xFF, ppsLength & 0xFF]);
	    const pps = new Uint8Array(ppsLength + 4);
	    pps.set(ppsFlag, 0);
	    pps.set(meta.pps, 4);
	    meta.pps = pps;
	  } // meta.avcc = arrayBuffer;


	  return meta;
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
	function avcEncoderNalePacketNotLength(oneNALBuffer, isIframe) {
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
	  tmp[4] = 0;
	  const arrayBuffer = new Uint8Array(tmp.length + oneNALBuffer.byteLength);
	  arrayBuffer.set(tmp, 0);
	  arrayBuffer.set(oneNALBuffer, tmp.length);
	  return arrayBuffer;
	}
	function addNaleHeaderLength(nalUnit) {
	  const nalUnitLength = nalUnit.byteLength;
	  const header = new Uint8Array(4);
	  header[0] = nalUnitLength >>> 24 & 0xff;
	  header[1] = nalUnitLength >>> 16 & 0xff;
	  header[2] = nalUnitLength >>> 8 & 0xff;
	  header[3] = nalUnitLength & 0xff;
	  const result = new Uint8Array(nalUnitLength + 4);
	  result.set(header, 0);
	  result.set(nalUnit, 4);
	  return result;
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
	  return !isAvcSeqHead(type) && !isHvcSEIType(type);
	}
	function isAvcNaluIFrame(type) {
	  return type === H264_NAL_TYPE.iFrame;
	}
	function isSameAvcNaluType(naluList) {
	  if (naluList.length === 0) {
	    return false;
	  }

	  const type = getAvcSeqHeadType(naluList[0]);

	  for (let i = 1; i < naluList.length; i++) {
	    if (type !== getAvcSeqHeadType(naluList[i])) {
	      return false;
	    }
	  }

	  return true;
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

	/**
	 *
	 * @param arrayBuffer
	 */

	function parseHEVCDecoderConfigurationRecord$2(arrayBuffer) {
	  let info = {};
	  info.width = 0;
	  info.height = 0;
	  info.profile = 0;
	  info.level = 0;
	  arrayBuffer = arrayBuffer.slice(5);

	  do {
	    let hevc = {};

	    if (arrayBuffer.length < 23) {
	      break;
	    }

	    hevc.configurationVersion = arrayBuffer[0];

	    if (hevc.configurationVersion != 1) {
	      break;
	    }

	    hevc.general_profile_space = arrayBuffer[1] >> 6 & 0x03;
	    hevc.general_tier_flag = arrayBuffer[1] >> 5 & 0x01;
	    hevc.general_profile_idc = arrayBuffer[1] & 0x1F;
	    hevc.general_profile_compatibility_flags = arrayBuffer[2] << 24 | arrayBuffer[3] << 16 | arrayBuffer[4] << 8 | arrayBuffer[5];
	    hevc.general_constraint_indicator_flags = arrayBuffer[6] << 24 | arrayBuffer[7] << 16 | arrayBuffer[8] << 8 | arrayBuffer[9];
	    hevc.general_constraint_indicator_flags = hevc.general_constraint_indicator_flags << 16 | arrayBuffer[10] << 8 | arrayBuffer[11];
	    hevc.general_level_idc = arrayBuffer[12];
	    hevc.min_spatial_segmentation_idc = (arrayBuffer[13] & 0x0F) << 8 | arrayBuffer[14];
	    hevc.parallelismType = arrayBuffer[15] & 0x03;
	    hevc.chromaFormat = arrayBuffer[16] & 0x03;
	    hevc.bitDepthLumaMinus8 = arrayBuffer[17] & 0x07;
	    hevc.bitDepthChromaMinus8 = arrayBuffer[18] & 0x07;
	    hevc.avgFrameRate = arrayBuffer[19] << 8 | arrayBuffer[20];
	    hevc.constantFrameRate = arrayBuffer[21] >> 6 & 0x03;
	    hevc.numTemporalLayers = arrayBuffer[21] >> 3 & 0x07;
	    hevc.temporalIdNested = arrayBuffer[21] >> 2 & 0x01;
	    hevc.lengthSizeMinusOne = arrayBuffer[21] & 0x03;
	    let numOfArrays = arrayBuffer[22];
	    let p = arrayBuffer.slice(23);

	    for (let i = 0; i < numOfArrays; i++) {
	      if (p.length < 3) {
	        break;
	      }

	      let nalutype = p[0] & 0x3F;
	      let n = p[1] << 8 | p[2]; // console.log('nalutype', nalutype,n)

	      p = p.slice(3);

	      for (let j = 0; j < n; j++) {
	        if (p.length < 2) {
	          break;
	        }

	        let k = p[0] << 8 | p[1]; // console.log('k', k)

	        if (p.length < 2 + k) {
	          break;
	        }

	        p = p.slice(2);

	        if (nalutype == 33) {
	          //SPS
	          let sps = new Uint8Array(k);
	          sps.set(p.slice(0, k), 0);
	          hevc.psps = HEVCParseSPS(sps, hevc);
	          info.profile = hevc.general_profile_idc;
	          info.level = hevc.general_level_idc / 30.0;
	          info.width = hevc.psps.pic_width_in_luma_samples - (hevc.psps.conf_win_left_offset + hevc.psps.conf_win_right_offset);
	          info.height = hevc.psps.pic_height_in_luma_samples - (hevc.psps.conf_win_top_offset + hevc.psps.conf_win_bottom_offset);
	        }

	        p = p.slice(k);
	      }
	    }
	  } while (0);

	  info.codecWidth = info.width || 1920;
	  info.codecHeight = info.height || 1080;
	  info.presentHeight = info.codecHeight;
	  info.presentWidth = info.codecWidth;
	  info.timescale = 1000;
	  info.refSampleDuration = 1000 * (1000 / 23976);
	  info.videoType = VIDEO_ENCODE_TYPE.h265;
	  return info;
	}
	function HEVCParsePtl(bitop, hevc, max_sub_layers_minus1) {
	  let general_ptl = {};
	  general_ptl.profile_space = bitop.read(2);
	  general_ptl.tier_flag = bitop.read(1);
	  general_ptl.profile_idc = bitop.read(5);
	  general_ptl.profile_compatibility_flags = bitop.read(32);
	  general_ptl.general_progressive_source_flag = bitop.read(1);
	  general_ptl.general_interlaced_source_flag = bitop.read(1);
	  general_ptl.general_non_packed_constraint_flag = bitop.read(1);
	  general_ptl.general_frame_only_constraint_flag = bitop.read(1);
	  bitop.read(32);
	  bitop.read(12);
	  general_ptl.level_idc = bitop.read(8);
	  general_ptl.sub_layer_profile_present_flag = [];
	  general_ptl.sub_layer_level_present_flag = [];

	  for (let i = 0; i < max_sub_layers_minus1; i++) {
	    general_ptl.sub_layer_profile_present_flag[i] = bitop.read(1);
	    general_ptl.sub_layer_level_present_flag[i] = bitop.read(1);
	  }

	  if (max_sub_layers_minus1 > 0) {
	    for (let i = max_sub_layers_minus1; i < 8; i++) {
	      bitop.read(2);
	    }
	  }

	  general_ptl.sub_layer_profile_space = [];
	  general_ptl.sub_layer_tier_flag = [];
	  general_ptl.sub_layer_profile_idc = [];
	  general_ptl.sub_layer_profile_compatibility_flag = [];
	  general_ptl.sub_layer_progressive_source_flag = [];
	  general_ptl.sub_layer_interlaced_source_flag = [];
	  general_ptl.sub_layer_non_packed_constraint_flag = [];
	  general_ptl.sub_layer_frame_only_constraint_flag = [];
	  general_ptl.sub_layer_level_idc = [];

	  for (let i = 0; i < max_sub_layers_minus1; i++) {
	    if (general_ptl.sub_layer_profile_present_flag[i]) {
	      general_ptl.sub_layer_profile_space[i] = bitop.read(2);
	      general_ptl.sub_layer_tier_flag[i] = bitop.read(1);
	      general_ptl.sub_layer_profile_idc[i] = bitop.read(5);
	      general_ptl.sub_layer_profile_compatibility_flag[i] = bitop.read(32);
	      general_ptl.sub_layer_progressive_source_flag[i] = bitop.read(1);
	      general_ptl.sub_layer_interlaced_source_flag[i] = bitop.read(1);
	      general_ptl.sub_layer_non_packed_constraint_flag[i] = bitop.read(1);
	      general_ptl.sub_layer_frame_only_constraint_flag[i] = bitop.read(1);
	      bitop.read(32);
	      bitop.read(12);
	    }

	    if (general_ptl.sub_layer_level_present_flag[i]) {
	      general_ptl.sub_layer_level_idc[i] = bitop.read(8);
	    } else {
	      general_ptl.sub_layer_level_idc[i] = 1;
	    }
	  }

	  return general_ptl;
	}
	function HEVCParseSPS(SPS, hevc) {
	  let psps = {};
	  let NumBytesInNALunit = SPS.length;
	  let rbsp_array = [];
	  let bitop = new Bitop(SPS);
	  bitop.read(1); //forbidden_zero_bit

	  bitop.read(6); //nal_unit_type

	  bitop.read(6); //nuh_reserved_zero_6bits

	  bitop.read(3); //nuh_temporal_id_plus1

	  for (let i = 2; i < NumBytesInNALunit; i++) {
	    if (i + 2 < NumBytesInNALunit && bitop.look(24) == 0x000003) {
	      rbsp_array.push(bitop.read(8));
	      rbsp_array.push(bitop.read(8));
	      i += 2;
	      bitop.read(8);
	      /* equal to 0x03 */
	    } else {
	      rbsp_array.push(bitop.read(8));
	    }
	  }

	  let rbsp = new Uint8Array(rbsp_array);
	  let rbspBitop = new Bitop(rbsp);
	  psps.sps_video_parameter_set_id = rbspBitop.read(4);
	  psps.sps_max_sub_layers_minus1 = rbspBitop.read(3);
	  psps.sps_temporal_id_nesting_flag = rbspBitop.read(1);
	  psps.profile_tier_level = HEVCParsePtl(rbspBitop, hevc, psps.sps_max_sub_layers_minus1);
	  psps.sps_seq_parameter_set_id = rbspBitop.read_golomb();
	  psps.chroma_format_idc = rbspBitop.read_golomb();

	  if (psps.chroma_format_idc == 3) {
	    psps.separate_colour_plane_flag = rbspBitop.read(1);
	  } else {
	    psps.separate_colour_plane_flag = 0;
	  }

	  psps.pic_width_in_luma_samples = rbspBitop.read_golomb();
	  psps.pic_height_in_luma_samples = rbspBitop.read_golomb();
	  psps.conformance_window_flag = rbspBitop.read(1);

	  if (psps.conformance_window_flag) {
	    let vert_mult = 1 + (psps.chroma_format_idc < 2);
	    let horiz_mult = 1 + (psps.chroma_format_idc < 3);
	    psps.conf_win_left_offset = rbspBitop.read_golomb() * horiz_mult;
	    psps.conf_win_right_offset = rbspBitop.read_golomb() * horiz_mult;
	    psps.conf_win_top_offset = rbspBitop.read_golomb() * vert_mult;
	    psps.conf_win_bottom_offset = rbspBitop.read_golomb() * vert_mult;
	  } else {
	    psps.conf_win_left_offset = 0;
	    psps.conf_win_right_offset = 0;
	    psps.conf_win_top_offset = 0;
	    psps.conf_win_bottom_offset = 0;
	  } // Logger.debug(psps);


	  return psps;
	}
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

	function checkInt(value) {
	  return parseInt(value) === value;
	}

	function checkInts(arrayish) {
	  if (!checkInt(arrayish.length)) {
	    return false;
	  }

	  for (var i = 0; i < arrayish.length; i++) {
	    if (!checkInt(arrayish[i]) || arrayish[i] < 0 || arrayish[i] > 255) {
	      return false;
	    }
	  }

	  return true;
	}

	function coerceArray(arg, copy) {
	  // ArrayBuffer view
	  if (arg.buffer && arg.name === 'Uint8Array') {
	    if (copy) {
	      if (arg.slice) {
	        arg = arg.slice();
	      } else {
	        arg = Array.prototype.slice.call(arg);
	      }
	    }

	    return arg;
	  } // It's an array; check it is a valid representation of a byte


	  if (Array.isArray(arg)) {
	    if (!checkInts(arg)) {
	      throw new Error('Array contains invalid value: ' + arg);
	    }

	    return new Uint8Array(arg);
	  } // Something else, but behaves like an array (maybe a Buffer? Arguments?)


	  if (checkInt(arg.length) && checkInts(arg)) {
	    return new Uint8Array(arg);
	  }

	  throw new Error('unsupported array-like object');
	}

	function createArray(length) {
	  return new Uint8Array(length);
	}

	function copyArray(sourceArray, targetArray, targetStart, sourceStart, sourceEnd) {
	  if (sourceStart != null || sourceEnd != null) {
	    if (sourceArray.slice) {
	      sourceArray = sourceArray.slice(sourceStart, sourceEnd);
	    } else {
	      sourceArray = Array.prototype.slice.call(sourceArray, sourceStart, sourceEnd);
	    }
	  }

	  targetArray.set(sourceArray, targetStart);
	}

	var convertUtf8 = function () {
	  function toBytes(text) {
	    var result = [],
	        i = 0;
	    text = encodeURI(text);

	    while (i < text.length) {
	      var c = text.charCodeAt(i++); // if it is a % sign, encode the following 2 bytes as a hex value

	      if (c === 37) {
	        result.push(parseInt(text.substr(i, 2), 16));
	        i += 2; // otherwise, just the actual byte
	      } else {
	        result.push(c);
	      }
	    }

	    return coerceArray(result);
	  }

	  function fromBytes(bytes) {
	    var result = [],
	        i = 0;

	    while (i < bytes.length) {
	      var c = bytes[i];

	      if (c < 128) {
	        result.push(String.fromCharCode(c));
	        i++;
	      } else if (c > 191 && c < 224) {
	        result.push(String.fromCharCode((c & 0x1f) << 6 | bytes[i + 1] & 0x3f));
	        i += 2;
	      } else {
	        result.push(String.fromCharCode((c & 0x0f) << 12 | (bytes[i + 1] & 0x3f) << 6 | bytes[i + 2] & 0x3f));
	        i += 3;
	      }
	    }

	    return result.join('');
	  }

	  return {
	    toBytes: toBytes,
	    fromBytes: fromBytes
	  };
	}();

	var convertHex = function () {
	  function toBytes(text) {
	    var result = [];

	    for (var i = 0; i < text.length; i += 2) {
	      result.push(parseInt(text.substr(i, 2), 16));
	    }

	    return result;
	  } // http://ixti.net/development/javascript/2011/11/11/base64-encodedecode-of-utf8-in-browser-with-js.html


	  var Hex = '0123456789abcdef';

	  function fromBytes(bytes) {
	    var result = [];

	    for (var i = 0; i < bytes.length; i++) {
	      var v = bytes[i];
	      result.push(Hex[(v & 0xf0) >> 4] + Hex[v & 0x0f]);
	    }

	    return result.join('');
	  }

	  return {
	    toBytes: toBytes,
	    fromBytes: fromBytes
	  };
	}(); // Number of rounds by keysize


	var numberOfRounds = {
	  16: 10,
	  24: 12,
	  32: 14
	}; // Round constant words

	var rcon = [0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91]; // S-box and Inverse S-box (S is for Substitution)

	var S = [0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, 0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16];
	var Si = [0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, 0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb, 0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e, 0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25, 0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92, 0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84, 0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06, 0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b, 0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73, 0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e, 0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b, 0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4, 0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f, 0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef, 0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61, 0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d]; // Transformations for encryption

	var T1 = [0xc66363a5, 0xf87c7c84, 0xee777799, 0xf67b7b8d, 0xfff2f20d, 0xd66b6bbd, 0xde6f6fb1, 0x91c5c554, 0x60303050, 0x02010103, 0xce6767a9, 0x562b2b7d, 0xe7fefe19, 0xb5d7d762, 0x4dababe6, 0xec76769a, 0x8fcaca45, 0x1f82829d, 0x89c9c940, 0xfa7d7d87, 0xeffafa15, 0xb25959eb, 0x8e4747c9, 0xfbf0f00b, 0x41adadec, 0xb3d4d467, 0x5fa2a2fd, 0x45afafea, 0x239c9cbf, 0x53a4a4f7, 0xe4727296, 0x9bc0c05b, 0x75b7b7c2, 0xe1fdfd1c, 0x3d9393ae, 0x4c26266a, 0x6c36365a, 0x7e3f3f41, 0xf5f7f702, 0x83cccc4f, 0x6834345c, 0x51a5a5f4, 0xd1e5e534, 0xf9f1f108, 0xe2717193, 0xabd8d873, 0x62313153, 0x2a15153f, 0x0804040c, 0x95c7c752, 0x46232365, 0x9dc3c35e, 0x30181828, 0x379696a1, 0x0a05050f, 0x2f9a9ab5, 0x0e070709, 0x24121236, 0x1b80809b, 0xdfe2e23d, 0xcdebeb26, 0x4e272769, 0x7fb2b2cd, 0xea75759f, 0x1209091b, 0x1d83839e, 0x582c2c74, 0x341a1a2e, 0x361b1b2d, 0xdc6e6eb2, 0xb45a5aee, 0x5ba0a0fb, 0xa45252f6, 0x763b3b4d, 0xb7d6d661, 0x7db3b3ce, 0x5229297b, 0xdde3e33e, 0x5e2f2f71, 0x13848497, 0xa65353f5, 0xb9d1d168, 0x00000000, 0xc1eded2c, 0x40202060, 0xe3fcfc1f, 0x79b1b1c8, 0xb65b5bed, 0xd46a6abe, 0x8dcbcb46, 0x67bebed9, 0x7239394b, 0x944a4ade, 0x984c4cd4, 0xb05858e8, 0x85cfcf4a, 0xbbd0d06b, 0xc5efef2a, 0x4faaaae5, 0xedfbfb16, 0x864343c5, 0x9a4d4dd7, 0x66333355, 0x11858594, 0x8a4545cf, 0xe9f9f910, 0x04020206, 0xfe7f7f81, 0xa05050f0, 0x783c3c44, 0x259f9fba, 0x4ba8a8e3, 0xa25151f3, 0x5da3a3fe, 0x804040c0, 0x058f8f8a, 0x3f9292ad, 0x219d9dbc, 0x70383848, 0xf1f5f504, 0x63bcbcdf, 0x77b6b6c1, 0xafdada75, 0x42212163, 0x20101030, 0xe5ffff1a, 0xfdf3f30e, 0xbfd2d26d, 0x81cdcd4c, 0x180c0c14, 0x26131335, 0xc3ecec2f, 0xbe5f5fe1, 0x359797a2, 0x884444cc, 0x2e171739, 0x93c4c457, 0x55a7a7f2, 0xfc7e7e82, 0x7a3d3d47, 0xc86464ac, 0xba5d5de7, 0x3219192b, 0xe6737395, 0xc06060a0, 0x19818198, 0x9e4f4fd1, 0xa3dcdc7f, 0x44222266, 0x542a2a7e, 0x3b9090ab, 0x0b888883, 0x8c4646ca, 0xc7eeee29, 0x6bb8b8d3, 0x2814143c, 0xa7dede79, 0xbc5e5ee2, 0x160b0b1d, 0xaddbdb76, 0xdbe0e03b, 0x64323256, 0x743a3a4e, 0x140a0a1e, 0x924949db, 0x0c06060a, 0x4824246c, 0xb85c5ce4, 0x9fc2c25d, 0xbdd3d36e, 0x43acacef, 0xc46262a6, 0x399191a8, 0x319595a4, 0xd3e4e437, 0xf279798b, 0xd5e7e732, 0x8bc8c843, 0x6e373759, 0xda6d6db7, 0x018d8d8c, 0xb1d5d564, 0x9c4e4ed2, 0x49a9a9e0, 0xd86c6cb4, 0xac5656fa, 0xf3f4f407, 0xcfeaea25, 0xca6565af, 0xf47a7a8e, 0x47aeaee9, 0x10080818, 0x6fbabad5, 0xf0787888, 0x4a25256f, 0x5c2e2e72, 0x381c1c24, 0x57a6a6f1, 0x73b4b4c7, 0x97c6c651, 0xcbe8e823, 0xa1dddd7c, 0xe874749c, 0x3e1f1f21, 0x964b4bdd, 0x61bdbddc, 0x0d8b8b86, 0x0f8a8a85, 0xe0707090, 0x7c3e3e42, 0x71b5b5c4, 0xcc6666aa, 0x904848d8, 0x06030305, 0xf7f6f601, 0x1c0e0e12, 0xc26161a3, 0x6a35355f, 0xae5757f9, 0x69b9b9d0, 0x17868691, 0x99c1c158, 0x3a1d1d27, 0x279e9eb9, 0xd9e1e138, 0xebf8f813, 0x2b9898b3, 0x22111133, 0xd26969bb, 0xa9d9d970, 0x078e8e89, 0x339494a7, 0x2d9b9bb6, 0x3c1e1e22, 0x15878792, 0xc9e9e920, 0x87cece49, 0xaa5555ff, 0x50282878, 0xa5dfdf7a, 0x038c8c8f, 0x59a1a1f8, 0x09898980, 0x1a0d0d17, 0x65bfbfda, 0xd7e6e631, 0x844242c6, 0xd06868b8, 0x824141c3, 0x299999b0, 0x5a2d2d77, 0x1e0f0f11, 0x7bb0b0cb, 0xa85454fc, 0x6dbbbbd6, 0x2c16163a];
	var T2 = [0xa5c66363, 0x84f87c7c, 0x99ee7777, 0x8df67b7b, 0x0dfff2f2, 0xbdd66b6b, 0xb1de6f6f, 0x5491c5c5, 0x50603030, 0x03020101, 0xa9ce6767, 0x7d562b2b, 0x19e7fefe, 0x62b5d7d7, 0xe64dabab, 0x9aec7676, 0x458fcaca, 0x9d1f8282, 0x4089c9c9, 0x87fa7d7d, 0x15effafa, 0xebb25959, 0xc98e4747, 0x0bfbf0f0, 0xec41adad, 0x67b3d4d4, 0xfd5fa2a2, 0xea45afaf, 0xbf239c9c, 0xf753a4a4, 0x96e47272, 0x5b9bc0c0, 0xc275b7b7, 0x1ce1fdfd, 0xae3d9393, 0x6a4c2626, 0x5a6c3636, 0x417e3f3f, 0x02f5f7f7, 0x4f83cccc, 0x5c683434, 0xf451a5a5, 0x34d1e5e5, 0x08f9f1f1, 0x93e27171, 0x73abd8d8, 0x53623131, 0x3f2a1515, 0x0c080404, 0x5295c7c7, 0x65462323, 0x5e9dc3c3, 0x28301818, 0xa1379696, 0x0f0a0505, 0xb52f9a9a, 0x090e0707, 0x36241212, 0x9b1b8080, 0x3ddfe2e2, 0x26cdebeb, 0x694e2727, 0xcd7fb2b2, 0x9fea7575, 0x1b120909, 0x9e1d8383, 0x74582c2c, 0x2e341a1a, 0x2d361b1b, 0xb2dc6e6e, 0xeeb45a5a, 0xfb5ba0a0, 0xf6a45252, 0x4d763b3b, 0x61b7d6d6, 0xce7db3b3, 0x7b522929, 0x3edde3e3, 0x715e2f2f, 0x97138484, 0xf5a65353, 0x68b9d1d1, 0x00000000, 0x2cc1eded, 0x60402020, 0x1fe3fcfc, 0xc879b1b1, 0xedb65b5b, 0xbed46a6a, 0x468dcbcb, 0xd967bebe, 0x4b723939, 0xde944a4a, 0xd4984c4c, 0xe8b05858, 0x4a85cfcf, 0x6bbbd0d0, 0x2ac5efef, 0xe54faaaa, 0x16edfbfb, 0xc5864343, 0xd79a4d4d, 0x55663333, 0x94118585, 0xcf8a4545, 0x10e9f9f9, 0x06040202, 0x81fe7f7f, 0xf0a05050, 0x44783c3c, 0xba259f9f, 0xe34ba8a8, 0xf3a25151, 0xfe5da3a3, 0xc0804040, 0x8a058f8f, 0xad3f9292, 0xbc219d9d, 0x48703838, 0x04f1f5f5, 0xdf63bcbc, 0xc177b6b6, 0x75afdada, 0x63422121, 0x30201010, 0x1ae5ffff, 0x0efdf3f3, 0x6dbfd2d2, 0x4c81cdcd, 0x14180c0c, 0x35261313, 0x2fc3ecec, 0xe1be5f5f, 0xa2359797, 0xcc884444, 0x392e1717, 0x5793c4c4, 0xf255a7a7, 0x82fc7e7e, 0x477a3d3d, 0xacc86464, 0xe7ba5d5d, 0x2b321919, 0x95e67373, 0xa0c06060, 0x98198181, 0xd19e4f4f, 0x7fa3dcdc, 0x66442222, 0x7e542a2a, 0xab3b9090, 0x830b8888, 0xca8c4646, 0x29c7eeee, 0xd36bb8b8, 0x3c281414, 0x79a7dede, 0xe2bc5e5e, 0x1d160b0b, 0x76addbdb, 0x3bdbe0e0, 0x56643232, 0x4e743a3a, 0x1e140a0a, 0xdb924949, 0x0a0c0606, 0x6c482424, 0xe4b85c5c, 0x5d9fc2c2, 0x6ebdd3d3, 0xef43acac, 0xa6c46262, 0xa8399191, 0xa4319595, 0x37d3e4e4, 0x8bf27979, 0x32d5e7e7, 0x438bc8c8, 0x596e3737, 0xb7da6d6d, 0x8c018d8d, 0x64b1d5d5, 0xd29c4e4e, 0xe049a9a9, 0xb4d86c6c, 0xfaac5656, 0x07f3f4f4, 0x25cfeaea, 0xafca6565, 0x8ef47a7a, 0xe947aeae, 0x18100808, 0xd56fbaba, 0x88f07878, 0x6f4a2525, 0x725c2e2e, 0x24381c1c, 0xf157a6a6, 0xc773b4b4, 0x5197c6c6, 0x23cbe8e8, 0x7ca1dddd, 0x9ce87474, 0x213e1f1f, 0xdd964b4b, 0xdc61bdbd, 0x860d8b8b, 0x850f8a8a, 0x90e07070, 0x427c3e3e, 0xc471b5b5, 0xaacc6666, 0xd8904848, 0x05060303, 0x01f7f6f6, 0x121c0e0e, 0xa3c26161, 0x5f6a3535, 0xf9ae5757, 0xd069b9b9, 0x91178686, 0x5899c1c1, 0x273a1d1d, 0xb9279e9e, 0x38d9e1e1, 0x13ebf8f8, 0xb32b9898, 0x33221111, 0xbbd26969, 0x70a9d9d9, 0x89078e8e, 0xa7339494, 0xb62d9b9b, 0x223c1e1e, 0x92158787, 0x20c9e9e9, 0x4987cece, 0xffaa5555, 0x78502828, 0x7aa5dfdf, 0x8f038c8c, 0xf859a1a1, 0x80098989, 0x171a0d0d, 0xda65bfbf, 0x31d7e6e6, 0xc6844242, 0xb8d06868, 0xc3824141, 0xb0299999, 0x775a2d2d, 0x111e0f0f, 0xcb7bb0b0, 0xfca85454, 0xd66dbbbb, 0x3a2c1616];
	var T3 = [0x63a5c663, 0x7c84f87c, 0x7799ee77, 0x7b8df67b, 0xf20dfff2, 0x6bbdd66b, 0x6fb1de6f, 0xc55491c5, 0x30506030, 0x01030201, 0x67a9ce67, 0x2b7d562b, 0xfe19e7fe, 0xd762b5d7, 0xabe64dab, 0x769aec76, 0xca458fca, 0x829d1f82, 0xc94089c9, 0x7d87fa7d, 0xfa15effa, 0x59ebb259, 0x47c98e47, 0xf00bfbf0, 0xadec41ad, 0xd467b3d4, 0xa2fd5fa2, 0xafea45af, 0x9cbf239c, 0xa4f753a4, 0x7296e472, 0xc05b9bc0, 0xb7c275b7, 0xfd1ce1fd, 0x93ae3d93, 0x266a4c26, 0x365a6c36, 0x3f417e3f, 0xf702f5f7, 0xcc4f83cc, 0x345c6834, 0xa5f451a5, 0xe534d1e5, 0xf108f9f1, 0x7193e271, 0xd873abd8, 0x31536231, 0x153f2a15, 0x040c0804, 0xc75295c7, 0x23654623, 0xc35e9dc3, 0x18283018, 0x96a13796, 0x050f0a05, 0x9ab52f9a, 0x07090e07, 0x12362412, 0x809b1b80, 0xe23ddfe2, 0xeb26cdeb, 0x27694e27, 0xb2cd7fb2, 0x759fea75, 0x091b1209, 0x839e1d83, 0x2c74582c, 0x1a2e341a, 0x1b2d361b, 0x6eb2dc6e, 0x5aeeb45a, 0xa0fb5ba0, 0x52f6a452, 0x3b4d763b, 0xd661b7d6, 0xb3ce7db3, 0x297b5229, 0xe33edde3, 0x2f715e2f, 0x84971384, 0x53f5a653, 0xd168b9d1, 0x00000000, 0xed2cc1ed, 0x20604020, 0xfc1fe3fc, 0xb1c879b1, 0x5bedb65b, 0x6abed46a, 0xcb468dcb, 0xbed967be, 0x394b7239, 0x4ade944a, 0x4cd4984c, 0x58e8b058, 0xcf4a85cf, 0xd06bbbd0, 0xef2ac5ef, 0xaae54faa, 0xfb16edfb, 0x43c58643, 0x4dd79a4d, 0x33556633, 0x85941185, 0x45cf8a45, 0xf910e9f9, 0x02060402, 0x7f81fe7f, 0x50f0a050, 0x3c44783c, 0x9fba259f, 0xa8e34ba8, 0x51f3a251, 0xa3fe5da3, 0x40c08040, 0x8f8a058f, 0x92ad3f92, 0x9dbc219d, 0x38487038, 0xf504f1f5, 0xbcdf63bc, 0xb6c177b6, 0xda75afda, 0x21634221, 0x10302010, 0xff1ae5ff, 0xf30efdf3, 0xd26dbfd2, 0xcd4c81cd, 0x0c14180c, 0x13352613, 0xec2fc3ec, 0x5fe1be5f, 0x97a23597, 0x44cc8844, 0x17392e17, 0xc45793c4, 0xa7f255a7, 0x7e82fc7e, 0x3d477a3d, 0x64acc864, 0x5de7ba5d, 0x192b3219, 0x7395e673, 0x60a0c060, 0x81981981, 0x4fd19e4f, 0xdc7fa3dc, 0x22664422, 0x2a7e542a, 0x90ab3b90, 0x88830b88, 0x46ca8c46, 0xee29c7ee, 0xb8d36bb8, 0x143c2814, 0xde79a7de, 0x5ee2bc5e, 0x0b1d160b, 0xdb76addb, 0xe03bdbe0, 0x32566432, 0x3a4e743a, 0x0a1e140a, 0x49db9249, 0x060a0c06, 0x246c4824, 0x5ce4b85c, 0xc25d9fc2, 0xd36ebdd3, 0xacef43ac, 0x62a6c462, 0x91a83991, 0x95a43195, 0xe437d3e4, 0x798bf279, 0xe732d5e7, 0xc8438bc8, 0x37596e37, 0x6db7da6d, 0x8d8c018d, 0xd564b1d5, 0x4ed29c4e, 0xa9e049a9, 0x6cb4d86c, 0x56faac56, 0xf407f3f4, 0xea25cfea, 0x65afca65, 0x7a8ef47a, 0xaee947ae, 0x08181008, 0xbad56fba, 0x7888f078, 0x256f4a25, 0x2e725c2e, 0x1c24381c, 0xa6f157a6, 0xb4c773b4, 0xc65197c6, 0xe823cbe8, 0xdd7ca1dd, 0x749ce874, 0x1f213e1f, 0x4bdd964b, 0xbddc61bd, 0x8b860d8b, 0x8a850f8a, 0x7090e070, 0x3e427c3e, 0xb5c471b5, 0x66aacc66, 0x48d89048, 0x03050603, 0xf601f7f6, 0x0e121c0e, 0x61a3c261, 0x355f6a35, 0x57f9ae57, 0xb9d069b9, 0x86911786, 0xc15899c1, 0x1d273a1d, 0x9eb9279e, 0xe138d9e1, 0xf813ebf8, 0x98b32b98, 0x11332211, 0x69bbd269, 0xd970a9d9, 0x8e89078e, 0x94a73394, 0x9bb62d9b, 0x1e223c1e, 0x87921587, 0xe920c9e9, 0xce4987ce, 0x55ffaa55, 0x28785028, 0xdf7aa5df, 0x8c8f038c, 0xa1f859a1, 0x89800989, 0x0d171a0d, 0xbfda65bf, 0xe631d7e6, 0x42c68442, 0x68b8d068, 0x41c38241, 0x99b02999, 0x2d775a2d, 0x0f111e0f, 0xb0cb7bb0, 0x54fca854, 0xbbd66dbb, 0x163a2c16];
	var T4 = [0x6363a5c6, 0x7c7c84f8, 0x777799ee, 0x7b7b8df6, 0xf2f20dff, 0x6b6bbdd6, 0x6f6fb1de, 0xc5c55491, 0x30305060, 0x01010302, 0x6767a9ce, 0x2b2b7d56, 0xfefe19e7, 0xd7d762b5, 0xababe64d, 0x76769aec, 0xcaca458f, 0x82829d1f, 0xc9c94089, 0x7d7d87fa, 0xfafa15ef, 0x5959ebb2, 0x4747c98e, 0xf0f00bfb, 0xadadec41, 0xd4d467b3, 0xa2a2fd5f, 0xafafea45, 0x9c9cbf23, 0xa4a4f753, 0x727296e4, 0xc0c05b9b, 0xb7b7c275, 0xfdfd1ce1, 0x9393ae3d, 0x26266a4c, 0x36365a6c, 0x3f3f417e, 0xf7f702f5, 0xcccc4f83, 0x34345c68, 0xa5a5f451, 0xe5e534d1, 0xf1f108f9, 0x717193e2, 0xd8d873ab, 0x31315362, 0x15153f2a, 0x04040c08, 0xc7c75295, 0x23236546, 0xc3c35e9d, 0x18182830, 0x9696a137, 0x05050f0a, 0x9a9ab52f, 0x0707090e, 0x12123624, 0x80809b1b, 0xe2e23ddf, 0xebeb26cd, 0x2727694e, 0xb2b2cd7f, 0x75759fea, 0x09091b12, 0x83839e1d, 0x2c2c7458, 0x1a1a2e34, 0x1b1b2d36, 0x6e6eb2dc, 0x5a5aeeb4, 0xa0a0fb5b, 0x5252f6a4, 0x3b3b4d76, 0xd6d661b7, 0xb3b3ce7d, 0x29297b52, 0xe3e33edd, 0x2f2f715e, 0x84849713, 0x5353f5a6, 0xd1d168b9, 0x00000000, 0xeded2cc1, 0x20206040, 0xfcfc1fe3, 0xb1b1c879, 0x5b5bedb6, 0x6a6abed4, 0xcbcb468d, 0xbebed967, 0x39394b72, 0x4a4ade94, 0x4c4cd498, 0x5858e8b0, 0xcfcf4a85, 0xd0d06bbb, 0xefef2ac5, 0xaaaae54f, 0xfbfb16ed, 0x4343c586, 0x4d4dd79a, 0x33335566, 0x85859411, 0x4545cf8a, 0xf9f910e9, 0x02020604, 0x7f7f81fe, 0x5050f0a0, 0x3c3c4478, 0x9f9fba25, 0xa8a8e34b, 0x5151f3a2, 0xa3a3fe5d, 0x4040c080, 0x8f8f8a05, 0x9292ad3f, 0x9d9dbc21, 0x38384870, 0xf5f504f1, 0xbcbcdf63, 0xb6b6c177, 0xdada75af, 0x21216342, 0x10103020, 0xffff1ae5, 0xf3f30efd, 0xd2d26dbf, 0xcdcd4c81, 0x0c0c1418, 0x13133526, 0xecec2fc3, 0x5f5fe1be, 0x9797a235, 0x4444cc88, 0x1717392e, 0xc4c45793, 0xa7a7f255, 0x7e7e82fc, 0x3d3d477a, 0x6464acc8, 0x5d5de7ba, 0x19192b32, 0x737395e6, 0x6060a0c0, 0x81819819, 0x4f4fd19e, 0xdcdc7fa3, 0x22226644, 0x2a2a7e54, 0x9090ab3b, 0x8888830b, 0x4646ca8c, 0xeeee29c7, 0xb8b8d36b, 0x14143c28, 0xdede79a7, 0x5e5ee2bc, 0x0b0b1d16, 0xdbdb76ad, 0xe0e03bdb, 0x32325664, 0x3a3a4e74, 0x0a0a1e14, 0x4949db92, 0x06060a0c, 0x24246c48, 0x5c5ce4b8, 0xc2c25d9f, 0xd3d36ebd, 0xacacef43, 0x6262a6c4, 0x9191a839, 0x9595a431, 0xe4e437d3, 0x79798bf2, 0xe7e732d5, 0xc8c8438b, 0x3737596e, 0x6d6db7da, 0x8d8d8c01, 0xd5d564b1, 0x4e4ed29c, 0xa9a9e049, 0x6c6cb4d8, 0x5656faac, 0xf4f407f3, 0xeaea25cf, 0x6565afca, 0x7a7a8ef4, 0xaeaee947, 0x08081810, 0xbabad56f, 0x787888f0, 0x25256f4a, 0x2e2e725c, 0x1c1c2438, 0xa6a6f157, 0xb4b4c773, 0xc6c65197, 0xe8e823cb, 0xdddd7ca1, 0x74749ce8, 0x1f1f213e, 0x4b4bdd96, 0xbdbddc61, 0x8b8b860d, 0x8a8a850f, 0x707090e0, 0x3e3e427c, 0xb5b5c471, 0x6666aacc, 0x4848d890, 0x03030506, 0xf6f601f7, 0x0e0e121c, 0x6161a3c2, 0x35355f6a, 0x5757f9ae, 0xb9b9d069, 0x86869117, 0xc1c15899, 0x1d1d273a, 0x9e9eb927, 0xe1e138d9, 0xf8f813eb, 0x9898b32b, 0x11113322, 0x6969bbd2, 0xd9d970a9, 0x8e8e8907, 0x9494a733, 0x9b9bb62d, 0x1e1e223c, 0x87879215, 0xe9e920c9, 0xcece4987, 0x5555ffaa, 0x28287850, 0xdfdf7aa5, 0x8c8c8f03, 0xa1a1f859, 0x89898009, 0x0d0d171a, 0xbfbfda65, 0xe6e631d7, 0x4242c684, 0x6868b8d0, 0x4141c382, 0x9999b029, 0x2d2d775a, 0x0f0f111e, 0xb0b0cb7b, 0x5454fca8, 0xbbbbd66d, 0x16163a2c]; // Transformations for decryption

	var T5 = [0x51f4a750, 0x7e416553, 0x1a17a4c3, 0x3a275e96, 0x3bab6bcb, 0x1f9d45f1, 0xacfa58ab, 0x4be30393, 0x2030fa55, 0xad766df6, 0x88cc7691, 0xf5024c25, 0x4fe5d7fc, 0xc52acbd7, 0x26354480, 0xb562a38f, 0xdeb15a49, 0x25ba1b67, 0x45ea0e98, 0x5dfec0e1, 0xc32f7502, 0x814cf012, 0x8d4697a3, 0x6bd3f9c6, 0x038f5fe7, 0x15929c95, 0xbf6d7aeb, 0x955259da, 0xd4be832d, 0x587421d3, 0x49e06929, 0x8ec9c844, 0x75c2896a, 0xf48e7978, 0x99583e6b, 0x27b971dd, 0xbee14fb6, 0xf088ad17, 0xc920ac66, 0x7dce3ab4, 0x63df4a18, 0xe51a3182, 0x97513360, 0x62537f45, 0xb16477e0, 0xbb6bae84, 0xfe81a01c, 0xf9082b94, 0x70486858, 0x8f45fd19, 0x94de6c87, 0x527bf8b7, 0xab73d323, 0x724b02e2, 0xe31f8f57, 0x6655ab2a, 0xb2eb2807, 0x2fb5c203, 0x86c57b9a, 0xd33708a5, 0x302887f2, 0x23bfa5b2, 0x02036aba, 0xed16825c, 0x8acf1c2b, 0xa779b492, 0xf307f2f0, 0x4e69e2a1, 0x65daf4cd, 0x0605bed5, 0xd134621f, 0xc4a6fe8a, 0x342e539d, 0xa2f355a0, 0x058ae132, 0xa4f6eb75, 0x0b83ec39, 0x4060efaa, 0x5e719f06, 0xbd6e1051, 0x3e218af9, 0x96dd063d, 0xdd3e05ae, 0x4de6bd46, 0x91548db5, 0x71c45d05, 0x0406d46f, 0x605015ff, 0x1998fb24, 0xd6bde997, 0x894043cc, 0x67d99e77, 0xb0e842bd, 0x07898b88, 0xe7195b38, 0x79c8eedb, 0xa17c0a47, 0x7c420fe9, 0xf8841ec9, 0x00000000, 0x09808683, 0x322bed48, 0x1e1170ac, 0x6c5a724e, 0xfd0efffb, 0x0f853856, 0x3daed51e, 0x362d3927, 0x0a0fd964, 0x685ca621, 0x9b5b54d1, 0x24362e3a, 0x0c0a67b1, 0x9357e70f, 0xb4ee96d2, 0x1b9b919e, 0x80c0c54f, 0x61dc20a2, 0x5a774b69, 0x1c121a16, 0xe293ba0a, 0xc0a02ae5, 0x3c22e043, 0x121b171d, 0x0e090d0b, 0xf28bc7ad, 0x2db6a8b9, 0x141ea9c8, 0x57f11985, 0xaf75074c, 0xee99ddbb, 0xa37f60fd, 0xf701269f, 0x5c72f5bc, 0x44663bc5, 0x5bfb7e34, 0x8b432976, 0xcb23c6dc, 0xb6edfc68, 0xb8e4f163, 0xd731dcca, 0x42638510, 0x13972240, 0x84c61120, 0x854a247d, 0xd2bb3df8, 0xaef93211, 0xc729a16d, 0x1d9e2f4b, 0xdcb230f3, 0x0d8652ec, 0x77c1e3d0, 0x2bb3166c, 0xa970b999, 0x119448fa, 0x47e96422, 0xa8fc8cc4, 0xa0f03f1a, 0x567d2cd8, 0x223390ef, 0x87494ec7, 0xd938d1c1, 0x8ccaa2fe, 0x98d40b36, 0xa6f581cf, 0xa57ade28, 0xdab78e26, 0x3fadbfa4, 0x2c3a9de4, 0x5078920d, 0x6a5fcc9b, 0x547e4662, 0xf68d13c2, 0x90d8b8e8, 0x2e39f75e, 0x82c3aff5, 0x9f5d80be, 0x69d0937c, 0x6fd52da9, 0xcf2512b3, 0xc8ac993b, 0x10187da7, 0xe89c636e, 0xdb3bbb7b, 0xcd267809, 0x6e5918f4, 0xec9ab701, 0x834f9aa8, 0xe6956e65, 0xaaffe67e, 0x21bccf08, 0xef15e8e6, 0xbae79bd9, 0x4a6f36ce, 0xea9f09d4, 0x29b07cd6, 0x31a4b2af, 0x2a3f2331, 0xc6a59430, 0x35a266c0, 0x744ebc37, 0xfc82caa6, 0xe090d0b0, 0x33a7d815, 0xf104984a, 0x41ecdaf7, 0x7fcd500e, 0x1791f62f, 0x764dd68d, 0x43efb04d, 0xccaa4d54, 0xe49604df, 0x9ed1b5e3, 0x4c6a881b, 0xc12c1fb8, 0x4665517f, 0x9d5eea04, 0x018c355d, 0xfa877473, 0xfb0b412e, 0xb3671d5a, 0x92dbd252, 0xe9105633, 0x6dd64713, 0x9ad7618c, 0x37a10c7a, 0x59f8148e, 0xeb133c89, 0xcea927ee, 0xb761c935, 0xe11ce5ed, 0x7a47b13c, 0x9cd2df59, 0x55f2733f, 0x1814ce79, 0x73c737bf, 0x53f7cdea, 0x5ffdaa5b, 0xdf3d6f14, 0x7844db86, 0xcaaff381, 0xb968c43e, 0x3824342c, 0xc2a3405f, 0x161dc372, 0xbce2250c, 0x283c498b, 0xff0d9541, 0x39a80171, 0x080cb3de, 0xd8b4e49c, 0x6456c190, 0x7bcb8461, 0xd532b670, 0x486c5c74, 0xd0b85742];
	var T6 = [0x5051f4a7, 0x537e4165, 0xc31a17a4, 0x963a275e, 0xcb3bab6b, 0xf11f9d45, 0xabacfa58, 0x934be303, 0x552030fa, 0xf6ad766d, 0x9188cc76, 0x25f5024c, 0xfc4fe5d7, 0xd7c52acb, 0x80263544, 0x8fb562a3, 0x49deb15a, 0x6725ba1b, 0x9845ea0e, 0xe15dfec0, 0x02c32f75, 0x12814cf0, 0xa38d4697, 0xc66bd3f9, 0xe7038f5f, 0x9515929c, 0xebbf6d7a, 0xda955259, 0x2dd4be83, 0xd3587421, 0x2949e069, 0x448ec9c8, 0x6a75c289, 0x78f48e79, 0x6b99583e, 0xdd27b971, 0xb6bee14f, 0x17f088ad, 0x66c920ac, 0xb47dce3a, 0x1863df4a, 0x82e51a31, 0x60975133, 0x4562537f, 0xe0b16477, 0x84bb6bae, 0x1cfe81a0, 0x94f9082b, 0x58704868, 0x198f45fd, 0x8794de6c, 0xb7527bf8, 0x23ab73d3, 0xe2724b02, 0x57e31f8f, 0x2a6655ab, 0x07b2eb28, 0x032fb5c2, 0x9a86c57b, 0xa5d33708, 0xf2302887, 0xb223bfa5, 0xba02036a, 0x5ced1682, 0x2b8acf1c, 0x92a779b4, 0xf0f307f2, 0xa14e69e2, 0xcd65daf4, 0xd50605be, 0x1fd13462, 0x8ac4a6fe, 0x9d342e53, 0xa0a2f355, 0x32058ae1, 0x75a4f6eb, 0x390b83ec, 0xaa4060ef, 0x065e719f, 0x51bd6e10, 0xf93e218a, 0x3d96dd06, 0xaedd3e05, 0x464de6bd, 0xb591548d, 0x0571c45d, 0x6f0406d4, 0xff605015, 0x241998fb, 0x97d6bde9, 0xcc894043, 0x7767d99e, 0xbdb0e842, 0x8807898b, 0x38e7195b, 0xdb79c8ee, 0x47a17c0a, 0xe97c420f, 0xc9f8841e, 0x00000000, 0x83098086, 0x48322bed, 0xac1e1170, 0x4e6c5a72, 0xfbfd0eff, 0x560f8538, 0x1e3daed5, 0x27362d39, 0x640a0fd9, 0x21685ca6, 0xd19b5b54, 0x3a24362e, 0xb10c0a67, 0x0f9357e7, 0xd2b4ee96, 0x9e1b9b91, 0x4f80c0c5, 0xa261dc20, 0x695a774b, 0x161c121a, 0x0ae293ba, 0xe5c0a02a, 0x433c22e0, 0x1d121b17, 0x0b0e090d, 0xadf28bc7, 0xb92db6a8, 0xc8141ea9, 0x8557f119, 0x4caf7507, 0xbbee99dd, 0xfda37f60, 0x9ff70126, 0xbc5c72f5, 0xc544663b, 0x345bfb7e, 0x768b4329, 0xdccb23c6, 0x68b6edfc, 0x63b8e4f1, 0xcad731dc, 0x10426385, 0x40139722, 0x2084c611, 0x7d854a24, 0xf8d2bb3d, 0x11aef932, 0x6dc729a1, 0x4b1d9e2f, 0xf3dcb230, 0xec0d8652, 0xd077c1e3, 0x6c2bb316, 0x99a970b9, 0xfa119448, 0x2247e964, 0xc4a8fc8c, 0x1aa0f03f, 0xd8567d2c, 0xef223390, 0xc787494e, 0xc1d938d1, 0xfe8ccaa2, 0x3698d40b, 0xcfa6f581, 0x28a57ade, 0x26dab78e, 0xa43fadbf, 0xe42c3a9d, 0x0d507892, 0x9b6a5fcc, 0x62547e46, 0xc2f68d13, 0xe890d8b8, 0x5e2e39f7, 0xf582c3af, 0xbe9f5d80, 0x7c69d093, 0xa96fd52d, 0xb3cf2512, 0x3bc8ac99, 0xa710187d, 0x6ee89c63, 0x7bdb3bbb, 0x09cd2678, 0xf46e5918, 0x01ec9ab7, 0xa8834f9a, 0x65e6956e, 0x7eaaffe6, 0x0821bccf, 0xe6ef15e8, 0xd9bae79b, 0xce4a6f36, 0xd4ea9f09, 0xd629b07c, 0xaf31a4b2, 0x312a3f23, 0x30c6a594, 0xc035a266, 0x37744ebc, 0xa6fc82ca, 0xb0e090d0, 0x1533a7d8, 0x4af10498, 0xf741ecda, 0x0e7fcd50, 0x2f1791f6, 0x8d764dd6, 0x4d43efb0, 0x54ccaa4d, 0xdfe49604, 0xe39ed1b5, 0x1b4c6a88, 0xb8c12c1f, 0x7f466551, 0x049d5eea, 0x5d018c35, 0x73fa8774, 0x2efb0b41, 0x5ab3671d, 0x5292dbd2, 0x33e91056, 0x136dd647, 0x8c9ad761, 0x7a37a10c, 0x8e59f814, 0x89eb133c, 0xeecea927, 0x35b761c9, 0xede11ce5, 0x3c7a47b1, 0x599cd2df, 0x3f55f273, 0x791814ce, 0xbf73c737, 0xea53f7cd, 0x5b5ffdaa, 0x14df3d6f, 0x867844db, 0x81caaff3, 0x3eb968c4, 0x2c382434, 0x5fc2a340, 0x72161dc3, 0x0cbce225, 0x8b283c49, 0x41ff0d95, 0x7139a801, 0xde080cb3, 0x9cd8b4e4, 0x906456c1, 0x617bcb84, 0x70d532b6, 0x74486c5c, 0x42d0b857];
	var T7 = [0xa75051f4, 0x65537e41, 0xa4c31a17, 0x5e963a27, 0x6bcb3bab, 0x45f11f9d, 0x58abacfa, 0x03934be3, 0xfa552030, 0x6df6ad76, 0x769188cc, 0x4c25f502, 0xd7fc4fe5, 0xcbd7c52a, 0x44802635, 0xa38fb562, 0x5a49deb1, 0x1b6725ba, 0x0e9845ea, 0xc0e15dfe, 0x7502c32f, 0xf012814c, 0x97a38d46, 0xf9c66bd3, 0x5fe7038f, 0x9c951592, 0x7aebbf6d, 0x59da9552, 0x832dd4be, 0x21d35874, 0x692949e0, 0xc8448ec9, 0x896a75c2, 0x7978f48e, 0x3e6b9958, 0x71dd27b9, 0x4fb6bee1, 0xad17f088, 0xac66c920, 0x3ab47dce, 0x4a1863df, 0x3182e51a, 0x33609751, 0x7f456253, 0x77e0b164, 0xae84bb6b, 0xa01cfe81, 0x2b94f908, 0x68587048, 0xfd198f45, 0x6c8794de, 0xf8b7527b, 0xd323ab73, 0x02e2724b, 0x8f57e31f, 0xab2a6655, 0x2807b2eb, 0xc2032fb5, 0x7b9a86c5, 0x08a5d337, 0x87f23028, 0xa5b223bf, 0x6aba0203, 0x825ced16, 0x1c2b8acf, 0xb492a779, 0xf2f0f307, 0xe2a14e69, 0xf4cd65da, 0xbed50605, 0x621fd134, 0xfe8ac4a6, 0x539d342e, 0x55a0a2f3, 0xe132058a, 0xeb75a4f6, 0xec390b83, 0xefaa4060, 0x9f065e71, 0x1051bd6e, 0x8af93e21, 0x063d96dd, 0x05aedd3e, 0xbd464de6, 0x8db59154, 0x5d0571c4, 0xd46f0406, 0x15ff6050, 0xfb241998, 0xe997d6bd, 0x43cc8940, 0x9e7767d9, 0x42bdb0e8, 0x8b880789, 0x5b38e719, 0xeedb79c8, 0x0a47a17c, 0x0fe97c42, 0x1ec9f884, 0x00000000, 0x86830980, 0xed48322b, 0x70ac1e11, 0x724e6c5a, 0xfffbfd0e, 0x38560f85, 0xd51e3dae, 0x3927362d, 0xd9640a0f, 0xa621685c, 0x54d19b5b, 0x2e3a2436, 0x67b10c0a, 0xe70f9357, 0x96d2b4ee, 0x919e1b9b, 0xc54f80c0, 0x20a261dc, 0x4b695a77, 0x1a161c12, 0xba0ae293, 0x2ae5c0a0, 0xe0433c22, 0x171d121b, 0x0d0b0e09, 0xc7adf28b, 0xa8b92db6, 0xa9c8141e, 0x198557f1, 0x074caf75, 0xddbbee99, 0x60fda37f, 0x269ff701, 0xf5bc5c72, 0x3bc54466, 0x7e345bfb, 0x29768b43, 0xc6dccb23, 0xfc68b6ed, 0xf163b8e4, 0xdccad731, 0x85104263, 0x22401397, 0x112084c6, 0x247d854a, 0x3df8d2bb, 0x3211aef9, 0xa16dc729, 0x2f4b1d9e, 0x30f3dcb2, 0x52ec0d86, 0xe3d077c1, 0x166c2bb3, 0xb999a970, 0x48fa1194, 0x642247e9, 0x8cc4a8fc, 0x3f1aa0f0, 0x2cd8567d, 0x90ef2233, 0x4ec78749, 0xd1c1d938, 0xa2fe8cca, 0x0b3698d4, 0x81cfa6f5, 0xde28a57a, 0x8e26dab7, 0xbfa43fad, 0x9de42c3a, 0x920d5078, 0xcc9b6a5f, 0x4662547e, 0x13c2f68d, 0xb8e890d8, 0xf75e2e39, 0xaff582c3, 0x80be9f5d, 0x937c69d0, 0x2da96fd5, 0x12b3cf25, 0x993bc8ac, 0x7da71018, 0x636ee89c, 0xbb7bdb3b, 0x7809cd26, 0x18f46e59, 0xb701ec9a, 0x9aa8834f, 0x6e65e695, 0xe67eaaff, 0xcf0821bc, 0xe8e6ef15, 0x9bd9bae7, 0x36ce4a6f, 0x09d4ea9f, 0x7cd629b0, 0xb2af31a4, 0x23312a3f, 0x9430c6a5, 0x66c035a2, 0xbc37744e, 0xcaa6fc82, 0xd0b0e090, 0xd81533a7, 0x984af104, 0xdaf741ec, 0x500e7fcd, 0xf62f1791, 0xd68d764d, 0xb04d43ef, 0x4d54ccaa, 0x04dfe496, 0xb5e39ed1, 0x881b4c6a, 0x1fb8c12c, 0x517f4665, 0xea049d5e, 0x355d018c, 0x7473fa87, 0x412efb0b, 0x1d5ab367, 0xd25292db, 0x5633e910, 0x47136dd6, 0x618c9ad7, 0x0c7a37a1, 0x148e59f8, 0x3c89eb13, 0x27eecea9, 0xc935b761, 0xe5ede11c, 0xb13c7a47, 0xdf599cd2, 0x733f55f2, 0xce791814, 0x37bf73c7, 0xcdea53f7, 0xaa5b5ffd, 0x6f14df3d, 0xdb867844, 0xf381caaf, 0xc43eb968, 0x342c3824, 0x405fc2a3, 0xc372161d, 0x250cbce2, 0x498b283c, 0x9541ff0d, 0x017139a8, 0xb3de080c, 0xe49cd8b4, 0xc1906456, 0x84617bcb, 0xb670d532, 0x5c74486c, 0x5742d0b8];
	var T8 = [0xf4a75051, 0x4165537e, 0x17a4c31a, 0x275e963a, 0xab6bcb3b, 0x9d45f11f, 0xfa58abac, 0xe303934b, 0x30fa5520, 0x766df6ad, 0xcc769188, 0x024c25f5, 0xe5d7fc4f, 0x2acbd7c5, 0x35448026, 0x62a38fb5, 0xb15a49de, 0xba1b6725, 0xea0e9845, 0xfec0e15d, 0x2f7502c3, 0x4cf01281, 0x4697a38d, 0xd3f9c66b, 0x8f5fe703, 0x929c9515, 0x6d7aebbf, 0x5259da95, 0xbe832dd4, 0x7421d358, 0xe0692949, 0xc9c8448e, 0xc2896a75, 0x8e7978f4, 0x583e6b99, 0xb971dd27, 0xe14fb6be, 0x88ad17f0, 0x20ac66c9, 0xce3ab47d, 0xdf4a1863, 0x1a3182e5, 0x51336097, 0x537f4562, 0x6477e0b1, 0x6bae84bb, 0x81a01cfe, 0x082b94f9, 0x48685870, 0x45fd198f, 0xde6c8794, 0x7bf8b752, 0x73d323ab, 0x4b02e272, 0x1f8f57e3, 0x55ab2a66, 0xeb2807b2, 0xb5c2032f, 0xc57b9a86, 0x3708a5d3, 0x2887f230, 0xbfa5b223, 0x036aba02, 0x16825ced, 0xcf1c2b8a, 0x79b492a7, 0x07f2f0f3, 0x69e2a14e, 0xdaf4cd65, 0x05bed506, 0x34621fd1, 0xa6fe8ac4, 0x2e539d34, 0xf355a0a2, 0x8ae13205, 0xf6eb75a4, 0x83ec390b, 0x60efaa40, 0x719f065e, 0x6e1051bd, 0x218af93e, 0xdd063d96, 0x3e05aedd, 0xe6bd464d, 0x548db591, 0xc45d0571, 0x06d46f04, 0x5015ff60, 0x98fb2419, 0xbde997d6, 0x4043cc89, 0xd99e7767, 0xe842bdb0, 0x898b8807, 0x195b38e7, 0xc8eedb79, 0x7c0a47a1, 0x420fe97c, 0x841ec9f8, 0x00000000, 0x80868309, 0x2bed4832, 0x1170ac1e, 0x5a724e6c, 0x0efffbfd, 0x8538560f, 0xaed51e3d, 0x2d392736, 0x0fd9640a, 0x5ca62168, 0x5b54d19b, 0x362e3a24, 0x0a67b10c, 0x57e70f93, 0xee96d2b4, 0x9b919e1b, 0xc0c54f80, 0xdc20a261, 0x774b695a, 0x121a161c, 0x93ba0ae2, 0xa02ae5c0, 0x22e0433c, 0x1b171d12, 0x090d0b0e, 0x8bc7adf2, 0xb6a8b92d, 0x1ea9c814, 0xf1198557, 0x75074caf, 0x99ddbbee, 0x7f60fda3, 0x01269ff7, 0x72f5bc5c, 0x663bc544, 0xfb7e345b, 0x4329768b, 0x23c6dccb, 0xedfc68b6, 0xe4f163b8, 0x31dccad7, 0x63851042, 0x97224013, 0xc6112084, 0x4a247d85, 0xbb3df8d2, 0xf93211ae, 0x29a16dc7, 0x9e2f4b1d, 0xb230f3dc, 0x8652ec0d, 0xc1e3d077, 0xb3166c2b, 0x70b999a9, 0x9448fa11, 0xe9642247, 0xfc8cc4a8, 0xf03f1aa0, 0x7d2cd856, 0x3390ef22, 0x494ec787, 0x38d1c1d9, 0xcaa2fe8c, 0xd40b3698, 0xf581cfa6, 0x7ade28a5, 0xb78e26da, 0xadbfa43f, 0x3a9de42c, 0x78920d50, 0x5fcc9b6a, 0x7e466254, 0x8d13c2f6, 0xd8b8e890, 0x39f75e2e, 0xc3aff582, 0x5d80be9f, 0xd0937c69, 0xd52da96f, 0x2512b3cf, 0xac993bc8, 0x187da710, 0x9c636ee8, 0x3bbb7bdb, 0x267809cd, 0x5918f46e, 0x9ab701ec, 0x4f9aa883, 0x956e65e6, 0xffe67eaa, 0xbccf0821, 0x15e8e6ef, 0xe79bd9ba, 0x6f36ce4a, 0x9f09d4ea, 0xb07cd629, 0xa4b2af31, 0x3f23312a, 0xa59430c6, 0xa266c035, 0x4ebc3774, 0x82caa6fc, 0x90d0b0e0, 0xa7d81533, 0x04984af1, 0xecdaf741, 0xcd500e7f, 0x91f62f17, 0x4dd68d76, 0xefb04d43, 0xaa4d54cc, 0x9604dfe4, 0xd1b5e39e, 0x6a881b4c, 0x2c1fb8c1, 0x65517f46, 0x5eea049d, 0x8c355d01, 0x877473fa, 0x0b412efb, 0x671d5ab3, 0xdbd25292, 0x105633e9, 0xd647136d, 0xd7618c9a, 0xa10c7a37, 0xf8148e59, 0x133c89eb, 0xa927eece, 0x61c935b7, 0x1ce5ede1, 0x47b13c7a, 0xd2df599c, 0xf2733f55, 0x14ce7918, 0xc737bf73, 0xf7cdea53, 0xfdaa5b5f, 0x3d6f14df, 0x44db8678, 0xaff381ca, 0x68c43eb9, 0x24342c38, 0xa3405fc2, 0x1dc37216, 0xe2250cbc, 0x3c498b28, 0x0d9541ff, 0xa8017139, 0x0cb3de08, 0xb4e49cd8, 0x56c19064, 0xcb84617b, 0x32b670d5, 0x6c5c7448, 0xb85742d0]; // Transformations for decryption key expansion

	var U1 = [0x00000000, 0x0e090d0b, 0x1c121a16, 0x121b171d, 0x3824342c, 0x362d3927, 0x24362e3a, 0x2a3f2331, 0x70486858, 0x7e416553, 0x6c5a724e, 0x62537f45, 0x486c5c74, 0x4665517f, 0x547e4662, 0x5a774b69, 0xe090d0b0, 0xee99ddbb, 0xfc82caa6, 0xf28bc7ad, 0xd8b4e49c, 0xd6bde997, 0xc4a6fe8a, 0xcaaff381, 0x90d8b8e8, 0x9ed1b5e3, 0x8ccaa2fe, 0x82c3aff5, 0xa8fc8cc4, 0xa6f581cf, 0xb4ee96d2, 0xbae79bd9, 0xdb3bbb7b, 0xd532b670, 0xc729a16d, 0xc920ac66, 0xe31f8f57, 0xed16825c, 0xff0d9541, 0xf104984a, 0xab73d323, 0xa57ade28, 0xb761c935, 0xb968c43e, 0x9357e70f, 0x9d5eea04, 0x8f45fd19, 0x814cf012, 0x3bab6bcb, 0x35a266c0, 0x27b971dd, 0x29b07cd6, 0x038f5fe7, 0x0d8652ec, 0x1f9d45f1, 0x119448fa, 0x4be30393, 0x45ea0e98, 0x57f11985, 0x59f8148e, 0x73c737bf, 0x7dce3ab4, 0x6fd52da9, 0x61dc20a2, 0xad766df6, 0xa37f60fd, 0xb16477e0, 0xbf6d7aeb, 0x955259da, 0x9b5b54d1, 0x894043cc, 0x87494ec7, 0xdd3e05ae, 0xd33708a5, 0xc12c1fb8, 0xcf2512b3, 0xe51a3182, 0xeb133c89, 0xf9082b94, 0xf701269f, 0x4de6bd46, 0x43efb04d, 0x51f4a750, 0x5ffdaa5b, 0x75c2896a, 0x7bcb8461, 0x69d0937c, 0x67d99e77, 0x3daed51e, 0x33a7d815, 0x21bccf08, 0x2fb5c203, 0x058ae132, 0x0b83ec39, 0x1998fb24, 0x1791f62f, 0x764dd68d, 0x7844db86, 0x6a5fcc9b, 0x6456c190, 0x4e69e2a1, 0x4060efaa, 0x527bf8b7, 0x5c72f5bc, 0x0605bed5, 0x080cb3de, 0x1a17a4c3, 0x141ea9c8, 0x3e218af9, 0x302887f2, 0x223390ef, 0x2c3a9de4, 0x96dd063d, 0x98d40b36, 0x8acf1c2b, 0x84c61120, 0xaef93211, 0xa0f03f1a, 0xb2eb2807, 0xbce2250c, 0xe6956e65, 0xe89c636e, 0xfa877473, 0xf48e7978, 0xdeb15a49, 0xd0b85742, 0xc2a3405f, 0xccaa4d54, 0x41ecdaf7, 0x4fe5d7fc, 0x5dfec0e1, 0x53f7cdea, 0x79c8eedb, 0x77c1e3d0, 0x65daf4cd, 0x6bd3f9c6, 0x31a4b2af, 0x3fadbfa4, 0x2db6a8b9, 0x23bfa5b2, 0x09808683, 0x07898b88, 0x15929c95, 0x1b9b919e, 0xa17c0a47, 0xaf75074c, 0xbd6e1051, 0xb3671d5a, 0x99583e6b, 0x97513360, 0x854a247d, 0x8b432976, 0xd134621f, 0xdf3d6f14, 0xcd267809, 0xc32f7502, 0xe9105633, 0xe7195b38, 0xf5024c25, 0xfb0b412e, 0x9ad7618c, 0x94de6c87, 0x86c57b9a, 0x88cc7691, 0xa2f355a0, 0xacfa58ab, 0xbee14fb6, 0xb0e842bd, 0xea9f09d4, 0xe49604df, 0xf68d13c2, 0xf8841ec9, 0xd2bb3df8, 0xdcb230f3, 0xcea927ee, 0xc0a02ae5, 0x7a47b13c, 0x744ebc37, 0x6655ab2a, 0x685ca621, 0x42638510, 0x4c6a881b, 0x5e719f06, 0x5078920d, 0x0a0fd964, 0x0406d46f, 0x161dc372, 0x1814ce79, 0x322bed48, 0x3c22e043, 0x2e39f75e, 0x2030fa55, 0xec9ab701, 0xe293ba0a, 0xf088ad17, 0xfe81a01c, 0xd4be832d, 0xdab78e26, 0xc8ac993b, 0xc6a59430, 0x9cd2df59, 0x92dbd252, 0x80c0c54f, 0x8ec9c844, 0xa4f6eb75, 0xaaffe67e, 0xb8e4f163, 0xb6edfc68, 0x0c0a67b1, 0x02036aba, 0x10187da7, 0x1e1170ac, 0x342e539d, 0x3a275e96, 0x283c498b, 0x26354480, 0x7c420fe9, 0x724b02e2, 0x605015ff, 0x6e5918f4, 0x44663bc5, 0x4a6f36ce, 0x587421d3, 0x567d2cd8, 0x37a10c7a, 0x39a80171, 0x2bb3166c, 0x25ba1b67, 0x0f853856, 0x018c355d, 0x13972240, 0x1d9e2f4b, 0x47e96422, 0x49e06929, 0x5bfb7e34, 0x55f2733f, 0x7fcd500e, 0x71c45d05, 0x63df4a18, 0x6dd64713, 0xd731dcca, 0xd938d1c1, 0xcb23c6dc, 0xc52acbd7, 0xef15e8e6, 0xe11ce5ed, 0xf307f2f0, 0xfd0efffb, 0xa779b492, 0xa970b999, 0xbb6bae84, 0xb562a38f, 0x9f5d80be, 0x91548db5, 0x834f9aa8, 0x8d4697a3];
	var U2 = [0x00000000, 0x0b0e090d, 0x161c121a, 0x1d121b17, 0x2c382434, 0x27362d39, 0x3a24362e, 0x312a3f23, 0x58704868, 0x537e4165, 0x4e6c5a72, 0x4562537f, 0x74486c5c, 0x7f466551, 0x62547e46, 0x695a774b, 0xb0e090d0, 0xbbee99dd, 0xa6fc82ca, 0xadf28bc7, 0x9cd8b4e4, 0x97d6bde9, 0x8ac4a6fe, 0x81caaff3, 0xe890d8b8, 0xe39ed1b5, 0xfe8ccaa2, 0xf582c3af, 0xc4a8fc8c, 0xcfa6f581, 0xd2b4ee96, 0xd9bae79b, 0x7bdb3bbb, 0x70d532b6, 0x6dc729a1, 0x66c920ac, 0x57e31f8f, 0x5ced1682, 0x41ff0d95, 0x4af10498, 0x23ab73d3, 0x28a57ade, 0x35b761c9, 0x3eb968c4, 0x0f9357e7, 0x049d5eea, 0x198f45fd, 0x12814cf0, 0xcb3bab6b, 0xc035a266, 0xdd27b971, 0xd629b07c, 0xe7038f5f, 0xec0d8652, 0xf11f9d45, 0xfa119448, 0x934be303, 0x9845ea0e, 0x8557f119, 0x8e59f814, 0xbf73c737, 0xb47dce3a, 0xa96fd52d, 0xa261dc20, 0xf6ad766d, 0xfda37f60, 0xe0b16477, 0xebbf6d7a, 0xda955259, 0xd19b5b54, 0xcc894043, 0xc787494e, 0xaedd3e05, 0xa5d33708, 0xb8c12c1f, 0xb3cf2512, 0x82e51a31, 0x89eb133c, 0x94f9082b, 0x9ff70126, 0x464de6bd, 0x4d43efb0, 0x5051f4a7, 0x5b5ffdaa, 0x6a75c289, 0x617bcb84, 0x7c69d093, 0x7767d99e, 0x1e3daed5, 0x1533a7d8, 0x0821bccf, 0x032fb5c2, 0x32058ae1, 0x390b83ec, 0x241998fb, 0x2f1791f6, 0x8d764dd6, 0x867844db, 0x9b6a5fcc, 0x906456c1, 0xa14e69e2, 0xaa4060ef, 0xb7527bf8, 0xbc5c72f5, 0xd50605be, 0xde080cb3, 0xc31a17a4, 0xc8141ea9, 0xf93e218a, 0xf2302887, 0xef223390, 0xe42c3a9d, 0x3d96dd06, 0x3698d40b, 0x2b8acf1c, 0x2084c611, 0x11aef932, 0x1aa0f03f, 0x07b2eb28, 0x0cbce225, 0x65e6956e, 0x6ee89c63, 0x73fa8774, 0x78f48e79, 0x49deb15a, 0x42d0b857, 0x5fc2a340, 0x54ccaa4d, 0xf741ecda, 0xfc4fe5d7, 0xe15dfec0, 0xea53f7cd, 0xdb79c8ee, 0xd077c1e3, 0xcd65daf4, 0xc66bd3f9, 0xaf31a4b2, 0xa43fadbf, 0xb92db6a8, 0xb223bfa5, 0x83098086, 0x8807898b, 0x9515929c, 0x9e1b9b91, 0x47a17c0a, 0x4caf7507, 0x51bd6e10, 0x5ab3671d, 0x6b99583e, 0x60975133, 0x7d854a24, 0x768b4329, 0x1fd13462, 0x14df3d6f, 0x09cd2678, 0x02c32f75, 0x33e91056, 0x38e7195b, 0x25f5024c, 0x2efb0b41, 0x8c9ad761, 0x8794de6c, 0x9a86c57b, 0x9188cc76, 0xa0a2f355, 0xabacfa58, 0xb6bee14f, 0xbdb0e842, 0xd4ea9f09, 0xdfe49604, 0xc2f68d13, 0xc9f8841e, 0xf8d2bb3d, 0xf3dcb230, 0xeecea927, 0xe5c0a02a, 0x3c7a47b1, 0x37744ebc, 0x2a6655ab, 0x21685ca6, 0x10426385, 0x1b4c6a88, 0x065e719f, 0x0d507892, 0x640a0fd9, 0x6f0406d4, 0x72161dc3, 0x791814ce, 0x48322bed, 0x433c22e0, 0x5e2e39f7, 0x552030fa, 0x01ec9ab7, 0x0ae293ba, 0x17f088ad, 0x1cfe81a0, 0x2dd4be83, 0x26dab78e, 0x3bc8ac99, 0x30c6a594, 0x599cd2df, 0x5292dbd2, 0x4f80c0c5, 0x448ec9c8, 0x75a4f6eb, 0x7eaaffe6, 0x63b8e4f1, 0x68b6edfc, 0xb10c0a67, 0xba02036a, 0xa710187d, 0xac1e1170, 0x9d342e53, 0x963a275e, 0x8b283c49, 0x80263544, 0xe97c420f, 0xe2724b02, 0xff605015, 0xf46e5918, 0xc544663b, 0xce4a6f36, 0xd3587421, 0xd8567d2c, 0x7a37a10c, 0x7139a801, 0x6c2bb316, 0x6725ba1b, 0x560f8538, 0x5d018c35, 0x40139722, 0x4b1d9e2f, 0x2247e964, 0x2949e069, 0x345bfb7e, 0x3f55f273, 0x0e7fcd50, 0x0571c45d, 0x1863df4a, 0x136dd647, 0xcad731dc, 0xc1d938d1, 0xdccb23c6, 0xd7c52acb, 0xe6ef15e8, 0xede11ce5, 0xf0f307f2, 0xfbfd0eff, 0x92a779b4, 0x99a970b9, 0x84bb6bae, 0x8fb562a3, 0xbe9f5d80, 0xb591548d, 0xa8834f9a, 0xa38d4697];
	var U3 = [0x00000000, 0x0d0b0e09, 0x1a161c12, 0x171d121b, 0x342c3824, 0x3927362d, 0x2e3a2436, 0x23312a3f, 0x68587048, 0x65537e41, 0x724e6c5a, 0x7f456253, 0x5c74486c, 0x517f4665, 0x4662547e, 0x4b695a77, 0xd0b0e090, 0xddbbee99, 0xcaa6fc82, 0xc7adf28b, 0xe49cd8b4, 0xe997d6bd, 0xfe8ac4a6, 0xf381caaf, 0xb8e890d8, 0xb5e39ed1, 0xa2fe8cca, 0xaff582c3, 0x8cc4a8fc, 0x81cfa6f5, 0x96d2b4ee, 0x9bd9bae7, 0xbb7bdb3b, 0xb670d532, 0xa16dc729, 0xac66c920, 0x8f57e31f, 0x825ced16, 0x9541ff0d, 0x984af104, 0xd323ab73, 0xde28a57a, 0xc935b761, 0xc43eb968, 0xe70f9357, 0xea049d5e, 0xfd198f45, 0xf012814c, 0x6bcb3bab, 0x66c035a2, 0x71dd27b9, 0x7cd629b0, 0x5fe7038f, 0x52ec0d86, 0x45f11f9d, 0x48fa1194, 0x03934be3, 0x0e9845ea, 0x198557f1, 0x148e59f8, 0x37bf73c7, 0x3ab47dce, 0x2da96fd5, 0x20a261dc, 0x6df6ad76, 0x60fda37f, 0x77e0b164, 0x7aebbf6d, 0x59da9552, 0x54d19b5b, 0x43cc8940, 0x4ec78749, 0x05aedd3e, 0x08a5d337, 0x1fb8c12c, 0x12b3cf25, 0x3182e51a, 0x3c89eb13, 0x2b94f908, 0x269ff701, 0xbd464de6, 0xb04d43ef, 0xa75051f4, 0xaa5b5ffd, 0x896a75c2, 0x84617bcb, 0x937c69d0, 0x9e7767d9, 0xd51e3dae, 0xd81533a7, 0xcf0821bc, 0xc2032fb5, 0xe132058a, 0xec390b83, 0xfb241998, 0xf62f1791, 0xd68d764d, 0xdb867844, 0xcc9b6a5f, 0xc1906456, 0xe2a14e69, 0xefaa4060, 0xf8b7527b, 0xf5bc5c72, 0xbed50605, 0xb3de080c, 0xa4c31a17, 0xa9c8141e, 0x8af93e21, 0x87f23028, 0x90ef2233, 0x9de42c3a, 0x063d96dd, 0x0b3698d4, 0x1c2b8acf, 0x112084c6, 0x3211aef9, 0x3f1aa0f0, 0x2807b2eb, 0x250cbce2, 0x6e65e695, 0x636ee89c, 0x7473fa87, 0x7978f48e, 0x5a49deb1, 0x5742d0b8, 0x405fc2a3, 0x4d54ccaa, 0xdaf741ec, 0xd7fc4fe5, 0xc0e15dfe, 0xcdea53f7, 0xeedb79c8, 0xe3d077c1, 0xf4cd65da, 0xf9c66bd3, 0xb2af31a4, 0xbfa43fad, 0xa8b92db6, 0xa5b223bf, 0x86830980, 0x8b880789, 0x9c951592, 0x919e1b9b, 0x0a47a17c, 0x074caf75, 0x1051bd6e, 0x1d5ab367, 0x3e6b9958, 0x33609751, 0x247d854a, 0x29768b43, 0x621fd134, 0x6f14df3d, 0x7809cd26, 0x7502c32f, 0x5633e910, 0x5b38e719, 0x4c25f502, 0x412efb0b, 0x618c9ad7, 0x6c8794de, 0x7b9a86c5, 0x769188cc, 0x55a0a2f3, 0x58abacfa, 0x4fb6bee1, 0x42bdb0e8, 0x09d4ea9f, 0x04dfe496, 0x13c2f68d, 0x1ec9f884, 0x3df8d2bb, 0x30f3dcb2, 0x27eecea9, 0x2ae5c0a0, 0xb13c7a47, 0xbc37744e, 0xab2a6655, 0xa621685c, 0x85104263, 0x881b4c6a, 0x9f065e71, 0x920d5078, 0xd9640a0f, 0xd46f0406, 0xc372161d, 0xce791814, 0xed48322b, 0xe0433c22, 0xf75e2e39, 0xfa552030, 0xb701ec9a, 0xba0ae293, 0xad17f088, 0xa01cfe81, 0x832dd4be, 0x8e26dab7, 0x993bc8ac, 0x9430c6a5, 0xdf599cd2, 0xd25292db, 0xc54f80c0, 0xc8448ec9, 0xeb75a4f6, 0xe67eaaff, 0xf163b8e4, 0xfc68b6ed, 0x67b10c0a, 0x6aba0203, 0x7da71018, 0x70ac1e11, 0x539d342e, 0x5e963a27, 0x498b283c, 0x44802635, 0x0fe97c42, 0x02e2724b, 0x15ff6050, 0x18f46e59, 0x3bc54466, 0x36ce4a6f, 0x21d35874, 0x2cd8567d, 0x0c7a37a1, 0x017139a8, 0x166c2bb3, 0x1b6725ba, 0x38560f85, 0x355d018c, 0x22401397, 0x2f4b1d9e, 0x642247e9, 0x692949e0, 0x7e345bfb, 0x733f55f2, 0x500e7fcd, 0x5d0571c4, 0x4a1863df, 0x47136dd6, 0xdccad731, 0xd1c1d938, 0xc6dccb23, 0xcbd7c52a, 0xe8e6ef15, 0xe5ede11c, 0xf2f0f307, 0xfffbfd0e, 0xb492a779, 0xb999a970, 0xae84bb6b, 0xa38fb562, 0x80be9f5d, 0x8db59154, 0x9aa8834f, 0x97a38d46];
	var U4 = [0x00000000, 0x090d0b0e, 0x121a161c, 0x1b171d12, 0x24342c38, 0x2d392736, 0x362e3a24, 0x3f23312a, 0x48685870, 0x4165537e, 0x5a724e6c, 0x537f4562, 0x6c5c7448, 0x65517f46, 0x7e466254, 0x774b695a, 0x90d0b0e0, 0x99ddbbee, 0x82caa6fc, 0x8bc7adf2, 0xb4e49cd8, 0xbde997d6, 0xa6fe8ac4, 0xaff381ca, 0xd8b8e890, 0xd1b5e39e, 0xcaa2fe8c, 0xc3aff582, 0xfc8cc4a8, 0xf581cfa6, 0xee96d2b4, 0xe79bd9ba, 0x3bbb7bdb, 0x32b670d5, 0x29a16dc7, 0x20ac66c9, 0x1f8f57e3, 0x16825ced, 0x0d9541ff, 0x04984af1, 0x73d323ab, 0x7ade28a5, 0x61c935b7, 0x68c43eb9, 0x57e70f93, 0x5eea049d, 0x45fd198f, 0x4cf01281, 0xab6bcb3b, 0xa266c035, 0xb971dd27, 0xb07cd629, 0x8f5fe703, 0x8652ec0d, 0x9d45f11f, 0x9448fa11, 0xe303934b, 0xea0e9845, 0xf1198557, 0xf8148e59, 0xc737bf73, 0xce3ab47d, 0xd52da96f, 0xdc20a261, 0x766df6ad, 0x7f60fda3, 0x6477e0b1, 0x6d7aebbf, 0x5259da95, 0x5b54d19b, 0x4043cc89, 0x494ec787, 0x3e05aedd, 0x3708a5d3, 0x2c1fb8c1, 0x2512b3cf, 0x1a3182e5, 0x133c89eb, 0x082b94f9, 0x01269ff7, 0xe6bd464d, 0xefb04d43, 0xf4a75051, 0xfdaa5b5f, 0xc2896a75, 0xcb84617b, 0xd0937c69, 0xd99e7767, 0xaed51e3d, 0xa7d81533, 0xbccf0821, 0xb5c2032f, 0x8ae13205, 0x83ec390b, 0x98fb2419, 0x91f62f17, 0x4dd68d76, 0x44db8678, 0x5fcc9b6a, 0x56c19064, 0x69e2a14e, 0x60efaa40, 0x7bf8b752, 0x72f5bc5c, 0x05bed506, 0x0cb3de08, 0x17a4c31a, 0x1ea9c814, 0x218af93e, 0x2887f230, 0x3390ef22, 0x3a9de42c, 0xdd063d96, 0xd40b3698, 0xcf1c2b8a, 0xc6112084, 0xf93211ae, 0xf03f1aa0, 0xeb2807b2, 0xe2250cbc, 0x956e65e6, 0x9c636ee8, 0x877473fa, 0x8e7978f4, 0xb15a49de, 0xb85742d0, 0xa3405fc2, 0xaa4d54cc, 0xecdaf741, 0xe5d7fc4f, 0xfec0e15d, 0xf7cdea53, 0xc8eedb79, 0xc1e3d077, 0xdaf4cd65, 0xd3f9c66b, 0xa4b2af31, 0xadbfa43f, 0xb6a8b92d, 0xbfa5b223, 0x80868309, 0x898b8807, 0x929c9515, 0x9b919e1b, 0x7c0a47a1, 0x75074caf, 0x6e1051bd, 0x671d5ab3, 0x583e6b99, 0x51336097, 0x4a247d85, 0x4329768b, 0x34621fd1, 0x3d6f14df, 0x267809cd, 0x2f7502c3, 0x105633e9, 0x195b38e7, 0x024c25f5, 0x0b412efb, 0xd7618c9a, 0xde6c8794, 0xc57b9a86, 0xcc769188, 0xf355a0a2, 0xfa58abac, 0xe14fb6be, 0xe842bdb0, 0x9f09d4ea, 0x9604dfe4, 0x8d13c2f6, 0x841ec9f8, 0xbb3df8d2, 0xb230f3dc, 0xa927eece, 0xa02ae5c0, 0x47b13c7a, 0x4ebc3774, 0x55ab2a66, 0x5ca62168, 0x63851042, 0x6a881b4c, 0x719f065e, 0x78920d50, 0x0fd9640a, 0x06d46f04, 0x1dc37216, 0x14ce7918, 0x2bed4832, 0x22e0433c, 0x39f75e2e, 0x30fa5520, 0x9ab701ec, 0x93ba0ae2, 0x88ad17f0, 0x81a01cfe, 0xbe832dd4, 0xb78e26da, 0xac993bc8, 0xa59430c6, 0xd2df599c, 0xdbd25292, 0xc0c54f80, 0xc9c8448e, 0xf6eb75a4, 0xffe67eaa, 0xe4f163b8, 0xedfc68b6, 0x0a67b10c, 0x036aba02, 0x187da710, 0x1170ac1e, 0x2e539d34, 0x275e963a, 0x3c498b28, 0x35448026, 0x420fe97c, 0x4b02e272, 0x5015ff60, 0x5918f46e, 0x663bc544, 0x6f36ce4a, 0x7421d358, 0x7d2cd856, 0xa10c7a37, 0xa8017139, 0xb3166c2b, 0xba1b6725, 0x8538560f, 0x8c355d01, 0x97224013, 0x9e2f4b1d, 0xe9642247, 0xe0692949, 0xfb7e345b, 0xf2733f55, 0xcd500e7f, 0xc45d0571, 0xdf4a1863, 0xd647136d, 0x31dccad7, 0x38d1c1d9, 0x23c6dccb, 0x2acbd7c5, 0x15e8e6ef, 0x1ce5ede1, 0x07f2f0f3, 0x0efffbfd, 0x79b492a7, 0x70b999a9, 0x6bae84bb, 0x62a38fb5, 0x5d80be9f, 0x548db591, 0x4f9aa883, 0x4697a38d];

	function convertToInt32(bytes) {
	  var result = [];

	  for (var i = 0; i < bytes.length; i += 4) {
	    result.push(bytes[i] << 24 | bytes[i + 1] << 16 | bytes[i + 2] << 8 | bytes[i + 3]);
	  }

	  return result;
	}

	var AesJs = function (key) {
	  if (!(this instanceof AesJs)) {
	    throw Error('AES must be instanitated with `new`');
	  }

	  Object.defineProperty(this, 'key', {
	    value: coerceArray(key, true)
	  });

	  this._prepare();
	};

	AesJs.prototype._prepare = function () {
	  var rounds = numberOfRounds[this.key.length];

	  if (rounds == null) {
	    throw new Error('invalid key size (must be 16, 24 or 32 bytes)');
	  } // encryption round keys


	  this._Ke = []; // decryption round keys

	  this._Kd = [];

	  for (var i = 0; i <= rounds; i++) {
	    this._Ke.push([0, 0, 0, 0]);

	    this._Kd.push([0, 0, 0, 0]);
	  }

	  var roundKeyCount = (rounds + 1) * 4;
	  var KC = this.key.length / 4; // convert the key into ints

	  var tk = convertToInt32(this.key); // copy values into round key arrays

	  var index;

	  for (var i = 0; i < KC; i++) {
	    index = i >> 2;
	    this._Ke[index][i % 4] = tk[i];
	    this._Kd[rounds - index][i % 4] = tk[i];
	  } // key expansion (fips-197 section 5.2)


	  var rconpointer = 0;
	  var t = KC,
	      tt;

	  while (t < roundKeyCount) {
	    tt = tk[KC - 1];
	    tk[0] ^= S[tt >> 16 & 0xFF] << 24 ^ S[tt >> 8 & 0xFF] << 16 ^ S[tt & 0xFF] << 8 ^ S[tt >> 24 & 0xFF] ^ rcon[rconpointer] << 24;
	    rconpointer += 1; // key expansion (for non-256 bit)

	    if (KC != 8) {
	      for (var i = 1; i < KC; i++) {
	        tk[i] ^= tk[i - 1];
	      } // key expansion for 256-bit keys is "slightly different" (fips-197)

	    } else {
	      for (var i = 1; i < KC / 2; i++) {
	        tk[i] ^= tk[i - 1];
	      }

	      tt = tk[KC / 2 - 1];
	      tk[KC / 2] ^= S[tt & 0xFF] ^ S[tt >> 8 & 0xFF] << 8 ^ S[tt >> 16 & 0xFF] << 16 ^ S[tt >> 24 & 0xFF] << 24;

	      for (var i = KC / 2 + 1; i < KC; i++) {
	        tk[i] ^= tk[i - 1];
	      }
	    } // copy values into round key arrays


	    var i = 0,
	        r,
	        c;

	    while (i < KC && t < roundKeyCount) {
	      r = t >> 2;
	      c = t % 4;
	      this._Ke[r][c] = tk[i];
	      this._Kd[rounds - r][c] = tk[i++];
	      t++;
	    }
	  } // inverse-cipher-ify the decryption round key (fips-197 section 5.3)


	  for (var r = 1; r < rounds; r++) {
	    for (var c = 0; c < 4; c++) {
	      tt = this._Kd[r][c];
	      this._Kd[r][c] = U1[tt >> 24 & 0xFF] ^ U2[tt >> 16 & 0xFF] ^ U3[tt >> 8 & 0xFF] ^ U4[tt & 0xFF];
	    }
	  }
	};

	AesJs.prototype.encrypt = function (plaintext) {
	  if (plaintext.length != 16) {
	    throw new Error('invalid plaintext size (must be 16 bytes)');
	  }

	  var rounds = this._Ke.length - 1;
	  var a = [0, 0, 0, 0]; // convert plaintext to (ints ^ key)

	  var t = convertToInt32(plaintext);

	  for (var i = 0; i < 4; i++) {
	    t[i] ^= this._Ke[0][i];
	  } // apply round transforms


	  for (var r = 1; r < rounds; r++) {
	    for (var i = 0; i < 4; i++) {
	      a[i] = T1[t[i] >> 24 & 0xff] ^ T2[t[(i + 1) % 4] >> 16 & 0xff] ^ T3[t[(i + 2) % 4] >> 8 & 0xff] ^ T4[t[(i + 3) % 4] & 0xff] ^ this._Ke[r][i];
	    }

	    t = a.slice();
	  } // the last round is special


	  var result = createArray(16),
	      tt;

	  for (var i = 0; i < 4; i++) {
	    tt = this._Ke[rounds][i];
	    result[4 * i] = (S[t[i] >> 24 & 0xff] ^ tt >> 24) & 0xff;
	    result[4 * i + 1] = (S[t[(i + 1) % 4] >> 16 & 0xff] ^ tt >> 16) & 0xff;
	    result[4 * i + 2] = (S[t[(i + 2) % 4] >> 8 & 0xff] ^ tt >> 8) & 0xff;
	    result[4 * i + 3] = (S[t[(i + 3) % 4] & 0xff] ^ tt) & 0xff;
	  }

	  return result;
	};

	AesJs.prototype.decrypt = function (ciphertext) {
	  if (ciphertext.length != 16) {
	    throw new Error('invalid ciphertext size (must be 16 bytes)');
	  }

	  var rounds = this._Kd.length - 1;
	  var a = [0, 0, 0, 0]; // convert plaintext to (ints ^ key)

	  var t = convertToInt32(ciphertext);

	  for (var i = 0; i < 4; i++) {
	    t[i] ^= this._Kd[0][i];
	  } // apply round transforms


	  for (var r = 1; r < rounds; r++) {
	    for (var i = 0; i < 4; i++) {
	      a[i] = T5[t[i] >> 24 & 0xff] ^ T6[t[(i + 3) % 4] >> 16 & 0xff] ^ T7[t[(i + 2) % 4] >> 8 & 0xff] ^ T8[t[(i + 1) % 4] & 0xff] ^ this._Kd[r][i];
	    }

	    t = a.slice();
	  } // the last round is special


	  var result = createArray(16),
	      tt;

	  for (var i = 0; i < 4; i++) {
	    tt = this._Kd[rounds][i];
	    result[4 * i] = (Si[t[i] >> 24 & 0xff] ^ tt >> 24) & 0xff;
	    result[4 * i + 1] = (Si[t[(i + 3) % 4] >> 16 & 0xff] ^ tt >> 16) & 0xff;
	    result[4 * i + 2] = (Si[t[(i + 2) % 4] >> 8 & 0xff] ^ tt >> 8) & 0xff;
	    result[4 * i + 3] = (Si[t[(i + 1) % 4] & 0xff] ^ tt) & 0xff;
	  }

	  return result;
	};
	/**
	 *  Mode Of Operation - Electonic Codebook (ECB)
	 */


	var ModeOfOperationECB = function (key) {
	  if (!(this instanceof ModeOfOperationECB)) {
	    throw Error('AES must be instanitated with `new`');
	  }

	  this.description = "Electronic Code Block";
	  this.name = "ecb";
	  this._aes = new AesJs(key);
	};

	ModeOfOperationECB.prototype.encrypt = function (plaintext) {
	  plaintext = coerceArray(plaintext);

	  if (plaintext.length % 16 !== 0) {
	    throw new Error('invalid plaintext size (must be multiple of 16 bytes)');
	  }

	  var ciphertext = createArray(plaintext.length);
	  var block = createArray(16);

	  for (var i = 0; i < plaintext.length; i += 16) {
	    copyArray(plaintext, block, 0, i, i + 16);
	    block = this._aes.encrypt(block);
	    copyArray(block, ciphertext, i);
	  }

	  return ciphertext;
	};

	ModeOfOperationECB.prototype.decrypt = function (ciphertext) {
	  ciphertext = coerceArray(ciphertext);

	  if (ciphertext.length % 16 !== 0) {
	    throw new Error('invalid ciphertext size (must be multiple of 16 bytes)');
	  }

	  var plaintext = createArray(ciphertext.length);
	  var block = createArray(16);

	  for (var i = 0; i < ciphertext.length; i += 16) {
	    copyArray(ciphertext, block, 0, i, i + 16);
	    block = this._aes.decrypt(block);
	    copyArray(block, plaintext, i);
	  }

	  return plaintext;
	};
	/**
	 *  Mode Of Operation - Cipher Block Chaining (CBC)
	 */


	var ModeOfOperationCBC = function (key, iv) {
	  if (!(this instanceof ModeOfOperationCBC)) {
	    throw Error('AES must be instanitated with `new`');
	  }

	  this.description = "Cipher Block Chaining";
	  this.name = "cbc";

	  if (!iv) {
	    iv = createArray(16);
	  } else if (iv.length != 16) {
	    throw new Error('invalid initialation vector size (must be 16 bytes)');
	  }

	  this._lastCipherblock = coerceArray(iv, true);
	  this._aes = new AesJs(key);
	};

	ModeOfOperationCBC.prototype.encrypt = function (plaintext) {
	  plaintext = coerceArray(plaintext);

	  if (plaintext.length % 16 !== 0) {
	    throw new Error('invalid plaintext size (must be multiple of 16 bytes)');
	  }

	  var ciphertext = createArray(plaintext.length);
	  var block = createArray(16);

	  for (var i = 0; i < plaintext.length; i += 16) {
	    copyArray(plaintext, block, 0, i, i + 16);

	    for (var j = 0; j < 16; j++) {
	      block[j] ^= this._lastCipherblock[j];
	    }

	    this._lastCipherblock = this._aes.encrypt(block);
	    copyArray(this._lastCipherblock, ciphertext, i);
	  }

	  return ciphertext;
	};

	ModeOfOperationCBC.prototype.decrypt = function (ciphertext) {
	  ciphertext = coerceArray(ciphertext);

	  if (ciphertext.length % 16 !== 0) {
	    throw new Error('invalid ciphertext size (must be multiple of 16 bytes)');
	  }

	  var plaintext = createArray(ciphertext.length);
	  var block = createArray(16);

	  for (var i = 0; i < ciphertext.length; i += 16) {
	    copyArray(ciphertext, block, 0, i, i + 16);
	    block = this._aes.decrypt(block);

	    for (var j = 0; j < 16; j++) {
	      plaintext[i + j] = block[j] ^ this._lastCipherblock[j];
	    }

	    copyArray(ciphertext, this._lastCipherblock, 0, i, i + 16);
	  }

	  return plaintext;
	};
	/**
	 *  Mode Of Operation - Cipher Feedback (CFB)
	 */


	var ModeOfOperationCFB = function (key, iv, segmentSize) {
	  if (!(this instanceof ModeOfOperationCFB)) {
	    throw Error('AES must be instanitated with `new`');
	  }

	  this.description = "Cipher Feedback";
	  this.name = "cfb";

	  if (!iv) {
	    iv = createArray(16);
	  } else if (iv.length != 16) {
	    throw new Error('invalid initialation vector size (must be 16 size)');
	  }

	  if (!segmentSize) {
	    segmentSize = 1;
	  }

	  this.segmentSize = segmentSize;
	  this._shiftRegister = coerceArray(iv, true);
	  this._aes = new AesJs(key);
	};

	ModeOfOperationCFB.prototype.encrypt = function (plaintext) {
	  if (plaintext.length % this.segmentSize != 0) {
	    throw new Error('invalid plaintext size (must be segmentSize bytes)');
	  }

	  var encrypted = coerceArray(plaintext, true);
	  var xorSegment;

	  for (var i = 0; i < encrypted.length; i += this.segmentSize) {
	    xorSegment = this._aes.encrypt(this._shiftRegister);

	    for (var j = 0; j < this.segmentSize; j++) {
	      encrypted[i + j] ^= xorSegment[j];
	    } // Shift the register


	    copyArray(this._shiftRegister, this._shiftRegister, 0, this.segmentSize);
	    copyArray(encrypted, this._shiftRegister, 16 - this.segmentSize, i, i + this.segmentSize);
	  }

	  return encrypted;
	};

	ModeOfOperationCFB.prototype.decrypt = function (ciphertext) {
	  if (ciphertext.length % this.segmentSize != 0) {
	    throw new Error('invalid ciphertext size (must be segmentSize bytes)');
	  }

	  var plaintext = coerceArray(ciphertext, true);
	  var xorSegment;

	  for (var i = 0; i < plaintext.length; i += this.segmentSize) {
	    xorSegment = this._aes.encrypt(this._shiftRegister);

	    for (var j = 0; j < this.segmentSize; j++) {
	      plaintext[i + j] ^= xorSegment[j];
	    } // Shift the register


	    copyArray(this._shiftRegister, this._shiftRegister, 0, this.segmentSize);
	    copyArray(ciphertext, this._shiftRegister, 16 - this.segmentSize, i, i + this.segmentSize);
	  }

	  return plaintext;
	};
	/**
	 *  Mode Of Operation - Output Feedback (OFB)
	 */


	var ModeOfOperationOFB = function (key, iv) {
	  if (!(this instanceof ModeOfOperationOFB)) {
	    throw Error('AES must be instanitated with `new`');
	  }

	  this.description = "Output Feedback";
	  this.name = "ofb";

	  if (!iv) {
	    iv = createArray(16);
	  } else if (iv.length != 16) {
	    throw new Error('invalid initialation vector size (must be 16 bytes)');
	  }

	  this._lastPrecipher = coerceArray(iv, true);
	  this._lastPrecipherIndex = 16;
	  this._aes = new AesJs(key);
	};

	ModeOfOperationOFB.prototype.encrypt = function (plaintext) {
	  var encrypted = coerceArray(plaintext, true);

	  for (var i = 0; i < encrypted.length; i++) {
	    if (this._lastPrecipherIndex === 16) {
	      this._lastPrecipher = this._aes.encrypt(this._lastPrecipher);
	      this._lastPrecipherIndex = 0;
	    }

	    encrypted[i] ^= this._lastPrecipher[this._lastPrecipherIndex++];
	  }

	  return encrypted;
	}; // Decryption is symetric


	ModeOfOperationOFB.prototype.decrypt = ModeOfOperationOFB.prototype.encrypt;
	/**
	 *  Counter object for CTR common mode of operation
	 */

	var Counter = function (initialValue) {
	  if (!(this instanceof Counter)) {
	    throw Error('Counter must be instanitated with `new`');
	  } // We allow 0, but anything false-ish uses the default 1


	  if (initialValue !== 0 && !initialValue) {
	    initialValue = 1;
	  }

	  if (typeof initialValue === 'number') {
	    this._counter = createArray(16);
	    this.setValue(initialValue);
	  } else {
	    this.setBytes(initialValue);
	  }
	};

	Counter.prototype.setValue = function (value) {
	  if (typeof value !== 'number' || parseInt(value) != value) {
	    throw new Error('invalid counter value (must be an integer)');
	  } // We cannot safely handle numbers beyond the safe range for integers


	  if (value > Number.MAX_SAFE_INTEGER) {
	    throw new Error('integer value out of safe range');
	  }

	  for (var index = 15; index >= 0; --index) {
	    this._counter[index] = value % 256;
	    value = parseInt(value / 256);
	  }
	};

	Counter.prototype.setBytes = function (bytes) {
	  bytes = coerceArray(bytes, true);

	  if (bytes.length != 16) {
	    throw new Error('invalid counter bytes size (must be 16 bytes)');
	  }

	  this._counter = bytes;
	};

	Counter.prototype.increment = function () {
	  for (var i = 15; i >= 0; i--) {
	    if (this._counter[i] === 255) {
	      this._counter[i] = 0;
	    } else {
	      this._counter[i]++;
	      break;
	    }
	  }
	};
	/**
	 *  Mode Of Operation - Counter (CTR)
	 */


	var ModeOfOperationCTR = function (key, counter) {
	  if (!(this instanceof ModeOfOperationCTR)) {
	    throw Error('AES must be instanitated with `new`');
	  }

	  this.description = "Counter";
	  this.name = "ctr";

	  if (!(counter instanceof Counter)) {
	    counter = new Counter(counter);
	  }

	  this._counter = counter;
	  this._remainingCounter = null;
	  this._remainingCounterIndex = 16;
	  this._aes = new AesJs(key);
	};

	ModeOfOperationCTR.prototype.encrypt = function (plaintext) {
	  var encrypted = coerceArray(plaintext, true);

	  for (var i = 0; i < encrypted.length; i++) {
	    if (this._remainingCounterIndex === 16) {
	      this._remainingCounter = this._aes.encrypt(this._counter._counter);
	      this._remainingCounterIndex = 0;

	      this._counter.increment();
	    }

	    encrypted[i] ^= this._remainingCounter[this._remainingCounterIndex++];
	  }

	  return encrypted;
	}; // Decryption is symetric


	ModeOfOperationCTR.prototype.decrypt = ModeOfOperationCTR.prototype.encrypt; ///////////////////////
	// Padding
	// See:https://tools.ietf.org/html/rfc2315

	function pkcs7pad(data) {
	  data = coerceArray(data, true);
	  var padder = 16 - data.length % 16;
	  var result = createArray(data.length + padder);
	  copyArray(data, result);

	  for (var i = data.length; i < result.length; i++) {
	    result[i] = padder;
	  }

	  return result;
	}

	function pkcs7strip(data) {
	  data = coerceArray(data, true);

	  if (data.length < 16) {
	    throw new Error('PKCS#7 invalid length');
	  }

	  var padder = data[data.length - 1];

	  if (padder > 16) {
	    throw new Error('PKCS#7 padding byte out of range');
	  }

	  var length = data.length - padder;

	  for (var i = 0; i < padder; i++) {
	    if (data[length + i] !== padder) {
	      throw new Error('PKCS#7 invalid padding byte');
	    }
	  }

	  var result = createArray(length);
	  copyArray(data, result, 0, 0, length);
	  return result;
	} ///////////////////////
	// Exporting
	// The block cipher


	const aesjs = {
	  AES: AesJs,
	  Counter: Counter,
	  ModeOfOperation: {
	    ecb: ModeOfOperationECB,
	    cbc: ModeOfOperationCBC,
	    cfb: ModeOfOperationCFB,
	    ofb: ModeOfOperationOFB,
	    ctr: ModeOfOperationCTR
	  },
	  utils: {
	    hex: convertHex,
	    utf8: convertUtf8
	  },
	  padding: {
	    pkcs7: {
	      pad: pkcs7pad,
	      strip: pkcs7strip
	    }
	  },
	  _arrayTest: {
	    coerceArray: coerceArray,
	    createArray: createArray,
	    copyArray: copyArray
	  }
	};

	function getNaluLength(data) {
	  let length = data[3] | data[2] << 8 | data[1] << 16 | data[0] << 24;
	  return length;
	} // aes-256-ctr 解密


	function aes256ctrDecrypt(arrayBuffer, key, iv) {
	  key = new Uint8Array(key);
	  iv = new Uint8Array(iv);
	  const totalLength = arrayBuffer.byteLength; // 17(23)/27(39)[是否i帧] ,0/1(), 0,0,0, 0,0,0,0,[NALU],0,0,0,0,[NALU].... 只需要解密NALU部分数据。
	  // 其中NALU部分需要跳过两个（nalu头 + 再加一个字节数据）

	  let startIndex = 5;

	  while (startIndex < totalLength) {
	    let tempNaluLength = getNaluLength(arrayBuffer.slice(startIndex, startIndex + 4));

	    if (tempNaluLength > totalLength) {
	      break;
	    }

	    let naluType = arrayBuffer[startIndex + 4];
	    naluType = naluType & 0x1f;

	    if (naluType === 1 || naluType === 5) {
	      const tempNalu = arrayBuffer.slice(startIndex + 4 + 2, startIndex + 4 + tempNaluLength);
	      let aesCtr = new aesjs.ModeOfOperation.ctr(key, new aesjs.Counter(iv));
	      const decryptMsg = aesCtr.decrypt(tempNalu); // release memory

	      aesCtr = null;
	      arrayBuffer.set(decryptMsg, startIndex + 4 + 2);
	    }

	    startIndex = startIndex + 4 + tempNaluLength;
	  }

	  return arrayBuffer;
	}

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
	  let isVideoFirstIFrame = false; // let lastDecodeVideoFrameTimestamp = 0;
	  // let lastDecodeVideoFrameLocalTimestamp = 0;
	  // let lastDecodeAudioFrameTimestamp = 0;
	  // let newDecodedVideoFrameTimestamp = 0;
	  // let newDecodedVideoFrameLocalTimestamp = 0;
	  // let newDecodedAudioFrameTimestamp = 0;

	  let isStreamTimeDiffMoreThanLocalTimeDiff = false;
	  let preLoopTimestamp = null;
	  let bufferStartDts = null;
	  let bufferStartLocalTs = null;
	  let audioOutputArray = [];
	  let audioRemain = 0;
	  let audioChannels = 0;
	  let preIframeTs = null;
	  let preTimestamp = null;
	  let preTimestampDuration = 0;
	  let prevPayloadBufferSize = 0;
	  let isWebglContextLost = false;
	  let isWidthOrHeightChanged = false;

	  let getDefaultOpt = () => {
	    const defaultOptions = getDefaultPlayerOptions();
	    return {
	      debug: defaultOptions.debug,
	      debugLevel: defaultOptions.debugLevel,
	      debugUuid: defaultOptions.debugUuid,
	      useOffscreen: defaultOptions.useOffscreen,
	      useWCS: defaultOptions.useWCS,
	      videoBuffer: defaultOptions.videoBuffer,
	      videoBufferDelay: defaultOptions.videoBufferDelay,
	      openWebglAlignment: defaultOptions.openWebglAlignment,
	      playType: defaultOptions.playType,
	      hasAudio: defaultOptions.hasAudio,
	      hasVideo: defaultOptions.hasVideo,
	      playbackRate: 1,
	      playbackForwardMaxRateDecodeIFrame: defaultOptions.playbackForwardMaxRateDecodeIFrame,
	      playbackIsCacheBeforeDecodeForFpsRender: defaultOptions.playbackConfig.isCacheBeforeDecodeForFpsRender,
	      sampleRate: 0,
	      networkDelay: defaultOptions.networkDelay,
	      visibility: true,
	      useSIMD: defaultOptions.useSIMD,
	      isRecording: false,
	      recordType: defaultOptions.recordType,
	      isNakedFlow: defaultOptions.isNakedFlow,
	      checkFirstIFrame: defaultOptions.checkFirstIFrame,
	      audioBufferSize: 1024,
	      isCrypto: defaultOptions.isCrypto,
	      cryptoKey: defaultOptions.cryptoKey,
	      cryptoIV: defaultOptions.cryptoIV
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

	    getNaluAudioDts() {
	      const audioContextSampleRate = decoder._opt.sampleRate;
	      const audioBufferSize = decoder._opt.audioBufferSize;
	      return nakedFlowDemuxer.localDts + parseInt(audioBufferSize / audioContextSampleRate * 1000);
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
	      if (nalu.byteLength <= 4) {
	        decoder.debug.warn('worker', `handleNALu nalu byteLength is ${nalu.byteLength} <= 4`);
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
	        // const nalType = getAvcSeqHeadType(nalu);
	        // if (nalType === H264_NAL_TYPE.pps) {
	        //     nakedFlowDemuxer.extractH264PPS(nalu);
	        // } else {
	        //     nakedFlowDemuxer.handleVideoH264Nalu(nalu);
	        // }
	        const tempNalu = nakedFlowDemuxer.handleAddNaluStartCode(nalu);
	        const naluList = nakedFlowDemuxer.extractNALu(tempNalu);

	        if (naluList.length === 0) {
	          decoder.debug.warn('worker', 'handleVideoNalu', 'naluList.length === 0');
	          return;
	        }

	        const newNaluList = [];
	        naluList.forEach(naluItem => {
	          const nalType = getAvcSeqHeadType(naluItem);

	          if (nalType === H264_NAL_TYPE.pps || nalType === H264_NAL_TYPE.sps) {
	            nakedFlowDemuxer.handleVideoH264Nalu(naluItem);
	          } else {
	            if (isNotAvcSeqHead(nalType)) {
	              newNaluList.push(naluItem);
	            }
	          }
	        });

	        if (newNaluList.length === 1) {
	          nakedFlowDemuxer.handleVideoH264Nalu(newNaluList[0]);
	        } else {
	          const isSameNaluType = isSameAvcNaluType(newNaluList);

	          if (isSameNaluType) {
	            const naluType = getAvcSeqHeadType(newNaluList[0]);
	            const isIFrame = isAvcNaluIFrame(naluType);
	            nakedFlowDemuxer.handleVideoH264NaluList(newNaluList, isIFrame, naluType);
	          } else {
	            newNaluList.forEach(naluItem => {
	              nakedFlowDemuxer.handleVideoH264Nalu(naluItem);
	            });
	          }
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
	      const nalType = getAvcSeqHeadType(nalu); // decoder.debug.log('worker', `handleVideoH264Nalu nalType is ${nalu[0]} , nalu[0] is ${nalType}`);

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

	          if (isIFrame) {
	            decoder.calcIframeIntervalTimestamp(dts);
	          }

	          decoder.calcNetworkDelay(dts);
	          const packet = avcEncoderNalePacket(nalu, isIFrame);
	          decoder.decode(packet, {
	            type: MEDIA_TYPE.video,
	            ts: dts,
	            isIFrame: isIFrame,
	            cts: 0
	          }); // decoder.decodeVideo(packet, dts, isIFrame, 0);
	        } else {
	          decoder.debug.warn('work', `handleVideoH264Nalu Avc Seq Head is ${nalType}`);
	        } // console.log('handleVideoH264Nalu packet', packet);

	      }
	    },

	    handleVideoH264NaluList(naluList, isIFrame, naluType) {
	      if (nakedFlowDemuxer.isSendSeqHeader) {
	        const dts = nakedFlowDemuxer.getNaluDts();
	        decoder.calcNetworkDelay(dts);
	        const newNalu = naluList.reduce((pre, cur) => {
	          const nalu2 = addNaleHeaderLength(pre);
	          const nalu3 = addNaleHeaderLength(cur);
	          const nalu4 = new Uint8Array(nalu2.byteLength + nalu3.byteLength);
	          nalu4.set(nalu2, 0);
	          nalu4.set(nalu3, nalu2.byteLength);
	          return nalu4;
	        });
	        const packet = avcEncoderNalePacketNotLength(newNalu, isIFrame);
	        decoder.decode(packet, {
	          type: MEDIA_TYPE.video,
	          ts: dts,
	          isIFrame: isIFrame,
	          cts: 0
	        });
	        decoder.debug.log('worker', `handleVideoH264NaluList list size is ${naluList.length} package length is ${packet.byteLength} isIFrame is ${isIFrame},nalu type is ${naluType}, dts is ${dts}`);
	      } else {
	        decoder.debug.warn('worker', 'handleVideoH264NaluList isSendSeqHeader is false');
	      }
	    },

	    handleVideoH265Nalu(nalu) {
	      const nalType = getHevcSeqHeadType(nalu); // decoder.debug.log('worker', `handleVideoH265Nalu nalType is ${nalType} , isHevcNalIFrame ${isHevcNalIFrame(nalType)} ,nalu[0] is ${nalu[0]} ,nalu.bytelength is ${nalu.byteLength}`);

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
	          decoder.calcNetworkDelay(dts);
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
	        } else {
	          decoder.debug.warn('work', `handleVideoH265Nalu HevcSeqHead is ${nalType}`);
	        }
	      }
	    }

	  };
	  let decoder = {
	    isPlayer: true,
	    isPlayback: false,
	    isPushDropping: false,
	    isDestroyed: false,
	    _opt: getDefaultOpt(),
	    startStreamRateAndStatsInterval: function () {
	      // decoder.debug.log('worker', 'startStreamRateAndStatsInterval');
	      decoder.stopStreamRateAndStatsInterval();
	      streamRateAndStatsInterval = setInterval(() => {
	        streamRate && streamRate(0);
	        const streamStats = JSON.stringify({
	          demuxBufferDelay: decoder.getVideoBufferLength(),
	          audioDemuxBufferDelay: decoder.getAudioBufferLength(),
	          flvBufferByteLength: decoder.getFlvBufferLength(),
	          netBuf: decoder.networkDelay || 0,
	          pushLatestDelay: decoder.pushLatestDelay || 0,
	          isDropping: isTrue(decoder.dropping) || isTrue(decoder.isPushDropping),
	          isStreamTsMoreThanLocal: isStreamTimeDiffMoreThanLocalTimeDiff
	        });
	        postMessage({
	          cmd: WORKER_CMD_TYPE.workerFetch,
	          type: EVENTS.streamStats,
	          value: streamStats
	        });
	      }, 1000);
	    },
	    stopStreamRateAndStatsInterval: function () {
	      // decoder.debug.log('worker', `stopStreamRateAndStatsInterval`);
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

	      if (decoder.preDelayTimestamp && decoder.preDelayTimestamp > timestamp) {
	        if (decoder.preDelayTimestamp - timestamp > 1000) {
	          decoder.debug.warn('worker', `getDelay() and preDelayTimestamp is ${decoder.preDelayTimestamp} > timestamp is ${timestamp} more than ${decoder.preDelayTimestamp - timestamp}ms`);
	        }

	        decoder.preDelayTimestamp = timestamp;
	        return decoder.delay;
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
	            isStreamTimeDiffMoreThanLocalTimeDiff = false;
	            decoder.delay = localTimestamp - timeTimestamp;
	          } else {
	            // if stream ts more than local ts, support stream rate is more than 1 rate
	            isStreamTimeDiffMoreThanLocalTimeDiff = true;
	            decoder.delay = timeTimestamp - localTimestamp;
	          }
	        }
	      }

	      decoder.preDelayTimestamp = timestamp;
	      return decoder.delay;
	    },
	    getDelayNotUpdateDelay: function (timestamp) {
	      if (!timestamp || decoder._opt.hasVideo && !isVideoFirstIFrame) {
	        return -1;
	      }

	      if (decoder.preDelayTimestamp && decoder.preDelayTimestamp - timestamp > 1000) {
	        decoder.debug.warn('worker', `getDelayNotUpdateDelay and preDelayTimestamp is ${decoder.preDelayTimestamp} > timestamp is ${timestamp} more than ${decoder.preDelayTimestamp - timestamp}ms`);
	        return -1;
	      }

	      if (!decoder.firstTimestamp) {
	        return -1;
	      } else {
	        let delay = -1;

	        if (timestamp) {
	          const localTimestamp = Date.now() - decoder.startTimestamp;
	          const timeTimestamp = timestamp - decoder.firstTimestamp;

	          if (localTimestamp >= timeTimestamp) {
	            isStreamTimeDiffMoreThanLocalTimeDiff = false;
	            delay = localTimestamp - timeTimestamp;
	          } else {
	            isStreamTimeDiffMoreThanLocalTimeDiff = true;
	            delay = timeTimestamp - localTimestamp;
	          }
	        }

	        return delay;
	      }
	    },
	    resetDelay: function () {
	      decoder.firstTimestamp = null;
	      decoder.startTimestamp = null;
	      decoder.delay = -1;
	    },
	    resetAllDelay: function () {
	      decoder.resetDelay();
	      decoder.preDelayTimestamp = null;
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
	      const isPlayback = decoder._opt.playType === PLAY_TYPE.playbackTF;
	      nakedFlowDemuxer.init();
	      decoder.isPlayer = isPlayer;
	      decoder.isPlayback = isPlayback; // player || (playback && not cache before decode for fps render)

	      if (!decoder.isPlaybackCacheBeforeDecodeForFpsRender()) {
	        // default loop
	        const loop = () => {
	          let data = null;

	          if (bufferList.length) {
	            if (decoder.isPushDropping) {
	              decoder.debug.warn('worker', `loop() isPushDropping is true and bufferList length is ${this.bufferList.length}`);
	              return;
	            }

	            if (decoder.dropping) {
	              // // dropping
	              data = bufferList.shift();
	              decoder.debug.warn('worker', `loop() dropBuffer is dropping and isIFrame ${data.isIFrame} and delay is ${decoder.delay} and bufferlist is ${bufferList.length}`); //

	              while (!data.isIFrame && bufferList.length) {
	                // decoder.debug.log('worker', 'loop is dropping = true, isIFrame is', data.isIFrame);
	                // dropping
	                data = bufferList.shift();
	              }

	              const tempDelay = decoder.getDelayNotUpdateDelay(data.ts);

	              if (data.isIFrame && tempDelay <= decoder.getNotDroppingDelayTs()) {
	                decoder.debug.log('worker', 'loop() is dropping = false, is iFrame');
	                decoder.dropping = false;
	                decoder.doDecode(data);
	              }
	            } else {
	              data = bufferList[0];

	              if (decoder.getDelay(data.ts) === -1 || decoder.isPlayback) {
	                if (decoder.isPlayer) {
	                  decoder.debug.log('worker', 'loop() common dumex delay is -1 ,data.ts is', data.ts);
	                }

	                bufferList.shift();
	                decoder.doDecode(data);
	              } else if (decoder.delay > decoder._opt.videoBuffer + decoder._opt.videoBufferDelay && isPlayer) {
	                if (decoder.hasIframeInBufferList()) {
	                  decoder.debug.log('worker', `delay is ${decoder.delay}, set dropping is true`);
	                  decoder.resetAllDelay();
	                  decoder.dropping = true;
	                } else {
	                  bufferList.shift();
	                  decoder.doDecode(data);
	                }
	              } else {
	                // 持续解码
	                while (bufferList.length) {
	                  data = bufferList[0];

	                  if (decoder.getDelay(data.ts) > decoder._opt.videoBuffer) {
	                    bufferList.shift();
	                    decoder.doDecode(data);
	                  } else {
	                    if (decoder.delay < 0) {
	                      decoder.debug.warn('worker', `loop() do not decode and delay is ${decoder.delay}, bufferList is ${bufferList.length}`);
	                    }

	                    break;
	                  }
	                }
	              }
	            }
	          } else {
	            if (decoder.delay !== -1) {
	              decoder.debug.log('worker', 'loop() bufferList is empty and reset delay');
	            }

	            decoder.resetAllDelay();
	          }
	        };

	        decoder.stopId = setInterval(() => {
	          let nowTime = new Date().getTime();

	          if (!preLoopTimestamp) {
	            preLoopTimestamp = nowTime;
	          }

	          const diffTime = nowTime - preLoopTimestamp;

	          if (diffTime > 100) {
	            decoder.debug.warn('worker', `loop demux diff time is ${diffTime}`);
	          }

	          loop();
	          preLoopTimestamp = new Date().getTime();
	        }, 10);
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
	      decoder.isDestroyed = true;
	      decoder.stopStreamRateAndStatsInterval();

	      if (decoder.stopId) {
	        clearInterval(decoder.stopId);
	        decoder.stopId = null;
	      }

	      if (audioDecoder) {
	        audioDecoder.clear && audioDecoder.clear();
	        audioDecoder = null;
	      }

	      if (videoDecoder) {
	        videoDecoder.clear && videoDecoder.clear();
	        videoDecoder = null;
	      }

	      streamRate = null;
	      preLoopTimestamp = null;
	      isStreamTimeDiffMoreThanLocalTimeDiff = false; // lastDecodeVideoFrameTimestamp = 0;
	      // lastDecodeVideoFrameLocalTimestamp = 0;
	      // lastDecodeAudioFrameTimestamp = 0;
	      // newDecodedVideoFrameTimestamp = 0;
	      // newDecodedVideoFrameLocalTimestamp = 0;
	      // newDecodedAudioFrameTimestamp = 0;

	      if (wcsVideoDecoder) {
	        if (wcsVideoDecoder.reset) {
	          wcsVideoDecoder.reset();
	        }

	        wcsVideoDecoder = null;
	      }

	      decoder.firstTimestamp = null;
	      decoder.startTimestamp = null;
	      decoder.networkDelay = 0;
	      decoder.streamFps = null; // audio + video all fps

	      decoder.streamAudioFps = null; //

	      decoder.streamVideoFps = null;
	      decoder.delay = -1;
	      decoder.pushLatestDelay = -1;
	      decoder.preDelayTimestamp = null;
	      decoder.dropping = false;
	      decoder.isPushDropping = false;
	      decoder.isPlayer = true;
	      decoder.isPlayback = false;
	      decoder._opt = getDefaultOpt();

	      if (decoder.webglObj) {
	        decoder.webglObj.destroy();
	        decoder.offscreenCanvas.removeEventListener('webglcontextlost', decoder.onOffscreenCanvasWebglContextLost);
	        decoder.offscreenCanvas.removeEventListener('webglcontextrestored', decoder.onOffscreenCanvasWebglContextRestored);
	        decoder.offscreenCanvas = null;
	        decoder.offscreenCanvasGL = null;
	        decoder.offscreenCanvasCtx = null;
	      }

	      bufferList = [];
	      tempAudioBuffer = [];
	      abort();
	      input = null;

	      if (socket) {
	        socket.close(1000, 'Client disconnecting');
	        socket = null;
	      }

	      videoWidth = null;
	      videoHeight = null;
	      hasInitVideoCodec = false;
	      hasInitAudioCodec = false;
	      isVideoFirstIFrame = false;
	      isWebglContextLost = false;
	      isWidthOrHeightChanged = false;
	      audioOutputArray = [];
	      audioRemain = 0;
	      audioChannels = 0;
	      bufferStartDts = null;
	      bufferStartLocalTs = null;
	      preIframeTs = null;
	      preTimestamp = null;
	      preTimestampDuration = 0;
	      prevPayloadBufferSize = 0;
	      nakedFlowDemuxer.destroy();
	      postMessage({
	        cmd: WORKER_CMD_TYPE.closeEnd
	      });
	    },
	    pushBuffer: function (bufferData, options) {
	      if (options.type === MEDIA_TYPE.audio && isAacCodecPacket(bufferData)) {
	        decoder.decodeAudio(bufferData, options.ts);
	        return;
	      }

	      if (decoder.isPlayer && preTimestampDuration > 0 && preTimestamp > 0 && options.type === MEDIA_TYPE.video) {
	        const diff = options.ts - preTimestamp;

	        if (diff > preTimestampDuration * 2 - 5) {
	          decoder.debug.warn('worker', `pushBuffer video
                    ts is ${options.ts}, preTimestamp is ${preTimestamp},
                    diff is ${diff} and preTimestampDuration is ${preTimestampDuration}
                    maybe trigger black screen or flower screen
                    `);
	        }
	      }

	      if (decoder.isPlayer && preTimestamp > 0 && options.type === MEDIA_TYPE.video && options.ts < preTimestamp && preTimestamp - options.ts > FRAME_TS_MAX_DIFF) {
	        decoder.debug.warn('worker', `pushBuffer,
                preTimestamp is ${preTimestamp}, options.ts is ${options.ts},
                diff is ${preTimestamp - options.ts} more than ${FRAME_TS_MAX_DIFF},
                and resetAllDelay`);
	        decoder.resetAllDelay();
	      }

	      if (decoder.isPlayer && options.ts <= preTimestamp && preTimestamp > 0 && options.type === MEDIA_TYPE.video) {
	        decoder.debug.warn('worker', `pushBuffer,
                options.ts is ${options.ts} less than (or equal) preTimestamp is ${preTimestamp} and
                payloadBufferSize is ${bufferData.byteLength} and prevPayloadBufferSize is ${prevPayloadBufferSize}`);
	      } // decoder.debug.error('worker', `pushBuffer and type is ${options.type}, isIframe is ${options.isIFrame}, ts is ${options.ts}`);


	      if (decoder.isPlayer && isVideoFirstIFrame) {
	        const maxDelay = decoder._opt.videoBuffer + decoder._opt.videoBufferDelay;
	        const pushLatestDelay = decoder.getDelayNotUpdateDelay(options.ts); // update push latest delay

	        decoder.pushLatestDelay = pushLatestDelay;

	        if (pushLatestDelay > maxDelay && decoder.delay < maxDelay && decoder.delay > 0) {
	          if (decoder.hasIframeInBufferList() && decoder.isPushDropping === false) {
	            decoder.debug.log('worker', `pushBuffer, pushLatestDelay is ${pushLatestDelay} more than ${maxDelay} and decoder.delay is ${decoder.delay} and has iIframe and next decoder.dropBuffer$2()`);
	            decoder.dropBuffer$2();
	          }
	        }
	      }

	      if (options.type === MEDIA_TYPE.video) {
	        if (preTimestamp > 0) {
	          preTimestampDuration = options.ts - preTimestamp;
	        }

	        prevPayloadBufferSize = bufferData.byteLength;
	        preTimestamp = options.ts;
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

	              decoder.playbackCacheLoop();
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

	              decoder.playbackCacheLoop();
	            }
	          }

	          if (isEmpty(decoder.streamVideoFps) && isEmpty(decoder.streamAudioFps)) {
	            decoder.debug.log('worker', `playbackCacheBeforeDecodeForFpsRender, calc streamAudioFps is ${streamAudioFps}, streamVideoFps is ${streamVideoFps}, bufferListLength  is ${bufferList.length}`);
	          }
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

	    hasIframeInBufferList() {
	      return bufferList.some(item => {
	        return item.type === MEDIA_TYPE.video && item.isIFrame;
	      });
	    },

	    getNotDroppingDelayTs() {
	      return decoder._opt.videoBuffer + decoder._opt.videoBufferDelay / 2;
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

	    getFlvBufferLength() {
	      let result = 0;

	      if (input && input.buffer) {
	        result = input.buffer.byteLength;
	      }

	      if (decoder._opt.isNakedFlow) {
	        if (nakedFlowDemuxer.lastBuf) {
	          result = nakedFlowDemuxer.lastBuf.byteLength;
	        }
	      }

	      return result;
	    },

	    fetchStream: function (url, options) {
	      decoder.debug.log('worker', 'fetchStream, url is ' + url, 'options:', JSON.stringify(options));
	      streamRate = calculationRate(rate => {
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
	          if (!isFetchSuccess(res)) {
	            decoder.debug.warn('worker', `fetch response status is ${res.status} and ok is ${res.ok} and emit error and next abort()`);
	            abort();
	            postMessage({
	              cmd: WORKER_CMD_TYPE.workerFetch,
	              type: EVENTS_ERROR.fetchError,
	              value: `fetch response status is ${res.status} and ok is ${res.ok}`
	            });
	            return;
	          }

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
	                abort();
	                postMessage({
	                  cmd: WORKER_CMD_TYPE.workerFetch,
	                  type: EVENTS.streamEnd,
	                  value: PLAYER_STREAM_TYPE.fetch
	                });
	              },
	              abort: e => {
	                input = null;

	                if (e.name === FETCH_ERROR.abort) {
	                  return;
	                }

	                abort();
	                postMessage({
	                  cmd: WORKER_CMD_TYPE.workerFetch,
	                  type: EVENTS_ERROR.fetchError,
	                  value: e.toString()
	                });
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
	                  abort();
	                  postMessage({
	                    cmd: WORKER_CMD_TYPE.workerFetch,
	                    type: EVENTS.streamEnd,
	                    value: PLAYER_STREAM_TYPE.fetch
	                  });
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

	                abort();
	                postMessage({
	                  cmd: WORKER_CMD_TYPE.workerFetch,
	                  type: EVENTS_ERROR.fetchError,
	                  value: e.toString()
	                });
	              });
	            };

	            fetchNext();
	          }
	        }).catch(e => {
	          if (e.name === FETCH_ERROR.abort) {
	            return;
	          }

	          abort();
	          postMessage({
	            cmd: WORKER_CMD_TYPE.workerFetch,
	            type: EVENTS_ERROR.fetchError,
	            value: e.toString()
	          });
	          input = null;
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
	          postMessage({
	            cmd: WORKER_CMD_TYPE.workerFetch,
	            type: EVENTS.websocketOpen
	          });
	        };

	        socket.onclose = event => {
	          decoder.debug.log('worker', `fetchStream, WebsocketStream socket close and code is ${event.code}`); // closed_abnormally

	          if (event.code === 1006) {
	            decoder.debug.warn('worker', `fetchStream, WebsocketStream socket close abnormally and code is ${event.code}`);
	          }

	          input = null;
	          postMessage({
	            cmd: WORKER_CMD_TYPE.workerFetch,
	            type: EVENTS.streamEnd,
	            value: PLAYER_STREAM_TYPE.websocket
	          });
	        };

	        socket.onerror = error => {
	          decoder.debug.error('worker', 'fetchStream, WebsocketStream socket error', error);
	          input = null;
	          postMessage({
	            cmd: WORKER_CMD_TYPE.workerFetch,
	            type: EVENTS_ERROR.websocketError,
	            value: error.isTrusted ? 'websocket user aborted' : 'websocket error'
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
	        tmp8[3] = t[11];
	        let ts = tmp32[0];
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
	              const isIFrame = payload[0] >> 4 === 1; // just for player

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
	              let cts = tmp32[0];
	              let payloadBuffer = payload;

	              if (decoder._opt.isCrypto) {
	                if (decoder._opt.cryptoIV && decoder._opt.cryptoIV.byteLength > 0 && decoder._opt.cryptoKey && decoder._opt.cryptoKey.byteLength > 0) {
	                  payloadBuffer = aes256ctrDecrypt(payload, decoder._opt.cryptoKey, decoder._opt.cryptoIV);
	                } else {
	                  decoder.debug.error('worker', `cryptoKey.length is ${decoder._opt.cryptoKey && decoder._opt.cryptoKey.byteLength} or cryptoIV.length is ${decoder._opt.cryptoIV && decoder._opt.cryptoIV.byteLength} null`);
	                }
	              }

	              decoder.decode(payloadBuffer, {
	                type: MEDIA_TYPE.video,
	                ts,
	                isIFrame,
	                cts
	              });
	            }

	            break;

	          default:
	            decoder.debug.log('worker', `demuxFlv() type is ${type}`);
	            break;
	        }
	      }
	    },
	    decode: function (payload, options) {
	      if (options.type === MEDIA_TYPE.audio) {
	        if (decoder._opt.hasAudio) {
	          postMessage({
	            cmd: WORKER_CMD_TYPE.workerFetch,
	            type: EVENTS.streamAbps,
	            value: payload.byteLength
	          });

	          if (decoder.isPlayer) {
	            decoder.pushBuffer(payload, {
	              type: options.type,
	              ts: options.ts,
	              cts: options.cts
	            });
	          } else if (decoder.isPlayback) {
	            if (decoder.isPlaybackCacheBeforeDecodeForFpsRender()) {
	              decoder.pushBuffer(payload, {
	                type: options.type,
	                ts: options.ts,
	                cts: options.cts
	              });
	            } else {
	              // for 1 rate
	              if (decoder._opt.playbackRate === 1) {
	                decoder.pushBuffer(payload, {
	                  type: options.type,
	                  ts: options.ts,
	                  cts: options.cts
	                });
	              } else {
	                // for more than 1 rate
	                decoder.decodeAudio(payload, options.ts);
	              }
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

	          if (decoder.isPlayer) {
	            // decoder.debug.log('worker','fetchStream, decode video isIFrame is', options.isIFrame);
	            decoder.pushBuffer(payload, {
	              type: options.type,
	              ts: options.ts,
	              isIFrame: options.isIFrame,
	              cts: options.cts
	            });
	          } else if (decoder.isPlayback) {
	            if (decoder.isPlaybackOnlyDecodeIFrame()) {
	              if (options.isIFrame) {
	                decoder.pushBuffer(payload, {
	                  type: options.type,
	                  ts: options.ts,
	                  cts: options.cts,
	                  isIFrame: options.isIFrame
	                });
	              }
	            } else {
	              if (decoder.isPlaybackCacheBeforeDecodeForFpsRender()) {
	                decoder.pushBuffer(payload, {
	                  type: options.type,
	                  ts: options.ts,
	                  cts: options.cts,
	                  isIFrame: options.isIFrame
	                });
	              } else {
	                decoder.pushBuffer(payload, {
	                  type: options.type,
	                  ts: options.ts,
	                  cts: options.cts,
	                  isIFrame: options.isIFrame
	                });
	              }
	            }
	          }
	        }
	      }
	    },
	    setCodecAudio: function (payload) {
	      const codecId = payload[0] >> 4;

	      if (isAacCodecPacket(payload) || codecId === AUDIO_ENC_CODE.ALAW || codecId === AUDIO_ENC_CODE.MULAW) {
	        decoder.debug.log('worker', `setCodecAudio: init audio codec, codeId is ${codecId}`);
	        const extraData = codecId === AUDIO_ENC_CODE.AAC ? payload.slice(2) : payload.slice(1);
	        audioDecoder.setCodec(codecId, decoder._opt.sampleRate, extraData);
	        hasInitAudioCodec = true;

	        {
	          const timeout = Number(experienceTimeout) || 1;
	          setTimeout(() => {
	            decoder.close();
	            postMessage({
	              cmd: WORKER_CMD_TYPE.workerEnd
	            });
	          }, timeout * 60 * 60 * 1000);
	        }
	      } else {
	        decoder.debug.warn('worker', 'setCodecAudio: hasInitAudioCodec is false, codecId is ', codecId);
	      }
	    },
	    decodeAudio: function (payload, ts) {
	      const codecId = payload[0] >> 4;

	      if (!hasInitAudioCodec) {
	        decoder.setCodecAudio(payload);
	      } else {
	        // lastDecodeAudioFrameTimestamp = ts;
	        audioDecoder.decode(codecId === AUDIO_ENC_CODE.AAC ? payload.slice(2) : payload.slice(1), ts);
	      }
	    },
	    setCodecVideo: function (payload) {
	      const codecId = payload[0] & 0x0F;

	      if (decoder.isVideoSequenceHeader(payload)) {
	        if (codecId === VIDEO_ENC_CODE.h264 || codecId === VIDEO_ENC_CODE.h265) {
	          decoder.debug.log('worker', `setCodecVideo: init video codec , codecId is ${codecId}`);
	          const extraData = payload.slice(5);

	          if (codecId === VIDEO_ENC_CODE.h264 && decoder._opt.useSIMD) {
	            // need check width and height is more than 4080。
	            const avcConfig = parseAVCDecoderConfigurationRecord(extraData);

	            if (avcConfig.codecWidth > SIMD_H264_DECODE_MAX_WIDTH || avcConfig.codecHeight > SIMD_H264_DECODE_MAX_WIDTH) {
	              postMessage({
	                cmd: WORKER_CMD_TYPE.simdH264DecodeVideoWidthIsTooLarge
	              });
	              decoder.debug.warn('worker', `setCodecVideo: SIMD H264 decode video width is too large, width is ${avcConfig.codecWidth}, height is ${avcConfig.codecHeight}`);
	              return;
	            }
	          }

	          hasInitVideoCodec = true;
	          videoDecoder.setCodec(codecId, extraData);

	          if (decoder._opt.recordType === FILE_SUFFIX.mp4) {
	            postMessage({
	              cmd: WORKER_CMD_TYPE.videoCodec,
	              buffer: payload,
	              codecId
	            }, [payload.buffer]);
	          }
	        } else {
	          decoder.debug.warn('worker', `setCodecVideo: hasInitVideoCodec is false, codecId is ${codecId} is not H264 or H265`);
	        }
	      } else {
	        decoder.debug.warn('worker', `decodeVideo: hasInitVideoCodec is false, codecId is ${codecId} and frameType is ${payload[0] >> 4} and packetType is ${payload[1]}`);
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
	          // check video width and height is changed
	          if (isIFrame && decoder.isVideoSequenceHeader(payload)) {
	            const codecId = payload[0] & 0x0F;
	            let config = {};

	            if (codecId === VIDEO_ENC_CODE.h264) {
	              let data = payload.slice(5);
	              config = parseAVCDecoderConfigurationRecord(data);
	            } else if (codecId === VIDEO_ENC_CODE.h265) {
	              config = parseHEVCDecoderConfigurationRecord$2(payload);
	            }

	            if (config.codecWidth && config.codecHeight && videoWidth && videoHeight && (config.codecWidth !== videoWidth || config.codecHeight !== videoHeight)) {
	              decoder.debug.warn('worker', `
                            decodeVideo: video width or height is changed,
                            old width is ${videoWidth}, old height is ${videoHeight},
                            new width is ${config.codecWidth}, new height is ${config.codecHeight},
                            and emit change event`);
	              isWidthOrHeightChanged = true;
	              postMessage({
	                cmd: WORKER_CMD_TYPE.wasmWidthOrHeightChange
	              });
	            }
	          }

	          if (isWidthOrHeightChanged) {
	            decoder.debug.warn('worker', `decodeVideo: video width or height is changed, and return`);
	            return;
	          }

	          if (decoder.isVideoSequenceHeader(payload)) {
	            decoder.debug.warn('worker', 'decodeVideo and payload is video sequence header so drop this frame');
	            return;
	          }

	          if (payload.byteLength < VIDEO_PAYLOAD_MIN_SIZE) {
	            decoder.debug.warn('worker', `decodeVideo and payload is too small , payload length is ${payload.byteLength}`);
	            return;
	          }

	          const buffer = payload.slice(5); // lastDecodeVideoFrameTimestamp = ts;
	          // lastDecodeVideoFrameLocalTimestamp = now();

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

	      decoder.resetAllDelay();
	      decoder.dropping = true;
	    },
	    dropBuffer$2: function () {
	      if (bufferList.length > 0) {
	        decoder.isPushDropping = true;
	        const iFrameIndex = bufferList.findIndex(bufferItem => {
	          return isTrue(bufferItem.isIFrame) && bufferItem.type === MEDIA_TYPE.video;
	        });

	        if (iFrameIndex >= 0) {
	          //
	          bufferList = bufferList.slice(iFrameIndex);
	          const iFrameItem = bufferList.shift();
	          const tempDelay = decoder.getDelayNotUpdateDelay(iFrameItem.ts);
	          decoder.doDecode(iFrameItem);
	          decoder.isPushDropping = false;
	          decoder.debug.log('worker', `dropBuffer$2() iFrameIndex is ${iFrameIndex},and bufferList length is ${bufferList.length} and tempDelay is ${tempDelay} ,delay is ${decoder.delay} `);
	        } else {
	          decoder.isPushDropping = false;
	        }
	      }

	      if (bufferList.length === 0) {
	        decoder.isPushDropping = false;
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
	      // decoder.debug.log('worker', `demuxNakedFlow, arrayBuffer length is ${arrayBuffer.byteLength}`);
	      // console.log('demuxNakedFlow', arrayBuffer);
	      nakedFlowDemuxer.dispatch(arrayBuffer);
	    },
	    calcNetworkDelay: function (dts) {
	      if (!(isVideoFirstIFrame && dts > 0)) {
	        return;
	      }

	      if (bufferStartDts === null) {
	        bufferStartDts = dts;
	        bufferStartLocalTs = now();
	      } else {
	        if (dts < bufferStartDts) {
	          decoder.debug.warn('worker', `calcNetworkDelay, dts is ${dts} less than bufferStartDts is ${bufferStartDts}`);
	          bufferStartDts = dts;
	          bufferStartLocalTs = now();
	        }
	      }

	      const diff1 = dts - bufferStartDts;
	      const localDiff = now() - bufferStartLocalTs;
	      const delay = localDiff > diff1 ? localDiff - diff1 : 0;
	      decoder.networkDelay = delay;

	      if (delay > decoder._opt.networkDelay && decoder._opt.playType === PLAY_TYPE.player) {
	        decoder.debug.warn('worker', `calcNetworkDelay now dts:${dts}, start dts is ${bufferStartDts} vs start is ${diff1},local diff is ${localDiff} ,delay is ${delay}`);
	        postMessage({
	          cmd: WORKER_CMD_TYPE.workerFetch,
	          type: EVENTS.networkDelayTimeout,
	          value: delay
	        });
	      }
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
	    canVisibilityDecodeNotDrop: function () {
	      return decoder._opt.visibility && videoWidth * videoHeight <= 1920 * 1080;
	    },
	    isPlaybackCacheBeforeDecodeForFpsRender: function () {
	      return !decoder.isPlayer && decoder._opt.playbackIsCacheBeforeDecodeForFpsRender;
	    },
	    isPlaybackOnlyDecodeIFrame: function () {
	      return decoder._opt.playbackRate >= decoder._opt.playbackForwardMaxRateDecodeIFrame;
	    },
	    playbackUpdatePlaybackRate: function () {
	      decoder.clearBuffer(true);
	    },
	    isVideoSequenceHeader: function (payload) {
	      return payload[0] >> 4 === FRAME_TYPE.keyFrame && payload[1] === AVC_PACKET_TYPE.sequenceHeader;
	    },
	    onOffscreenCanvasWebglContextLost: function (event) {
	      decoder.debug.error('worker', 'handleOffscreenCanvasWebglContextLost and next try to create webgl');
	      event.preventDefault();
	      isWebglContextLost = true;
	      decoder.webglObj.destroy();
	      decoder.webglObj = null;
	      decoder.offscreenCanvasGL = null;
	      setTimeout(() => {
	        decoder.offscreenCanvasGL = decoder.offscreenCanvas.getContext("webgl");

	        if (decoder.offscreenCanvasGL && decoder.offscreenCanvasGL.getContextAttributes().stencil) {
	          decoder.webglObj = createWebGL(decoder.offscreenCanvasGL, decoder._opt.openWebglAlignment);
	          isWebglContextLost = false;
	        } else {
	          decoder.debug.error('worker', 'handleOffscreenCanvasWebglContextLost, stencil is false'); // todo: 通知主线程，webgl context lost
	        }
	      }, 500);
	    },
	    onOffscreenCanvasWebglContextRestored: function (event) {
	      decoder.debug.log('worker', 'handleOffscreenCanvasWebglContextRestored');
	      event.preventDefault();
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
	        decoder.webglObj = createWebGL(decoder.offscreenCanvasGL, decoder._opt.openWebglAlignment); // lost

	        decoder.offscreenCanvas.addEventListener('webglcontextlost', decoder.onOffscreenCanvasWebglContextLost, false); // restored

	        decoder.offscreenCanvas.addEventListener('webglcontextrestored', decoder.onOffscreenCanvasWebglContextRestored, false);
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
	      if (decoder.isDestroyed) {
	        decoder.debug.log('worker', 'yuvData, decoder is destroyed and return');
	        return;
	      }

	      const size = videoWidth * videoHeight * 3 / 2;
	      let out = Module.HEAPU8.subarray(yuv, yuv + size);
	      let data = new Uint8Array(out); // newDecodedVideoFrameTimestamp = ts;
	      // newDecodedVideoFrameLocalTimestamp = now();

	      if (decoder.useOffscreen()) {
	        try {
	          if (isWebglContextLost) {
	            return;
	          }

	          decoder.webglObj.renderYUV(videoWidth, videoHeight, data);
	          let image_bitmap = decoder.offscreenCanvas.transferToImageBitmap();
	          postMessage({
	            cmd: WORKER_CMD_TYPE.render,
	            buffer: image_bitmap,
	            delay: decoder.delay,
	            ts
	          }, [image_bitmap]);
	        } catch (e) {
	          decoder.debug.error('worker', 'yuvData, transferToImageBitmap error is', e);
	        }
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
	      if (decoder.isDestroyed) {
	        decoder.debug.log('worker', 'pcmData, decoder is destroyed and return');
	        return;
	      }

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
	          } // newDecodedAudioFrameTimestamp = ts;


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
	        } // newDecodedAudioFrameTimestamp = ts;


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
	    sendWebsocketMessage: function (message) {
	      if (socket) {
	        if (socket.readyState === WEBSOCKET_STATUS_CODE.open) {
	          socket.send(message);
	        } else {
	          decoder.debug.error('worker', 'socket is not open');
	        }
	      } else {
	        decoder.debug.error('worker', 'socket is null');
	      }
	    },
	    timeEnd: function () {
	      decoder.debug.error('worker', 'jessibuca pro 体验结束,请刷新页面再次体验，如需要购买商业授权，可以联系微信：bosswancheng');
	      decoder.close();
	      postMessage({
	        cmd: WORKER_CMD_TYPE.workerEnd
	      });
	    }
	  };
	  decoder.debug = new Debug(decoder);
	  let audioDecoder = null;

	  if (Module.AudioDecoder) {
	    audioDecoder = new Module.AudioDecoder(decoder);
	  }

	  let videoDecoder = null;

	  if (Module.VideoDecoder) {
	    videoDecoder = new Module.VideoDecoder(decoder);
	  }

	  postMessage({
	    cmd: WORKER_CMD_TYPE.init
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
	        decoder.debug.log('worker', 'updateConfig', msg.key, msg.value);
	        decoder._opt[msg.key] = msg.value;

	        if (msg.key === 'playbackRate') {
	          decoder.playbackUpdatePlaybackRate();

	          if (decoder.isPlaybackCacheBeforeDecodeForFpsRender()) {
	            decoder.playbackCacheLoop();
	          }
	        }

	        break;

	      case WORKER_SEND_TYPE.sendWsMessage:
	        decoder.sendWebsocketMessage(msg.message);
	        break;
	    }
	  };
	}

	decoderProSimd.postRun = function () {
	  workerPostRun(decoderProSimd);
	};

}));
//# sourceMappingURL=decoder-pro-simd.js.map
