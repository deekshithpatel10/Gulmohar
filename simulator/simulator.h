#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#ifndef SIMULATOR
#define SIMULATOR

extern FILE* input_file;
extern int64_t registers[32];
extern char* current_file_name;

_Bool load_file(char* file_name);
void initialise_registers();
ssize_t getline(char**, size_t*, FILE*);

void initialise_data_memory();
void initialise_text_memory();
void write_memory_byte(uint32_t address, uint8_t value);
uint8_t read_memory_byte(uint32_t address);
uint32_t read_memory_word(uint32_t address);
void display_memory(uint32_t start_address, size_t num_bytes);
void display_memory_table();
void load_data();
void load_data_byte(char* string);
void load_data_word(char* string);
void load_data_half(char* string);
void load_data_dword(char* string);

void display_registers();
void show_stack();
void insert_break(int break_line);
void delete_break(int break_line);

void step();

void execute(uint32_t instruction);
void execute_r(uint32_t instruction);
void execute_i(uint32_t instruction);
void execute_l(uint32_t instruction);
void execute_jr(uint32_t instruction);
void execute_s(uint32_t instruction);
void execute_b(uint32_t instruction);
void execute_u(uint32_t instruction);
void execute_j(uint32_t instruction);
#endif