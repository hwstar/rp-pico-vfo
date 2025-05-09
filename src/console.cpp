#include <Arduino.h>
#include "console.h"
#include "pll.h"
#include "persistent_storage.h"
#include "config_keys.h"


/*
 * Make a list of pointers to arguments from string passed in.
 *
 * Note: modifies the string passed in.
 *
 * Return number of arguments found.
 */

 enum {FIND_FIRST_CHAR=0, FIND_FIRST_SPACE};
 enum {PS_PROMPT=0, PS_WAIT_INPUT};

/*
* External classes
*/


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

 bool cal_on(Holder_Type *vars, uint8_t *error_code);
 bool cal_off(Holder_Type *vars, uint8_t *error_code);
 bool cal_set(Holder_Type *vars, uint8_t *error_code);
 bool cal_get(Holder_Type *vars, uint8_t *error_code);

 static const Command_Table_Entry_Type cal[] = {
    {NULL, cal_on, args_none, "on"},
    {NULL, cal_off, args_none, "off"},
    {NULL, cal_set, args_single_int, "set"},
    {NULL, cal_get, args_none, "get"},
    {NULL, NULL, NULL, ""} // End of table
 };

 static const Command_Table_Entry_Type top[] = {
    {cal, NULL, args_single_int, "cal"},
    {NULL, NULL, NULL, ""} // End of table
 };

 

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
* Console class methods
*/


 void Console::setup(void) {
    this->_poll_state = PS_PROMPT;
    this->_line_index = 0;
}


/*
 * Called by top.cpp repeatedly to process console data
 */

void Console::poll(void) {
	int argc;
	

    if(this->_poll_state == PS_PROMPT) {
        Serial1.println();
        Serial1.print(":");
        this->_poll_state = PS_WAIT_INPUT;
    }
    else {

        if(!this->_get_line()) {
            return;
        }
        else {
            if(strlen(this->_line_buffer)) {
                argc = this->_make_arglist((char **) &this->_arg_list, MAX_ARGS, this->_line_buffer);
                bool res = this->_parse_command(argc, top);
                if(!res) {
                    Serial1.println();
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


/*
 * Simple line input with backspace support
 */

bool Console::_get_line() {
    char c;

    if (!Serial1.available()) {
        return false;
    }
    c = Serial1.read();

    if(c == 0x0a) { /* Ignore line feed */
        return false;
    }
    else if(c == 0x0d) { /* Enter key (linux) */
        this->_line_buffer[this->_line_index] = 0;
        this->_line_index = 0;
        return true;

    }
    else if(c == 0x08) { /* Backspace key */
        if(this->_line_index) {
            Serial1.write(0x08);
            Serial1.write(' ');
            Serial1.write(0x08);
            this->_line_index--;
        }

    }
    else if(c < 0x20) { /* Ignore other control characters */
        return false;
    }
    else { /* Printable characters */
        if(this->_line_index == (LINE_BUFFER_SIZE - 1)) {
            /* No more room in line buffer */
            this->_line_buffer[this->_line_index] = 0;
            Serial1.write('\n');
            this->_line_index = 0;
            return true;
            
        }
        else {
            Serial1.write(c);
            this->_line_buffer[this->_line_index++] = c;
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
