#pragma once
#include <LiquidCrystal.h>

#define DISPLAY_MAX_VIEWS 4
#define DISPLAY_MAX_LINE_LENGTH 16
#define DISPLAY_MAX_LINES 2

enum {VIEW_NORMAL=0, VIEW_MENU=1, VIEW_SPECIAL=2, VIEW_ERROR=3};


typedef struct displaychars {
    bool dirty;
    /* Memory for 16x2 display */
    char lines[DISPLAY_MAX_LINES][DISPLAY_MAX_LINE_LENGTH]; 


} DisplayChars;

class Display {
    public:
    /*
    * Display initialization
    */
    void begin();

    void set_current_view(uint8_t new_view);

    /*
    * Update frequency portion of display
    */

    void update_freq(char *freq);

    /*
    * Update transmit status portion of display
    */

    void update_tx(bool tx);

    /*
    * Update status portion of display
    */

    void update_status_text(char *textL);

    /*
    * Clear a display view
    */

    void clear_view(uint8_t view);

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