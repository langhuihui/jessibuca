// a simple ajax
!(function () {

    var jsonType = 'application/json';
    var htmlType = 'text/html';
    var xmlTypeRE = /^(?:text|application)\/xml/i;
    var blankRE = /^\s*$/; // \s

    /*
     * default setting
     * */
    var _settings = {

        type: "GET",

        beforeSend: noop,

        success: noop,

        error: noop,

        complete: noop,

        context: null,

        xhr: function () {
            return new window.XMLHttpRequest();
        },

        accepts: {
            json: jsonType,
            xml: 'application/xml, text/xml',
            html: htmlType,
            text: 'text/plain'
        },

        crossDomain: false,

        timeout: 0,

        username: null,

        password: null,

        processData: true,

        promise: noop
    };

    function noop() {
    }

    var ajax = function (options) {

        //
        var settings = extend({}, options || {});

        //
        for (var key in _settings) {
            if (settings[key] === undefined) {
                settings[key] = _settings[key];
            }
        }

        //
        try {
            var q = {};
            var promise = new Promise(function (resolve, reject) {
                q.resolve = resolve;
                q.reject = reject;
            });

            promise.resolve = q.resolve;
            promise.reject = q.reject;

            settings.promise = promise;
        }
        catch (e) {
            //
            settings.promise = {
                resolve: noop,
                reject: noop
            };
        }


        //
        if (!settings.crossDomain) {
            settings.crossDomain = /^([\w-]+:)?\/\/([^\/]+)/.test(settings.url) && RegExp.$2 !== window.location.href;
        }

        var dataType = settings.dataType;
        // jsonp
        if (dataType === 'jsonp') {
            //
            var hasPlaceholder = /=\?/.test(settings.url);
            if (!hasPlaceholder) {
                var jsonpCallback = (settings.jsonp || 'callback') + '=?';

                settings.url = appendQuery(settings.url, jsonpCallback)
            }
            return JSONP(settings);
        }

        // url
        if (!settings.url) {
            settings.url = window.location.toString();
        }

        //
        serializeData(settings);

        var mime = settings.accepts[dataType]; // mime
        var baseHeader = {}; // header
        var protocol = /^([\w-]+:)\/\//.test(settings.url) ? RegExp.$1 : window.location.protocol; // protocol
        var xhr = _settings.xhr();
        var abortTimeout;

        // X-Requested-With header
        // For cross-domain requests, seeing as conditions for a preflight are
        // akin to a jigsaw puzzle, we simply never set it to be sure.
        // (it can always be set on a per-request basis or even using ajaxSetup)
        // For same-domain requests, won't change header if already provided.
        if (!settings.crossDomain && !baseHeader['X-Requested-With']) {
            baseHeader['X-Requested-With'] = 'XMLHttpRequest';
        }

        // mime
        if (mime) {
            //
            baseHeader['Accept'] = mime;

            if (mime.indexOf(',') > -1) {
                mime = mime.split(',', 2)[0]
            }
            //
            xhr.overrideMimeType && xhr.overrideMimeType(mime);
        }

        // contentType
        if (settings.contentType || (settings.data && settings.type.toUpperCase() !== 'GET')) {
            baseHeader['Content-Type'] = (settings.contentType || 'application/x-www-form-urlencoded; charset=UTF-8');
        }

        // headers
        settings.headers = extend(baseHeader, settings.headers || {});

        // on ready state change
        xhr.onreadystatechange = function () {
            // readystate
            if (xhr.readyState === 4) {
                clearTimeout(abortTimeout);
                var result;
                var error = false;
                //
                if ((xhr.status >= 200 && xhr.status < 300) || xhr.status === 304) {
                    dataType = dataType || mimeToDataType(xhr.getResponseHeader('content-type'));
                    result = xhr.responseText;

                    try {
                        // xml
                        if (dataType === 'xml') {
                            result = xhr.responseXML;
                        }
                        // json
                        else if (dataType === 'json') {
                            result = blankRE.test(result) ? null : JSON.parse(result);
                        }
                    }
                    catch (e) {
                        error = e;
                    }

                    if (error) {
                        ajaxError(error, 'parseerror', xhr, settings);
                    }
                    else {
                        ajaxSuccess(result, xhr, settings);
                    }
                }
                else {
                    ajaxError(null, 'error', xhr, settings);
                }

            }
        };

        // async
        var async = 'async' in settings ? settings.async : true;

        // open
        xhr.open(settings.type, settings.url, async, settings.username, settings.password);

        // xhrFields
        if (settings.xhrFields) {
            for (var name in settings.xhrFields) {
                xhr[name] = settings.xhrFields[name];
            }
        }

        // Override mime type if needed
        if (settings.mimeType && xhr.overrideMimeType) {
            xhr.overrideMimeType(settings.mimeType);
        }


        // set request header
        for (var name in settings.headers) {
            // Support: IE<9
            // IE's ActiveXObject throws a 'Type Mismatch' exception when setting
            // request header to a null-value.
            //
            // To keep consistent with other XHR implementations, cast the value
            // to string and ignore `undefined`.
            if (settings.headers[name] !== undefined) {
                xhr.setRequestHeader(name, settings.headers[name] + "");
            }
        }

        // before send
        if (ajaxBeforeSend(xhr, settings) === false) {
            xhr.abort();
            return false;
        }

        // timeout
        if (settings.timeout > 0) {
            abortTimeout = window.setTimeout(function () {
                xhr.onreadystatechange = noop;
                xhr.abort();
                ajaxError(null, 'timeout', xhr, settings);
            }, settings.timeout);
        }

        // send
        xhr.send(settings.data ? settings.data : null);

        return settings.promise;
    };

    /*
     * method  get
     * */
    ajax.get = function (url, data, success, dataType) {
        if (isFunction(data)) {
            dataType = dataType || success;
            success = data;
            data = undefined;
        }

        return ajax({
            url: url,
            data: data,
            success: success,
            dataType: dataType
        });
    };

    /*
     * method post
     *
     * dataType:
     * */
    ajax.post = function (url, data, success, dataType) {
        if (isFunction(data)) {
            dataType = dataType || success;
            success = data;
            data = undefined;
        }
        return ajax({
            type: 'POST',
            url: url,
            data: data,
            success: success,
            dataType: dataType
        })
    };

    /*
     * method getJSON
     * */
    ajax.getJSON = function (url, data, success) {

        if (isFunction(data)) {
            success = data;
            data = undefined;
        }

        return ajax({
            url: url,
            data: data,
            success: success,
            dataType: 'json'
        })
    };

    /*
     * method  ajaxSetup
     * */
    ajax.ajaxSetup = function (target, settings) {
        return settings ? extend(extend(target, _settings), settings) : extend(_settings, target);
    };

    /*
     * utils
     *
     * */


    // triggers and extra global event ajaxBeforeSend that's like ajaxSend but cancelable
    function ajaxBeforeSend(xhr, settings) {
        var context = settings.context;
        //
        if (settings.beforeSend.call(context, xhr, settings) === false) {
            return false;
        }
    }

    // ajax success
    function ajaxSuccess(data, xhr, settings) {
        var context = settings.context;
        var status = 'success';
        settings.success.call(context, data, status, xhr);
        settings.promise.resolve(data, status, xhr);
        ajaxComplete(status, xhr, settings);
    }

    // status: "success", "notmodified", "error", "timeout", "abort", "parsererror"
    function ajaxComplete(status, xhr, settings) {
        var context = settings.context;
        settings.complete.call(context, xhr, status);
    }

    // type: "timeout", "error", "abort", "parsererror"
    function ajaxError(error, type, xhr, settings) {
        var context = settings.context;
        settings.error.call(context, xhr, type, error);
        settings.promise.reject(xhr, type, error);
        ajaxComplete(type, xhr, settings);
    }


    // jsonp
    /*
     * tks: https://www.cnblogs.com/rubylouvre/archive/2011/02/13/1953087.html
     * */
    function JSONP(options) {
        //
        var callbackName = options.jsonpCallback || 'jsonp' + (new Date().getTime());

        var script = window.document.createElement('script');

        var abort = function () {
            // 设置 window.xxx = noop
            if (callbackName in window) {
                window[callbackName] = noop;
            }
        };

        var xhr = {abort: abort};
        var abortTimeout;

        var head = window.document.getElementsByTagName('head')[0] || window.document.documentElement;

        // ie8+
        script.onerror = function (error) {
            _error(error);
        };

        function _error(error) {
            window.clearTimeout(abortTimeout);
            xhr.abort();
            ajaxError(error.type, xhr, error.type, options);
            _removeScript();
        }

        window[callbackName] = function (data) {
            window.clearTimeout(abortTimeout);
            ajaxSuccess(data, xhr, options);
            _removeScript();
        };

        //
        serializeData(options);

        script.src = options.url.replace(/=\?/, '=' + callbackName);
        //
        script.src = appendQuery(script.src, '_=' + (new Date()).getTime());
        //
        script.async = true;

        // script charset
        if (options.scriptCharset) {
            script.charset = options.scriptCharset;
        }

        //
        head.insertBefore(script, head.firstChild);

        //
        if (options.timeout > 0) {
            abortTimeout = window.setTimeout(function () {
                xhr.abort();
                ajaxError('timeout', xhr, 'timeout', options);
                _removeScript();
            }, options.timeout);
        }

        // remove script
        function _removeScript() {
            if (script.clearAttributes) {
                script.clearAttributes();
            } else {
                script.onload = script.onreadystatechange = script.onerror = null;
            }

            if (script.parentNode) {
                script.parentNode.removeChild(script);
            }
            //
            script = null;

            delete window[callbackName];
        }

        return options.promise;
    }

    //  mime to data type
    function mimeToDataType(mime) {
        return mime && (mime === htmlType ? 'html' : mime === jsonType ? 'json' : xmlTypeRE.test(mime) && 'xml') || 'text'
    }

    // append query
    function appendQuery(url, query) {
        return (url + '&' + query).replace(/[&?]{1,2}/, '?');
    }

    // serialize data
    function serializeData(options) {
        // formData
        if (isObject(options) && !isFormData(options.data) && options.processData) {
            options.data = param(options.data);
        }

        if (options.data && (!options.type || options.type.toUpperCase() === 'GET')) {
            options.url = appendQuery(options.url, options.data);
        }
    }

    // serialize
    function serialize(params, obj, traditional, scope) {
        var _isArray = isArray(obj);

        for (var key in obj) {
            var value = obj[key];

            if (scope) {
                key = traditional ? scope : scope + '[' + (_isArray ? '' : key) + ']';
            }

            // handle data in serializeArray format
            if (!scope && _isArray) {
                params.add(value.name, value.value);

            }
            else if (traditional ? _isArray(value) : isObject(value)) {
                serialize(params, value, traditional, key);
            }
            else {
                params.add(key, value);
            }
        }

    }

    // param
    function param(obj, traditional) {
        var params = [];
        //
        params.add = function (k, v) {
            this.push(encodeURIComponent(k) + '=' + encodeURIComponent(v));
        };
        serialize(params, obj, traditional);
        return params.join('&').replace('%20', '+');
    }

    // extend
    function extend(target) {
        var slice = Array.prototype.slice;
        var args = slice.call(arguments, 1);
        //
        for (var i = 0, length = args.length; i < length; i++) {
            var source = args[i] || {};
            for (var key in  source) {
                if (source.hasOwnProperty(key) && source[key] !== undefined) {
                    target[key] = source[key];
                }
            }
        }

        return target;
    }

    // is object
    function isObject(obj) {
        var type = typeof obj;
        return type === 'function' || type === 'object' && !!obj;
    }

    // is formData
    function isFormData(obj) {
        return obj instanceof FormData;
    }

    // is array
    function isArray(value) {
        return Object.prototype.toString.call(value) === "[object Array]";
    }

    // is function
    function isFunction(value) {
        return typeof value === "function";
    }

    // browser
    window.ajax = ajax;
})();







