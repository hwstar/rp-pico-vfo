#include <Arduino.h>
#include <LiquidCrystal.h>
#include <stdarg.h>
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
#define TX_MODE_LINE 0
#define TX_MODE_OFFSET 10
#define TX_MODE_LEN 2

// Band Name
#define BAND_NAME_LINE 1
#define BAND_NAME_OFFSET 4
#define BAND_NAME_LEN 4

// AGC
#define AGC_LINE 1
#define AGC_OFFSET 9
#define AGC_LEN 3

// Tuning step size
#define TUNE_STEP_LINE 1
#define TUNE_STEP_OFFSET 12
#define TUNE_STEP_LEN 3



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

uint8_t Display::get_current_view() {
    return this->_current_view;
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

void Display::update_tx(bool tx, bool tune_mode) {
    DisplayChars *vp = &this->_views[VIEW_NORMAL];
    if(tune_mode) {
        memcpy(vp->lines[TX_MODE_LINE] + TX_MODE_OFFSET, "TU", TX_MODE_LEN);
    }
    else {
        memcpy(vp->lines[TX_MODE_LINE] + TX_MODE_OFFSET, tx ? "TX" : "RX", TX_MODE_LEN);
    }
    
    vp->dirty = true;


}

void Display::update_agc(bool agc){
    DisplayChars *vp = &this->_views[VIEW_NORMAL];
    memcpy(vp->lines[AGC_LINE] + AGC_OFFSET, agc ? "AGC" : "   ", AGC_LEN);
    vp->dirty = true;

}


void Display::update_tune_step_size(uint16_t step_size) {
    switch(step_size) {
        case 10:
        case 100:
        case 500:
            this->printf(TUNE_STEP_LINE, TUNE_STEP_OFFSET, TUNE_STEP_LEN, "%3u", step_size);
            break;

        case 1000:
            this->printf(TUNE_STEP_LINE, TUNE_STEP_OFFSET, TUNE_STEP_LEN, "%3s", "1K");
            break;

        case 10000:
            this->printf(TUNE_STEP_LINE, TUNE_STEP_OFFSET, TUNE_STEP_LEN, "%3s", "10K");
            break;
    }

}

void Display::update_band_name(char *band_name) {
    this->printf(BAND_NAME_LINE, BAND_NAME_OFFSET, BAND_NAME_LEN, "%4s", band_name);
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


void Display::printf(uint8_t row, uint8_t col, uint8_t len, const char *format, ...) {
    char ws[DISPLAY_MAX_LINE_LENGTH + 1];
    va_list args; 
    va_start(args, format); 
    DisplayChars *vp = &this->_views[this->_current_view];
    
    if((row >= DISPLAY_MAX_LINES) || (col >= DISPLAY_MAX_LINE_LENGTH )) {
        return; /* Out of bounds */
    }
   
    char *line_buffer_pos = vp->lines[row] + col;
 

    vsnprintf(ws, DISPLAY_MAX_LINE_LENGTH + 1, format, args);
    memcpy(line_buffer_pos, ws, len);
    vp->dirty = true;
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

