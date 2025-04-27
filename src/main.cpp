#include <Arduino.h>
#include <Ticker.h>
#include <Wire.h>
#include "gpio.h"
#include "24cw640.h"
#include "EncoderSwitch.h"
#include "display.h"
#include "pll.h"
#include "control.h"
#include "config_default.h"


/* Forward declarations */

void two_mS();
void encoder_callback(uint8_t event);

/* Class Instatiations */

Ticker ticker(two_mS, 2, 0, MILLIS);
Display display;
Pll pll;
Control control;
EncoderSwitch encoder;
MbedI2C I2C_int(12, 13); // Must be GPxx as a number predefined arduino pins do not work! 
MbedI2C I2C_ext(10, 11); // Must be GPxx as a number predefined arduino pins do not work!
Eeprom24C640 eeprom;



/* Variables */

uint64_t ms_ticks;

void setup() {
    // Initialize ticker
    ticker.start();
    // Initialize I2C
    I2C_int.begin(); // Internal bus with SI5351, EEPROM
    I2C_ext.begin(); // External I2C bus
    
   


    // Initialize USB Serial
    Serial.begin(115200);
    // Initialize debug port
    Serial1.begin(115200);
    // Initialize encoder
    encoder.begin(GPIO_ENCODER_I, GPIO_ENCODER_Q, GPIO_ENCODER_SWITCH, encoder_callback);
    // Initialize display
    display.begin();
    // Turn on backlight
    display.set_backlight(true);
    // Initialize EEPROM 
    eeprom.begin(&I2C_int, 0x50);
    // Initialize control
    control.begin(&display, &pll);

    // Boot up banner 
    display.set_current_view(VIEW_SPECIAL);
    display.printf(0, 0, 13, "40 METER XVCR");
    display.printf(1, 0, 12, "V 0.0 WA6ZFT");
    

}


void loop() {
    /*
    static uint32_t previous_gpios = 0xFFFFFFFF;
    */
    // Update ticker
    ticker.update();
    display.update();
}

/*
* Display missing eeprom error
*/

void error_missing_eeprom() {
    display.clear_view(VIEW_SPECIAL);
    display.printf(0, 0, 6, "ERROR:");
    display.printf(1, 0, 14, "EEPROM MISSING");
    for(;;){
        display.update();
    }

}

/*
* Display missing SI5351 error
*/

void error_missing_si5351() {
    display.clear_view(VIEW_SPECIAL);
    display.printf(0, 0, 6, "ERROR:");
    display.printf(1, 0, 14, "MISSING SI5351");
    for(;;){
        display.update();
    }

}

/* This gets called every 2 milliseconds on average */

void two_mS() {
    static bool initialized = false;
    ms_ticks++;
    control.tick();
    encoder.tick();
    if(!initialized && (ms_ticks == 2500)){ // Display boot up banner displays for 5 seconds
        // Check for eeprom I2C device present 
        if(!eeprom.present()) {
            error_missing_eeprom();
        }
        // Initialize PLL 
        // See config_default.h for constants 
        bool res = pll.begin(&I2C_int, CONFIG_DEFAULT_REF_CLK_FREQ, CONFIG_DEFAULT_IF_ZERO_HZ_FREQ, 10000000, CONFIG_DEFAULT_REF_CLK_CAL);
        if(!res) {
            error_missing_si5351();
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
