#ifndef INCLUDE_LOGGER_H
#define INCLUDE_LOGGER_H

#include <string>
#include <ostream>
#include <sstream>
#include <iostream>
#include <vector>
#include <stdarg.h>
#include "Timer.h"

namespace bylog{
    extern bool isNumber[256];

    void init();

    enum class Level{
        FATAL,
        CRITICAL,
        WARNING,
        INFO,
        TRACE,
        LEVEL_COUNT
    };

    class Logger{
        public:
            Logger(const std::string& name = "Unnamed logger", 
                const std::string& msg = "",
                Level l = Level::TRACE);

            ~Logger();

            void trace(const char* format, ...);
            void info(const char* format, ...);
            void warning(const char* format, ...);
            void critical(const char* format, ...);
            void fatal(const char* format, ...);

            void setLevel(Level l);
            void setName(const std::string& name);
            void setMsg(const std::string& msg);
            Level getLevel();
            std::string getName();

        private:
            void printMSG(int count);
            void printFormat(const char* format, va_list& args);
            void print(const char* format, ...);

            std::string name;
            std::string msg;

            Level level;

            Timer totalTime;
            Timer elapsedTime;

            int counts[(unsigned long long)Level::LEVEL_COUNT] = { 0 };
    };
}

#endif
