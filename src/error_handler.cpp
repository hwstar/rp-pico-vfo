#include <Arduino.h>
#include "error_handler.h"
#include "display.h"

extern Display display;

void Error_Handler::post(uint32_t error_code) {
    display.clear_view(VIEW_ERROR);
    display.set_current_view(VIEW_ERROR);
    display.printf(0, 0, 6, "ERROR:");
    switch(error_code) {
        case ERROR_NO_BANDS_ENABLED:
            this->_hard_error = true;
            display.printf(1, 0, 14, "NO BAND ENA.");
            break;

        case ERROR_MISSING_SI5351:
            this->_hard_error = true;
            display.printf(1, 0, 14, "NO SI5351");
            break;

        case ERROR_MISSING_EEPROM:
            this->_hard_error = true;
            display.printf(1, 0, 14, "NO EEPROM");
            break;
        
        default:
            this->_hard_error = true;
            display.printf(1, 0, 14, "UNHANDLED ERR");
            break;

    }
}

bool Error_Handler::hard_error_posted() {
    return this-> _hard_error;
}


