#include <Arduino.h>
#include <Wire.h>
#include <si5351_ek_modified.h>
#include "pll.h"

bool Pll::begin(MbedI2C *i2c_bus, uint32_t ref_freq, uint32_t cf_zero_hz_freq, uint32_t initial_tune_freq, int32_t correction) {
    this->_i2c_bus = i2c_bus;
    this->_tx_state = false;
    this->_correction = correction;
    this->_tune_freq = initial_tune_freq;
    this->_ref_freq = ref_freq;
    this->_cf_zero_hz_freq = cf_zero_hz_freq;
    this->_usb_mode = false;
    this->_cal_mode = false;

    /* Create Si5351 object statically and then save a reference to it */
    static Si5351 _si5351_obj = Si5351(i2c_bus);
    this->_si5351 = &_si5351_obj;

    /* Initialize the SI5351 chip */
    bool res = this->_si5351->init(SI5351_CRYSTAL_LOAD_8PF, ref_freq, correction);
    if(!res) {
        return res;
    }

    /* Set up Si5351 */

    /* Drive strength */
    this->_si5351->drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
    this->_si5351->drive_strength(SI5351_CLK2, SI5351_DRIVE_8MA);

    /* Clock output enables */
    this->_si5351->output_enable(SI5351_CLK0, 1);
    this->_si5351->output_enable(SI5351_CLK2, 1);

    /* Set intial clock frequencies */
    this->_set_clock_freqs();
    return false;
}

void Pll::set_freq(unsigned freq) {
    this->_tune_freq = freq;
    this->_set_clock_freqs();
}

void Pll::set_cal_mode(void) {
    this->_cal_mode = true;
    this->_set_clock_freqs();
}

void Pll::clear_cal_mode(void) {
    this->_cal_mode = false;
    this->_set_clock_freqs();
}

void Pll::set_tx_state(bool state) {
    this->_tx_state = state;
    this->_set_clock_freqs();
}

void Pll::set_usb_mode(bool mode){
    this->_usb_mode = mode;
    this->_set_clock_freqs();
}

void Pll::_set_freq_hz(uint32_t freq_hz, enum si5351_clock output) {
    uint64_t freq = 100ULL * freq_hz;
    this->_si5351->set_freq(freq, output);
}

void Pll::_set_clock_freqs() {
    if(this->_cal_mode) {
        uint32_t freq = 10000000;
        this->_set_freq_hz(freq, SI5351_CLK0);
        this->_set_freq_hz(freq, SI5351_CLK2);
    }


}