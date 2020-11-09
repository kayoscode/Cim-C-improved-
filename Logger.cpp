#include "Logger.h"

bylog::Logger Logger::debug;

void Logger::init(){
    Logger::debug.setName("Debug");
}