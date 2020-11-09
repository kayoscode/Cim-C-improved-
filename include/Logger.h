#ifndef INCLUDE_CPULOGGER_H
#define INCLUDE_CPULOGGER_H

#include "defs.h"
#include <stdarg.h>
#include "../Logger/include/Logger.h"

void init(); 

class Logger{
    public:
        static bylog::Logger debug;

    static void init();
};

#define LINIT() do { \
        Logger::init(); \
    } while (0)

#ifdef DEBUG
#define LTRACE(...) do {  \
        Logger::debug.trace(__VA_ARGS__); \
    } while(0)
#define LINFO(...) do { \
        Logger::debug.info(__VA_ARGS__); \
    } while(0)
#define LWARN(...) do { \
        Logger::debug.warning(__VA_ARGS__); \
    } while(0)
#define LCRITICAL(...) do { \
        Logger::debug.critical(__VA_ARGS__); \
    } while(0)
#define LFATAL(...) do { \
        Logger::debug.fatal(__VA_ARGS__); \
    } while(0)
#endif

#ifdef RELEASE
#define LTRACE
#define LINFO
#define LWARN(...) do { \
        Logger::debug.warning(__VA_ARGS__); \
    } while(0)
#define LCRITICAL(...) do { \
        Logger::debug.critical(__VA_ARGS__); \
    } while(0)
#define LFATAL(...) do { \
        Logger::debug.fatal(__VA_ARGS__); \
    } while(0)
#endif

#ifdef DISTRIBUTION
#define LTRACE
#define LINFO
#define LWARN
#define LCRITICAL(...) do { \
        engine::Logger::debug.critical(__VA_ARGS__); \
    } while(0)
#define LFATAL(...) do { \
        engine::Logger::debug.fatal(__VA_ARGS__); \
    } while(0)
#endif

#endif
