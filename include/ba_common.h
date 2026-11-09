#ifndef BA_COMMON
#define BA_COMMON

#include <stdio.h>
#include <time.h>
#include <portaudio.h>


#define true    (1)
#define false   (0)

// #define max(m,n) ((m)>(n)?(m):(n))
// #define min(m,n) ((m)<(n)?(m):(n))

#define BA_INFO    "\033[32m[INFO]\033[0m "
#define BA_WARN    "\033[33m[WARN]\033[0m "
#define BA_ERROR   "\033[31m[ERROR]\033[0m "
#define BA_UNIMPLEMENTED "\e[33m[UNIMPLEMENTED]\e[0m"

#define LINFO(fmt, ...) do {                            \
        time_t rawtime;                                 \
        struct tm timeinfo;                             \
        char timebuffer[32];                            \
        time(&rawtime);                                 \
        localtime_s(&timeinfo, &rawtime);               \
        strftime(timebuffer, sizeof(timebuffer),        \
            "%Y-%m-%d %H:%M:%S", &timeinfo);            \
        fprintf((stderr), BA_INFO "[%s] " fmt "\n",     \
            timebuffer, ##__VA_ARGS__);                 \
    } while (0)
#define LWARN(fmt, ...) do {                            \
        time_t rawtime;                                 \
        struct tm timeinfo;                             \
        char timebuffer[32];                            \
        time(&rawtime);                                 \
        localtime_s(&timeinfo, &rawtime);               \
        strftime(timebuffer, sizeof(timebuffer),        \
            "%Y-%m-%d %H:%M:%S", &timeinfo);            \
        fprintf((stderr), BA_WARN "[%s] " fmt "\n",     \
            timebuffer, ##__VA_ARGS__);                 \
    } while (0);
#define LERROR(fmt, ...) do {                           \
        time_t rawtime;                                 \
        struct tm timeinfo;                             \
        char timebuffer[32];                            \
        time(&rawtime);                                 \
        localtime_s(&timeinfo, &rawtime);               \
        strftime(timebuffer, sizeof(timebuffer),        \
            "%Y-%m-%d %H:%M:%S", &timeinfo);            \
        fprintf((stderr), BA_ERROR "[%s] " fmt "\n",    \
            timebuffer, ##__VA_ARGS__);                 \
    } while (0);

#endif

