#pragma once
#include "si5351_ek_modified.h"

class Pll {
    public:
    bool begin(MbedI2C *i2c_bus, uint32_t ref_freq, uint32_t cf_zero_hz_freq, uint32_t initial_tune_freq, int32_t correction);
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
    MbedI2C *_i2c_bus;
    Si5351 *_si5351;



};