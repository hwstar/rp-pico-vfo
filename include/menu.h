#pragma once
#include <Arduino.h>
#include <EncoderSwitch.h>

#define MAX_MENU_ITEMS 8
#define MAX_MENU_LEVELS 4

#define MENU_ITEM_TYPE_ACTION 0
#define MENU_ITEM_TYPE_SUBMENU 1


#define MENU_KNOB_CCW ENCODER_SWITCH_REVERSE
#define MENU_KNOB_CW ENCODER_SWITCH_FORWARD
#define MENU_KNOB_PRESS_SHORT ENCODER_KNOB_SWITCH_PRESSED_SHORT
#define MENU_KNOB_PRESS_LONG ENCODER_KNOB_SWITCH_PRESSED_LONG

#define MENU_STATE_SCROLL 0
#define MENU_STATE_SELECTED 1



// Structure for describing the type of menu item
typedef struct menu_item
{
    const char *menu_text;
    uint8_t item_type;
    const struct menu_level *lower_level;
    void (*on_entry)(); // Used to pre-fetch the data, and set up what is to be displayed.
    void (*action)(uint8_t event); // Event handler - sends CW or CCW events 
    bool (*on_exit)(bool confirm); // Used to write data or clean up.
} menu_item;

// Structure for a set of menu items
typedef struct menu_level
{
    const uint8_t item_count; // Number if items in the menu or submenu
    const char *name; // Menu name
    const menu_item *items[MAX_MENU_ITEMS]; // Storage for menu items
} menu_level;


class Menu
{
    public:
    //Initialization
    // Call at setup
    // Supply a drawing function
    // Optionally supply the exit action function.
    void begin(const menu_level *top,void (*drawer)(const char *line1, const char *line2), void (*exit_action)() = NULL );
    // Show menu 
    // Call this to show top level menu
    void show(); 
    // Draw item value.
    /// Call when a knob event needs to have the value updated on the display.
    void draw_item_value(const char *value);
    // Call this when there is an encoder knob event
    // Supply the event type
    // Returns true if the user requested exit from the menu system
    bool handler(uint8_t event); // Called by user when the rotary encoder state changes.

    protected:
    void _reset();
    void _push(const menu_level *existing);
    const menu_level *_pop();
    void _next_menu_item();
    void _prev_menu_item();
    void _draw_current_level(); 

    void (*_exit_action)();
    void (*_drawer)(const char *line1, const char *line2);
    uint8_t _menu_stack_pointer;
    uint8_t _selection;
    uint8_t _state;
    const menu_level *_menu_stack[MAX_MENU_LEVELS];
    const menu_level *_top_level;
    const menu_level *_cur_level;
};