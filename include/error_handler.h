#pragma once
#include <Arduino.h>

#define ERROR_NO_BANDS_ENABLED 1
#define ERROR_MISSING_SI5351 2
#define ERROR_MISSING_EEPROM 3

class Error_Handler {
    public:
    void post(uint32_t error_code);
    bool hard_error_posted();

    protected:
    bool _hard_error;
};

