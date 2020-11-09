#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include "Logger.h"
#include "Color.h"
#include "Timer.h"

#define VA_INT(args) va_arg(args, int)
#define VA_DOUBLE(args) va_arg(args, double)
#define VA_STRING(args) va_arg(args, char*)

#define OCTAL (unsigned long long)8
#define DECIMAL (unsigned long long)10
#define HEX (unsigned long long)16

#define UPPER_HEX "0123456789ABCDEF"
#define LOWER_HEX "0123456789abcdef"

static void printChar(uint64_t character, bool rightAligned, int spaces, char alignmentFill);

static int getDigitCount(uint64_t integer, int base){
    int count = 0;

    while(integer){
        integer /= base;
        count++;
    }

    return count;
}

static void printOctal(uint64_t integer, bool rightAligned, int spaces, char alignmentFill){
    if(integer == 0) {
        printChar('0', rightAligned, spaces, alignmentFill);
        return;
    }

    int numDigits = getDigitCount(integer, OCTAL);

    if(rightAligned)
        while(spaces > numDigits){
            std::cout << alignmentFill;
            spaces--;
        }

    char* buffer = new char[(unsigned long long)numDigits + 1];
    buffer[numDigits] = 0;

    int index = 0;
    while(integer){
        buffer[index++] = (integer % OCTAL) + '0';
        integer /= (unsigned long long)OCTAL;
    }

    for(int i = (int)numDigits - 1; i >= 0; i--)
        printf("%c", buffer[i]);

    if(!rightAligned)
        while(spaces > numDigits){
            std::cout << alignmentFill;
            spaces--;
        }

	delete[] buffer;
}

static void printHex(uint64_t integer, bool capitalized, bool rightAligned, int spaces, char alignmentFill){
    if(integer == 0) {
        printChar('0', rightAligned, spaces, alignmentFill);
        return;
    }

    int numDigits = getDigitCount(integer, HEX);

    if(rightAligned)
        while(spaces > numDigits){
            std::cout << alignmentFill;
            spaces--;
        }

	char* buffer = new char[(unsigned long long)(numDigits) + 1];
    buffer[numDigits] = 0;

    int index = 0;

    if(capitalized){
        while(integer){
            buffer[index++] = UPPER_HEX[(integer % HEX)];
            integer /= (unsigned long long)HEX;
        }
	}
    else {
        while (integer) {
            buffer[index++] = LOWER_HEX[(integer % HEX)];
            integer /= (unsigned long long)HEX;
        }
    }

    for(int i = numDigits - 1; i >= 0; i--)
        printf("%c", buffer[i]);

    if (!rightAligned) {
        while (spaces > numDigits) {
            std::cout << alignmentFill;
            spaces--;
        }
    }

	delete[] buffer;
}

static void printUnsigned(uint64_t integer, bool rightAligned, int spaces, char alignmentFill){
    std::string value = std::to_string(integer);
    int numDigits = (int)value.size();

    if(rightAligned)
        while(spaces > numDigits){
            std::cout << alignmentFill;
            spaces--;
        }

    std::cout << value;

    if(!rightAligned){
        while(spaces > numDigits){
            std::cout << alignmentFill;
            spaces--;
        }
    }
}

static void printSigned(int integer, bool rightAligned, int spaces, char alignmentFill){
	
	std::string value = std::to_string(integer);
    int numDigits = (int)value.size();

    if(rightAligned)
        while(spaces > numDigits){
            std::cout << alignmentFill;
            spaces--;
        }

    std::cout << value;

    if(!rightAligned){
        while(spaces > numDigits){
            std::cout << alignmentFill;
            spaces--;
        }
    }
}

static void printFloat(double floatingPt, bool rightAligned, int spaces, char alignmentFill){
    std::string value = std::to_string(floatingPt);

    int numDigits = (int)value.size();

    if(rightAligned){
        while(spaces > numDigits){
            std::cout << alignmentFill;
            spaces--;
        }
    }

    std::cout << value;

    if(!rightAligned){
        while(spaces > numDigits){
            std::cout << alignmentFill;
            spaces--;
        }
    }
}

static void printChar(uint64_t character, bool rightAligned, int spaces, char alignmentFill){
    int numDigits = 1;

    if(rightAligned)
        while(spaces > numDigits){
            std::cout << alignmentFill;
            spaces--;
        }

    printf("%c", (char)character);

    if(!rightAligned)
        while(spaces > numDigits){
            std::cout << alignmentFill;
            spaces--;
        }
}

static void printString(const char* str, bool rightAligned, int spaces, char alignmentFill){

    int numDigits = (int)strlen(str);

    if(rightAligned)
        while(spaces > numDigits){
            std::cout << alignmentFill;
            spaces--;
        }

    printf("%s", str);

    if(!rightAligned)
        while(spaces > numDigits){
            std::cout << alignmentFill;
            spaces--;
        }
}

static inline const char* parseArguments(const char* format, bool& rightAligned, char& character, int& spaces){
    if(*format == '<'){
        rightAligned = false;
        format++;

        character = *format;
        format++;
    }
    else{
        format++;
        rightAligned = true;

        character = *format;
        format++;
    }

    while(bylog::isNumber[(int)*format]){
        spaces *= 10;
        spaces += *format - '0';
        format++;
    }
    
    return format;
}

static const char* parseFormat(const char* format, va_list& arguments){
    char type = *format++;
    bool rightAligned = false;
    char character = ' ';
    int spaces = 0;

    parseArguments(format, rightAligned, character, spaces);
    while(*format && *format != '}') format++;

    switch(type){
        case 'O':
        case 'o':
            printOctal(va_arg(arguments, uint32_t), rightAligned, spaces, character);
            break;
        case 'x':
            printHex(va_arg(arguments, uint32_t), false, rightAligned, spaces, character);
            break;
        case 'X':
            printHex(va_arg(arguments, uint32_t), true, rightAligned, spaces, character);
            break;
        case 'U':
        case 'u':
            printUnsigned(va_arg(arguments, uint32_t), rightAligned, spaces, character);
            break;
        case 'D':
        case 'd':
            printSigned(va_arg(arguments, uint32_t), rightAligned, spaces, character);
            break;
        case 'F':
        case 'f':
            printFloat(va_arg(arguments, double), rightAligned, spaces, character);
            break;
        case 'C':
        case 'c':
            printChar(va_arg(arguments, uint32_t), rightAligned, spaces, character);
            break;
        case 'S':
        case 's':
            printString(va_arg(arguments, const char*), rightAligned, spaces, character);
            break;
    }

    return format;
}

namespace bylog{
    bool isNumber[256] = { false };

    void init(){
        for(int i = '0'; i <= '9'; i++){
            isNumber[i] = true;
        }
    }

    Logger::Logger(const std::string& name, const std::string& msg, Level level)
        :name(name),
        msg(msg),
        level(level)
    {
        totalTime.reset();
        elapsedTime.reset();
        init();
    }

    Logger::~Logger(){
    }

    void Logger::printMSG(int count){
        print("[E{d}|T{d}|#{d}] {s}: ", elapsedTime.milliseconds(), totalTime.milliseconds(), count, name.c_str());
        elapsedTime.reset();
    }

    void Logger::print(const char* format, ...){
        va_list args;
        va_start(args, format);
        printFormat(format, args);
        va_end(args);
    }

    void Logger::printFormat(const char* format, va_list& args){
        while(*format){
            if(*format == '{'){
                format++;

                if(*format != '{'){
                    format = parseFormat(format, args);
                    continue;
                }
            }
            else if(*format == '}'){
                format++;

                if(*format == '}')
                    printf("%c", *format);

                continue;
            }

            printf("%c", *format);
            format++;
        }
    }

    void Logger::trace(const char* format, ...){
        counts[(int)Level::TRACE]++;
        if(level < Level::TRACE)
            return;

        SET_COLOR_GREEN;
        SET_COLOR_BOLD;
        printMSG(counts[(int)Level::TRACE]);
        COLOR_RESET;

        SET_COLOR_GREEN;

        va_list args;

        va_start(args, format);
        printFormat(format, args);
        va_end(args);

        std::cout << "\n";

        COLOR_RESET;
    }

    void Logger::info(const char* format, ...){
        counts[(int)Level::INFO]++;
        if(level < Level::INFO)
            return;

        SET_COLOR_BLUE;
        SET_COLOR_BOLD;
        printMSG(counts[(int)Level::INFO]);
        COLOR_RESET;

        SET_COLOR_BLUE;

        va_list args;
        va_start(args, format);
        printFormat(format, args);
        va_end(args);

        std::cout << "\n";

        COLOR_RESET;
    }

    void Logger::warning(const char* format, ...){
        counts[(int)Level::WARNING]++;
        if(level < Level::WARNING)
            return;

        SET_COLOR_YELLOW;
        SET_COLOR_BOLD;
        printMSG(counts[(int)Level::WARNING]);
        COLOR_RESET;

        SET_COLOR_YELLOW;

        va_list args;
        va_start(args, format);
        printFormat(format, args);
        va_end(args);

        std::cout << "\n";

        COLOR_RESET;
    }

    void Logger::critical(const char* format, ...){
        counts[(int)Level::CRITICAL]++;
        if(level < Level::CRITICAL)
            return;

        SET_COLOR_RED;
        SET_COLOR_BOLD;
        printMSG(counts[(int)Level::CRITICAL]);
        COLOR_RESET;

        SET_COLOR_RED;

        va_list args;
        va_start(args, format);
        printFormat(format, args);
        va_end(args);

        std::cout << "\n";

        COLOR_RESET;
    }

    void Logger::fatal(const char* format, ...){
        counts[(int)Level::FATAL]++;
        if(level < Level::FATAL)
            return;

        SET_COLOR_RED;
        SET_COLOR_BOLD;
        SET_COLOR_UNDERLINE;
        printMSG(counts[(int)Level::FATAL]);

        va_list args;
        va_start(args, format);
        printFormat(format, args);
        va_end(args);

        std::cout << "\n";

        COLOR_RESET;
    }

    void Logger::setLevel(Level l){
        this->level = l;
    }

    void Logger::setName(const std::string& name){
        this->name = name;
    }   

    void Logger::setMsg(const std::string& msg){
        this->msg = msg;
    }

    Level Logger::getLevel(){
        return this->level;
    }

    std::string Logger::getName(){
        return this->name;
    }

}
