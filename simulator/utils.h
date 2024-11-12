#include <stdarg.h>  // Required for variadic functions
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef UTILS
#define UTILS

void replace_commas_with_spaces(char* str);
void replace_tabs_with_spaces(char* str);
void remove_extra_spaces(char* str, _Bool add_newline);
void ignore_comments(char* str);
void format_input(char*);
void format_input_no_newline(char*);

char identify_type(const char*);
uint32_t parse_register(char*);
uint32_t get_funct3(char, char*);
uint32_t get_funct7(char, char*);

_Bool check_has_label(char*);
void create_label_array(FILE*);
_Bool check_load_or_not(char* str);
char* get_error_string(int error_code);
int get_label_line_num(char*);
void debug_enabled(const char* format, ...);

unsigned int hash(uint64_t);
void write_memory_byte(uint32_t address, uint8_t value);
void write_memory_half(uint32_t address, uint16_t value);
void write_memory_word(uint32_t address, uint32_t value);
void write_memory_dword(uint32_t address, uint64_t value);

#define R_INSTRUCTIONS_SIZE 10
#define I_INSTRUCTIONS_SIZE 15
#define S_INSTRUCTIONS_SIZE 4
#define B_INSTRUCTIONS_SIZE 6
#define U_INSTRUCTIONS_SIZE 1
#define J_INSTRUCTIONS_SIZE 1
#define MEMORY_TABLE_SIZE 5000

extern size_t file_line_num;
extern int text_line_num;  // points to the current line in .text section being parsed. Ignores blank lines, and lines with just labels. Starts after .text, so .text is not counted. Special ability: can make you cry sometimes!
extern int error_code;
extern _Bool debug_flag;
extern _Bool break_line_found;
typedef struct memory_node {
    uint64_t address;          // Memory address (key)
    uint8_t value;             // Memory value (1 byte)
    struct memory_node* next;  // Pointer to next node (for collision handling)
} memory_node;

typedef struct instruction_line {
    char* instruction;
    bool break_point;
    int file_line_num;
} instruction_line;

extern instruction_line instructions_array[1000];

extern size_t max_instructions;
extern size_t current_instruction;

extern uint32_t pc;

typedef struct {
    char* label_name;
    int line_num;
    int file_line_num;
} label;

typedef struct stack_node {
    char* function_name;
    int line_num;
    struct stack_node* next;
} stack_node;

extern stack_node* main_node;

extern label* label_array;
extern int label_count;
void free_label_array();
extern memory_node* memory_table[MEMORY_TABLE_SIZE];

int decode_type(uint32_t instruction);
int64_t sign_extend_12bit(uint32_t value);

// Formatting Output
extern _Bool colors_enabled;
void underlined(const char* format, ...);

void bold(const char* format, ...);
void bold_underlined(const char* format, ...);

void blue(const char* format, ...);
void blue_underlined(const char* format, ...);

void red(const char* format, ...);
void green(const char* format, ...);

void magenta(const char* format, ...);
void cyan(const char* format, ...);

void yellow(const char* format, ...);

void clear_screen();

void free_instructions_array();

void push_to_stack(char*, int);
void pop_from_stack();
stack_node* return_top_of_stack();
void free_stack();
void clean_memory();

void display_help();
char* strdup(const char*);
#endif
