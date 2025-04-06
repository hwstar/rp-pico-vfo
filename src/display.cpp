#include <Arduino.h>
#include <LiquidCrystal.h>
#include "gpio.h"
#include "display.h"

/* Freq: format MM.KKKXXX */
#define FREQ_LINE 0
#define FREQ_OFFSET 0
#define FREQ_LEN 9

// TX or RX
#define TX_MODE_LINE 0
#define TX_MODE_OFFSET 10
#define TX_MODE_LEN 2

// USB or LSB
#define SIDEBAND_LINE 0
#define SIDEBAND_OFFSET 13
#define SIDEBAND_LEN 3

LiquidCrystal lcd_display(GPIO_LCD_RS, GPIO_LCD_E, GPIO_LCD_DB4, GPIO_LCD_DB5, GPIO_LCD_DB6, GPIO_LCD_DB7);

void Display::begin() {
    /* Clear all views */
    this->_current_view = VIEW_NORMAL;
    for(uint8_t i = 0; i < DISPLAY_MAX_VIEWS; i++) {
        this->clear_view(i);
    }
    lcd_display.begin(16,2);
    lcd_display.print("It works!");


}

void  Display::set_current_view(uint8_t new_view) {
    DisplayChars *vp = &this->_views[this->_current_view];
    vp->dirty = true;
    this->_current_view = new_view;

}

void Display::update_freq(char *freq){
    DisplayChars *vp = &this->_views[VIEW_NORMAL];



    

}

void Display::update_tx(bool tx){
    DisplayChars *vp = &this->_views[VIEW_NORMAL];

}

void Display::update_status_text(char *msg) {
    DisplayChars *vp = &this->_views[VIEW_NORMAL];

}

void Display::clear_view(uint8_t view) {
    DisplayChars *vp = &this->_views[view];
    for(uint8_t line = 0; line < DISPLAY_MAX_LINES; line++) {
        memset(vp->lines[line], 0x20, DISPLAY_MAX_LINE_LENGTH);
    }
    /* Signal that this view needs to be sent to the display */
    vp->dirty = true;

}

void Display::set_backlight(bool state) {
    digitalWrite(GPIO_LCD_BACKLIGHT, state);
}

void Display::update() {

    DisplayChars *vp = &this->_views[this->_current_view];

    if(vp->dirty) {
        /* TODO: Write lines from view to physical display */

        vp->dirty = false;
    }



}

