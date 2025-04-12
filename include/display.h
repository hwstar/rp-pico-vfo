#pragma once
#include <LiquidCrystal.h>

#define DISPLAY_MAX_VIEWS 4
#define DISPLAY_MAX_LINE_LENGTH 16
#define DISPLAY_MAX_LINES 2

enum {VIEW_NORMAL=0, VIEW_MENU=1, VIEW_SPECIAL=2, VIEW_ERROR=3};


typedef struct displaychars {
    bool dirty;
    /* Memory for 16x2 display */
    /* Note: One extra byte added to accomodate zero terminated strings */
    char lines[DISPLAY_MAX_LINES][DISPLAY_MAX_LINE_LENGTH + 1]; 


} DisplayChars;

class Display {
    public:
    /*
    * Display initialization
    */
    void begin();

    void set_current_view(uint8_t new_view);

    uint8_t get_current_view();

    /*
    * Update frequency portion on display
    */

    void update_freq(uint32_t hz);

    /*
    * Update transmit status portion on display
    */

    void update_tx(bool tx, bool tune_mode = false);

    /*
    * Update sideband on display
    */

    void update_sideband(bool sideband);

    /*
    * Update AGC on display
    */

    void update_agc(bool agc);

    /*
    * Clear a display view
    */

    void clear_view(uint8_t view);

    /*
    * Printf to the current view
    */
    void printf(uint8_t row, uint8_t col, const char *format, ...);

    /* Set the display backlight state */

    void set_backlight(bool state);

    /*
    * Must be called periodically so the display gets updated from a view
    */

    void update();

    protected:
    LiquidCrystal *lcd;
    uint8_t _current_view;
    DisplayChars _views[DISPLAY_MAX_VIEWS];

    
   

};