#include "utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// addi, andi, ori, xori, slli, srli, srai, ld, lw, lh, lb, lwu, lhu, lbu, jalr
//  sd, sw, sh, sb, beq, bne, blt, bge, bltu, bgeu, jal, lui

char* R_INSTRUCTIONS[R_INSTRUCTIONS_SIZE] = {"add", "sub", "xor", "or", "and", "sll", "srl", "sra", "slt", "sltu"};
char* I_INSTRUCTIONS[I_INSTRUCTIONS_SIZE] = {"addi", "xori", "ori", "andi", "slli", "srli", "srai", "lb", "lh", "lw", "ld", "lbu", "lhu", "lwu", "jalr"};
char* S_INSTRUCTIONS[S_INSTRUCTIONS_SIZE] = {"sb", "sh", "sw", "sd"};
char* B_INSTRUCTIONS[B_INSTRUCTIONS_SIZE] = {"beq", "bne", "blt", "bge", "bltu", "bgeu"};
char* U_INSTRUCTIONS[U_INSTRUCTIONS_SIZE] = {"lui"};
char* J_INSTRUCTIONS[J_INSTRUCTIONS_SIZE] = {"jal"};

uint32_t R_FUNCT3[R_INSTRUCTIONS_SIZE] = {0b000, 0b000, 0b100, 0b110, 0b111, 0b001, 0b101, 0b101, 0b010, 0b011};
uint32_t R_FUNCT7[R_INSTRUCTIONS_SIZE] = {0b0000000, 0b0100000, 0b0000000, 0b0000000, 0b0000000, 0b0000000, 0b0000000, 0b0100000, 0b0000000, 0b0000000};

uint32_t I_FUNCT3[I_INSTRUCTIONS_SIZE] = {0b000, 0b100, 0b110, 0b111, 0b001, 0b101, 0b101, 0b000, 0b001, 0b010, 0b011, 0b100, 0b101, 0b110, 0b000};

uint32_t S_FUNCT3[S_INSTRUCTIONS_SIZE] = {0b000, 0b001, 0b010, 0b011};

uint32_t B_FUNCT3[B_INSTRUCTIONS_SIZE] = {0b000, 0b001, 0b100, 0b101, 0b110, 0b111};

void format_input(char* str) {
    ignore_comments(str);
    replace_tabs_with_spaces(str);
    replace_commas_with_spaces(str);
    remove_extra_spaces(str, 1);
}
void format_input_no_newline(char* str) {
    ignore_comments(str);
    replace_tabs_with_spaces(str);
    replace_commas_with_spaces(str);
    remove_extra_spaces(str, 0);
}

char identify_type(const char* string) {
    char* string_copy = strdup(string);
    if (string_copy == NULL) {
        // error handling
        printf("NULL string in identify_type.\n");
        return 0;
    }

    char* str = strtok(string_copy, " ");
    if (str == NULL) {
        // error handling
        printf("NULL string in identify_type.\n");
        return 0;
    }

    char result = 0;
    for (size_t i = 0; i < R_INSTRUCTIONS_SIZE; i++) {
        if (strcmp(R_INSTRUCTIONS[i], str) == 0) {
            result = 'r';
            break;
        }
    }

    for (size_t i = 0; i < I_INSTRUCTIONS_SIZE; i++) {
        if (strcmp(I_INSTRUCTIONS[i], str) == 0) {
            result = 'i';
            break;
        }
    }

    for (size_t i = 0; i < S_INSTRUCTIONS_SIZE; i++) {
        if (strcmp(S_INSTRUCTIONS[i], str) == 0) {
            result = 's';
            break;
        }
    }

    for (size_t i = 0; i < B_INSTRUCTIONS_SIZE; i++) {
        if (strcmp(B_INSTRUCTIONS[i], str) == 0) {
            result = 'b';
            break;
        }
    }

    if (strcmp(U_INSTRUCTIONS[0], str) == 0) {
        result = 'u';
    }

    if (strcmp(J_INSTRUCTIONS[0], str) == 0) {
        result = 'j';
    }

    free(string_copy);
    return result;
}

uint32_t parse_register(char* token_string) {
    char* token = strtok(token_string, "\n");

    if (token == NULL) {
        strcpy(token, token_string);
    }

    if (strcmp(token, "x0") == 0 || strcmp(token, "zero") == 0) {
        return 0b00000;
    } else if (strcmp(token, "x1") == 0 || strcmp(token, "ra") == 0) {
        return 0b00001;
    } else if (strcmp(token, "x2") == 0 || strcmp(token, "sp") == 0) {
        return 0b00010;
    } else if (strcmp(token, "x3") == 0 || strcmp(token, "gp") == 0) {
        return 0b00011;
    } else if (strcmp(token, "x4") == 0 || strcmp(token, "tp") == 0) {
        return 0b00100;
    } else if (strcmp(token, "x5") == 0 || strcmp(token, "t0") == 0) {
        return 0b00101;
    } else if (strcmp(token, "x6") == 0 || strcmp(token, "t1") == 0) {
        return 0b00110;
    } else if (strcmp(token, "x7") == 0 || strcmp(token, "t2") == 0) {
        return 0b00111;
    } else if (strcmp(token, "x8") == 0 || strcmp(token, "s0") == 0 || strcmp(token, "fp") == 0) {
        return 0b01000;
    } else if (strcmp(token, "x9") == 0 || strcmp(token, "s1") == 0) {
        return 0b01001;
    } else if (strcmp(token, "x10") == 0 || strcmp(token, "a0") == 0) {
        return 0b01010;
    } else if (strcmp(token, "x11") == 0 || strcmp(token, "a1") == 0) {
        return 0b01011;
    } else if (strcmp(token, "x12") == 0 || strcmp(token, "a2") == 0) {
        return 0b01100;
    } else if (strcmp(token, "x13") == 0 || strcmp(token, "a3") == 0) {
        return 0b01101;
    } else if (strcmp(token, "x14") == 0 || strcmp(token, "a4") == 0) {
        return 0b01110;
    } else if (strcmp(token, "x15") == 0 || strcmp(token, "a5") == 0) {
        return 0b01111;
    } else if (strcmp(token, "x16") == 0 || strcmp(token, "a6") == 0) {
        return 0b10000;
    } else if (strcmp(token, "x17") == 0 || strcmp(token, "a7") == 0) {
        return 0b10001;
    } else if (strcmp(token, "x18") == 0 || strcmp(token, "s2") == 0) {
        return 0b10010;
    } else if (strcmp(token, "x19") == 0 || strcmp(token, "s3") == 0) {
        return 0b10011;
    } else if (strcmp(token, "x20") == 0 || strcmp(token, "s4") == 0) {
        return 0b10100;
    } else if (strcmp(token, "x21") == 0 || strcmp(token, "s5") == 0) {
        return 0b10101;
    } else if (strcmp(token, "x22") == 0 || strcmp(token, "s6") == 0) {
        return 0b10110;
    } else if (strcmp(token, "x23") == 0 || strcmp(token, "s7") == 0) {
        return 0b10111;
    } else if (strcmp(token, "x24") == 0 || strcmp(token, "s8") == 0) {
        return 0b11000;
    } else if (strcmp(token, "x25") == 0 || strcmp(token, "s9") == 0) {
        return 0b11001;
    } else if (strcmp(token, "x26") == 0 || strcmp(token, "s10") == 0) {
        return 0b11010;
    } else if (strcmp(token, "x27") == 0 || strcmp(token, "s11") == 0) {
        return 0b11011;
    } else if (strcmp(token, "x28") == 0 || strcmp(token, "t3") == 0) {
        return 0b11100;
    } else if (strcmp(token, "x29") == 0 || strcmp(token, "t4") == 0) {
        return 0b11101;
    } else if (strcmp(token, "x30") == 0 || strcmp(token, "t5") == 0) {
        return 0b11110;
    } else if (strcmp(token, "x31") == 0 || strcmp(token, "t6") == 0) {
        return 0b11111;
    } else {
        return -1;
    }
}

uint32_t get_funct3(char type, char* str) {
    switch (type) {
        case 'r': {
            for (int i = 0; i < R_INSTRUCTIONS_SIZE; i++) {
                if (strcmp(R_INSTRUCTIONS[i], str) == 0) {
                    return R_FUNCT3[i];
                }
            }
            break;
        }

        case 'i': {
            for (int i = 0; i < I_INSTRUCTIONS_SIZE; i++) {
                if (strcmp(I_INSTRUCTIONS[i], str) == 0) {
                    return I_FUNCT3[i];
                }
            }
            break;
        }

        case 's': {
            for (int i = 0; i < S_INSTRUCTIONS_SIZE; i++) {
                if (strcmp(S_INSTRUCTIONS[i], str) == 0) {
                    return S_FUNCT3[i];
                }
            }
            break;
        }

        case 'b': {
            for (int i = 0; i < B_INSTRUCTIONS_SIZE; i++) {
                if (strcmp(B_INSTRUCTIONS[i], str) == 0) {
                    return B_FUNCT3[i];
                }
            }
            break;
        }

        default: {
            return -1;
            break;
        }
    }
    return -1u;
}

uint32_t get_funct7(char type, char* str) {
    switch (type) {
        case 'r': {
            for (size_t i = 0; i < R_INSTRUCTIONS_SIZE; i++) {
                if (strcmp(R_INSTRUCTIONS[i], str) == 0) {
                    return R_FUNCT7[i];
                }
            }
        }
        default:
            break;
    }
    return -1;
}

// I instructions can have one of two opcodes
_Bool check_load_or_not(char* str) {
    int first_load_instruction_index = 7;
    //-1 here to exclude "jalr"
    for (int i = 0; i < I_INSTRUCTIONS_SIZE - 1; i++) {
        if (strcmp(str, I_INSTRUCTIONS[i]) == 0) {
            if (i >= first_load_instruction_index) {
                return true;
            }
        }
    }

    return false;
}

void replace_commas_with_spaces(char* str) {
    int i = 0;
    while (str[i] != 0) {
        if (str[i] == ',') {
            str[i] = ' ';
        }
        i++;
    }
}

void replace_tabs_with_spaces(char* str) {
    char* src = str;
    while (*src != '\0') {
        if (*src == '\t') {
            *src = ' ';  // Replace tab with space
        }
        src++;
    }
}

void remove_extra_spaces(char* str, _Bool add_newline) {
    int i = 0, j = 0;
    int in_space = 0;

    while (str[i] != '\0') {
        // Ignore spaces and newlines
        if (str[i] != ' ' && str[i] != '\n') {
            // Check if the current character is a parenthesis
            if ((str[i] == '(' || str[i] == ')') && j > 0 && str[j - 1] == ' ') {
                // Remove the space before a parenthesis
                j--;
            }
            str[j++] = str[i];
            in_space = 0;
        } else if (str[i] == ' ' && !in_space && j > 0) {
            // Avoid adding space before or after parentheses
            if (str[j - 1] != '(' && str[j - 1] != ')') {
                str[j++] = ' ';
                in_space = 1;
            }
        }
        i++;
    }

    // Remove trailing space if it exists
    if (j > 0 && str[j - 1] == ' ') {
        j--;
    }

    // Conditionally add a newline at the end
    if (add_newline) {
        str[j++] = '\n';
    }

    // Null-terminate the string
    str[j] = '\0';
}

void ignore_comments(char* str) {
    int i = 0;
    while (str[i] != 0) {
        if (str[i] == ';' || str[i] == '#') {
            str[i] = 0;
            return;
        }
        i++;
    }
}

_Bool check_has_label(char* str) {
    int i = 0;
    while (str[i] != 0) {
        if (str[i] == ' ' || str[i] == ',') {
            return false;
        } else if (str[i] == ':' && str[i + 1] == ' ') {
            return true;
        } else if (str[i] == ':' && str[i + 1] == '\n') {
            return true;
        }
        i++;
    }

    return false;
}

void add_label(const char* label_name, int line_num) {
    // increase label count by 1 and reallocate label_array
    label_count++;
    label_array = realloc(label_array, label_count * sizeof(label));

    // Error handling: exit program if memory allocation fails
    if (label_array == NULL) {
        red("Memory allocation failed while expanding label array!\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the label name and copy it
    label_array[label_count - 1].label_name = malloc(strlen(label_name) + 1);
    strcpy(label_array[label_count - 1].label_name, label_name);
    label_array[label_count - 1].line_num = line_num;
    label_array[label_count - 1].file_line_num = file_line_num;
}

void create_label_array(FILE* file_pointer) {
    // error handling: check if the input file is NULL
    if (file_pointer == NULL) {
        red("Error: File pointer is NULL in create_label_array.\n");
        return;
    }

    char line[256];                       // Buffer to hold each line (assuming no line is longer than 256 characters)
    int start_text_line = file_line_num;  // save the line number of the line after to .text

    // Ensure the file pointer is positioned at the start of the .text section
    fseek(file_pointer, 0, SEEK_SET);  // Set the pointer to the beginning of the file

    // Move the file pointer to where line after .text starts (first instruction/blank line/label) (skip .data section)
    for (int i = 1; i < start_text_line; i++) {
        if (fgets(line, sizeof(line), file_pointer) == NULL) {
            printf("Error: Failed to find the .text section.\n");
            return;
        }
    }

    // Process the file line by line, add label to the dynamic label array
    while (fgets(line, sizeof(line), file_pointer) != NULL) {
        // format input to remove spaces and append a new line at the end
        format_input(line);

        // if the line is empty, do nothing
        // don't increment text_line_num since it doesn't account empty lines/ lines with just labels
        if (strcmp(line, "\n") == 0) {
            file_line_num++;
            continue;
        }

        // if a non-empty line without a label is found, increment both counters
        // don't worry about lines with just comments since formating is done
        if (!check_has_label(line)) {
            file_line_num++;
            text_line_num++;
            continue;
        }

        // after above two conditions, line must have a label, with or without instruction
        int is_just_label = false;

        // Check if the line contains only the label
        if (line[strlen(line) - 2] == ':') {
            is_just_label = true;
        }

        // Extract the label (before the colon)
        char* label_str = strtok(line, ":");

        // Check for duplicate labels in the array
        if (label_str != NULL) {
            for (int i = 0; i < label_count; i++) {
                if (strcmp(label_array[i].label_name, label_str) == 0) {
                    printf("Line %zu: Label '%s' already exists at line %d.\n", file_line_num, label_str, label_array[i].file_line_num);
                    error_code = 200;
                    return;
                }
            }

            // Add the label to the dynamic label array
            add_label(label_str, text_line_num);

            // conditionally increment text_line_num, if line has an instruction
            if (!is_just_label) {
                text_line_num++;
            }

            file_line_num++;
        }
    }
    text_line_num = 1;

    // After creating the label array, rewind to the .text section (stored as start_text_line)
    fseek(file_pointer, 0, SEEK_SET);  // Rewind the file to the beginning
    for (int i = 1; i < start_text_line; i++) {
        fgets(line, sizeof(line), file_pointer);  // Move back to the line after .text
    }

    file_line_num = start_text_line;  // Restore the file line number
    return;
}

// Cleanup function to free memory after processing
void free_label_array() {
    for (int i = 0; i < label_count; i++) {
        free(label_array[i].label_name);  // Free each label's name
    }
    free(label_array);  // Free the entire array
    label_array = NULL;
    label_count = 0;
}

char* get_error_string(int error_code) {
    switch (error_code) {
        case 101:
            return "\tUnidentified instruction name.\n";
        case 102:
            return "\tUnknown destination register.\n";
        case 103:
            return "\tUnknown source register.\n";
        case 104:
            return "\tUnknown character sequences at end-of-line.\n";
        case 105:
            return "\tExpected an *integer* immediate value.\n";
        case 106:
            return "\tImmediate value should be between -4096 to 4094.\n";
        case 107:
            return "\tImmediate value should be between 1 to 64.\n";
        case 108:
            return "\tImmediate value (offset in bytes) should be between -2048 to 2047.\n";
        case 109:
            return "\tLabel not found.\n";
        case 110:
            return "\tImmediate value must be positive and fit within 32 bits.\n";
        case 111:
            return "\tImmediate value doesn't fit in 21 bits.\n";
        case 112:
            return "\tThe format for 'jalr' is jalr rd, rs, imm.\n";
        case 200:
            return "\nMultiple labels with same name found.\n";
        // Mem related
        case 401:
            return "\nUnknown directive in input file\n";
        default:
            break;
    }

    return 0;
}

int get_label_line_num(char* label) {
    if (label == NULL) {
        printf("Error: Null label passed to get_label_line_num.\n");
        return -1;
    }

    // Iterate through the dynamically allocated label array
    for (int i = 0; i < label_count; i++) {
        // Check if the label matches
        if (strcmp(label, label_array[i].label_name) == 0) {
            // Return the relative line number in the `.text` section
            // Subtract the starting line number of the .text section
            return label_array[i].line_num;
        }
    }

    // If the label is not found, return -1
    red("Error: Label '%s' not found in get_label_line_num.\n", label);
    return -1;
}

void debug_enabled(const char* format, ...) {
    if (debug_flag && format != NULL) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
    return;
}

// Colors:

// Reset formatting
#define RESET "\033[0m"

// Color and formatting codes
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"

void print_color(const char* color, const char* format, va_list args) {
    printf("%s", color);
    vprintf(format, args);
    printf("\033[0m");  // Reset color
}

// Functions for each color
void red(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print_color(RED, format, args);
    va_end(args);
}

void green(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print_color(GREEN, format, args);
    va_end(args);
}

void yellow(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print_color(YELLOW, format, args);
    va_end(args);
}

void blue(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print_color(BOLD BLUE, format, args);
    va_end(args);
}

void blue_underlined(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print_color(BLUE UNDERLINE BOLD, format, args);
    va_end(args);
}

void magenta(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print_color(MAGENTA, format, args);
    va_end(args);
}

void cyan(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print_color(CYAN, format, args);
    va_end(args);
}

// Functions for bold and underlined text
void bold(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print_color(BOLD, format, args);
    va_end(args);
}

void underlined(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print_color(UNDERLINE, format, args);
    va_end(args);
}
void bold_underlined(const char* format, ...) {
    va_list args;
    va_start(args, format);
    print_color(UNDERLINE BOLD, format, args);
    va_end(args);
}

void clear_screen() {
    printf("\033[2J\033[H");  // Clear the screen and move cursor to home position
    fflush(stdout);           // Ensure the output is flushed immediately
}

unsigned int hash(uint64_t address) {
    return address % MEMORY_TABLE_SIZE;
}

void write_memory_byte(uint32_t address, uint8_t value) {
    unsigned int index = hash(address);  // Get the index using hash function
    memory_node* current = memory_table[index];

    // Traverse the chain at this index to find the memory address
    while (current != NULL && current->address != address) {
        current = current->next;
    }

    if (current != NULL) {
        // Address found, update its value
        current->value = value;
        return;
    }

    // If address is not found, create a new node and add it to the chain
    memory_node* new_node = (memory_node*)malloc(sizeof(memory_node));
    if (new_node == NULL) {
        printf("Error: Memory allocation failed!\n");
        return;
    }

    new_node->address = address;
    new_node->value = value;
    new_node->next = memory_table[index];  // Insert at the head of the chain
    memory_table[index] = new_node;        // Update the head of the chain
}

void write_memory_half(uint32_t address, uint16_t value) {
    // Break down the halfword (16 bits) into two 8-bit parts (bytes)
    uint8_t byte1 = value & 0xFF;         // Lower 8 bits
    uint8_t byte2 = (value >> 8) & 0xFF;  // Upper 8 bits

    // Write the two bytes to consecutive memory locations
    write_memory_byte(address, byte1);
    write_memory_byte(address + 1, byte2);
}

void write_memory_word(uint32_t address, uint32_t value) {
    // Break down the word (32 bits) into four 8-bit parts (bytes)
    uint8_t byte1 = value & 0xFF;          // Lowest 8 bits
    uint8_t byte2 = (value >> 8) & 0xFF;   // Next 8 bits
    uint8_t byte3 = (value >> 16) & 0xFF;  // Next 8 bits
    uint8_t byte4 = (value >> 24) & 0xFF;  // Highest 8 bits

    // Write the four bytes to consecutive memory locations
    write_memory_byte(address, byte1);
    write_memory_byte(address + 1, byte2);
    write_memory_byte(address + 2, byte3);
    write_memory_byte(address + 3, byte4);
}

void write_memory_dword(uint32_t address, uint64_t value) {
    // Break down the double word (64 bits) into eight 8-bit parts (bytes)
    uint8_t byte1 = value & 0xFF;          // Lowest 8 bits
    uint8_t byte2 = (value >> 8) & 0xFF;   // Next 8 bits
    uint8_t byte3 = (value >> 16) & 0xFF;  // Next 8 bits
    uint8_t byte4 = (value >> 24) & 0xFF;  // Next 8 bits
    uint8_t byte5 = (value >> 32) & 0xFF;  // Next 8 bits
    uint8_t byte6 = (value >> 40) & 0xFF;  // Next 8 bits
    uint8_t byte7 = (value >> 48) & 0xFF;  // Next 8 bits
    uint8_t byte8 = (value >> 56) & 0xFF;  // Highest 8 bits

    // Write the eight bytes to consecutive memory locations
    write_memory_byte(address, byte1);
    write_memory_byte(address + 1, byte2);
    write_memory_byte(address + 2, byte3);
    write_memory_byte(address + 3, byte4);
    write_memory_byte(address + 4, byte5);
    write_memory_byte(address + 5, byte6);
    write_memory_byte(address + 6, byte7);
    write_memory_byte(address + 7, byte8);
}

// Decode instruction type from 32 bit instruction word
int decode_type(uint32_t instruction) {
    int opcode = instruction & 0b1111111;
    switch (opcode) {
        case 0b0110011:  // r
            return 10;
            break;
        case 0b0010011:  // immediate (excluding load, jalr)
            return 20;
            break;
        case 0b0000011:  // load
            return 21;
            break;
        case 0b1100111:  // jalr
            return 22;
            break;
        case 0b0100011:  // store
            return 30;
            break;
        case 0b1100011:  // branch
            return 40;
            break;
        case 0b0110111:  // u
            return 50;
            break;
        case 0b1101111:  // j
            return 60;
            break;

        default:
            break;
    }
    return -1;
}

int64_t sign_extend_12bit(uint32_t value) {
    // Sign-extend the 12-bit value by shifting it left by 20 (to put the sign bit in the 32nd bit),
    // then perform an arithmetic shift right to bring it back, preserving the sign.
    return (int64_t)((int32_t)(value << 20) >> 20);
}

void free_instructions_array() {
    for (size_t i = 1; i <= max_instructions; i++) {
        free(instructions_array[i].instruction);
    }
}

void push_to_stack(char* function_name, int line_num) {
    stack_node* new = malloc(sizeof(stack_node));

    new->function_name = malloc(sizeof(function_name));
    strcpy(new->function_name, function_name);
    new->line_num = line_num;
    new->next = NULL;

    stack_node* current = main_node;
    while (current->next != NULL) {
        current = current->next;
    }

    current->next = new;
}

// can call pop even for main
void pop_from_stack() {
    stack_node* current = main_node;
    stack_node* last_but_one = NULL;

    if (current == NULL) {
        return;
    }

    if (main_node->next == NULL) {
        free(main_node->function_name);
        free(main_node);
        main_node = NULL;
        return;
    }

    while (current->next != NULL) {
        last_but_one = current;
        current = current->next;
    }

    free(current->function_name);
    free(current);
    last_but_one->next = NULL;
}

void free_stack() {
    while (main_node != NULL) {
        pop_from_stack();
    }
}

stack_node* return_top_of_stack() {
    stack_node* current = main_node;

    if (current == NULL) {
        return NULL;
    }

    while (current->next != NULL) {
        current = current->next;
    }

    return current;
}

void clean_memory() {
    for (int i = 0; i < MEMORY_TABLE_SIZE; i++) {
        memory_table[i] = NULL;
    }
}
