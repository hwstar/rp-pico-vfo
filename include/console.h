#include <Arduino.h>
const uint8_t MAX_KEYWORD = 16;
const uint8_t MAX_ARGS = 8;

const uint16_t LINE_BUFFER_SIZE = 256;
const uint8_t KW_STACK_SIZE = 8;

enum {CEC_NONE = 0, CEC_UNK_CMD, CEC_INC_NUM_PARAMS, CEC_PARAM_ERROR, CEC_TABLE_ERROR, CEC_SETUP_ERROR, CEC_NUM_ERROR_CODES};
enum {AT_END = 0, AT_UINT, AT_INT, AT_STR32};


typedef struct Holder_Type {
	uint8_t id;
    union {
	    uint32_t uint;
        int32_t intg;
        char str32[32];
    };
} Holder_Type;

typedef bool (*Exec_Command_Type)(Holder_Type *vars, uint8_t *error_code);

typedef struct Command_Table_Entry_Type {
	const struct Command_Table_Entry_Type *next_table;
	Exec_Command_Type command;
	const uint8_t *arguments;
	const char keyword[MAX_KEYWORD];

} Command_Table_Entry_Type;

class Console {
    public:
    void setup(void);
    void poll(void);
    void rp_specific_reboot();
    void set_factory_defaults();
    bool confirm_action(const char *query_string);
    bool help_kw_stack_init(char *ws, int size);
    bool help_kw_stack_push(const char *keyword); 
    bool help_kw_stack_pop();
    char *help_kw_stack_cat();

    protected:
    unsigned _make_arglist(char **argv, int max_args, char *str);
    bool _parse_command(unsigned argc, const Command_Table_Entry_Type *top);
    bool _get_line(char *line_buffer, uint16_t line_buffer_size, uint16_t *line_index);

    int _kw_work_string_size;
    char * _kw_work_string;
    const char *_kw_stack[KW_STACK_SIZE];
    uint8_t _kw_stack_pointer;


    char *_arg_list[MAX_ARGS];
    Holder_Type _args[MAX_ARGS];
    uint8_t _poll_state;
    uint8_t _max_parameters;
    uint8_t _error_code;
    char _line_buffer[LINE_BUFFER_SIZE];

};