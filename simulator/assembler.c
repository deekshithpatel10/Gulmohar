#include "assembler.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

uint32_t assemble(const char* string_input) {
    char* string = strdup(string_input);
    format_input(string);
    if (check_has_label(string)) {
        // remove label
        strtok(string, ": ");
        string = strtok(NULL, "\n");
    }

    // not an error when string is a blank line
    if ((string == NULL) || (string[0] == '\0') || (strcmp(string, "\n") == 0)) {
        return 0u;
    };
    char type = identify_type(string);
    switch (type) {
        case 'r':
            return assemble_r(string);
        case 'i':
            return assemble_i(string);
        case 's':
            return assemble_s(string);
        case 'b':
            return assemble_b(string);
        case 'u':
            return assemble_u(string);
        case 'j':
            return assemble_j(string);
        default:
            break;
    }

    // error code for unidentified instruction is 101
    error_code = 101;
    return -1u;
}

uint32_t assemble_r(const char* string) {
    uint32_t opcode = 0b0110011;
    char* string_copy = strdup(string);

    if (string_copy == NULL) {
        // error handling
        printf("Something went wrong while assembling.\n");
        return 0;
    }

    // Tokenize the string and store them
    char* instruction_str = strtok(string_copy, " ");
    char* rd_str = strtok(NULL, " ");
    char* rs1_str = strtok(NULL, " ");
    char* rs2_str = strtok(NULL, "\n");

    uint32_t funct3 = get_funct3('r', instruction_str) << 12;
    uint32_t funct7 = get_funct7('r', instruction_str) << 25;

    uint32_t rd = parse_register(rd_str);
    uint32_t rs1 = parse_register(rs1_str);
    uint32_t rs2 = parse_register(rs2_str);

    // unidentified rd and rs error codes are 102 and 103
    if (rd == -1u) {
        error_code = 102;
        return -1u;
    }
    if (rs1 == -1u || rs2 == -1u) {
        error_code = 103;
        return -1u;
    }

    rd <<= 7;
    rs1 <<= 15;
    rs2 <<= 20;
    return opcode + rd + funct3 + rs1 + rs2 + funct7;
}

uint32_t assemble_i(const char* string) {
    uint32_t opcode;
    char* string_copy = strdup(string);

    if (string_copy == NULL) {
        // error handling
        printf("Something went wrong in assemble_i.\n");
        return 0;
    }

    char* instruction = strtok(string_copy, " ");

    if (check_load_or_not(instruction)) {
        opcode = 0b0000011;

        char* dest = strtok(NULL, " ");
        char* immediate = strtok(NULL, "(");
        char* source = strtok(NULL, "\n");

        if (source == NULL) {
            error_code = 103;
            return -1u;
        }

        char* source1 = (char*)malloc(strlen(source) * sizeof(char));

        strcpy(source1, source);
        char* source2 = strtok(source1, ")");

        if (strcmp(source2, source) == 0) {
            error_code = 103;
            return -1u;
        }

        uint32_t rd = parse_register(dest);
        uint32_t rs = parse_register(source2);
        uint32_t funct3 = get_funct3('i', instruction) << 12;

        char* endptr;
        long int num = strtol(immediate, &endptr, 0L);
        // 105 is immediate value error
        if (*endptr != '\0') {
            error_code = 105;
            return -1u;
        }
        // 108 is immediate value out of bounds
        if (num < -2048 || num > 2047) {
            error_code = 108;
            return -1u;
        }
        // unidentified rd and rs error codes are 102 and 103
        if (rd == -1u) {
            error_code = 102;
            return -1u;
        }
        if (rs == -1u) {
            error_code = 103;
            return -1u;
        }

        uint32_t imm11_0 = num & 0b111111111111;

        rd <<= 7;
        rs <<= 15;
        imm11_0 <<= 20;
        return opcode + rd + funct3 + rs + imm11_0;

    } else if (strcmp(instruction, "jalr") == 0) {
        opcode = 0b1100111;
        uint32_t funct3 = 0b000;

        char* dest = strtok(NULL, " ");
        char* immediate = strtok(NULL, "(");
        char* source = strtok(NULL, "\n");
        if (source == NULL) {
            error_code = 103;
            return -1u;
        }

        char* source1 = (char*)malloc(strlen(source) * sizeof(char));

        strcpy(source1, source);
        char* source2 = strtok(source1, ")");

        if (strcmp(source2, source) == 0) {
            error_code = 103;
            return -1u;
        }

        uint32_t rd = parse_register(dest);
        uint32_t rs = parse_register(source2);
        char* endptr;
        if (immediate == NULL) {
            error_code = 112;
            return -1u;
        }
        long int num = strtol(immediate, &endptr, 0L);
        // 105 is immediate value error
        if (*endptr != '\0') {
            error_code = 105;
            return -1u;
        }
        // 108 is immediate value out of bounds
        if (num < -2048 || num > 2047) {
            error_code = 108;
            return -1u;
        }
        // unidentified rd and rs error codes are 102 and 103
        if (rd == -1u) {
            error_code = 102;
            return -1u;
        }
        if (rs == -1u) {
            error_code = 103;
            return -1u;
        }

        // get first 12bits of num
        uint32_t imm11_0 = num & 0b111111111111;

        rd <<= 7;
        rs <<= 15;
        imm11_0 <<= 20;

        return opcode + rd + funct3 + rs + imm11_0;
    }

    // following is executed only if instruction is neither load type nor jalr
    opcode = 0b0010011;
    uint32_t funct3 = get_funct3('i', instruction) << 12;

    char* dest = strtok(NULL, " ");
    char* source = strtok(NULL, " ");
    char* immediate = strtok(NULL, "\n");

    uint32_t rd = parse_register(dest);
    uint32_t rs = parse_register(source);

    // unidentified rd and rs error codes are 102 and 103
    if (rd == -1u) {
        error_code = 102;
        return -1u;
    }
    if (rs == -1u) {
        error_code = 103;
        return -1u;
    }
    if (immediate == NULL) {
        error_code = 105;
        return -1u;
    }

    char* endptr;
    long int num = strtol(immediate, &endptr, 0L);
    // 105 is immediate value error
    if (*endptr != '\0') {
        error_code = 105;
        return -1u;
    }

    int shift_instruction_flag = 0;
    if (strcmp(instruction, "slli") == 0) {
        shift_instruction_flag = 1;
    } else if (strcmp(instruction, "srli") == 0) {
        shift_instruction_flag = 2;
    } else if (strcmp(instruction, "srai") == 0) {
        shift_instruction_flag = 3;
    }

    rd <<= 7;
    rs <<= 15;

    // check immediate values here
    if (shift_instruction_flag == 0) {
        if (num < -2048 || num > 2047) {
            error_code = 108;
            return -1u;
        }

        num <<= 20;
    } else {
        if (num < 1 || num > 64) {
            error_code = 107;
            return -1u;
        }

        uint32_t funct6 = 0;
        uint32_t imm50 = num & 0b111111;
        imm50 <<= 20;

        // srai has a different funct6
        if (shift_instruction_flag == 3) {
            funct6 = 0b010000;
            funct6 <<= 26;
        }

        return opcode + rd + funct3 + rs + imm50 + funct6;
    }

    return opcode + rd + funct3 + rs + num;
}

uint32_t assemble_s(const char* string) {
    int32_t opcode = 0b0100011;
    char* string_copy = strdup(string);

    if (string_copy == NULL) {
        // error handling
        printf("Something went wrong in assemble_s.\n");
        return 0;
    }

    char* instruction = strtok(string_copy, " ");

    uint32_t funct3 = get_funct3('s', instruction);
    char* source1 = strtok(NULL, " ");
    char* offset = strtok(NULL, "(");
    char* source2 = strtok(NULL, "\n");

    if (source2 == NULL) {
        error_code = 103;
        return -1u;
    }

    char* source2_temp = (char*)malloc(strlen(source2) * sizeof(char));

    strcpy(source2_temp, source2);
    char* source2_final = strtok(source2_temp, ")");

    if (strcmp(source2_final, source2) == 0) {
        error_code = 103;
        return -1u;
    }
    // sd/w/b rs2 n(rs1)
    uint32_t rs2 = parse_register(source1);
    uint32_t rs1 = parse_register(source2_final);

    // unidentified rs error code is 103 (both are source)
    if (rs1 == -1u || rs2 == -1u) {
        error_code = 103;
        return -1u;
    }

    char* endptr;
    long int num = strtol(offset, &endptr, 0L);

    // 105 is immediate value error (for offet) (-2048 to 2047)
    if (*endptr != '\0') {
        error_code = 105;
        return -1u;
    }

    if (num < -2048 || num > 2047) {
        error_code = 108;
        return -1u;
    }

    // num[4:0] and num[11:5]
    uint32_t imm40 = num & 0b11111;
    uint32_t imm115 = num & 0b111111100000;

    // shifting bits
    rs1 <<= 15;
    imm40 <<= 7;
    imm115 <<= 20;
    rs2 <<= 20;
    funct3 <<= 12;

    return opcode + imm40 + funct3 + rs1 + rs2 + imm115;
};

uint32_t assemble_b(const char* string) {
    uint32_t opcode = 0b1100011;

    char* string_copy = strdup(string);

    if (string_copy == NULL) {
        // error handling
        printf("Something went wrong in assemble_b.\n");
        return 0;
    }

    char* instruction = strtok(string_copy, " ");
    uint32_t funct3 = get_funct3('b', instruction) << 12;

    char* source_1 = strtok(NULL, " ");
    char* source_2 = strtok(NULL, " ");
    char* label = strtok(NULL, "\n");

    uint32_t rs1 = parse_register(source_1);
    uint32_t rs2 = parse_register(source_2);
    int label_line_num = get_label_line_num(label);

    // unidentified rs error code is 103 (both are source)
    if (rs1 == -1u || rs2 == -1u) {
        error_code = 103;
        return -1u;
    }

    int num_bytes_to_jump;
    if (label_line_num == -1) {
        // check if a number has been entered
        char* endptr;
        long int num = strtol(label, &endptr, 0L);
        // 109 is label not found
        if (*endptr != '\0') {
            error_code = 109;
            return -1u;
        }

        // 106 is value out of bounds
        if (num < -4096 || num > 4094) {
            error_code = 106;
            return -1u;
        }

        num_bytes_to_jump = num;
    } else {
        num_bytes_to_jump = (label_line_num - text_line_num) * 4;
    }

    uint32_t imm11 = (num_bytes_to_jump & 0b100000000000) >> 11;
    uint32_t imm4_1 = (num_bytes_to_jump & 0b011110) >> 1;
    uint32_t imm10_5 = (num_bytes_to_jump & 0b11111100000) >> 5;
    uint32_t imm12 = (num_bytes_to_jump & 0b1000000000000) >> 12;

    rs1 <<= 15;
    rs2 <<= 20;
    imm12 <<= 31;
    imm11 <<= 7;
    imm4_1 <<= 8;
    imm10_5 <<= 25;

    return opcode + imm11 + imm4_1 + funct3 + rs1 + rs2 + imm10_5 + imm12;
};

uint32_t assemble_u(const char* string) {
    uint32_t opcode = 0b0110111;
    char* string_copy = strdup(string);

    if (string_copy == NULL) {
        // error handling
        printf("Something went wrong in assemble_u.\n");
        return 0;
    }

    // discard first token as only lui is possible
    strtok(string_copy, " ");
    char* dest = strtok(NULL, " ");
    char* immediate = strtok(NULL, "\n");

    uint32_t rd = parse_register(dest);

    char* endptr;
    long int num = strtol(immediate, &endptr, 0L);
    // 105 is immediate value error
    if (*endptr != '\0') {
        error_code = 105;
        return -1u;
    }

    // 102 is unidentified destination register
    if (rd == -1u) {
        error_code = 102;
        return -1u;
    }
    // 110 is when num is -ve or too big
    if (num < 0 || num > 4294967295) {
        error_code = 110;
        return -1u;
    }

    uint32_t imm = num << 12;
    rd <<= 7;

    return opcode + rd + imm;
};

uint32_t assemble_j(const char* string) {
    uint32_t opcode = 0b1101111;

    char* string_copy = strdup(string);
    if (string_copy == NULL) {
        // error handling
        printf("Something went wrong in assemble_j.\n");
        return 0;
    }

    // discard first token as only jal is possible
    strtok(string_copy, " ");
    char* dest = strtok(NULL, " ");
    char* label = strtok(NULL, "\n");

    int label_line_num = get_label_line_num(label);

    long int num_bytes_to_jump;
    if (label_line_num == -1) {
        // check if a number has been entered
        char* endptr;
        long int num = strtol(label, &endptr, 0L);
        // 109 is label not found
        if (*endptr != '\0') {
            error_code = 109;
            return -1u;
        }

        // 111 is value out of bounds
        if (num < -1048576 || num > 1048575) {
            error_code = 111;
            return -1u;
        }

        num_bytes_to_jump = num;
    } else {
        num_bytes_to_jump = (label_line_num - text_line_num) * 4;
    }

    uint32_t rd = parse_register(dest);
    // 102 is unidentified destination register
    if (rd == -1u) {
        error_code = 102;
        return -1u;
    }

    uint32_t imm10_1 = (num_bytes_to_jump & 0b11111111110) >> 1;
    uint32_t imm19_12 = (num_bytes_to_jump & 0b11111111000000000000) >> 12;
    uint32_t imm11 = (num_bytes_to_jump & 0b100000000000) >> 11;
    uint32_t imm20 = (num_bytes_to_jump & 0b100000000000000000000) >> 20;

    rd <<= 7;
    imm19_12 <<= 12;
    imm11 <<= 20;
    imm10_1 <<= 21;
    imm20 <<= 31;

    return opcode + rd + imm19_12 + imm11 + imm10_1 + imm20;
};