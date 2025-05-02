#pragma once

#include <Arduino.h>

#define ENCODER_SWITCH_FORWARD 0x01
#define ENCODER_SWITCH_REVERSE 0x02
#define ENCODER_KNOB_SWITCH_PRESSED_SHORT 0x04
#define ENCODER_KNOB_SWITCH_PRESSED_LONG 0x08

#define ENCODER_KNOB_STATE_UP 0
#define ENCODER_KNOB_STATE_DOWN 1

#define KNOB_LONG_PRESS_TIME 1000 // Number of milliseconds to recognize a long press of the tuning knob


class EncoderSwitch {
    public:
    void begin(pin_size_t gpio_i, pin_size_t gpio_q, pin_size_t gpio_knob_switch,  void(*callback)(uint8_t event));
    void tick();

    protected:
    uint8_t _raw_read_i_q();

    uint8_t _current_encoder_state;
    uint8_t _previous_encoder_state;
    bool _previous_knob_switch_state;
    uint8_t _knob_state;
    uint32_t _knob_switch_timer;

    pin_size_t _gpio_i;
    pin_size_t _gpio_q;
    pin_size_t _gpio_knob_switch;
    void(*_callback)(uint8_t event);
    

};