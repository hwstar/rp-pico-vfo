#include <Arduino.h>
#include <Ticker.h>
#include <Wire.h>
#include "gpio.h"
#include "display.h"
#include "pll.h"


/* Forward declarations */

void two_mS();

/* Class Instatiations */

Ticker ticker(two_mS, 2, 0, MILLIS);
Display display;
Pll pll;


/* Variables */

uint64_t ms_ticks;

void setup() {
    /* Initializer ticker */
    ticker.start();
    /* Initialize I2C */
    Wire.begin();
    /* Initialize USB Serial */
    Serial.begin(115200);
    /* Initialize debug port */
    Serial1.begin(115200);
    /* Initialize display*/
    display.begin();
    /* Turn on backlight */
    display.set_backlight(true);
    /* Initialize PLL */
    //pll.begin(26000000, 12287407, 7150000, 0);
    

}

void loop() {
    /* Update ticker */
    ticker.update();
    display.update();

}


/* This gets called every 2 milliseconds on average */
void two_mS() {
    ms_ticks++;
}

