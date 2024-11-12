#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./simulator/assembler.h"
#include "./simulator/simulator.h"
#include "./simulator/utils.h"
#include "./simulator/cache.h"

_Bool cache_enabled = false;
cache_struct* cache = NULL;


label* label_array = NULL;  // array to store the labels
int label_count = 0;        // number of labels in the input file
int error_code = 0;         // specifies the error, if any. Value 0 is no error

int text_line_num = 1;      // points to the current instruction being executed. Doesn't include line with .text section. Ignores blank lines/lines with just labels/comment lines. Special ability: can make you cry. 
size_t file_line_num = 1;   // equals the absolute line number of the line being parsed in the input file

uint32_t pc = 0;  // program counter, points to the memory location of current instruction. Is updated in each step.

instruction_line instructions_array[1000] = {}; // holds the instruction strings to help in printing on command line when step/run is used
size_t max_instructions = 0;            // total no of instructions in the current file
size_t current_instruction = 1;         // points to the current instruction being executed

char* current_file_name = NULL;
// Global input file
FILE* input_file = NULL;

// Register array
int64_t registers[32];

memory_node* memory_table[MEMORY_TABLE_SIZE];

stack_node* main_node = NULL;
_Bool debug_flag = false;
_Bool file_load_success = false;        // if this flag is false, step/run/break won't work. It is set to true if file loading was successfull. Changes value on every file load.
_Bool break_line_found = false;         // if this flag is true, run will stop;
_Bool execute_command(char*);

int main() {
    // clear_screen();
    //blue_underlined("RISC-V Simulator\n");
    //printf("For viewing the table of commands, their aliases and their function, type \"help\" or \"h\"\n\n");
    char* command = NULL;
    size_t size = 0;

    do {
        //magenta("simulator> ");
        getline(&command, &size, stdin);
    } while (command == NULL || (strcmp(command, "\n") == 0));
    
    remove_extra_spaces(command, 1);
    while (strcmp(command, "exit\n") != 0) {
        execute_command(command);
        
        do {
            //magenta("simulator> ");
            getline(&command, &size, stdin);
        } while(command == NULL || (strcmp(command, "\n") == 0));
    }

    free_instructions_array();
    printf("Exited the simulator\n");
    //blue("Bye :)\n\n");
    return 0;
}

_Bool execute_command(char* command) {
    char* token = NULL;
    remove_extra_spaces(command, 0);
    token = strtok(command, " ");

    if(token == NULL) {
        return 0;
    }

    // check for clear screen first
    if (strcmp(token, "cls") == 0 || strcmp(token, "clear") == 0) {
        clear_screen();
        return 0;
    }

    if (error_code && strcmp(token, "load") != 0) {
        yellow("Warning: Previous file load failed.\n");
    }

    if (strcmp(token, "load") == 0) {
        free_instructions_array();
        max_instructions = 0;   // new file, new max
        current_instruction = 1;   // start from the beginning
        pc = 0;
        token = strtok(NULL, "\n");
        if(token == NULL) {
            red("No file specified.\n\n");
            file_load_success = false;
            return 0;
        }
        current_file_name = malloc(sizeof(char) * strlen(token));
        strcpy(current_file_name, token);
        file_load_success = load_file(token);

    } else if (strcmp(token, "mem") == 0) {

        token = strtok(NULL, " ");
        if (token == NULL) {
            red("Provide a memory address.\n");
            return 0;
        }

        uint64_t start_address = strtoll(token, NULL, 0);
        token = strtok(NULL, " ");
        if (token == NULL) {
            red("Provide a memory address.\n");
            return 0;
        }
        size_t num_bytes = strtoul(token, NULL, 0);
        display_memory(start_address, num_bytes);

    } else if (strcmp(token, "regs") == 0) {
        char* residue = strtok(NULL, "\0");
        if(residue != NULL) {
            red("Did you mean 'regs'?\n");
        } else {
            display_registers();
        }

    } else if (strcmp(token, "step") == 0) {

        char* residue = strtok(NULL, "\0");
        if(residue != NULL) {
            red("Did you mean 'step'?\n");
        } else if(!file_load_success) {
            printf("Nothing loaded. \n");
        }
        else if(current_instruction > max_instructions) {
            printf("Nothing to step\n");
        } else {
            step();
        }

    } else if (strcmp(token, "run") == 0) {
        char* residue = strtok(NULL, "\0");
        if(residue != NULL) {
            red("Did you mean 'run'?\n");
        } else if(!file_load_success) {
            printf("Nothing loaded. \n");
        } else if (current_instruction > max_instructions) {
            printf("Reached end of program. Load the file again to re-run.\n");
        }
        long int i = 0;
        while(current_instruction <= max_instructions && residue == NULL) {
            step();
            i++;
            if(break_line_found) {
                break;
            }

            if( i > 1000000) {
                printf("Timeout! Enter run again.");
            }
        }

    } else if (strcmp(token, "break") == 0) {
        if(!file_load_success) {
            printf("Nothing loaded. \n");
        } else {
            token = strtok(NULL, "\0");
            char* residue = strtok(NULL, "\0");
            if(residue != NULL) {
                red("Not a line number.\n");
            }
            int break_line;
            if(token == NULL) {
                red("Specify a line number.\n");
            } else {
                break_line = strtol(token, &residue, 0);
                if(strcmp(residue, "") != 0) {
                    red("Not a line number.\n", residue);
                } else {
                    insert_break(break_line);
                }
            }
        }

    } else if(strcmp(token, "del") == 0) {

        // check for break, line num, residue
        token = strtok(NULL, " ");

        if(token == NULL || (strcmp(token, "break") != 0)) {
            red("Did you mean 'del break'?\n");
            
        } else {
            token = strtok(NULL, " ");
            char* residue = strtok(NULL, "\0");

            if(residue != NULL) {
                red("Not a line number.\n");
            } else if (true){

                int break_line;
                if(token == NULL) {
                    red("Specify a line number.\n");
                } else {
                    break_line = strtol(token, &residue, 0);
                    if(strcmp(residue, "") != 0) {
                        red("Not a line number.\n", residue);
                    } else {
                        if(!file_load_success) {
                            printf("Nothing loaded. \n");
                        } else {
                            delete_break(break_line);
                        }
                    }
                }
            }
        }

    } else if(strcmp(token, "show-stack") == 0) {
        //show stack logic here
        char* residue = strtok(NULL, "\0");
        if(residue != NULL) {
            red("Did you mean 'show-stack'?\n");
        } else if(!file_load_success) {
            printf("Nothing loaded. \n");
        } else {
            show_stack();
        }

    // all CACHE related commands
    } else if(strcmp(token, "cache_sim") == 0) {

        char* cache_command = strtok(NULL, " ");
        if(cache_command == NULL) {
            red("Please specify a command.\n");

        //1. cache_sim enable config_file
        } else if(strcmp(cache_command, "enable") == 0) {
            char* file_name = strtok(NULL, "\0");
            if(file_name) {
                enable_cache(file_name);
            } else {
                red("No config file provided.\n");
            }

        //2. cache_sim disable
        } else if(strcmp(cache_command, "disable") == 0) {
            char* residue = strtok(NULL, "\0");
            if(!residue) {
                printf("Going to disable cache.\n");
            } else {
                red("Not a valid command.\n");
            }

        //3. cache_sim status
        } else if(strcmp(cache_command, "status") == 0) {
            char* residue = strtok(NULL, "\0");
            if(!residue) {
                printf("Going to print status.\n");
            } else {
                red("Not a valid command.\n");
            }

        //4. cache_sim invalidate
        } else if(strcmp(cache_command, "invalidate") == 0) {
            char* residue = strtok(NULL, "\0");
            if(!residue) {
                printf("Going to invalidate cache.\n");
            } else {
                red("Not a valid command.\n");
            }

        //5. cache_sim dump myFile.out (myFile.out is specified by the user)
        } else if(strcmp(cache_command, "dump") == 0) {
            char* file_name = strtok(NULL, "\0");
            if(file_name) {
                printf("This is your file %s\n", file_name);
            } else {
                red("No output location provided.\n");
            }

        //6. cache_sim stats
        } else if(strcmp(cache_command, "stats") == 0) {
            char* residue = strtok(NULL, "\0");
            if(!residue) {
                printf("Going to print stats.\n");
            } else {
                red("Not a valid command.\n");
            }

        } else {
            red("Unknown command.\n");
        }

    } else {
        red("Unknown command \"%s\".\n", token);
    }

    printf("\n");
    return 0;
}
