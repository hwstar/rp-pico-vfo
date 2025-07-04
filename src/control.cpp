#include "error_handler.h"
#include "control.h"
#include "display.h"
#include "menu.h"
#include "gpio.h"
#include "persistent_storage.h"
#include "config_default.h"
#include "config_keys.h"
#include "config_structure.h"


// Enums

enum {TX_IDLE=0, TX_MUTE_WAIT, TX_PTT, TX_UNMUTE_WAIT};

// External object access needed for menu procedural functions 
extern Control control;
extern Pll pll;
extern Display display;
extern PersistentStorage ps;
extern Error_Handler error_handler;

// Variables
bool agc_state = CONFIG_INITIAL_AGC; // Todo move to class 
int32_t correction_ppb;
// Objects
Menu menu;



// ForwBand_Info *band_info = (Band_Info *) ps.get_value_pointer(KEY_BAND_INFO_TABLE);ard declarations
void menu_item_mode_on_entry();
void menu_item_mode_action(uint8_t event);
bool menu_item_mode_on_exit(bool confirm);
void menu_item_agc_on_entry();
void menu_item_agc_action(uint8_t event);
bool menu_item_agc_on_exit(bool confirm);
void menu_item_cal_on_entry();
void menu_item_vfomem_action(uint8_t event);
bool menu_item_vfomem_on_exit(bool confirm);
void menu_item_vfomem_on_entry();
void menu_item_cal_action(uint8_t event);
bool menu_item_cal_on_exit(bool confirm);


void draw_menu(const char *line1, const char *line2);
void at_menu_exit();

// Menu system data structures

const menu_item item_top_mode = {"MODE", MENU_ITEM_TYPE_ACTION, NULL, menu_item_mode_on_entry, menu_item_mode_action, menu_item_mode_on_exit};
const menu_item item_top_cal = {"CAL", MENU_ITEM_TYPE_ACTION, NULL, menu_item_cal_on_entry, menu_item_cal_action, menu_item_cal_on_exit};
const menu_item item_top_agc = {"AGC", MENU_ITEM_TYPE_ACTION, NULL, menu_item_agc_on_entry, menu_item_agc_action, menu_item_agc_on_exit};
const menu_item item_top_vfomem = {"VFOMEM", MENU_ITEM_TYPE_ACTION, NULL, menu_item_vfomem_on_entry, menu_item_vfomem_action, menu_item_vfomem_on_exit};
const menu_level top_mode = {4,"*** Top Menu ***",{&item_top_vfomem, &item_top_mode, &item_top_agc, &item_top_cal}};
const menu_level top_no_mode = {3,"*** Top Menu ***",{&item_top_vfomem, &item_top_agc, &item_top_cal}};



// Menu initialization

void menu_init(const menu_level *top) {
    // Called from mainBand_Info *band_info = (Band_Info *) ps.get_value_pointer(KEY_BAND_INFO_TABLE);.cpp to initialize the menu object
    menu.init(top, draw_menu, at_menu_exit);
}


// Handler functions for menu system
void menu_item_mode_on_entry() {
    // Called when mode is selected by the user
    // Show mode setting

    const char *setting = pll.get_usb_mode() ? "USB" : "LSB";
    menu.draw_item_value(setting);
}

void menu_item_mode_action(uint8_t event) {
    // Called when therBand_Info *band_info = (Band_Info *) ps.get_value_pointer(KEY_BAND_INFO_TABLE);e is CW or CCW rotation with mode selected

    pll.set_usb_mode(!pll.get_usb_mode());
    bool mode = pll.get_usb_mode();
    const char *setting = mode ? "USB" : "LSB";
    display.update_sideband(mode);
    menu.draw_item_value(setting);
}

bool menu_item_mode_on_exit(bool confirm) {
    // Called when the user selects a value for mode, or aborts
    return true;
}

void menu_item_agc_on_entry() {
    // Called when agc is selected by the user
    // Show AGC setting
    const char *setting = agc_state ? "ON" : "OFF";
    menu.draw_item_value(setting);
    

}

void menu_item_agc_action(uint8_t event) {
    // Called when there is CW or CCW rotation with agc selected
    agc_state = !agc_state;
    digitalWrite(GPIO_AGC_DISABLE, !agc_state);
    display.update_agc(agc_state);
    const char *setting = agc_state ? "ON" : "OFF";
    menu.draw_item_value(setting);


}

bool menu_item_agc_on_exit(bool confirm) {
    // Called when the user selects a value for agc, or aborts
    return true;
}


void menu_item_vfomem_on_entry() {
    // Called when vfomem is selected by the user
    // Show VFOMEM setting
    const char *setting = control.get_channelized() ? "MEM" : "VFO";
    menu.draw_item_value(setting);
}

void menu_item_vfomem_action(uint8_t event) {
    // Called when there is CW or CCW rotation with agc selected
    // Only execute this if there is at least 1 channel defined
    if(control.get_channel_count()) {
        control.set_channelized(!control.get_channelized());
        control.update_channel_freq();
        const char *setting = control.get_channelized() ? "MEM" : "VFO";
        menu.draw_item_value(setting);
    }
}

bool menu_item_vfomem_on_exit(bool confirm) {
    // Called when the user selects a value for vfomem, or aborts
     return true;
}

void draw_menu(const char *line1, const char *line2) {
    // Called by menu object to update the display when the user interacts with the knob
    display.printf(0, 0, 16, "%-16s", line1);
    display.printf(1, 0, 16, "%-16s", line2);
}

void menu_item_cal_update_display(int32_t corr_ppb) {
    // Update display with ppb value
    char ppb_str[9];
    snprintf(ppb_str, 9, "%-8ld", corr_ppb);
    menu.draw_item_value(ppb_str);
}


void menu_item_cal_on_entry() {
    // Called when cal is selected by the user
    ps.read(KEY_CALIB, &correction_ppb);
    menu_item_cal_update_display(correction_ppb);
    pll.cal_mode(true);
}



void menu_item_cal_action(uint8_t event) {
    // Called when there is CW or CCW rotation with CAL selected

    if(event == MENU_KNOB_CW) {
        correction_ppb+=10;
    }
    else {
        correction_ppb-=10;
    }
    pll.set_correction(correction_ppb);
    menu_item_cal_update_display(correction_ppb);

}

bool menu_item_cal_on_exit(bool confirm) {
    // Called when the user selects a value for cal, or aborts
    if(confirm) {
        ps.write(KEY_CALIB, correction_ppb);
    }
    pll.cal_mode(false);
    return true;
}

void at_menu_exit() {
    // Called when the menu system is exited by the user
    ps.commit();
}

// Control class

void Control::begin() {
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
    uint8_t band;
    Band_Info *band_info = (Band_Info *) ps.get_value_pointer(KEY_BAND_INFO_TABLE);
    
    // Find first enabled band
    for(band = 0; band < CONFIG_MAX_NUM_OF_BANDS; band++) {
        if(band_info->flags & BAND_FLAG_ACTIVE) {
            break;
        }
    }
    if(band == CONFIG_CHANNEL_NAME_SIZE) {
        error_handler.post(ERROR_NO_BANDS_ENABLED);
        return;
    }
    // Select the first channel number
    this->_channel_number = 0;
    // Count the valid memory channels
    uint32_t channel_count = this->get_channel_count();
    // Select the correct band table
    this->_current_band = band;
    band_info = band_info + band;
    this->_step_size_index = (band_info->flags & BAND_VFO_STEP_500HZ) ? 2 : 3;

    // Determine whether to power up in VFO or memory mode
    
    if(band_info->flags & BAND_FLAG_CHANNELIZED) {
        /* Is in memory mode */
        /* Set up for channelized operation */
        if(channel_count) {
            Channel *c = this->_get_channel_pointer(this->_channel_number);
            this->_is_channelized = true;
            this->_tune_freq_hz = c->freq;
        }
        else {
            Serial1.println("Warning: memory mode selected, but no channels are programmed. Reverting to VFO mode ");
            goto revert_to_vfo_mode;
        }
    }
    else {
revert_to_vfo_mode:
        /* Is in VFO mode */
        /* If channel 1 is programmed, use that as the power on frequency */
        this->_is_channelized = false;
        Channel *c = this->_get_channel_pointer(0);
        if (c->freq) {
            this->_tune_freq_hz = c->freq;
        }
        else {
            //* Channel 1 isn't programmed, use the midpoint of the band as the power up frequency.
            this->_tune_freq_hz = ((band_info->upper_limit + band_info->lower_limit) / 2); // Set freq to band midpoint
            this->_tune_freq_hz = this->_tune_freq_hz - (this->_tune_freq_hz % 1000); // Select and even kHz boundary
        }
    }


   

    // Setup
    this->update_channel_freq();
    pll.set_freq(this->_tune_freq_hz);
   
    display.update_tx(this->_is_transmitting, this->_tune_mode);
   
    bool mode = (band_info->flags & BAND_FLAG_MODE_USB) > 0;
    pll.set_usb_mode(mode);
    display.update_sideband(mode);
    
    display.update_band_name(band_info->name);
    display.update_agc(this->_agc_enabled);
    display.update_tune_step_size(this->_step_size_table[this->_step_size_index]);


    // Indicate released
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
    switch(display.get_current_view()) {
        case VIEW_NORMAL:
            this->_handle_normal_view(event);
            break;

        case VIEW_ERROR:
            break;

        case VIEW_MENU:
            this->_handle_menu_view(event);

        case VIEW_SPECIAL:
            break;

        default:
            break;

    }

}

void Control::update_channel_freq() {
    Band_Info *band_info = (Band_Info *) ps.get_value_pointer(KEY_BAND_INFO_TABLE);
    if(this->_is_channelized) {
        display.update_freq((char *)((this->_get_channel_pointer(this->_channel_number))->name));
        this->_tune_freq_hz = this->_get_channel_pointer(this->_channel_number)->freq;
    }
    else {
       
        display.update_freq(this->_tune_freq_hz, band_info[this->_current_band].freq_offset_display);
    }
    pll.set_freq(this->_tune_freq_hz);
}

uint32_t Control::get_channel_count() {
    uint32_t count = 0;
    for(uint32_t i = 0; i < CONFIG_MAX_NUM_CHANNELS; i++) {
        Channel *c = this->_get_channel_pointer(i);
        if(c->freq) {
            count++;
        } 
    }
    return count;
}

Channel *Control::_get_channel_pointer(uint32_t channel_number) {
    return (((Channel_Info *) ps.get_value_pointer(KEY_CHANNEL_INFO))->channel+channel_number);
}


uint32_t Control::_next_channel() {
    uint32_t i;
    uint32_t next_channel = this->_channel_number;
    for(i = 0; i < CONFIG_MAX_NUM_CHANNELS; i++) {
        next_channel++;
        if(next_channel >= CONFIG_MAX_NUM_CHANNELS) {
            next_channel = 0;
        }
        Channel *c = this->_get_channel_pointer(next_channel);
        if(c->freq) {
            break;
        }
    }
   return (i >= CONFIG_MAX_NUM_CHANNELS) ? this->_channel_number : next_channel;
}

uint32_t Control::_prev_channel() {
    uint32_t i;
    uint32_t next_channel = this->_channel_number;
       for(i = 0; i < CONFIG_MAX_NUM_CHANNELS; i++) {
        if(next_channel == 0) {
            next_channel = CONFIG_MAX_NUM_CHANNELS - 1;
        }
        else {
            next_channel--;
        }
        Channel *c = this->_get_channel_pointer(next_channel);
        if(c->freq) {
            break;
        }
    }
   return (i >= CONFIG_MAX_NUM_CHANNELS) ? this->_channel_number : next_channel;



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
                pll.set_tx_state(true);    pll.set_freq(this->_tune_freq_hz);
                /* Update status on display */
                display.update_tx(ptt, tb);
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
                pll.set_tx_state(false);
                this->_tx_state = TX_UNMUTE_WAIT;
            }
            break;

        case TX_UNMUTE_WAIT:
            if(this->_tx_timer >= 10) {
                this->_is_transmitting = false;
                /* Unmute RX audio */
                digitalWrite(GPIO_MUTE_OUT, false);
                /* Update status on display */
                display.update_tx(false);
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
    Band_Info *band_info = (Band_Info *) ps.get_value_pointer(KEY_BAND_INFO_TABLE);
    band_info+=this->_current_band; // Select the correct band table for the current band.

    switch(event) {
       
        case ENCODER_SWITCH_FORWARD: /* Rotation in forward direction */
            if(this->_is_channelized) {
                /* Memory mode */
                this->_channel_number = this->_next_channel();
                this->update_channel_freq();
            }
            else {
                /* VFO mode */
                /* Test band upper limit */
                if(this->_tune_freq_hz + step_size <= band_info->upper_limit) {
                    /* Will still be in band, so increase tune freq by the step size and update everything */
                    this->_tune_freq_hz += step_size;
                    this->update_channel_freq();
                    //display.update_freq(this->_tune_freq_hz, band_info[this->_current_band].freq_offset_display);
                
                }
            }
            break;

        case ENCODER_SWITCH_REVERSE: /* Rotation in reverse direction */
            if(this->_is_channelized) {
                /* Memory Mode */
                this->_channel_number = this->_prev_channel();
                this->update_channel_freq();            
            }
            else {
                /* Test band lower limit */
                /* VFO mode */
                if(this->_tune_freq_hz - step_size >= band_info->lower_limit) {
                    /* Will still be in band, so decrease tune freq by the step size and update everything */
                    this->_tune_freq_hz -= step_size;
                    this->update_channel_freq();
                    //display.update_freq(this->_tune_freq_hz, band_info[this->_current_band].freq_offset_display);
                }
            }
            break;

        case ENCODER_KNOB_SWITCH_PRESSED_SHORT: /* Short duration encoder knob press */
            /* Change frequency step */
            this->_step_size_index++;
            if(this->_step_size_index >= NUMBER_OF_STEP_SIZES) {
                this->_step_size_index = 0;
            }
            display.update_tune_step_size(this->_step_size_table[this->_step_size_index]);
            break;
        
        case ENCODER_KNOB_SWITCH_PRESSED_LONG: { /* Long duration encoder knob press */
            
            // Select modeless top menue if no mode switch flag is set
            const menu_level *top = (band_info->flags & BAND_FLAG_NO_MODE_SWITCH) ? &top_no_mode : &top_mode; 
            // Initialize and show menu
            menu_init(top);
            display.clear_view(VIEW_MENU);
            display.set_current_view(VIEW_MENU);
            menu.show();
          
            break;
        }
          
        default:
            break;

    }


}

void Control::_handle_menu_view(uint8_t event) {
    bool exited;

    exited = menu.handler(event); 
   
    if(exited) {
        display.set_current_view(VIEW_NORMAL);
    }
}
