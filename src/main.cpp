#include <Arduino.h>
#include <Ticker.h>
#include <Wire.h>
#include "error_handler.h"
#include "gpio.h"
#include "24cw640.h"
#include "persistent_storage.h"
#include "config_keys.h"
#include "EncoderSwitch.h"
#include "display.h"
#include "pll.h"
#include "control.h"
#include "config_default.h"
#include "console.h"
#include "version.h"


extern void menu_init(void);

/* Forward declarations */

void two_mS();
void encoder_callback(uint8_t event);
const char *get_radio_os_version_string(bool abbr);

/* Class Instatiations */

Ticker ticker(two_mS, 2, 0, MILLIS);
Error_Handler error_handler;
Display display;
Pll pll;
Control control;
EncoderSwitch encoder;
MbedI2C I2C_int(12, 13); // Must be GPxx as a number predefined arduino pins do not work! 
MbedI2C I2C_ext(10, 11); // Must be GPxx as a number predefined arduino pins do not work!
Eeprom24C640 eeprom;
PersistentStorage ps;
Console console;




/* Variables */

uint64_t ms_ticks;



void setup() {
    // Initialize ticker
    ticker.start();
    // Initialize I2C
    I2C_int.begin(); // Internal bus with SI5351, EEPROM
    I2C_ext.begin(); // External I2C bus
    // Initialize debug port
    Serial1.begin(115200);
    Serial1.println();
    Serial1.print("Radio OS ");
    Serial1.print(get_radio_os_version_string(false));
    Serial1.println(" WA6ZFT");
    // Initialize encoder
    encoder.begin(GPIO_ENCODER_I, GPIO_ENCODER_Q, GPIO_ENCODER_SWITCH, encoder_callback);
    // Initialize display
    display.begin();
    // Turn on backlight
    display.set_backlight(true);
    // Initialize EEPROM 
    eeprom.begin(&I2C_int, 0x50);
    // Initialize console port
    console.setup();
    // Initialize menu
    menu_init();
    // Initialize control
    control.begin();

    // Boot up banner 
    display.set_current_view(VIEW_SPECIAL);
    display.printf(0, 0, 13, "RADIO OS");
    display.printf(1, 0, 12, "V 0.0 WA6ZFT");
    

}


void loop() {
    ticker.update();
    display.update();
    console.poll();
}


/* This gets called every 2 milliseconds on average */

void two_mS() {
    static bool initialized = false;
    ms_ticks++;

   
   
    // Don't proceed further if hard fault posted

    if(error_handler.hard_error_posted()) {
        return;
    }

    encoder.tick();
    control.tick();


    if(!initialized && (ms_ticks == 2500)){ // Display boot up banner displays for 5 seconds
        // Try to read in the contents of the eeprom I2C device
        if(!ps.begin(&eeprom)) {
            error_handler.post(ERROR_MISSING_EEPROM);
            return;
        }
        if(!ps.validate_contents()) {
            // EEPROM invalid. Set factory defaults
            Serial1.println("Error: Invalid EEPROM contents, reformatting to factory default");
            console.set_factory_defaults();
        }



        // *** Initialize PLL ***
        // Point to radio configuration data
        Radio_Info *radio_info = (Radio_Info *) ps.get_value_pointer(KEY_RADIO_CONFIG);
        // Determine if we need to swap the local oscillators on TX
        bool swap_lo_on_tx = !((radio_info->flags & RADIO_FLAG_BITX_MODE) > 0);
        // Get the calibration value
        int32_t calibration_value;
        bool res;
        res = ps.read(KEY_CALIB, &calibration_value);
        // Configure the PLL
        res = pll.begin(&I2C_int, radio_info->ref_clk_freq,
            radio_info->if_zero_hz_freq, 10000000, calibration_value, swap_lo_on_tx);
        if(!res) {
            // No I2C connectivity to the Si5351
            error_handler.post(ERROR_MISSING_SI5351);
            return;
        }
      

        display.clear_view(VIEW_SPECIAL);
        // Switch to normal view
        display.set_current_view(VIEW_NORMAL);
        // Allow user to use the encoder
        control.release();
        initialized = true;
    }

}


/*
* This callback is called whenever the state of the encoder knob changes 
* Knob events are passed to control for processing 
*/
void encoder_callback(uint8_t event) {
    control.encoder_event(event);
}

const char *get_radio_os_version_string(bool abbr) {
    static char version_string[13];
    if(abbr) {
        snprintf(version_string, 13, "V %1d.%1d",VERSION_MAJOR, VERSION_MINOR);  
    }
    else {
        snprintf(version_string, 13, "Version %1d.%1d", VERSION_MAJOR, VERSION_MINOR);
    }
    return version_string;
}
    
    
