#include <Arduino.h>
#include "config_default.h"
#include "menu.h"


void Menu::_push(const menu_level *existing)
{
    if(this->_menu_stack_pointer){
        this->_menu_stack[this->_menu_stack_pointer] = existing;
        this->_menu_stack_pointer--;
    }
}

const menu_level *Menu::_pop()
{
    if(this->_menu_stack_pointer < MAX_MENU_LEVELS - 1)
        this->_menu_stack_pointer++;
    return this->_menu_stack[this->_menu_stack_pointer];
}


void Menu::_next_menu_item() {
    this->_selection++;
    if(this->_selection >= this->_cur_level->item_count) {
        this->_selection = 0;
    }
    this->_draw_current_level();
}

void Menu::_prev_menu_item() {
    if(this->_selection == 0) {
        this->_selection = this->_cur_level->item_count - 1;
    }
    else {
        this->_selection--;
    }
    this->_draw_current_level();
}

void Menu::_draw_current_level() {
    const char *l1 = this->_cur_level->name;
    const char *l2 = this->_cur_level->items[this->_selection]->menu_text;
    (*this->_drawer)(l1, l2);
}

void Menu::_reset() {
    this->_menu_stack_pointer = MAX_MENU_LEVELS - 1; // Menu stack pointer set to top of stack
    this->_selection = 0; // First menu selection
    this->_state = MENU_STATE_SCROLL;
    this->_cur_level = this->_top_level; // Current level set to top
}

void Menu::init(const menu_level *top,void (*drawer)(const char *line1, const char *line2), void (*exit_action)()) {
    this->_top_level = top; // Top level menu
    this->_drawer = drawer; // Drawing function
    this->_exit_action = exit_action; // The user supplied menu exit function
    this->_reset();
}

void Menu::show() {
    this->_reset();
    this->_draw_current_level();
}

void Menu::draw_item_value(const char *value) {
    const char *l1 = this->_cur_level->items[this->_selection]->menu_text;
    const char *l2 = value;
    (*this->_drawer)(l1, l2);
}


bool Menu::handler(uint8_t event) {

    bool exit_menu_system = false;

    if(this->_state == MENU_STATE_SELECTED) {
        // User needs to adjust a setting 
        if ((event == MENU_KNOB_CW) || (event == MENU_KNOB_CCW)) {
            if (this->_cur_level->items[this->_selection]->action) {
                (*this->_cur_level->items[this->_selection]->action)(event);
            }
        }
        else {
            if (event == MENU_KNOB_PRESS_SHORT) { // Selected by user 
                if (this->_cur_level->items[this->_selection]->on_exit) {
                    exit_menu_system = (*this->_cur_level->items[this->_selection]->on_exit)(true);
                    // Allow user to scroll through menu options again
                    this->_state = MENU_STATE_SCROLL;
                    // Redraw menu level
                    this->_draw_current_level();
                }
            }
            else { // Long press: Aborted by user
                if(this->_cur_level->items[this->_selection]->on_exit) {
                    exit_menu_system = (*this->_cur_level->items[this->_selection]->on_exit)(false);
                    // Allow user to scroll through menu options again
                    this->_state = MENU_STATE_SCROLL;
                    // Redraw menu level
                    this->_draw_current_level();
                }
   


            }      
        }

    } else if (this->_state == MENU_STATE_SCROLL) {
        // User is scrolling through menu options
        if (event == MENU_KNOB_CW) {
            this->_next_menu_item();

        }
        else if (event == MENU_KNOB_CCW) {
            this->_prev_menu_item();

        }
        else if (event == MENU_KNOB_PRESS_SHORT) {
            // User has selected this menu item
            // Determine if the item is another menu level, or if it
            // is a menu item.
            if (this->_cur_level->items[this->_selection]->item_type == MENU_ITEM_TYPE_SUBMENU) {
                // Is another submenu
                this->_push(this->_cur_level);
                this->_cur_level = this->_cur_level->items[this->_selection]->lower_level;
                this->_selection = 0; // Select first entry in lower level.
                // Draw next level
                this->_draw_current_level();
            }
            else if (this->_cur_level->items[this->_selection]->item_type == MENU_ITEM_TYPE_ACTION) {
                // User selected a menu item
                this->_state = MENU_STATE_SELECTED;
                // Call entry function if supplied
                // In the entry function, the user will update the screen by using the draw_item_value() method
                if (this->_cur_level->items[this->_selection]->on_entry) {
                    (*this->_cur_level->items[this->_selection]->on_entry)();

                }
            }

        }
        else { // Menu knob long press means abort
            // If at top level, exit the menu system
            if (this->_cur_level == this->_top_level) {
                exit_menu_system = true;
            }
            else {
                this->_cur_level = this->_pop();
                this->_selection = 0; // Select first entry in previous level.
                // Draw next level
                this->_draw_current_level();
            }
            
        }

    }
    // Call the exit action function if it was supplied
    if(exit_menu_system && this->_exit_action) {
        (*this->_exit_action)();
    }

    return exit_menu_system;
}



