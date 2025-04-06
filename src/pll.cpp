#include <Arduino.h>
#include <Wire.h>
#include <si5351_ek_wrapper.h>
#include "pll.h"

void Pll::begin(uint32_t ref_freq, uint32_t cf_zero_hz_freq, uint32_t initial_tune_freq, int32_t correction) {
    this->_tx_state = false;
    this->_correction = correction;
    this->_tune_freq = initial_tune_freq;
    this->_ref_freq = ref_freq;
    this->_cf_zero_hz_freq = cf_zero_hz_freq;
    this->_usb_mode = false;

    this->_si5351.init_sans_wire_begin(SI5351_CRYSTAL_LOAD_8PF, ref_freq, correction);

    this->_set_clock_freqs();
}

void Pll::set_freq(unsigned freq) {
    this->_tune_freq = freq;
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

void Pll::_set_clock_freqs() {

}