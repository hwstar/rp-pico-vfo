#pragma once
#include "si5351_ek_modified.h"

class Pll {
    public:
    /* Set up the PLL */
    bool begin(MbedI2C *i2c_bus, uint32_t ref_freq, uint32_t cf_zero_hz_freq, uint32_t initial_tune_freq, int32_t correction);
    /* Get frequency pll is tuned to*/
    uint32_t get_freq();
    /* Set pll frequency */
    void set_freq(unsigned freq);
    /* Set correction */
    void set_correction(int32_t corr);
    /* Set the TX/RX state (TX = true, RX = False)*/
    void set_tx_state(bool state);
    /* Set USB mode. (true = USB, false = LSB)*/
    void set_usb_mode(bool mode);
    /* Retreive operating mode (LSB/USB)*/
    bool get_usb_mode();
    /* Set calibration mode  (true = Calibration, false = Normal)*/
    void cal_mode(bool mode);
   

    protected:
    void _set_freq_hz(uint32_t freq_hz, enum si5351_clock output);
    void _set_clock_freqs();

 
    bool _tx_state;
    bool _usb_mode;
    bool _cal_mode;
    int32_t _correction;
    uint32_t _cf_zero_hz_freq;
    uint32_t _ref_freq;
    uint32_t _tune_freq;
    MbedI2C *_i2c_bus;
    Si5351 *_si5351;



};