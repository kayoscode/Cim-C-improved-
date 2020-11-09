
#ifndef INCLUDE_REGISTER_H
#define INCLUDE_REGISTER_H

#include "defs.h"

struct Register {
    public:
        Register(uint32_t value = 0) 
            :registerName("NONAME"), value(value)
        { }

        void operator=(uint32_t value) {
            this->value = value;
        }

        operator uint32_t() {
            return this->value;
        }

        inline uint16_t getHigh() {
            uint16_t* high = ((uint16_t*)&value);

            #ifdef DEBUG_REGISTERS
            LTRACE("Read[high] Register {s}: {d} {x}", registerName.c_str(), *high, *high);
            #endif

            return *high;
        }

        inline uint16_t getLow() {
            uint16_t* low = ((uint16_t*)&value + 1);

            #ifdef DEBUG_REGISTERS
            LTRACE("Read[low] Register {s}: {d} {x}", registerName.c_str(), *low, *low);
            #endif

            return *low;
        }

        inline void setHigh(uint16_t v) {
            #ifdef DEBUG_REGISTERS
            LTRACE("Write[high] Register {s}: {d} {x}", registerName.c_str(), v, v);
            #endif

            uint16_t* high = ((uint16_t*)&value);
            *high = v;
        }

        inline void setLow(uint16_t v) {
            #ifdef DEBUG_REGISTERS
            LTRACE("Write[low] Register {s}: {d} {x}", registerName.c_str(), v, v);
            #endif

            uint16_t* low = ((uint16_t*)&value + 1);
            *low = v;
        }

        inline void setValue(uint32_t value) {
            #ifdef DEBUG_REGISTERS
            LTRACE("Write[full] Register {s}: {d} {x}", registerName.c_str(), value, value);
            #endif

            this->value = value;
        }

        inline uint32_t getValue() {
            #ifdef DEBUG_REGISTERS
            LTRACE("Read[full] Register {s}: {d} {x}", registerName.c_str(), value, value);
            #endif

            return value;
        }

        inline void setRegisterName(const std::string& name) {
            this->registerName = name;
        }

        inline std::string getRegisterName() {
            return this->registerName;
        }

        uint32_t value;
        std::string registerName;
};

struct FRegister {
    public: 
        FRegister(float value = 0) 
            :value(value)
        {

        }

        void operator=(float value) {
            this->value = value;
        }

        operator float() {
            return this->value;
        }

        inline void setRegisterName(const std::string& name) {
            this->registerName = name;
        }

        inline std::string getRegisterName() {
            return this->registerName;
        }

        inline float getValue() {
            #ifdef DEBUG_REGISTERS
            LTRACE("Read[full] Register {s}: {f}", registerName.c_str(), value);
            #endif

            return value;
        }

        inline void setValue(float value) {
            #ifdef DEBUG_REGISTERS
            LTRACE("Write[full] Register {s}: {f}", registerName.c_str(), value);
            #endif

            this->value = value;
        }

        float value;
        std::string registerName;
};

#endif