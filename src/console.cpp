#include <Arduino.h>
#include "console.h"
#include "pll.h"
#include "persistent_storage.h"
#include "config_default.h"
#include "config_keys.h"
#include "config_structure.h"


#define CTE_END  {NULL, NULL, NULL, ""}

 enum {FIND_FIRST_CHAR=0, FIND_FIRST_SPACE};
 enum {PS_PROMPT=0, PS_WAIT_INPUT};


/*
* External classes
*/

 extern Console console;
 extern Pll pll;
 extern PersistentStorage ps;

/*
* Variables
*/

static bool in_cal;

const static char *error_strings[] = {
    "",
    "Unknown command",
    "Incorrect number of parameters",
    "Parameter error",
    "Table error",
    "Setup error",
    "",
    "",
    "",
    "",
    "",
    "",
};

 /*
 *  Command tables
 */
 static const uint8_t args_none[] = {AT_END};
 static const uint8_t args_single_int[] = {AT_INT, AT_END};
 static const uint8_t args_single_unsigned[] = {AT_UINT, AT_END};
 static const uint8_t args_double_unsigned[] = {AT_UINT, AT_UINT, AT_END};

 bool cal_on(Holder_Type *vars, uint8_t *error_code);
 bool cal_off(Holder_Type *vars, uint8_t *error_code);
 bool cal_set(Holder_Type *vars, uint8_t *error_code);
 bool cal_get(Holder_Type *vars, uint8_t *error_code);
 bool config_set_factory_defaults(Holder_Type *vars, uint8_t *error_code);
 bool info_get_eeprom_layout(Holder_Type *vars, uint8_t *error_code);
 bool info_get_band(Holder_Type *vars, uint8_t *error_code);
 bool print_help(Holder_Type *vars, uint8_t *error_code);
 bool config_set_band_disable(Holder_Type *vars, uint8_t *error_code);
 bool config_set_band_enable(Holder_Type *vars, uint8_t *error_code);
 bool config_set_band_start(Holder_Type *vars, uint8_t *error_code);
 bool config_set_band_stop(Holder_Type *vars, uint8_t *error_code);
 bool config_eeprom_save(Holder_Type *vars, uint8_t *error_code);


 static const Command_Table_Entry_Type cal[] = {
    {NULL, cal_on, args_none, "on"},
    {NULL, cal_off, args_none, "off"},
    {NULL, cal_set, args_single_int, "set"},
    {NULL, cal_get, args_none, "get"},
    CTE_END
 };

 static const Command_Table_Entry_Type config_set_factory[] = {
    { NULL, config_set_factory_defaults, args_none, "defaults"},
    CTE_END

 };

 static const Command_Table_Entry_Type config_set_band[] = {
    { NULL, config_set_band_disable, args_single_unsigned, "disable"},
    { NULL, config_set_band_enable, args_single_unsigned, "enable"},

    { NULL, config_set_band_start, args_double_unsigned, "start"},
    { NULL, config_set_band_stop, args_double_unsigned, "stop"},
    CTE_END
 };

 static const Command_Table_Entry_Type config_set[] = {
    {config_set_factory, NULL, args_single_int, "factory"},
    {config_set_band, NULL, args_none, "band"},
    CTE_END

 };

 static const Command_Table_Entry_Type config_eeprom[] = {
    {NULL, config_eeprom_save, args_none, "save"},
    CTE_END
 };
 

 static const Command_Table_Entry_Type config[] = {
    {config_set, NULL, args_none, "set"},
    {config_eeprom, NULL, args_single_int, "eeprom"},
    CTE_END

 };

 static const Command_Table_Entry_Type info_get_eeprom[] = {
    {NULL, info_get_eeprom_layout, args_none, "layout"},
    CTE_END
 };


 static const Command_Table_Entry_Type info_get[] = {
    {info_get_eeprom, NULL, args_none, "eeprom"},
    {NULL, info_get_band, args_single_unsigned, "band"},
    CTE_END
 };


 static const Command_Table_Entry_Type info[] = {
    {info_get, NULL, args_none, "get"},
    CTE_END
 };


 static const Command_Table_Entry_Type top[] = {
    {cal, NULL, args_none, "cal"},
    {config, NULL, args_none, "config"},
    {NULL, print_help, args_none, "help"},
    {info, NULL, args_none, "info"},
    CTE_END
 };

/* 
* Config commands 
*/

 bool config_set_factory_defaults(Holder_Type *vars, uint8_t *error_code) {

    if(console.confirm_action("Reset everything to factory defaults?")) {
        console.set_factory_defaults();
    }

    return true;
 }
 

 /*
 * Calibrate commands
 */

 bool cal_on(Holder_Type *vars, uint8_t *error_code) {
    in_cal = true;
    pll.cal_mode(true);
    return true;

 }

 bool cal_off(Holder_Type *vars, uint8_t *error_code) {
    pll.cal_mode(false);
    ps.commit();
    in_cal = false;
    return true;
 }

 bool cal_set(Holder_Type *vars, uint8_t *error_code) {
    if(!in_cal) {
        *error_code = CEC_SETUP_ERROR;
        return false;
    }
    if((vars->intg >= -40000) && (vars->intg <= 40000)) {
        pll.set_correction(vars->intg);
        ps.write(KEY_CALIB, vars->intg);
    }
    else {
        return false;
    }
    return true;
 }

 bool cal_get(Holder_Type *vars, uint8_t *error_code) {
    int32_t ppb;
    ps.read(KEY_CALIB, &ppb);
    Serial1.println();
    Serial1.println(ppb);
    return true;
 }

 /*
 * Info commands
 */

 bool info_get_eeprom_layout(Holder_Type *vars, uint8_t *error_code) {
    ps.print_eeprom_info();
    return true;
 }

 Band_Info *get_band_info_pointer(uint32_t band) {
    return ((Band_Info *) ps.get_value_pointer(KEY_BAND_INFO_TABLE)) + band;
}

const char *bool_to_yes_no(bool state) {
    return state ? "YES" : "NO";
}

 bool info_get_band(Holder_Type *vars, uint8_t *error_code) {
    uint32_t band = vars->uint;
    
    // Validate band
    if((band < 1) || (band > 8)) {
        return false;
    }
    // Change band number to to 0-7 range
    band--;
    // Point to the correct band table
    Band_Info *bi = get_band_info_pointer(band);
    char ws[80];
    ws[79] = 0;
    Serial1.println();
    snprintf(ws, 79, "%-20s : %-7s","Band Enabled", bool_to_yes_no((bi->flags & BAND_FLAG_ACTIVE) != 0));
    Serial1.println(ws);
    snprintf(ws, 79, "%-20s : %-7lu","Band Start", bi->lower_limit);
    Serial1.println(ws);
    snprintf(ws, 79, "%-20s : %-7lu","Band Stop", bi->upper_limit);
    Serial1.println(ws);
    return true;
 }

 bool print_help_recursive(const Command_Table_Entry_Type * cur_command_table) {
    // Sanity check
    if(!cur_command_table) {
        return false;
    }
    while(cur_command_table->next_table || cur_command_table->command) {
        if(!console.help_kw_stack_push(cur_command_table->keyword)) {
            return false;
        }
        if(cur_command_table->next_table) {
            // Is a branch in the tree
            if(!print_help_recursive(cur_command_table->next_table))
                return false;
        }
        else {
            // Is a leaf in the tree
            // Print command sequence
            Serial1.print(console.help_kw_stack_cat());
            // Print argument types
            for(int i = 0; cur_command_table->arguments[i] != AT_END; i++) {
                switch(cur_command_table->arguments[i]) {
                    case AT_INT:
                        Serial1.print("<i> ");
                        break;
                        
                    case AT_UINT:
                        Serial1.print("<u> ");
                        break;
                }

            }
            Serial1.println();
        }
        if(!console.help_kw_stack_pop()) 
            return false;
        cur_command_table++;
    }

   return true;
 }

 bool print_help(Holder_Type *vars, uint8_t *error_code) {
    char ws[80];

    // Walk the command tree and print each keyword combination
    Serial1.println();
    Serial1.println("*** List of valid commands ***");
    console.help_kw_stack_init(ws, 80);
    return print_help_recursive(top);
 }

bool validate_band_start_stop(uint32_t band, uint32_t freq) {
    if((freq < 1800000) || (freq > 30000000))
        return false;
    if((band < 1) || (band > 8)) {
        return false;
    }
    return true;
}

bool validate_band_number(uint32_t band) {
    if((band < 1) || (band > 8)) {
        return false;
    }
    return true;
}

bool config_set_band_start(Holder_Type *vars, uint8_t *error_code) {
    uint32_t band = vars[0].uint;
    uint32_t start_freq_hz = vars[1].uint;
    if(!validate_band_start_stop(band, start_freq_hz)) {
        return false;
    }
    band--; // Change band number to to 0-7 range
    Band_Info *bi = get_band_info_pointer(band);
    // Set the lower limit of the band 
    bi->lower_limit = start_freq_hz;
    // Flag that a write to the eeprom is required
    ps.force_dirty();
    return true;
}

bool config_set_band_stop(Holder_Type *vars, uint8_t *error_code) {
    uint32_t band = vars[0].uint;
    uint32_t stop_freq_hz = vars[1].uint;
    if(!validate_band_start_stop(band, stop_freq_hz)) {
        return false;
    }
    band--; // Change band number to to 0-7 range
    Band_Info *bi = get_band_info_pointer(band);
    // Set the upper limit of the band
    bi->upper_limit = stop_freq_hz;
    ps.force_dirty();
    // Flag that a write to the eeprom is required 
    return true;
}

bool config_set_band_disable(Holder_Type *vars, uint8_t *error_code) {
    uint32_t band = vars->uint;
    if(!validate_band_number(band))
        return false;
    band--;

    Band_Info *bi = get_band_info_pointer(band);

    bi->flags &= ~BAND_FLAG_ACTIVE;
    ps.force_dirty();

    return true;
}

bool config_set_band_enable(Holder_Type *vars, uint8_t *error_code){
    uint32_t band = vars->uint;
    if(!validate_band_number(band))
        return false;
    band--;
    
    Band_Info *bi = get_band_info_pointer(band);

    bi->flags |= BAND_FLAG_ACTIVE;
    ps.force_dirty();
    
    return true;
}

bool config_eeprom_save(Holder_Type *vars, uint8_t *error_code) {
    // Write RAM to EEPROM if necessary
    ps.commit();
    return true;
}



/*
* Console class methods
*/


 void Console::setup(void) {
    this->_poll_state = PS_PROMPT;
}


/*
 * Called by top.cpp repeatedly to process console data
 */

void Console::poll(void) {
	int argc;
    static uint16_t length;
	

    if(this->_poll_state == PS_PROMPT) {
        Serial1.println();
        Serial1.print(":");
        this->_poll_state = PS_WAIT_INPUT;
        length = 0;
    }
    else {

        if(!this->_get_line(this->_line_buffer, LINE_BUFFER_SIZE, &length)) {
            return;
        }
        else {
            if(strlen(this->_line_buffer)) {
                argc = this->_make_arglist((char **) &this->_arg_list, MAX_ARGS, this->_line_buffer);
                bool res = this->_parse_command(argc, top);
                if(!res) {
                    Serial1.println();
                    // Print error message
                    Serial1.print("?");
                    if(this->_error_code < CEC_NUM_ERROR_CODES ) {
                        Serial1.print(error_strings[this->_error_code]);
                    }
                    Serial1.println();
                }
        
            }
            this->_poll_state = PS_PROMPT;
        }
    }
}

bool Console::confirm_action(const char *query_string) {
    char line[2];
    Serial1.println();
    uint16_t length = 0;
    while(true) {
    
        Serial1.print(query_string);
        Serial1.print(" (Y/N): ");
        // NB: this blocks everything so use with care
        while(!this->_get_line(line, 2, &length));
        if((line[0] == 'y') || (line[0] == 'Y')) {
            return true;

        }
        else if ((line[0] == 'n') || (line[0] == 'N'))
            return false;
    }
    return false;
}

void Console::set_factory_defaults() { 

    Serial1.println();
    Serial1.println("Setting factory defaults...");
    ps.format();

    // Band Tables
    // Initialized empty
    ps.add_key(KEY_BAND_INFO_TABLE, sizeof(Band_Info) * MAX_NUM_OF_BANDS);

    // TCXO calibration
    ps.add_key(KEY_CALIB, sizeof(int32_t));
    ps.write(KEY_CALIB, (int32_t) CONFIG_DEFAULT_REF_CLK_CAL);
    // Initial frequency
    ps.add_key(KEY_INIT_FREQ, sizeof(uint32_t));
    ps.write(KEY_INIT_FREQ, (uint32_t) CONFIG_DEFAULT_BAND_INITIAL_FREQUENCY_0);

    // Write it all to the EEPROM
    ps.commit();
    Serial1.println("Restarting...");
    delay(1000);

    // Do a RP pico-specific software reset to force everything to reload
    *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004;
}


bool Console::help_kw_stack_init(char *ws, int size) {
    if(!ws) {
        return false;
    }

    this->_kw_stack_pointer = KW_STACK_SIZE - 1;
    this->_kw_work_string = ws;
    this->_kw_work_string[0] = 0;
    this->_kw_work_string_size = size;
    return true;
}

bool Console::help_kw_stack_push(const char *keyword) {
    if(this->_kw_stack_pointer) {
        this->_kw_stack[this->_kw_stack_pointer] = keyword;
        this->_kw_stack[this->_kw_stack_pointer--];
        return true;
    }
    else {
        return false;
    }
}

bool Console::help_kw_stack_pop() {
    if(this->_kw_stack_pointer != (KW_STACK_SIZE - 1)) {
        this->_kw_stack_pointer++;
        return true;
    }
    return false;

}

char *Console::help_kw_stack_cat() {
    uint8_t our_sp = (KW_STACK_SIZE - 1);
    
    this->_kw_work_string[0] = 0;
 
    while(our_sp != this->_kw_stack_pointer) {
        int lkw = strlen(this->_kw_stack[our_sp]);
        int ltot = strlen(this->_kw_work_string);
        if((lkw + ltot + 1) < (this->_kw_work_string_size - 1)) {
            // Append the keyword
            strcat(this->_kw_work_string, this->_kw_stack[our_sp]);
            // Append a space
            strcat(this->_kw_work_string, " ");
            our_sp--;
        }
        else {
            // All of the keywords don't fit in the avialable string size
            return this->_kw_work_string;
        }
    }
    return this->_kw_work_string;
}


/*
 * Simple line input with backspace support
 * Note: Nonblocking. Returns true if a line + enter is received, otherwise false
 */

bool Console::_get_line(char *line_buffer, uint16_t line_buffer_size, uint16_t *line_index) {
    char c;

    // Sanity check
    if(!line_buffer || !line_index || (*line_index >= line_buffer_size)) {
        return false; // Es una tontería
    }
   
    if (!Serial1.available()) {
        return false;
    }
    c = Serial1.read();

    if(c == 0x0a) { /* Ignore line feed */
        return false;
    }
    else if(c == 0x0d) { /* Enter key (linux) */
        line_buffer[*line_index] = 0;
        return true;

    }
    else if(c == 0x08) { /* Backspace key */
        if(*line_index) {
            Serial1.write(0x08);
            Serial1.write(' ');
            Serial1.write(0x08);
            (*line_index)--;
        }

    }
    else if(c < 0x20) { /* Ignore other control characters */
        return false;
    }
    else { /* Printable characters */
        if(*line_index == (line_buffer_size - 1)) {
            /* No more room in line buffer */
            line_buffer[*line_index] = 0;
            Serial1.write('\n');
            return true;
            
        }
        else {
            Serial1.write(c);
            line_buffer[*line_index] = c;
            (*line_index)++;
            return false;
        }

    }
    return false;
}


 unsigned Console::_make_arglist(char **argv, int max_args, char *str) {
     int argc = 0;
     int state = FIND_FIRST_CHAR;
     bool done = false;
     char *arg_start = NULL;
 
     /* Trim spaces from end, if any */
 
     int index = strlen(str);
 
     /* index to last char */
     if(index) {
         index--;
     }
 
     while (index) {
         if(str[index] != ' ') {
             break;
         }
         str[index] = 0;
         index--;
     }
 
     /* Build Argument list */
 
     for(int index = 0; done == false; index++) {
         char c = str[index];
         switch(state) {
             case FIND_FIRST_CHAR:
                 if(c != ' ') {
                     arg_start = str + index;
                     state = FIND_FIRST_SPACE;
 
                 }
                 else if (!c) {
                     /* Found end of string */
                     done = true;
                 }
                 break;
 
             case FIND_FIRST_SPACE:
                 if((c == ' ') || (!c)) {
                     if(arg_start) {
                         if(c) {
                             /* Terminate argument */
                             str[index] = 0;
                         }
                         argv[argc++] = arg_start;
                         arg_start = NULL;
                         state = FIND_FIRST_CHAR;
                         if(argc >= max_args) {
                             done = true;
                         }
                     }
                     if(!c) {
                         /* Found end of string */
                         done = true;
                     }
 
                 }
                 break;
         }
 
     }
     return argc;
 
 }

 /*
 * Parse a command entered by the user
 */

bool Console::_parse_command(unsigned argc, const Command_Table_Entry_Type *top) {
	const Command_Table_Entry_Type *current_table = top;
	bool done = false;
	bool res = true;
	unsigned arg = 0;
	unsigned command_table_index;
	const Command_Table_Entry_Type *cte = NULL;

	this->_error_code = CEC_NONE;

	while(!done) {

		for(command_table_index = 0; !done; command_table_index++) {
			cte = current_table + command_table_index;
			/* Test for end of argument list */
			/* Test for end of table */
			if((arg >= argc) || (cte->keyword[0] == 0)) {
				done = true;
				res = false;
				this->_error_code = CEC_UNK_CMD;
				break;
			}
			if(!strcmp(cte->keyword, this->_arg_list[arg])) {
				/* Keyword Match */
				if(cte->next_table) { /* Case 1: Another command table? */
					current_table = cte->next_table;
					arg++;
					break;
				}
				else if(cte->command) { /* Case 2: Call a command function */
					/* Check for args */
					if(cte->arguments) {
						uint32_t table_arguments;
						arg++;

						/* Calculate the number of parameters */
						uint32_t num_params = argc - arg;

						/* Calculate the number of arguments in the table */
						for(table_arguments = 0; (cte->arguments[table_arguments] != AT_END) &&
							(table_arguments < MAX_ARGS) ; table_arguments++);

						/* Check that the correct number of arguments have been processed */
						if(num_params != table_arguments) {
							/* Argument mismatch error */
							this->_error_code = CEC_INC_NUM_PARAMS;
							done = true;
							res = false;
						}
						/* Save number of parameters for get unsigned method */
						this->_max_parameters = num_params;
						/* Process arguments here */
						uint32_t a_index;
						bool ap_done = false;
						for(a_index = 0; !done && !ap_done && (a_index < MAX_ARGS); a_index++) {
							uint8_t a_type = cte->arguments[a_index];

							switch(a_type) {
								case AT_UINT: {
									uint32_t val;
									if(sscanf(this->_arg_list[arg],"%lu", &val) != 1) {
										this->_error_code = CEC_PARAM_ERROR;
										done = true;
										res = false;
										break;
									}

									this->_args[a_index].id = AT_UINT;
									this->_args[a_index].uint = val;
									break;
								}

								case AT_INT: {
									int32_t val;
									if(sscanf(this->_arg_list[arg],"%ld", &val) != 1) {
										this->_error_code = CEC_PARAM_ERROR;
										done = true;
										res = false;
										break;
									}
									this->_args[a_index].id = AT_INT;
									this->_args[a_index].intg = val;
									break;
								}

								default:
									/* No more arguments to convert */
									ap_done = true;
									this->_args[a_index].id = AT_END;
									break;

							}
							arg++;
						}

					}
					else {
						this->_args[0].id = AT_END; /* No arguments for this command */
					}

					/* Ensure no previous error */
					if(!done) {
						/* Execute command */
						if(cte->command) {
							res = (*cte->command)(this->_args, &this->_error_code);
						}
						else {
							this->_error_code = CEC_TABLE_ERROR;
							res = false;
						}
						done = true;
					}

				}
				else {
					/* Unrecognized action */
					this->_error_code = CEC_TABLE_ERROR;
					res = false;
					done = true;
				}
			}
		}
	}
	return res;
}
