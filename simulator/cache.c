#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache.h"
#include "utils.h"
#include "assembler.h"


void enable_cache(char* config_file_name) {
   //first read the file and extract all data
   //file_name not null verified in main
   FILE *config_file = fopen(config_file_name, "r");

   if (config_file == NULL) {
      red("Error opening file %s.\n", config_file_name);
      return;
   }

   int cache_size = 0;
   int block_size = 0;
   int associativity = 0;

   int rep_policy;      // 0 is LRU, 1 is FIFO, 2 is RANDOM
   int write_policy;    // 0 is write back, 1 is write through

   // Read the first three lines as integers
   if (fscanf(config_file, "%d", &cache_size) != 1) {
      red("Config file format invalid.\n");
      fclose(config_file);
      return;
   }
   fgetc(config_file);  // Consume the newline character after the integer

   if (fscanf(config_file, "%d", &block_size) != 1) {
      red("Config file format invalid.\n");
      fclose(config_file);
      return;
   }
   fgetc(config_file);  // Consume the newline character after the integer

   if (fscanf(config_file, "%d", &associativity) != 1) {
      red("Config file format invalid.\n");
      fclose(config_file);
      return;
   }
   fgetc(config_file);  // Consume the newline character after the integer

   // Allocate memory for strings
   char* policy_str = (char*)malloc(256 * sizeof(char));  
   char* write_str = (char*)malloc(256 * sizeof(char));  
   if (policy_str == NULL || write_str == NULL) {
      red("Memory allocation failed in cache_enable.\n");
      fclose(config_file);
      return;
   }

   // Read the next two lines as strings
   if (fgets(policy_str, 256, config_file) == NULL) {
      red("Config file format invalid.\n");
      free(policy_str);
      free(write_str);
      fclose(config_file);
      return;
   }

   if (fgets(write_str, 256, config_file) == NULL) {
      red("Config file format invalid.\n");
      free(policy_str);
      free(write_str);
      fclose(config_file);
      return;
   }

   // set rep_policy and write_policy
   if (strcmp(policy_str, "LRU\n") == 0) {
      rep_policy = 0;
   } else if (strcmp(policy_str, "FIFO\n") == 0) {
      rep_policy = 1;
   } else if (strcmp(policy_str, "RANDOM\n") == 0) {
      rep_policy = 2;
   } else {
      printf("%s\n", policy_str);
      red("Invalid REPLACEMENT POLICY in config file.\n");
      return;
   }

   if (strcmp(write_str, "WB\n") == 0 || strcmp(write_str, "WB") == 0) {
      write_policy = 0;
   } else if (strcmp(write_str, "WT\n") == 0 || strcmp(write_str, "WT") == 0) {
      write_policy = 1;
   } else {
      red("Invalid WRITE POLICY in config file.\n");
      return;      
   }

   printf("C=%d, B=%d, A=%d, Rep=%d, Wri=%d\n", cache_size, block_size, associativity, rep_policy, write_policy);

   // Close the file after reading
   fclose(config_file);
   free(policy_str);
   free(write_str);

   cache_enabled = true;
}

void disable_cache() {
   cache_enabled = false;
}

void free_cache() {

}