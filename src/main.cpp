#include <Arduino.h>
#include <Ticker.h>
#include <Wire.h>
#include "gpio.h"
#include "EncoderSwitch.h"
#include "display.h"
#include "pll.h"


/* Forward declarations */

void two_mS();
void encoder_callback(uint8_t event);

/* Class Instatiations */

Ticker ticker(two_mS, 2, 0, MILLIS);
Display display;
Pll pll;
EncoderSwitch encoder;
MbedI2C I2C_int(p12, p13);
MbedI2C I2C_ext(p10, p11);



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
    Serial1.println("A test to see if the serial port works");
    /* Initialize encoder */
    encoder.begin(GPIO_ENCODER_I, GPIO_ENCODER_Q, GPIO_ENCODER_SWITCH, encoder_callback);

    /* Initialize display*/
    display.begin();
    /* Turn on backlight */
    display.set_backlight(true);
    /* Initialize PLL */
    //pll.begin(26000000, 12287407, 7150000, 0);
    

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
    encoder.tick();
    ms_ticks++;
}


/* This callback is called whenever the state of the encoder knob changes */
void encoder_callback(uint8_t event) {
    if(event == ENCODER_SWITCH_FORWARD)
        Serial1.println("Forward");
    if(event == ENCODER_SWITCH_REVERSE)
        Serial1.println("Reverse");
    if(event == ENCODER_KNOB_SWITCH_PRESSED_SHORT)
        Serial1.println("Release after short press");
    if(event == ENCODER_KNOB_SWITCH_PRESSED_LONG)
        Serial1.println("Release after long press");

}
