#pragma once

#include <Arduino.h>
#include "EncoderSwitch.h"
#include "display.h"
#include "pll.h"

class Control {
    public:
    void begin(Display *display, Pll *pll);
    void release();
    void tick();
    void encoder_event(uint8_t event);


    protected:
    bool _tune_or_ptt();
    void _every_ms10();
    void _handle_normal_view(uint8_t event);
    uint16_t _step_size;
    uint32_t _tune_freq_hz;
    uint16_t _tx_timer;
    uint8_t _tx_state;
    bool _released;
    bool _is_transmitting;
    bool _tune_mode;
    bool _sideband;
    bool _agc_enabled;

    Display *_display;
    Pll *_pll;
};
