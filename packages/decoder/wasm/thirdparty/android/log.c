#include "log.h"
#include <stdarg.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
EM_JS(void, em_log, (const char* str), {
    console.log(UTF8ToString(str));
});
#endif

#define MAX_LOG_BUFSIZE   512
static const char*  log_level[] = {
    "Debug",
    "Verbose",
    "Info",
    "Wran",
    "Error"
};

size_t strlcatf(char *dst, size_t size, const char *fmt, ...)
{
    size_t len = strlen(dst);
    va_list vl;
    
    va_start(vl, fmt);
    len += vsnprintf(dst + len, size > len ? size - len : 0, fmt, vl);
    va_end(vl);
    
    return len;
}

void aslog(LOG_LEVEL level, const char* format, ...) {
    
    char logbuf[MAX_LOG_BUFSIZE + 2];
    bool iscopy = false;
    size_t l = 0;
    time_t t = time(NULL);
    
    struct tm  ltm;
    localtime_r(&t, &ltm);
    
    l = strftime (logbuf, 64, "%Y-%m-%d %H:%M:%S", &ltm);
    l = strlcatf(logbuf, MAX_LOG_BUFSIZE - l, "[%s]", log_level[level]);
    char* vp = logbuf + l;
    
    va_list args;
    va_start(args, format);
    int vn = vsnprintf(vp, MAX_LOG_BUFSIZE - l, format, args);
    if(vn > (MAX_LOG_BUFSIZE - l)) {
        l = MAX_LOG_BUFSIZE;
    }
    else if (vn > 0 && vn <= (MAX_LOG_BUFSIZE - l)) {
        l += vn;
    }
    else {
        // vsnprintf error
        iscopy = false;
    }
    va_end(args);

    if(iscopy) {
        if(logbuf[l-1] != '\n')
            logbuf[l++] = '\n';
        logbuf[l] = '\0';
#if defined(__ANDROID__)
#elif defined(__APPLE__)
#elif defined(_WIN32)
#elif defined(__EMSCRIPTEN__)
       em_log(logbuf);
#else
        printf("%s", logbuf);
#endif
    }
}