#include "control.h"
#include "gpio.h"
#include "config_default.h"
#include "persistent_storage.h"
#include "config_keys.h"

enum {TX_IDLE=0, TX_MUTE_WAIT, TX_PTT, TX_UNMUTE_WAIT};


void Control::begin(Display *display, Pll *pll, PersistentStorage *ps) {
    this->_display = display;
    this->_pll = pll;
    this->_ps = ps;

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

     /* Initialize defaults */

    this->_agc_enabled = true;
   

  
   
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
    // Initialization
    this->_step_size_index = CONFIG_DEFAULT_STEP_SIZE_INDEX;
    this->_current_band = CONFIG_DEFAULT_INITIAL_BAND_INDEX;

    // Initialize band table
    #ifdef CONFIG_DEFAULT_BAND_NAME_0
    strncpy(this->_band_table[0].name, CONFIG_DEFAULT_BAND_NAME_0, BAND_TABLE_NAME_SIZE);
    this->_band_table[0].lower_limit = CONFIG_DEFAULT_BAND_LOWER_LIMIT_HZ_0;
    this->_band_table[0].upper_limit = CONFIG_DEFAULT_BAND_UPPER_LIMIT_HZ_0;
    this->_ps->read(KEY_INIT_FREQ, &this->_band_table[0].tune_freq_hz);
    this->_band_table[0].sideband = CONFIG_DEFAULT_BAND_SIDEBAND_0;
    #endif

    // Setup
    this->_pll->set_freq(this->_band_table[this->_current_band].tune_freq_hz);
    this->_display->update_freq(this->_band_table[this->_current_band].tune_freq_hz);
    this->_display->update_tx(this->_is_transmitting, this->_tune_mode);
    this->_display->update_sideband(this->_band_table[this->_current_band].sideband);
    this->_display->update_band_name(this->_band_table[this->_current_band].name);
    this->_display->update_agc(this->_agc_enabled);
    this->_display->update_tune_step_size(this->_step_size_table[this->_step_size_index]);
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
    bool ptt = digitalRead(GPIO_PTT_BUTTON);
    bool tb = digitalRead(GPIO_TUNE_BUTTON);
    /* PTT sequencer*/
    switch(this->_tx_state) {
        case TX_IDLE:
            this->_is_transmitting = false;
            if(ptt || tb) {
                /* User keyed */
                if(tb) {
                    /* If user pressed the tune button, then we need to set tune out high */
                    digitalWrite(GPIO_TUNE_OUT, true);
                }
                this->_is_transmitting = true;
                digitalWrite(GPIO_MUTE_OUT, true);
                /* Tell PLL to switch to transmit injection frequencies */
                this->_pll->set_tx_state(true);
                /* Update status on display */
                this->_display->update_tx(ptt, tb);
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
            if(!(ptt || tb)) { 
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
                /* Update status on display */
                this->_display->update_tx(false);
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

void Control::_handle_normal_view(uint8_t event) {
    uint16_t step_size = this->_step_size_table[this->_step_size_index];
    band_table *current_band = &this->_band_table[this->_current_band];
    switch(event) {
       
        case ENCODER_SWITCH_FORWARD: /* Rotation in forward direction */
            /* Test band upper limit */
            if(current_band->tune_freq_hz + step_size <= current_band->upper_limit) {
                /* Will still be in band, so increase tune freq by the step size and update everything */
                current_band->tune_freq_hz += step_size;
                this->_display->update_freq(current_band->tune_freq_hz);
                this->_pll->set_freq(current_band->tune_freq_hz);
            }

            break;

        case ENCODER_SWITCH_REVERSE: /* Rotation in reverse direction */
            /* Test band lower limit */
            if(current_band->tune_freq_hz - step_size >= current_band->lower_limit) {
                /* Will still be in band, so decrease tune freq by the step size and update everything */
                current_band->tune_freq_hz -= step_size;
                this->_display->update_freq(current_band->tune_freq_hz);
                this->_pll->set_freq(current_band->tune_freq_hz);
            }
            break;

        case ENCODER_KNOB_SWITCH_PRESSED_SHORT: /* Short duration encoder knob press */
            /* Change frequency step */
            this->_step_size_index++;
            if(this->_step_size_index >= NUMBER_OF_STEP_SIZES) {
                this->_step_size_index = 0;
            }
            this->_display->update_tune_step_size(this->_step_size_table[this->_step_size_index]);
            break;
            

        default:
            break;

    }


}