#include "EncoderSwitch.h"

#define KNOB_LONG_PRESS_TICKS (KNOB_LONG_PRESS_TIME/2)


void EncoderSwitch::begin(pin_size_t gpio_i, pin_size_t gpio_q, pin_size_t gpio_knob_switch, void(*callback)(uint8_t event)) {
    this->_gpio_i = gpio_i;
    this->_gpio_q = gpio_q;
    this->_gpio_knob_switch = gpio_knob_switch;
    this->_callback = callback;
    this->_knob_state = ENCODER_KNOB_STATE_UP;

    pinMode(this->_gpio_i, INPUT);
    pinMode(this->_gpio_q, INPUT);
    pinMode(this->_gpio_knob_switch, INPUT);
    /* Initialize interal state variables to what the pin states currently are set to */
    this->_previous_encoder_state = this->_raw_read_i_q();
    this->_previous_knob_switch_state = digitalRead(this->_gpio_knob_switch);

}


void EncoderSwitch::tick() {
    this->_raw_read_i_q();
    if(this->_current_encoder_state != this->_previous_encoder_state) {
        /* Gray code decoder. Make a 4 bit word from the current and previous encoder states*/
        uint8_t encoder_bits = (this->_current_encoder_state << 2) | this->_previous_encoder_state;

        switch(encoder_bits){
            case 0b1110: /* Forward */        
                if(this->_callback) 
                    this->_callback(ENCODER_SWITCH_FORWARD);
                break;

            case 0b1011: /* Reverse */
                if(this->_callback) 
                    this->_callback(ENCODER_SWITCH_REVERSE);
                break;

        }
        this->_previous_encoder_state = this->_current_encoder_state;
    }

    /* Check state of knob switch */
    bool current_knob_switch_state = digitalRead(this->_gpio_knob_switch);

    this->_knob_switch_timer++;

    switch(this->_knob_state) {
        case ENCODER_KNOB_STATE_UP:
            if((current_knob_switch_state != this->_previous_knob_switch_state) && (current_knob_switch_state == true) ) {
                this->_knob_switch_timer = 0;
                this->_previous_knob_switch_state = current_knob_switch_state;
                this->_knob_state = ENCODER_KNOB_STATE_DOWN;
            }
            break;

        case ENCODER_KNOB_STATE_DOWN:
            if((current_knob_switch_state != this->_previous_knob_switch_state) && (current_knob_switch_state == false)) {
                this->_previous_knob_switch_state = current_knob_switch_state;
                if(this->_knob_switch_timer >= KNOB_LONG_PRESS_TICKS) { /* if down for gtr. than knob long press ticks */
                    if(this->_callback) {
                        this->_callback(ENCODER_KNOB_SWITCH_PRESSED_LONG);
                    }
                }
                else {
                    if(this->_callback) {
                        this->_callback(ENCODER_KNOB_SWITCH_PRESSED_SHORT);
                    }
                }
                this->_knob_state = ENCODER_KNOB_STATE_UP;
            }
            break;

        default:
            this->_knob_state = ENCODER_KNOB_STATE_UP;
    }
}

uint8_t EncoderSwitch::_raw_read_i_q() {
    /* Read the 2 GPIO pins and combine them into a 2 bit value */
    this->_current_encoder_state = ((digitalRead(this->_gpio_i) << 1) + digitalRead(this->_gpio_q));
    /* Return the current encoder state */
    return this->_current_encoder_state;

}
