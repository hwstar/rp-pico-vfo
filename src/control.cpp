#include "control.h"
#include "gpio.h"
#include "config_default.h"

enum {TX_IDLE=0, TX_MUTE_WAIT, TX_PTT, TX_UNMUTE_WAIT};

void Control::begin(Display *display, Pll *pll) {
    this->_display = display;
    this->_pll = pll;

    /* GPIO Initialization */

    pinMode(GPIO_PTT_BUTTON, INPUT);
    pinMode(GPIO_TUNE_BUTTON, INPUT);
    pinMode(GPIO_ENCODER_SWITCH, INPUT);

    pinMode(GPIO_PTT_OUT, OUTPUT);
    digitalWrite(GPIO_PTT_OUT, false);

    pinMode(GPIO_TUNE_OUT, OUTPUT);
    digitalWrite(GPIO_TUNE_OUT, false);

    pinMode(GPIO_MUTE_OUT, OUTPUT);
    digitalWrite(GPIO_MUTE_OUT, false);

    pinMode(GPIO_AGC_DISABLE, OUTPUT);
    digitalWrite(GPIO_AGC_DISABLE, false);

    this->_agc_enabled = true;
    this->_tune_freq_hz = CONFIG_DEFAULT_TUNE_FREQ;
    this->_step_size = CONFIG_DEFAULT_STEP_SIZE;

}


void Control::tick() {
    static uint8_t counter_ms10 = 0;

    /* We only need to process TX state changes every 10 mS */
    if(!counter_ms10) {
        this->_every_ms10();
    }
    counter_ms10++;
    if(counter_ms10 >= 5)
        counter_ms10 = 0;


}


void Control::release() {
    this->_pll->set_freq(this->_tune_freq_hz);
    this->_display->update_freq(this->_tune_freq_hz);
    this->_display->update_tx(this->_is_transmitting, this->_tune_mode);
    this->_display->update_sideband(this->_sideband);
    this->_display->update_agc(this->_agc_enabled);
    this->_released = true;
   
}

void Control::encoder_event(uint8_t event) {

    /* Ignore encoder events if not released or we are transmitting */
    if(!this->_released || this->_is_transmitting) {
        return;
    }
    /*
    * Select handler based on view
    */
    switch(this->_display->get_current_view()) {
        case VIEW_NORMAL:
            this->_handle_normal_view(event);
            break;

        case VIEW_ERROR:
        case VIEW_MENU:
        case VIEW_SPECIAL:
        default:
            break;

    }

}

void Control::_every_ms10() {
    /* Ignore ticks if not released */
    if(!this->_released) {
        return;
    }
    /* PTT sequencer*/
    switch(this->_tx_state) {
        case TX_IDLE:
            this->_is_transmitting = false;
            if(this->_tune_or_ptt()) {
                /* User keyed */
                if(digitalRead(GPIO_TUNE_BUTTON)) {
                    /* If user pressed the tune button, then we need to set tune out high*/
                    digitalWrite(GPIO_TUNE_OUT, true);
                }
                this->_is_transmitting = true;
                digitalWrite(GPIO_MUTE_OUT, true);
                /* Tell PLL to switch to transmit injection frequencies */
                this->_pll->set_tx_state(true);
                this->_tx_timer = 0;
                this->_tx_state = TX_MUTE_WAIT;
            }
            break;

        case TX_MUTE_WAIT:
            if(this->_tx_timer >= 10) {
                /* Set ptt out high to tell the RF boards to reconfigure for transmit */
                digitalWrite(GPIO_PTT_OUT, true);
                this->_tx_state = TX_PTT;
            }
            break;

        case TX_PTT:
            if(!this->_tune_or_ptt()) { 
                this->_tx_timer = 0; /* User unkeyed */
                digitalWrite(GPIO_TUNE_OUT, false);
                digitalWrite(GPIO_PTT_OUT, false);
                /* Tell PLL to switch to receive injection frequencies */
                this->_pll->set_tx_state(false);
                this->_tx_state = TX_UNMUTE_WAIT;
            }
            break;

        case TX_UNMUTE_WAIT:
            if(this->_tx_timer >= 10) {
                this->_is_transmitting = false;
                /* Unmute RX audio */
                digitalWrite(GPIO_MUTE_OUT, false);
                this->_tx_state = TX_IDLE;
            }
            break;

        default:
            this->_tx_state = TX_IDLE;
            break;
   }
   /* This variable keeps track of the number of ticks since being zeroed. Used by the TX sequencer */
   this->_tx_timer++;

}

bool Control::_tune_or_ptt() {
    /* Return true if PTT or TUNE switches are closed */
    return ( digitalRead(GPIO_PTT_BUTTON) || digitalRead(GPIO_TUNE_BUTTON));
}


void Control::_handle_normal_view(uint8_t event) {
    switch(event) {
        case ENCODER_SWITCH_FORWARD:
            this->_tune_freq_hz += this->_step_size;
            this->_display->update_freq(this->_tune_freq_hz);
            this->_pll->set_freq(this->_tune_freq_hz);

            break;

        case ENCODER_SWITCH_REVERSE:
            this->_tune_freq_hz -= this->_step_size;
            this->_display->update_freq(this->_tune_freq_hz);
            this->_pll->set_freq(this->_tune_freq_hz);


            break;

        default:
            break;

    }


}