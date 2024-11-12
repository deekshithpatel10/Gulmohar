#include "./simulator.h"

#include <stdint.h>
#include <string.h>

#include "./assembler.h"
#include "./utils.h"
#include "./cache.h"

void load_data();

uint32_t data_memory_pointer = 0x10000;

_Bool load_file(char* file_name) {
    free_label_array();
    free_stack();
    clean_memory();
    error_code = 0;
    text_line_num = 1;
    file_line_num = 1;

    // Reset critical variables
    data_memory_pointer = 0x10000;
    pc = 0;
    current_instruction = 1;
    max_instructions = 0;

    if (file_name == NULL) {
        red("Error: Missing file name after 'load' command.\n");
        return false;
    }

    // Close any previously opened file
    if (input_file != NULL) {
        fclose(input_file);
    }
    input_file = fopen(file_name, "r");
    if (input_file == NULL) {
        red("File \"%s\" not found.\n", file_name);
    } else {
        initialise_data_memory();

        // After initialise_data_memory, file_line_num points to the first line in the .text section, excluding .text
        if (error_code == 0) {
            initialise_registers();
            if (error_code == 0) {
                create_label_array(input_file);
                debug_enabled("Created label array %s\n", file_name);
                initialise_text_memory();
                if (error_code == 0) {
                    //cyan("Loaded file %s\n", file_name);

                    // Initialize stack
                    main_node = malloc(sizeof(stack_node));
                    main_node->function_name = malloc(sizeof("main"));
                    strcpy(main_node->function_name, "main");
                    main_node->line_num = instructions_array[1].file_line_num - 1;
                    main_node->next = NULL;

                    // Initialize cache
                    if(cache_enabled) {
                        clear_cache();
                        open_cache_output_file(file_name);
                    }

                    return true;
                } else {
                    red("Text memory initialization failed.\n");
                }

            } else {
                red("Compilation error in %s\n", file_name);
            }

        } else {
            fclose(input_file);
            red("%s", get_error_string(error_code));
            red("Input file was not loaded\n");
        }
    }
    return false;
}

void initialise_registers() {
    for (size_t i = 0; i < 32; i++) {
        registers[i] = 0;
    }
    debug_enabled("Initialised all registers to 0x%llx\n", registers[0]);
    return;
}

// Read memory at an address
uint8_t read_memory_byte(uint32_t address) {
    unsigned int index = hash(address);  // Get the index using hash function
    memory_node* current = memory_table[index];

    // Traverse the chain at this index to find the memory address
    while (current != NULL) {
        if (current->address == address) {
            return current->value;  // Return the value if address found
        }
        current = current->next;
    }

    // Address not found, return 0x0 (default for uninitialized memory)
    return 0x0;
}

// Read 32-bit word from memory starting at the given address
uint32_t read_memory_word(uint32_t address) {
    uint32_t word = 0;

    // Read four consecutive bytes from memory, starting from the given address
    for (int i = 0; i < 4; i++) {
        uint8_t byte = read_memory_byte(address + i);  // Read each byte
        word |= ((uint32_t)byte << (i * 8));                     // Shift and combine into the word
    }

    return word;
}

// Read 16-bit word from memory starting at the given address
uint16_t read_memory_half(uint32_t address) {
    uint16_t word = 0;

    // Read four consecutive bytes from memory, starting from the given address
    for (int i = 0; i < 2; i++) {
        uint8_t byte = read_memory_byte(address + i);  // Read each byte
        word |= ((uint16_t)byte << (i * 8));                     // Shift and combine into the word
    }

    return word;
}

// Read 64-bit word from memory starting at the given address
uint64_t read_memory_dword(uint32_t address) {
    uint64_t word = 0;

    // Read four consecutive bytes from memory, starting from the given address
    for (int i = 0; i < 8; i++) {
        uint8_t byte = read_memory_byte(address + i);  // Read each byte
        word |= ((uint64_t)byte << (i * 8));                     // Shift and combine into the word
    }

    return word;
}

// Display a range of memory addresses
void display_memory(uint32_t start_address, size_t num_bytes) {
    for (size_t i = 0; i < num_bytes; i++) {
        uint32_t address = start_address + i;
        uint8_t value = read_memory_byte(address);
        //cyan("Memory[0x%X] = 0x%02X\n", address, value);
        printf("Memory[0x%X] = 0x%X\n", address, value);
    }
}

void display_memory_table() {
}

// initialise data segment and text segment by reading input file until you find .text
void initialise_data_memory() {
    char* line = NULL;  // Pointer to hold the line
    size_t len = 0;
    bool found_data_section = 0;
    while (getline(&line, &len, input_file) != -1) {
        format_input(line);
        if (line != NULL && strcmp(line, ".data\n") == 0) {
            found_data_section = 1;
            file_line_num++;
            load_data();
            break;
        }
        file_line_num++;        // increment file line number
    }
    if (!found_data_section) {
        debug_enabled("No data section was found in the input file\n");
        // if data section wasn't found, set file_line_num to line after .text
        fseek(input_file, 0, SEEK_SET); 
        file_line_num = 1;
        int found_text_section = 0;
        while(getline(&line, &len, input_file) != -1) {
            format_input(line);
            if(line!=NULL && strcmp(line, ".text\n") == 0) {
                file_line_num++;
                found_text_section = 1;
                break;
            }
            file_line_num++;
        }

        if(!found_text_section) {
            file_line_num = 1;      // if .text wasn't found at all, and since .data was also not found, set it to 1
        }

    }

    free(line);
    return;
}

void initialise_text_memory() {
    uint32_t mem = 0;
    char* line = NULL;
    char* linecpy = NULL;
    size_t len = 0;
    int temp = file_line_num;
    while (getline(&line, &len, input_file) != -1) {

        linecpy = (char*)malloc(strlen(line) + 1);  // Create a copy of the line
        strcpy(linecpy, line);
        replace_tabs_with_spaces(linecpy);
        remove_extra_spaces(linecpy, 0);
        if (check_has_label(linecpy)) {     // to store line without label
            // remove label
            strtok(linecpy, " ");
            linecpy = strtok(NULL, "\n");
        }

        format_input(line);
        int32_t reponse = assemble(line);


        if (reponse == -1) {
            red("\n\033[4m%s:%d\033[0m: %s", current_file_name, file_line_num, linecpy);
            red("Error: ");
            printf("%s", get_error_string(error_code));
            file_line_num++;
            text_line_num++;
            return;
        } else if (reponse == 0) {
            file_line_num++;
            // empty line/line with just a label encountered, do nothing

        } else {
            write_memory_word(mem, reponse);

            // save the instruction in an array, and set break_point to false
            instructions_array[max_instructions + 1].instruction = (char*)malloc(strlen(linecpy) + 1);
            strcpy(instructions_array[max_instructions + 1].instruction, linecpy);
            instructions_array[max_instructions + 1].file_line_num = file_line_num;
            instructions_array[max_instructions + 1].break_point = false;
            max_instructions += 1;
            mem += 4;
            text_line_num++;
            file_line_num++;
        }
    }

    file_line_num = temp;   // equals to the line next to .text
    rewind(input_file);
    for (size_t i = 1; i < file_line_num; i++) {
        getline(&line, &len, input_file);
    }

    text_line_num = 1;
}

void load_data() {
    char* line = NULL;
    size_t len = 0;
    char* current_directive = NULL;  // Tracks the active directive

    while (getline(&line, &len, input_file) != -1) {
        format_input_no_newline(line);  // Remove extra spaces/newlines

        if (strcmp(line, "") == 0) {
            file_line_num++;
            continue;  // Skip empty lines
        }

        if (strcmp(line, ".text") == 0) {
            file_line_num++;
            break;  // Stop on .text
        }

        char* linecpy = strdup(line);  // Create a copy of the line

        char* token = strtok(line, " ");  // Get the first token

        if (token == NULL) {
            file_line_num++;
            free(linecpy);
            continue;  // Skip lines without tokens
        }

        // Check if the token is a directive
        if (strcmp(token, ".byte") == 0 || strcmp(token, ".half") == 0 ||
            strcmp(token, ".word") == 0 || strcmp(token, ".dword") == 0) {
            // Update current directive
            if (current_directive != NULL) {
                free(current_directive);
            }
            current_directive = strdup(token);

            // Get the remaining data on the same line, if any
            char* data = strtok(NULL, "");  // The rest of the line after the directive

            // Process data if present
            if (data != NULL && strcmp(data, "") != 0) {
                if (strcmp(current_directive, ".byte") == 0) {
                    load_data_byte(data);
                } else if (strcmp(current_directive, ".half") == 0) {
                    load_data_half(data);
                } else if (strcmp(current_directive, ".word") == 0) {
                    load_data_word(data);
                } else if (strcmp(current_directive, ".dword") == 0) {
                    load_data_dword(data);
                }
            }
        } else {
            // Process data line using the current directive
            if (current_directive != NULL) {
                if (strcmp(current_directive, ".byte") == 0) {
                    load_data_byte(linecpy);
                } else if (strcmp(current_directive, ".half") == 0) {
                    load_data_half(linecpy);
                } else if (strcmp(current_directive, ".word") == 0) {
                    load_data_word(linecpy);
                } else if (strcmp(current_directive, ".dword") == 0) {
                    load_data_dword(linecpy);
                }
            } else {
                red("Error: Data values provided without a directive at line %d.\n", file_line_num);
                error_code = 402;
                free(linecpy);
                free(line);
                return;
            }
        }

        free(linecpy);  // Free the line copy after processing
        file_line_num++;
    }

    if (current_directive != NULL) {
        free(current_directive);
    }
    free(line);  // Free the allocated line buffer
}

void load_data_byte(char* string) {
    char* token = strtok(string, " ");
    while (token != NULL) {
        // Convert token to uint8_t (byte)
        long num = strtol(token, NULL, 0);  // Use long for range checking

        // Check if the value is out of range for a byte (0 to 255)
        if (num < -128 || num > 255) {
            red("Error: byte value \"%ld\" in line %d not in range (-128 - 127).\n", num, file_line_num);
        } else {
            write_memory_byte(data_memory_pointer, (uint8_t)num);  // Write byte to memory
            data_memory_pointer += 1;                              // Move to the next memory address
        }

        token = strtok(NULL, " ");
    }
}

void load_data_half(char* string) {
    char* token = strtok(string, " ");
    while (token != NULL) {
        // Convert token to uint16_t (halfword)
        long num = strtol(token, NULL, 0);

        // Check if the value is out of range for a halfword (0 to 65535)
        if (num < -32768 || num > 65535) {
            printf("Error: halfword value \"%ld\" in line %d not in range (0-65535).\n", num, text_line_num);
        } else {
            write_memory_half(data_memory_pointer, (uint16_t)num);  // Write halfword to memory
            data_memory_pointer += 2;                               // Move to the next memory address
        }

        token = strtok(NULL, " ");
    }
}

void load_data_word(char* string) {
    char* token = strtok(string, " ");
    while (token != NULL) {
        // Convert token to uint32_t (word)
        long num = strtol(token, NULL, 0);  // strtol for 32-bit range

        // Check if the value is out of range for a word
        if (num < -2147483648L || num > 4294967295L) {
            printf("Error: word value \"%ld\" in line %d not in range (0-4294967295).\n", num, text_line_num);
        } else {
            write_memory_word(data_memory_pointer, (uint32_t)num);  // Write word to memory
            data_memory_pointer += 4;                               // Move to the next memory address
        }

        token = strtok(NULL, " ");
    }
}

void load_data_dword(char* string) {
    char* token = strtok(string, " ");
    while (token != NULL) {
        // Convert token to uint64_t (dword)
        unsigned long long num = (int64_t)strtoll(token, NULL, 0);  // strtoll for 64-bit range

        // Check if the value is out of range for a dword
        if (num > 18446744073709551615ULL) {
            printf("Error: dword value \"%lld\" in line %d not in range (0-18446744073709551615).\n", num, text_line_num);
        } else {
            write_memory_dword(data_memory_pointer, num);  // Write dword to memory
            data_memory_pointer += 8;                                // Move to the next memory address
        }

        token = strtok(NULL, " ");
    }
}

void display_registers() {
    printf("Registers:\n");
    for (size_t i = 0; i < 32; i++) {
        //green("x%i = 0x%llX\n", i, registers[i]);
        printf("x%-2i = 0x%lX\n", (int)i, (uint64_t)registers[i]);
    }
    return;
}

void step() {
    uint32_t instruction = read_memory_word(pc);
    if(!break_line_found && instructions_array[current_instruction].break_point) {
        //cyan("Execution stopped at break point.\n");
        printf("Execution stopped at breakpoint\n");
        //green("Next: ");
        //cyan("\033[4m%s:%d\033[0m %s", current_file_name, instructions_array[current_instruction].file_line_num, instructions_array[current_instruction].instruction);
        break_line_found = true;
        return;
    }

    break_line_found = false;
    //stack handling
    stack_node* function = return_top_of_stack();
    if(function != NULL) {
        function->line_num = instructions_array[current_instruction].file_line_num;
    }

    //cyan("Executed \033[4m%s:%d\033[0m %s", current_file_name, instructions_array[current_instruction].file_line_num, instructions_array[current_instruction].instruction);
    //green("PC:\033[0m 0x%08X\n\n", pc);
    printf("Executed %s; PC=0x%08X\n", instructions_array[current_instruction].instruction, pc);

    execute(instruction);
    current_instruction += 1;
    pc += 4;

    if(current_instruction > max_instructions) {
        pop_from_stack();
    }
    return;
};

void execute(uint32_t instruction) {
    int type = decode_type(instruction);
    switch (type) {
        case 10:  // R-type instruction
            execute_r(instruction);
            break;
        case 20:  // I-type instruction (excluding jalr and load)
            execute_i(instruction);
            break;
        case 21:  // load instruction
            execute_l(instruction);
            break;
        case 22:  // jalr instruction
            execute_jr(instruction);
            break;
        case 30:  // S-type instruction
            execute_s(instruction);
            break;
        case 40:  // B-type instruction
            execute_b(instruction);
            break;
        case 50:  // J-type instruction
            execute_u(instruction);
            break;
        case 60:  // U-type instruction
            execute_j(instruction);
            break;

        default:
            break;
    }
}

void execute_r(uint32_t instruction) {
    uint8_t rd = (instruction >> 7) & 0b11111;
    if (rd == 0) {
        return;
    }

    uint8_t rs1 = (instruction >> 15) & 0b11111;
    uint8_t rs2 = (instruction >> 20) & 0b11111;

    uint8_t funct3 = (instruction >> 12) & 0b111;
    uint8_t funct7 = (instruction >> 25) & 0b111111;

    switch (funct7) {
        case 0x0: {
            switch (funct3) {
                case 0x0: {  // ADD
                    registers[rd] = registers[rs1] + registers[rs2];
                    break;
                }
                case 0x4: {  // XOR
                    registers[rd] = registers[rs1] ^ registers[rs2];
                    break;
                }
                case 0x6: {  // OR
                    registers[rd] = registers[rs1] | registers[rs2];
                    break;
                }
                case 0x7: {  // AND
                    registers[rd] = registers[rs1] & registers[rs2];
                    break;
                }
                case 0x1: {  // Shift left logical
                    registers[rd] = registers[rs1] << registers[rs2];
                    break;
                }
                case 0x5: {  // Shift Right logical
                    registers[rd] = (uint64_t)registers[rs1] >> registers[rs2];
                    break;
                }
                case 0x2: {  // Set Less Than
                    registers[rd] = (registers[rs1] < registers[rs2]) ? 1 : 0;
                    break;
                }
                case 0x3: {  // Set Less Than (U)
                    registers[rd] = ((uint64_t)registers[rs1] < (uint64_t)registers[rs2]) ? 1 : 0;
                    break;
                }

                default:
                    break;
            }
            break;
        }
        case 0x20: {
            switch (funct3) {
                case 0x0: {  // SUB
                    registers[rd] = registers[rs1] - registers[rs2];
                    break;
                }
                case 0x5: {  // Shift right arithmetic
                    registers[rd] = (registers[rs1] >> registers[rs2]);
                    break;
                }
                default:
                    break;
            }
            break;
        }

        default:
            break;
    }
    return;
}

void execute_i(uint32_t instruction) {
    uint8_t rd = (instruction >> 7) & 0b11111;
    if (rd == 0) {
        return;
    }
    uint8_t funct3 = (instruction >> 12) & 0b111;
    uint8_t rs1 = (instruction >> 15) & 0b11111;
    int64_t imm = sign_extend_12bit(((instruction >> 20) & 0b111111111111));

    switch (funct3) {
        case 0x0: {  // ADDI
            registers[rd] = (registers[rs1] + imm);
            break;
        }
        case 0x4: {  // XORI
            registers[rd] = (registers[rs1] ^ imm);
            break;
        }
        case 0x6: {  // ORI
            registers[rd] = (registers[rs1] | imm);
            break;
        }
        case 0x7: {  // ANDI
            registers[rd] = (registers[rs1] & imm);
            break;
        }
        case 0x1: {  // Shift Left Logical
            uint8_t funct6 = (instruction >> 26) & 0b111111;
            uint8_t imm = (uint8_t)(instruction >> 20) & 0b111111;
            switch (funct6) {
                case 0x0: {
                    registers[rd] = (registers[rs1] << imm);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case 0x5: {  // Shift Right
            uint8_t funct6 = (instruction >> 26) & 0b111111;
            uint8_t imm = (uint8_t)(instruction >> 20) & 0b111111;
            switch (funct6) {
                case 0x00: {  // Logical
                    registers[rd] = ((uint64_t)registers[rs1] >> imm);
                    break;
                }
                case 0x10: {  // Arithmetic
                    registers[rd] = (registers[rs1] >> imm);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

void execute_l(uint32_t instruction) {
    uint8_t rd = (instruction >> 7) & 0b11111;

    uint8_t funct3 = (instruction >> 12) & 0b111;
    uint8_t rs1 = (instruction >> 15) & 0b11111;
    uint32_t offset_raw = (instruction >> 20) & 0b111111111111;
    int64_t offset = sign_extend_12bit(offset_raw);

    if(cache_enabled) {
        uint32_t address = (uint64_t)(registers[rs1] + offset);
        registers[rd] = get_data_for_register(address, funct3);
        if(rd == 0) {registers[0] = 0;}
        return;
    }

    if (rd == 0) {
        return;
    }
    switch (funct3) {
        case 0x0: {  // load byte
            registers[rd] = (int64_t)((int8_t)read_memory_byte((uint64_t)(registers[rs1] + offset)));
            break;
        }
        case 0x1: {  // load half
            registers[rd] = (int64_t)((int16_t)read_memory_half((uint64_t)(registers[rs1] + offset)));
            break;
        }
        case 0x2: {  // load word
            registers[rd] = (int64_t)((int32_t)read_memory_word((uint64_t)(registers[rs1] + offset)));
            break;
        }
        case 0x3: {  // load dword
            registers[rd] = ((int64_t)read_memory_dword((uint64_t)(registers[rs1] + offset)));
            break;
        }
        case 0x4: {  // load byte (unsigned)
            registers[rd] = (uint64_t)read_memory_byte((uint64_t)(registers[rs1] + offset));
            break;
        }
        case 0x5: {  // load half (unsigned)
            registers[rd] = (uint64_t)read_memory_half((uint64_t)(registers[rs1] + offset));
            break;
        }
        case 0x6: {  // load word (unsigned)
            registers[rd] = (uint64_t)read_memory_word((uint64_t)(registers[rs1] + offset));
            break;
        }

        default:
            break;
    }
}

void execute_jr(uint32_t instruction) {
    // imm[11:0] rs1 funct3 rd opcode   ; funct3 = 0x0 always
    uint8_t rd = (instruction >> 7) & 0b11111;
    if (rd != 0) {
        registers[rd] = pc + 4; // linking 
    }

    uint8_t rs1 = (instruction >> 15) & 0b11111;
    uint32_t offset_raw = (instruction >> 20) & 0b111111111111;
    int64_t imm = sign_extend_12bit(offset_raw);

    imm -= 4;    // since pc gets incremented by 4 in all cases
    imm += registers[rs1];

    pc = imm;   // unconditional branch
    current_instruction = (imm/4) + 1;

    pop_from_stack();
}

void execute_s(uint32_t instruction) {
    //imm[11:5] rs2 rs1 funct3 imm[4:0] opcode

    // extract immediate first
    uint32_t imm = (instruction >> 7) & 0b11111; // imm[4:0]
    imm = imm | ((instruction >> 25) << 5);      // imm[11:5]

    uint8_t funct3 = (instruction >> 12) & 0b111;
    uint8_t rs1 = (instruction >> 15) & 0b11111;
    uint8_t rs2 = (instruction >> 20) & 0b11111;
    int64_t offset = sign_extend_12bit(imm);

    if(cache_enabled) {
        uint32_t address = registers[rs1] + offset;
        int size = 0;
        switch(funct3) {
            case 0x0:
                size = 1;
                break;

            case 0x1:
                size = 2;
                break;
            
            case 0x2:
                size = 4;
                break;
            
            case 0x3:
                size = 8;
                break;
        }

        write_cache(address, registers[2], size);
        return;
    }

    switch (funct3) {
        case 0x0: {  // store byte
            write_memory_byte(registers[rs1] + offset, registers[rs2]);
            break;
        }
        case 0x1: {  // store half
            write_memory_half(registers[rs1] + offset, registers[rs2]);
            break;
        }
        case 0x2: {  // store word
            write_memory_word(registers[rs1] + offset, registers[rs2]);
            break;
        }
        case 0x3: {  // store dword
            write_memory_dword(registers[rs1] + offset, registers[rs2]);
            break;
        }      
        default:
            break;
    }

}

void execute_b(uint32_t instruction) {
    // imm[12|10:5] rs2 rs1 funct3 imm[4:1|11] opcode 

    // extract immediate first
    uint32_t imm = (instruction >> 8) & 0b1111; // imm[4:1]
    imm = imm << 1;     // first bit is always zero
    imm = imm | (((instruction >> 25) & 0b0111111) << 5 ); // imm[10:5]
    imm = imm | (((instruction >> 7) & 0b1) << 11); // imm[11]
    imm = imm | ((instruction >> 31) << 12); // imm[12]

    uint8_t funct3 = (instruction >> 12) & 0b111;
    uint8_t rs1 = (instruction >> 15) & 0b11111;
    uint8_t rs2 = (instruction >> 20) & 0b11111;
    int64_t offset = sign_extend_12bit(imm >> 1) << 1;

    offset -= 4;   // since pc gets incremented by 4 in all cases

    switch(funct3) {
        case 0x0:   // beq
            if(registers[rs1] == registers[rs2]) {
                current_instruction += offset/4;
                pc += offset;
            }
            break;
        case 0x1:   // bne
            if(registers[rs1] != registers[rs2]) {
                current_instruction += offset/4;
                pc += offset;
            }
            break;
        case 0x4:   // blt
            if(registers[rs1] < registers[rs2]) {
                current_instruction += offset/4;
                pc += offset;
            }
            break;
        case 0x5:   // bge
            if(registers[rs1] >= registers[rs2]) {
                current_instruction += offset/4;
                pc += offset;
            }
            break;
        case 0x6:   // bltu
            if((uint64_t)registers[rs1] < (uint64_t)registers[rs2]) {
                current_instruction += offset/4;
                pc += offset;
            }
            break;
        case 0x7:   // bgeu
            if((uint64_t)registers[rs1] >= (uint64_t)registers[rs2]) {
                current_instruction += offset/4;
                pc += offset;
            }
            break;
        default:
            break;
    }
}

void execute_u(uint32_t instruction) {
    // imm[31:12] rd opcode
    uint8_t rd = (instruction >> 7) & 0b11111;
    if (rd != 0) {
        registers[rd] = (int64_t)((int32_t)(instruction >> 12) << 12);
    }

    return;
}

void execute_j(uint32_t instruction) {
    // imm[20|10:1|11|19:12] rd opcode 
    uint8_t rd = (instruction >> 7) & 0b11111;

    // shift to left once to remove bit 32(= 20), 
    // shift right by 22 to bring the last 10 bits to the front
    // and shift left once, since the first bit has to be zero
    uint32_t imm = (((instruction << 1) >> 22) << 1);  
    imm = imm + (((instruction >> 20) & 0b1) << 11);    // get the 12th bit(index 11)
    imm = imm + (((instruction >> 12) & 0b11111111) << 12); // get bits from 13 to 20(index 12 to 19)
    imm = imm + ((instruction >> 31) << 20);    // get the last, 21st bit(index 20)


    int64_t result = (int64_t)((int32_t)(imm << 11) >> 11);
    result -= 4;    // since pc gets incremented by 4 in all cases

    if (rd != 0) {
        registers[rd] = pc + 4; // linking 
    }

    pc += result;   // unconditional branch
    current_instruction += result/4;

    int temp = 0;
    char jump_location_label[50] = {};
    for(int i = 0; i < label_count; i++) {
        if(label_array[i].line_num == ((int)current_instruction)+1){
            strcpy(jump_location_label, label_array[i].label_name);
            temp = label_array[i].file_line_num - 1;
        }
    }

    push_to_stack(jump_location_label, temp);
    // 0 cause first instruction in the called function hasn't been executed yet. 

}

void show_stack() {
    stack_node* current = main_node;

    if(current == NULL) {
        if(current_instruction > max_instructions) {
            //cyan("Stack empty. Execution complete.\n");
            printf("Empty Call Stack: Execution complete\n");
        } else {
            cyan("Stack empty.\n");
        }
        return;
    } else {
        //cyan("Call stack:\n");
        printf("Call Stack:\n");
    }

    while(current != NULL) {
        //blue("%s: %d\n", current->function_name, current->line_num);
        printf("%s:%d\n", current->function_name, current->line_num);
        current = current->next;
    }

}

void insert_break(int break_line) {
    int line_found = false;
    for(size_t i = 1; i <= max_instructions; i++) {
        if(instructions_array[i].file_line_num == break_line) {
            instructions_array[i].break_point = true;
            //cyan("Break point added on line %d.\n", break_line);
            printf("Breakpoint set at line %d\n", break_line);
            line_found = true;
            break;
        }
    }

    if(!line_found) {
        red("No instruction on this line.\n");
    }
    return;
}

void delete_break(int break_line) {
    int line_found = false;
    for(size_t i = 1; i <= max_instructions; i++) {
        if(instructions_array[i].file_line_num == break_line) {
            if(instructions_array[i].break_point) {
                instructions_array[i].break_point = false;
                cyan("Break point removed from line %d.\n", break_line);
            } else {
                red("Line %d doesn't have a break point.\n", break_line);
            }

            line_found = true;
            break;
        }

    }

    if(!line_found) {
        red("No instruction on this line.\n");
    }
}