#pragma once

#include <Arduino.h>
#include "EncoderSwitch.h"
#include "menu.h"
#include "display.h"
#include "pll.h"
#include "config_default.h"
#include "persistent_storage.h"
#include "config_structure.h"


#define BAND_TABLE_NAME_SIZE 8
#define NUMBER_OF_STEP_SIZES 5


class Control {
    public:
    void begin();
    void release();
    void tick();
    void encoder_event(uint8_t event);


    protected:
    void _every_ms10();
    void _handle_normal_view(uint8_t event);
    void _handle_menu_view(uint8_t event);

    Band_Info _band_info_table[MAX_NUM_OF_BANDS];
    const uint16_t _step_size_table[NUMBER_OF_STEP_SIZES] = {10, 100, 500, 1000, 10000};
    uint32_t _tune_freq_hz;
    uint16_t _tx_timer;
    uint8_t _tx_state;
    uint8_t _current_band;
    uint8_t _step_size_index;
    bool _released;
    bool _is_transmitting;
    bool _tune_mode;
    bool _agc_enabled;

    
};
