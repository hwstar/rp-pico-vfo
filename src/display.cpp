#include <Arduino.h>
#include <LiquidCrystal.h>
#include "gpio.h"
#include "display.h"

/* Freq: format MM.KKKHHH */
#define FREQ_LINE 0
#define FREQ_OFFSET 0
#define FREQ_LEN 9

// USB or LSB
#define SIDEBAND_LINE 0
#define SIDEBAND_OFFSET 13
#define SIDEBAND_LEN 3

// TX or RX
#define TX_MODE_LINE 1
#define TX_MODE_OFFSET 15
#define TX_MODE_LEN 2

// AGC
#define AGC_LINE 1
#define AGC_OFFSET 10
#define AGC_LEN 3

LiquidCrystal lcd_display(GPIO_LCD_RS, GPIO_LCD_E, GPIO_LCD_DB4, GPIO_LCD_DB5, GPIO_LCD_DB6, GPIO_LCD_DB7);

void Display::begin() {
    /* Clear all views */
    this->_current_view = VIEW_NORMAL;
    for(uint8_t i = 0; i < DISPLAY_MAX_VIEWS; i++) {
        this->clear_view(i);
    }
    lcd_display.begin(16,2);
}

void  Display::set_current_view(uint8_t new_view) {
    DisplayChars *vp = &this->_views[this->_current_view];
    vp->dirty = true;
    this->_current_view = new_view;

}

void Display::update_freq(uint32_t hz){

    char freq[12];
    sprintf(freq, "%2lu.", hz / 1000000);
    sprintf(freq + 3, "%06lu", hz % 1000000);

    DisplayChars *vp = &this->_views[VIEW_NORMAL];
    memcpy(vp->lines[FREQ_LINE] + FREQ_OFFSET, freq, FREQ_LEN);
    vp->dirty = true;

}

void Display::update_sideband(bool sideband){
    DisplayChars *vp = &this->_views[VIEW_NORMAL];
    memcpy(vp->lines[SIDEBAND_LINE] + SIDEBAND_OFFSET, sideband ? "USB" : "LSB", SIDEBAND_LEN);
    vp->dirty = true;

}

void Display::update_tx(bool tx) {
    DisplayChars *vp = &this->_views[VIEW_NORMAL];
    memcpy(vp->lines[TX_MODE_LINE] + TX_MODE_OFFSET, tx ? "TX" : "RX", TX_MODE_LEN);
    vp->dirty = true;


}

void Display::update_agc(bool agc){
    DisplayChars *vp = &this->_views[VIEW_NORMAL];
    memcpy(vp->lines[AGC_LINE] + AGC_OFFSET, agc ? "AGC" : "   ", AGC_LEN);
    vp->dirty = true;

}

void Display::clear_view(uint8_t view) {
    DisplayChars *vp = &this->_views[view];
    for(uint8_t line = 0; line < DISPLAY_MAX_LINES; line++) {
        memset(vp->lines[line], 0x20, DISPLAY_MAX_LINE_LENGTH);
        /* Zero terminate last position in buffer */
        vp->lines[line][DISPLAY_MAX_LINE_LENGTH] = 0;
    }
    /* Signal that this view needs to be sent to the display */
    vp->dirty = true;

}

void Display::print_text(const char *str, uint8_t row, uint8_t col) {
    DisplayChars *vp = &this->_views[this->_current_view];
    int len = strlen(str);
    if((col >= DISPLAY_MAX_LINES) || (row >= DISPLAY_MAX_LINE_LENGTH )) {
        return; /* Out of bounds */
    }
    if((row + len) > DISPLAY_MAX_LINE_LENGTH) {
        return; /* Out of bounds */
    }
    memcpy(vp->lines[row]+col, str, len);
}

void Display::set_backlight(bool state) {
    digitalWrite(GPIO_LCD_BACKLIGHT, state);
}

void Display::update() {

    DisplayChars *vp = &this->_views[this->_current_view];

    if(vp->dirty) {
        /* Write lines from view to physical display */
        lcd_display.setCursor(0,0);
        lcd_display.print(vp->lines[0]);
        lcd_display.setCursor(0,1);
        lcd_display.print(vp->lines[1]);

        vp->dirty = false;
    }



}

