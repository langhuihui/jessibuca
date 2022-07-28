#ifndef  __AS_LOG_H__
#define  __AS_LOG_H__
#ifdef   __cplusplus
extern "C" {
#endif
typedef enum LOG_LEVEL {
    LOG_DEBUG = 0,
    LOG_VERBOSE,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
} LOG_LEVEL;

void aslog(LOG_LEVEL level, const char* format, ...);

#ifdef __cplusplus
}
#endif
#endif