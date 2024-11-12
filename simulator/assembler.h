#include<stdint.h>
#ifndef ASSEMBLER
#define ASSEMBLER

// void instruction_R(char*);
uint32_t assemble(const char*);
uint32_t assemble_r(const char*);
uint32_t assemble_i(const char*);
uint32_t assemble_s(const char*);
uint32_t assemble_b(const char*);
uint32_t assemble_u(const char*);
uint32_t assemble_j(const char*);
#endif