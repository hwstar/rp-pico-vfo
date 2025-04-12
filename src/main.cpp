#include <Arduino.h>
#include <Ticker.h>
#include <Wire.h>
#include "gpio.h"
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
MbedI2C I2C_int(12, 13); /* Must be GPxx as a number predefined arduino pins do not work! */
MbedI2C I2C_ext(10, 11); /* Must be GPxx as a number predefined arduino pins do not work! */


/* Variables */

uint64_t ms_ticks;

void setup() {
    /* Initializer ticker */
    ticker.start();
    /* Initialize I2C */
    I2C_int.begin(); /* Internal bus with SI5351 */
    I2C_ext.begin(); /* External I2C bus */
    
   


    /* Initialize USB Serial */
    Serial.begin(115200);
    /* Initialize debug port */
    Serial1.begin(115200);
    /* Initialize encoder */
    encoder.begin(GPIO_ENCODER_I, GPIO_ENCODER_Q, GPIO_ENCODER_SWITCH, encoder_callback);
    /* Initialize display*/
    display.begin();
    /* Turn on backlight */
    display.set_backlight(true);
    /* Initialize PLL */
    /* See config_default.h for constants */
    pll.begin(&I2C_int, CONFIG_DEFAULT_REF_CLK_FREQ, CONFIG_DEFAULT_IF_ZERO_HZ_FREQ, 10000000, CONFIG_DEFAULT_REF_CLK_CAL);
    /* pll.cal_mode(true); */

    /* Initialize control */
    control.begin(&display, &pll);

    /* Boot up banner */
    display.set_current_view(VIEW_SPECIAL);
    display.printf(0, 0, "40 METER XVCR");
    display.printf(1, 0,"V 0.0 WA6ZFT");
    

}

/*
uint32_t read_gpios() {
    uint32_t gpio_bits = 0;

    static const pin_size_t pins[23] = {0,0,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19,p20,p21,p22};

    for(uint8_t pos = 2; pos < 23; pos++) {
        if(digitalRead(pins[pos]))
            gpio_bits |= (1 << pos);
    }
    return gpio_bits;
}
*/



void loop() {
    /*
    static uint32_t previous_gpios = 0xFFFFFFFF;
    */
    /* Update ticker */
    ticker.update();
    display.update();

    /*
    uint32_t current_gpios = read_gpios();
    if(current_gpios != previous_gpios) {
        char str[32];
        sprintf(str, "%08lx", current_gpios);
        Serial1.println(str);
        previous_gpios = current_gpios;

    }
    */
   


}


/* This gets called every 2 milliseconds on average */
void two_mS() {
    ms_ticks++;
    control.tick();
    encoder.tick();
    if(ms_ticks == 2500){ /* Boot up banner displays for 5 seconds*/
        display.clear_view(VIEW_SPECIAL);
        /* Switch to normal view */
        display.set_current_view(VIEW_NORMAL);
        control.release();
    }

}


/* This callback is called whenever the state of the encoder knob changes */
/* Knob events are passed to control for processing */
void encoder_callback(uint8_t event) {
    control.encoder_event(event);
}
