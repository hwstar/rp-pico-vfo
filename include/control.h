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
    uint32_t get_channel_count();
    bool get_channelized() {
        return this->_is_channelized;
    }
    void set_channelized(bool state) {
        this->_is_channelized = state;
    }
    void update_channel_freq(); 

    protected:
    void _every_ms10();
    void _handle_normal_view(uint8_t event);
    void _handle_menu_view(uint8_t event);
    Channel *_get_channel_pointer(uint32_t channel_number);
    uint32_t _next_channel();
    uint32_t _prev_channel();
    Band_Info _band_info_table[CONFIG_MAX_NUM_OF_BANDS];
    const uint16_t _step_size_table[NUMBER_OF_STEP_SIZES] = {10, 100, 500, 1000, 10000};
    uint32_t _is_channelized;
    uint32_t _channel_number;
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
