#pragma once
#include "si5351_ek_wrapper.h"

class Pll {
    public:
    void begin( uint32_t ref_freq, uint32_t cf_zero_hz_freq, uint32_t initial_tune_freq, int32_t correction);
    void set_freq(unsigned freq);
    void set_tx_state(bool state);
    void set_usb_mode(bool mode);

    protected:
    void _set_clock_freqs();

    bool _tx_state;
    bool _usb_mode;
    int32_t _correction;
    uint32_t _cf_zero_hz_freq;
    uint32_t _ref_freq;
    uint32_t _tune_freq;
    SI5351_EK_WRAPPER _si5351;



};