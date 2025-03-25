var __create = Object.create;
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __getProtoOf = Object.getPrototypeOf;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __commonJS = (cb, mod) => function __require() {
  return mod || (0, cb[__getOwnPropNames(cb)[0]])((mod = { exports: {} }).exports, mod), mod.exports;
};
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toESM = (mod, isNodeMode, target) => (target = mod != null ? __create(__getProtoOf(mod)) : {}, __copyProps(
  // If the importer is in node compatibility mode or this is not an ESM
  // file that has been converted to a CommonJS file using a Babel-
  // compatible transform (i.e. "__esModule" has not been set), then set
  // "default" to the CommonJS "module.exports" for node compatibility.
  isNodeMode || !mod || !mod.__esModule ? __defProp(target, "default", { value: mod, enumerable: true }) : target,
  mod
));
var __decorateClass = (decorators, target, key, kind) => {
  var result = kind > 1 ? void 0 : kind ? __getOwnPropDesc(target, key) : target;
  for (var i = decorators.length - 1, decorator; i >= 0; i--)
    if (decorator = decorators[i])
      result = (kind ? decorator(target, key, result) : decorator(result)) || result;
  if (kind && result)
    __defProp(target, key, result);
  return result;
};

// ../../node_modules/.pnpm/eventemitter3@4.0.7/node_modules/eventemitter3/index.js
var require_eventemitter3 = __commonJS({
  "../../node_modules/.pnpm/eventemitter3@4.0.7/node_modules/eventemitter3/index.js"(exports, module) {
    "use strict";
    var has = Object.prototype.hasOwnProperty;
    var prefix = "~";
    function Events() {
    }
    if (Object.create) {
      Events.prototype = /* @__PURE__ */ Object.create(null);
      if (!new Events().__proto__)
        prefix = false;
    }
    function EE(fn, context, once) {
      this.fn = fn;
      this.context = context;
      this.once = once || false;
    }
    function addListener(emitter, event, fn, context, once) {
      if (typeof fn !== "function") {
        throw new TypeError("The listener must be a function");
      }
      var listener = new EE(fn, context || emitter, once), evt = prefix ? prefix + event : event;
      if (!emitter._events[evt])
        emitter._events[evt] = listener, emitter._eventsCount++;
      else if (!emitter._events[evt].fn)
        emitter._events[evt].push(listener);
      else
        emitter._events[evt] = [emitter._events[evt], listener];
      return emitter;
    }
    function clearEvent(emitter, evt) {
      if (--emitter._eventsCount === 0)
        emitter._events = new Events();
      else
        delete emitter._events[evt];
    }
    function EventEmitter3() {
      this._events = new Events();
      this._eventsCount = 0;
    }
    EventEmitter3.prototype.eventNames = function eventNames() {
      var names = [], events, name;
      if (this._eventsCount === 0)
        return names;
      for (name in events = this._events) {
        if (has.call(events, name))
          names.push(prefix ? name.slice(1) : name);
      }
      if (Object.getOwnPropertySymbols) {
        return names.concat(Object.getOwnPropertySymbols(events));
      }
      return names;
    };
    EventEmitter3.prototype.listeners = function listeners(event) {
      var evt = prefix ? prefix + event : event, handlers = this._events[evt];
      if (!handlers)
        return [];
      if (handlers.fn)
        return [handlers.fn];
      for (var i = 0, l = handlers.length, ee = new Array(l); i < l; i++) {
        ee[i] = handlers[i].fn;
      }
      return ee;
    };
    EventEmitter3.prototype.listenerCount = function listenerCount(event) {
      var evt = prefix ? prefix + event : event, listeners = this._events[evt];
      if (!listeners)
        return 0;
      if (listeners.fn)
        return 1;
      return listeners.length;
    };
    EventEmitter3.prototype.emit = function emit(event, a1, a2, a3, a4, a5) {
      var evt = prefix ? prefix + event : event;
      if (!this._events[evt])
        return false;
      var listeners = this._events[evt], len = arguments.length, args, i;
      if (listeners.fn) {
        if (listeners.once)
          this.removeListener(event, listeners.fn, void 0, true);
        switch (len) {
          case 1:
            return listeners.fn.call(listeners.context), true;
          case 2:
            return listeners.fn.call(listeners.context, a1), true;
          case 3:
            return listeners.fn.call(listeners.context, a1, a2), true;
          case 4:
            return listeners.fn.call(listeners.context, a1, a2, a3), true;
          case 5:
            return listeners.fn.call(listeners.context, a1, a2, a3, a4), true;
          case 6:
            return listeners.fn.call(listeners.context, a1, a2, a3, a4, a5), true;
        }
        for (i = 1, args = new Array(len - 1); i < len; i++) {
          args[i - 1] = arguments[i];
        }
        listeners.fn.apply(listeners.context, args);
      } else {
        var length = listeners.length, j;
        for (i = 0; i < length; i++) {
          if (listeners[i].once)
            this.removeListener(event, listeners[i].fn, void 0, true);
          switch (len) {
            case 1:
              listeners[i].fn.call(listeners[i].context);
              break;
            case 2:
              listeners[i].fn.call(listeners[i].context, a1);
              break;
            case 3:
              listeners[i].fn.call(listeners[i].context, a1, a2);
              break;
            case 4:
              listeners[i].fn.call(listeners[i].context, a1, a2, a3);
              break;
            default:
              if (!args)
                for (j = 1, args = new Array(len - 1); j < len; j++) {
                  args[j - 1] = arguments[j];
                }
              listeners[i].fn.apply(listeners[i].context, args);
          }
        }
      }
      return true;
    };
    EventEmitter3.prototype.on = function on(event, fn, context) {
      return addListener(this, event, fn, context, false);
    };
    EventEmitter3.prototype.once = function once(event, fn, context) {
      return addListener(this, event, fn, context, true);
    };
    EventEmitter3.prototype.removeListener = function removeListener(event, fn, context, once) {
      var evt = prefix ? prefix + event : event;
      if (!this._events[evt])
        return this;
      if (!fn) {
        clearEvent(this, evt);
        return this;
      }
      var listeners = this._events[evt];
      if (listeners.fn) {
        if (listeners.fn === fn && (!once || listeners.once) && (!context || listeners.context === context)) {
          clearEvent(this, evt);
        }
      } else {
        for (var i = 0, events = [], length = listeners.length; i < length; i++) {
          if (listeners[i].fn !== fn || once && !listeners[i].once || context && listeners[i].context !== context) {
            events.push(listeners[i]);
          }
        }
        if (events.length)
          this._events[evt] = events.length === 1 ? events[0] : events;
        else
          clearEvent(this, evt);
      }
      return this;
    };
    EventEmitter3.prototype.removeAllListeners = function removeAllListeners(event) {
      var evt;
      if (event) {
        evt = prefix ? prefix + event : event;
        if (this._events[evt])
          clearEvent(this, evt);
      } else {
        this._events = new Events();
        this._eventsCount = 0;
      }
      return this;
    };
    EventEmitter3.prototype.off = EventEmitter3.prototype.removeListener;
    EventEmitter3.prototype.addListener = EventEmitter3.prototype.on;
    EventEmitter3.prefixed = prefix;
    EventEmitter3.EventEmitter = EventEmitter3;
    if ("undefined" !== typeof module) {
      module.exports = EventEmitter3;
    }
  }
});

// ../../node_modules/.pnpm/afsm@2.4.9/node_modules/afsm/index.js
var import_eventemitter3 = __toESM(require_eventemitter3(), 1);
var instance = Symbol("instance");
var abortCtrl = Symbol("abortCtrl");
var cacheResult = Symbol("cacheResult");
var MiddleState = class {
  constructor(oldState, newState, action) {
    this.oldState = oldState;
    this.newState = newState;
    this.action = action;
    this.aborted = false;
  }
  abort(fsm) {
    this.aborted = true;
    setState.call(fsm, this.oldState, new Error(`action '${this.action}' aborted`));
  }
  toString() {
    return `${this.action}ing`;
  }
};
var FSMError = class extends Error {
  /*************  ✨ Codeium Command ⭐  *************/
  /**
     * Create a new instance of FSMError.
     * @param state current state.
     * @param message error message.
     * @param cause original error.
  /******  625fa23f-3ee1-42ac-94bd-4f6ffd4578ff  *******/
  constructor(state, message, cause) {
    super(message);
    this.state = state;
    this.message = message;
    this.cause = cause;
  }
};
function thenAble(val) {
  return typeof val === "object" && val && "then" in val;
}
var stateDiagram = /* @__PURE__ */ new Map();
function ChangeState(from, to, opt = {}) {
  return (target, propertyKey, descriptor) => {
    const action = opt.action || propertyKey;
    if (!opt.context) {
      const stateConfig = stateDiagram.get(target) || [];
      if (!stateDiagram.has(target))
        stateDiagram.set(target, stateConfig);
      stateConfig.push({ from, to, action });
    }
    const origin = descriptor.value;
    descriptor.value = function(...arg) {
      let fsm = this;
      if (opt.context) {
        fsm = FSM.get(typeof opt.context === "function" ? opt.context.call(this, ...arg) : opt.context);
      }
      if (fsm.state === to)
        return opt.sync ? fsm[cacheResult] : Promise.resolve(fsm[cacheResult]);
      else if (fsm.state instanceof MiddleState) {
        if (fsm.state.action == opt.abortAction) {
          fsm.state.abort(fsm);
        }
      }
      let err = null;
      if (Array.isArray(from)) {
        if (from.length == 0) {
          if (fsm.state instanceof MiddleState)
            fsm.state.abort(fsm);
        } else if (typeof fsm.state != "string" || !from.includes(fsm.state)) {
          err = new FSMError(fsm._state, `${fsm.name} ${action} to ${to} failed: current state ${fsm._state} not from ${from.join("|")}`);
        }
      } else {
        if (from !== fsm.state) {
          err = new FSMError(fsm._state, `${fsm.name} ${action} to ${to} failed: current state ${fsm._state} not from ${from}`);
        }
      }
      const returnErr = (err2) => {
        if (opt.fail)
          opt.fail.call(this, err2);
        if (opt.sync) {
          if (opt.ignoreError)
            return err2;
          throw err2;
        } else {
          if (opt.ignoreError)
            return Promise.resolve(err2);
          return Promise.reject(err2);
        }
      };
      if (err)
        return returnErr(err);
      const old = fsm.state;
      const middle = new MiddleState(old, to, action);
      setState.call(fsm, middle);
      const success = (result) => {
        var _a;
        fsm[cacheResult] = result;
        if (!middle.aborted) {
          setState.call(fsm, to);
          (_a = opt.success) === null || _a === void 0 ? void 0 : _a.call(this, fsm[cacheResult]);
        }
        return result;
      };
      const failed = (err2) => {
        setState.call(fsm, old, err2);
        return returnErr(err2);
      };
      try {
        const result = origin.apply(this, arg);
        if (thenAble(result))
          return result.then(success).catch(failed);
        else
          return opt.sync ? success(result) : Promise.resolve(success(result));
      } catch (err2) {
        return failed(new FSMError(fsm._state, `${fsm.name} ${action} from ${from} to ${to} failed: ${err2}`, err2 instanceof Error ? err2 : new Error(String(err2))));
      }
    };
  };
}
function Includes(...states) {
  return (target, propertyKey, descriptor) => {
    const origin = descriptor.value;
    const action = propertyKey;
    descriptor.value = function(...arg) {
      if (!states.includes(this.state.toString()))
        throw new FSMError(this.state, `${this.name} ${action} failed: current state ${this.state} not in ${states}`);
      return origin.apply(this, arg);
    };
  };
}
var sendDevTools = (() => {
  const hasDevTools = typeof window !== "undefined" && window["__AFSM__"];
  const inWorker = typeof importScripts !== "undefined";
  return hasDevTools ? (name, detail) => {
    window.dispatchEvent(new CustomEvent(name, { detail }));
  } : inWorker ? (type, payload) => {
    postMessage({ type, payload });
  } : () => {
  };
})();
function setState(value, err) {
  const old = this._state;
  this._state = value;
  const state = value.toString();
  if (value)
    this.emit(state, old);
  this.emit(FSM.STATECHANGED, value, old, err);
  this.updateDevTools({ value, old, err: err instanceof Error ? err.message : String(err) });
}
var FSM = class _FSM extends import_eventemitter3.default {
  constructor(name, groupName, prototype) {
    super();
    this.name = name;
    this.groupName = groupName;
    this._state = _FSM.INIT;
    if (!name)
      name = Date.now().toString(36);
    if (!prototype)
      prototype = Object.getPrototypeOf(this);
    else
      Object.setPrototypeOf(this, prototype);
    if (!groupName)
      this.groupName = this.constructor.name;
    const names = prototype[instance];
    if (!names)
      prototype[instance] = { name: this.name, count: 0 };
    else
      this.name = names.name + "-" + names.count++;
    this.updateDevTools({ diagram: this.stateDiagram });
  }
  get stateDiagram() {
    const protoType = Object.getPrototypeOf(this);
    const stateConfig = stateDiagram.get(protoType) || [];
    let result = /* @__PURE__ */ new Set();
    let plain = [];
    let forceTo = [];
    const allState = /* @__PURE__ */ new Set();
    const parent = Object.getPrototypeOf(protoType);
    if (stateDiagram.has(parent)) {
      parent.stateDiagram.forEach((stateDesc) => result.add(stateDesc));
      parent.allStates.forEach((state) => allState.add(state));
    }
    stateConfig.forEach(({ from, to, action }) => {
      if (typeof from === "string") {
        plain.push({ from, to, action });
      } else {
        if (from.length) {
          from.forEach((f) => {
            plain.push({ from: f, to, action });
          });
        } else
          forceTo.push({ to, action });
      }
    });
    plain.forEach(({ from, to, action }) => {
      allState.add(from);
      allState.add(to);
      allState.add(action + "ing");
      result.add(`${from} --> ${action}ing : ${action}`);
      result.add(`${action}ing --> ${to} : ${action} \u{1F7E2}`);
      result.add(`${action}ing --> ${from} : ${action} \u{1F534}`);
    });
    forceTo.forEach(({ to, action }) => {
      result.add(`${action}ing --> ${to} : ${action} \u{1F7E2}`);
      allState.forEach((f) => {
        if (f !== to)
          result.add(`${f} --> ${action}ing : ${action}`);
      });
    });
    const value = [...result];
    Object.defineProperties(protoType, {
      stateDiagram: { value },
      allStates: { value: allState }
    });
    return value;
  }
  static get(context) {
    let fsm;
    if (typeof context === "string") {
      fsm = _FSM.instances.get(context);
      if (!fsm) {
        _FSM.instances.set(context, fsm = new _FSM(context, void 0, Object.create(_FSM.prototype)));
      }
    } else {
      fsm = _FSM.instances2.get(context);
      if (!fsm) {
        _FSM.instances2.set(context, fsm = new _FSM(context.constructor.name, void 0, Object.create(_FSM.prototype)));
      }
    }
    return fsm;
  }
  static getState(context) {
    var _a;
    return (_a = _FSM.get(context)) === null || _a === void 0 ? void 0 : _a.state;
  }
  updateDevTools(payload = {}) {
    sendDevTools(_FSM.UPDATEAFSM, Object.assign({ name: this.name, group: this.groupName }, payload));
  }
  get state() {
    return this._state;
  }
  set state(value) {
    setState.call(this, value);
  }
};
FSM.STATECHANGED = "stateChanged";
FSM.UPDATEAFSM = "updateAFSM";
FSM.INIT = "[*]";
FSM.ON = "on";
FSM.OFF = "off";
FSM.instances = /* @__PURE__ */ new Map();
FSM.instances2 = /* @__PURE__ */ new WeakMap();

// ../../node_modules/.pnpm/oput@1.2.2/node_modules/oput/dist/index.js
var __awaiter = function(thisArg, _arguments, P, generator) {
  function adopt(value) {
    return value instanceof P ? value : new P(function(resolve) {
      resolve(value);
    });
  }
  return new (P || (P = Promise))(function(resolve, reject) {
    function fulfilled(value) {
      try {
        step(generator.next(value));
      } catch (e) {
        reject(e);
      }
    }
    function rejected(value) {
      try {
        step(generator["throw"](value));
      } catch (e) {
        reject(e);
      }
    }
    function step(result) {
      result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected);
    }
    step((generator = generator.apply(thisArg, _arguments || [])).next());
  });
};
var U32 = Symbol(32);
var U16 = Symbol(16);
var U8 = Symbol(8);
var OPut = class {
  constructor(g) {
    this.g = g;
    this.consumed = 0;
    if (g)
      this.need = g.next().value;
  }
  setG(g) {
    this.g = g;
    this.demand(g.next().value, true);
  }
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
    return __awaiter(this, void 0, void 0, function* () {
      if (this.lastReadPromise) {
        yield this.lastReadPromise;
      }
      return this.lastReadPromise = new Promise((resolve, reject) => {
        var _a;
        this.reject = reject;
        this.resolve = (data) => {
          delete this.lastReadPromise;
          delete this.resolve;
          delete this.need;
          resolve(data);
        };
        const result = this.demand(need, true);
        if (!result)
          (_a = this.pull) === null || _a === void 0 ? void 0 : _a.call(this, need);
      });
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
    var _a;
    if (this.g)
      this.g.return();
    if (this.buffer)
      this.buffer.subarray(0, 0);
    (_a = this.reject) === null || _a === void 0 ? void 0 : _a.call(this, new Error("EOF"));
    delete this.lastReadPromise;
  }
  flush() {
    if (!this.buffer || !this.need)
      return;
    let returnValue = null;
    const unread = this.buffer.subarray(this.consumed);
    let n = 0;
    const notEnough = (x) => unread.length < (n = x);
    if (typeof this.need === "number") {
      if (notEnough(this.need))
        return;
      returnValue = unread.subarray(0, n);
    } else if (this.need === U32) {
      if (notEnough(4))
        return;
      returnValue = unread[0] << 24 | unread[1] << 16 | unread[2] << 8 | unread[3];
    } else if (this.need === U16) {
      if (notEnough(2))
        return;
      returnValue = unread[0] << 8 | unread[1];
    } else if (this.need === U8) {
      if (notEnough(1))
        return;
      returnValue = unread[0];
    } else if (!("buffer" in this.need)) {
      if (notEnough(this.need.byteLength))
        return;
      new Uint8Array(this.need).set(unread.subarray(0, n));
      returnValue = this.need;
    } else if ("byteOffset" in this.need) {
      if (notEnough(this.need.byteLength - this.need.byteOffset))
        return;
      new Uint8Array(this.need.buffer, this.need.byteOffset).set(unread.subarray(0, n));
      returnValue = this.need;
    } else if (this.g) {
      this.g.throw(new Error("Unsupported type"));
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
    if (value instanceof Uint8Array) {
      this.malloc(value.length).set(value);
    } else if ("buffer" in value) {
      this.malloc(value.byteLength).set(new Uint8Array(value.buffer, value.byteOffset, value.byteLength));
    } else {
      this.malloc(value.byteLength).set(new Uint8Array(value));
    }
    if (this.g || this.resolve)
      this.flush();
    else
      return new Promise((resolve) => this.pull = resolve);
  }
  writeU32(value) {
    this.malloc(4).set([value >> 24 & 255, value >> 16 & 255, value >> 8 & 255, value & 255]);
    this.flush();
  }
  writeU16(value) {
    this.malloc(2).set([value >> 8 & 255, value & 255]);
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
      } else {
        const n = new Uint8Array(nl);
        n.set(this.buffer);
        this.buffer = n;
      }
      return this.buffer.subarray(l, nl);
    } else {
      this.buffer = new Uint8Array(size);
      return this.buffer;
    }
  }
};
OPut.U32 = U32;
OPut.U16 = U16;
OPut.U8 = U8;

// src/base.ts
function fibonacci(n, ac1 = 1, ac2 = 1) {
  return n <= 1 ? ac2 : fibonacci(n - 1, ac2, ac1 + ac2);
}
function getReconnectionTimeout(reconnectionCount) {
  const n = Math.round(reconnectionCount / 2) + 1;
  return n > 6 ? 13 * 1e3 : fibonacci(n) * 1e3;
}
var TransmissionStatistics = class {
  total = 0;
  _buffer = 0;
  lastTime = 0;
  _bps = 0;
  add(size) {
    const now = Date.now();
    this._buffer += size;
    if (this.lastTime === 0) {
      this.lastTime = now;
    } else if (now - this.lastTime > 1e3) {
      this._bps = this._buffer * 1e3 / (now - this.lastTime) >> 0;
      this._buffer = 0;
      this.lastTime = now;
    }
    this.total += size;
  }
  get bps() {
    return Date.now() - this.lastTime > 5e3 ? 0 : this._bps;
  }
};
var Connection = class extends FSM {
  constructor(url, options = {}) {
    super(url || "conn", "Connection");
    this.url = url;
    this.options = options;
    if (!this.options.reconnectTimeout) {
      this.options.reconnectTimeout = getReconnectionTimeout;
    }
  }
  oput;
  up = new TransmissionStatistics();
  down = new TransmissionStatistics();
  underlyingSink = {
    write: async (chunk) => {
      this.down.add(chunk.length || chunk.byteLength);
      return this.oput?.write(chunk);
    }
  };
  abortCtrl;
  read(need) {
    return Promise.reject("not connected");
  }
  async connect() {
    this.abortCtrl = new AbortController();
    console.log(`connected: ${this.url}`);
    console.time(this.url);
    this.onConnected(await this._connect());
  }
  _close() {
  }
  _send(data) {
  }
  async reconnect() {
    console.log(`reconnect: ${this.url}`);
    console.time(this.url);
    this.abortCtrl = new AbortController();
    this.onConnected(await this._connect());
  }
  onConnected(readable) {
    console.timeEnd(this.url);
    if (!readable)
      return;
    if (!this.oput)
      this.oput = new OPut();
    this.read = this.oput.read.bind(this.oput);
    return readable.pipeTo(new WritableStream(this.underlyingSink), this.abortCtrl).catch((err) => {
      if (this.abortCtrl.signal.aborted)
        return;
      this.disconnect(err);
    });
  }
  disconnect(reason) {
    console.warn(`disconnect: ${this.url}`, reason);
    if (this.options.reconnectCount)
      this.reconnectAfter();
  }
  reconnectAfter(delay = 1e3, count = 0) {
    console.log(`reconnect after ${delay}ms`);
    setTimeout(() => {
      this.reconnect().catch((err) => {
        console.log(`reconnect failed: ${this.url}`, err);
        if (count < this.options.reconnectCount)
          this.reconnectAfter(this.options.reconnectTimeout(count), count + 1);
      });
    }, delay);
  }
  close() {
    this.abortCtrl?.abort();
    this._close();
  }
  send(data) {
    this.up.add(data.byteLength - ("byteOffset" in data ? data.byteOffset : 0));
    this._send(data);
  }
};
__decorateClass([
  Includes("connected" /* CONNECTED */)
], Connection.prototype, "read", 1);
__decorateClass([
  ChangeState(
    ["disconnected" /* DISCONNECTED */, FSM.INIT],
    "connected" /* CONNECTED */
  )
], Connection.prototype, "connect", 1);
__decorateClass([
  ChangeState("disconnected" /* DISCONNECTED */, "reconnected" /* RECONNECTED */)
], Connection.prototype, "reconnect", 1);
__decorateClass([
  ChangeState("connected" /* CONNECTED */, "disconnected" /* DISCONNECTED */, { sync: true })
], Connection.prototype, "disconnect", 1);
__decorateClass([
  ChangeState([], FSM.INIT, { sync: true })
], Connection.prototype, "close", 1);

// src/ws.ts
var WebSocketConnection = class extends Connection {
  ws;
  async _connect() {
    const ws = new WebSocket(this.url);
    this.ws = ws;
    ws.binaryType = "arraybuffer";
    return new Promise((resolve, reject) => {
      ws.onerror = reject;
      ws.onopen = () => {
        resolve(
          new ReadableStream({
            start: (controller) => {
              ws.onclose = (e) => controller.error(e);
              ws.onmessage = (evt) => controller.enqueue(evt.data);
            }
          })
        );
      };
    });
  }
  _close() {
    this.ws?.close(1e3, "close");
  }
  _send(data) {
    this.ws?.send(data);
  }
};

// src/wt.ts
var WebTransportConnection = class extends Connection {
  wt;
  writer;
  async _connect() {
    const wt = new WebTransport(this.url);
    await wt.ready;
    const stream = await wt.createBidirectionalStream();
    this.writer = stream.writable.getWriter();
    return stream.readable;
  }
  _close() {
    this.wt?.close();
  }
  _send(data) {
    return this.writer?.write(data);
  }
};

// src/http.ts
var HttpConnection = class extends Connection {
  async _connect() {
    const res = await fetch(this.url, {
      ...this.options.requestInit,
      signal: this.abortCtrl.signal
    });
    if (!res.body)
      throw new Error("no body");
    return res.body;
  }
};

// src/webrtc.ts
var import_eventemitter32 = __toESM(require_eventemitter3(), 1);
var DataChannelConnection = class extends Connection {
  constructor(dc) {
    super(dc.label);
    this.dc = dc;
  }
  async _connect() {
    return new ReadableStream({
      start: (controller) => {
        this.dc.onclose = () => {
          controller.close();
        };
        this.dc.onerror = (e) => {
          controller.error(e);
        };
        this.dc.onmessage = (evt) => {
          controller.enqueue(evt.data);
        };
      }
    });
  }
  _close() {
    this.dc.close();
  }
  _send(data) {
    this.dc.send(data);
  }
};
var WebRTCStream = class extends import_eventemitter32.EventEmitter {
  constructor(id, direction = "recvonly") {
    super();
    this.id = id;
    this.direction = direction;
  }
  _mediaStream;
  get mediaStream() {
    return this._mediaStream;
  }
  set mediaStream(value) {
    this._mediaStream = value;
    if (value)
      this.emit("change", value);
  }
  audioTransceiver;
  videoTransceiver;
  _videoTrack;
  _audioTrack;
  set audioTrack(track) {
    this._audioTrack = track;
    if (this._mediaStream) {
      const oldTrack = this._mediaStream.getAudioTracks()[0];
      if (oldTrack)
        this._mediaStream.removeTrack(oldTrack);
    }
    if (track) {
      if (!this._mediaStream)
        this._mediaStream = new MediaStream([track]);
      else
        this._mediaStream.addTrack(track);
    }
    if (this._mediaStream)
      this.emit("change", this._mediaStream);
  }
  get audioTrack() {
    return this._audioTrack || this._mediaStream?.getAudioTracks()[0];
  }
  set videoTrack(track) {
    this._videoTrack = track;
    if (this._mediaStream) {
      const oldTrack = this._mediaStream.getVideoTracks()[0];
      if (oldTrack)
        this._mediaStream.removeTrack(oldTrack);
    }
    if (track) {
      if (!this._mediaStream)
        this._mediaStream = new MediaStream([track]);
      else
        this._mediaStream.addTrack(track);
    }
    if (this._mediaStream)
      this.emit("change", this._mediaStream);
  }
  get videoTrack() {
    return this._videoTrack || this._mediaStream?.getVideoTracks()[0];
  }
  close() {
    this.emit("close");
  }
};
var WebRTCConnection = class extends Connection {
  webrtc = new RTCPeerConnection(this.options.rtcConfig);
  streams = /* @__PURE__ */ new Map();
  videoTransceiver = new Array();
  audioTransceiver = new Array();
  async _connect() {
    const offer = await this.webrtc.createOffer();
    await this.webrtc.setLocalDescription(offer);
    const res = await fetch(this.url, {
      method: "POST",
      body: offer.sdp,
      ...this.options.requestInit || {}
    });
    const answer = await res.text();
    await this.webrtc.setRemoteDescription({ type: "answer", sdp: answer });
    this.webrtc.ontrack = ({ track, streams, transceiver }) => {
      console.log(track, streams, transceiver);
      if (streams.length) {
        const info = this.streams.get(streams[0].id);
        if (info && info.direction === "recvonly") {
          info.mediaStream = streams[0];
        }
      }
    };
    return new Promise((resolve, reject) => {
      this.webrtc.onconnectionstatechange = (evt) => {
        switch (this.webrtc.connectionState) {
          case "disconnected":
            this.disconnect(evt);
            break;
          case "connected":
            resolve();
            break;
          case "failed":
            reject(evt);
        }
      };
    });
  }
  addStream(stream) {
    if (this.audioTransceiver.length) {
      stream.audioTransceiver = this.audioTransceiver.pop();
      stream.audioTransceiver.direction = stream.direction;
    } else {
      stream.audioTransceiver = this.webrtc.addTransceiver(stream.audioTrack || "audio", {
        direction: stream.direction
      });
    }
    if (this.videoTransceiver.length) {
      stream.videoTransceiver = this.videoTransceiver.pop();
      stream.videoTransceiver.direction = stream.direction;
    } else {
      stream.videoTransceiver = this.webrtc.addTransceiver(stream.videoTrack || "video", {
        direction: stream.direction
      });
    }
    this.streams.set(stream.id, stream);
    return stream;
  }
  deleteStream(id) {
    const s = this.streams.get(id);
    if (s) {
      delete s.mediaStream;
      s.audioTransceiver.direction = "inactive";
      s.videoTransceiver.direction = "inactive";
      this.audioTransceiver.push(s.audioTransceiver);
      this.videoTransceiver.push(s.videoTransceiver);
      this.streams.delete(id);
    }
  }
  _close() {
    this.webrtc?.close();
  }
};

// src/file.ts
var FileConnection = class extends Connection {
  constructor(file) {
    super(file.name);
    this.file = file;
  }
  async _connect() {
    return this.file.stream();
  }
};

// src/index.ts
var Protocol = /* @__PURE__ */ ((Protocol2) => {
  Protocol2["WS"] = "ws://";
  Protocol2["WSS"] = "wss://";
  Protocol2["HTTP"] = "http://";
  Protocol2["HTTPS"] = "https://";
  Protocol2["WebTransport"] = "wt://";
  Protocol2["WebRTC"] = "webrtc://";
  return Protocol2;
})(Protocol || {});
function getURLType(url) {
  if (url instanceof File) {
    return "file";
  }
  if (url.startsWith("ws://" /* WS */) || url.startsWith("wss://" /* WSS */)) {
    return "ws";
  } else if (url.startsWith("http://" /* HTTP */) || url.startsWith("https://" /* HTTPS */)) {
    return "http";
  } else if (url.startsWith("wt://" /* WebTransport */)) {
    return "wt";
  } else if (url.startsWith("webrtc://" /* WebRTC */)) {
    return "webrtc";
  } else {
    throw new Error("url is invalid");
  }
}
export {
  DataChannelConnection,
  FileConnection,
  HttpConnection,
  Protocol,
  WebRTCConnection,
  WebRTCStream,
  WebSocketConnection,
  WebTransportConnection,
  getURLType
};
//# sourceMappingURL=index.js.map
