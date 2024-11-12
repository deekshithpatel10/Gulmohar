#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cache.h"
#include "utils.h"
#include "assembler.h"


void enable_cache(char* config_file_name) {
   cache_enabled = false;
   free_cache();     // always free cache since new cache structure is being given 


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

   //printf("C=%d, B=%d, A=%d, Rep=%d, Wri=%d\n", cache_size, block_size, associativity, rep_policy, write_policy);

   // Close the file after reading
   fclose(config_file);
   free(policy_str);
   free(write_str);

   // before creating cache, check if it's fully associative
   if(associativity == 0) {
      associativity = cache_size / block_size;
   }

   create_cache(cache_size, block_size, associativity, rep_policy, write_policy);
   cache_enabled = true;
}


void disable_cache() {
   cache_enabled = false;
   free_cache();
}

void output_cache_status() {
   if(cache_enabled) {
      printf("Cache Size: %d\n", cache->cache_size);
      printf("Block Size: %d\n", cache->block_size);
      printf("Associativity: %d\n", cache->associativity);

      printf("Replacement Policy: ");
      switch(cache->rep_policy) {
         case 0:
            printf("LRU\n");
            break;
         case 1:
            printf("FIFO\n");
            break;
         case 2:
            printf("RANDOM\n");
            break;
      }

      printf("Write Back Policy: ");
      switch(cache->write_policy) {
         case 0:
            printf("WB\n");
            break;
         case 1:
            printf("WT\n");
            break;
      }
   } else {
      printf("Cache disabled\n");
   }
}

void free_cache() {
   if (cache == NULL) {
     return;  // No cache to wipe
   }

   int no_of_sets = (int)pow((double)2, (double)cache->index_length);
   // Free each cache set, 
   if (cache->sets != NULL) {
      for (int i = 0; i < no_of_sets; i++) {
         cache_set *set = &cache->sets[i];

         // Free each cache line
         if (set->lines != NULL) {
            for (int j = 0; j < cache->associativity; j++) {
               cache_line *line = &set->lines[j];

               // Free the block memory for each line
               if (line->block != NULL) {
                  free(line->block);
                  line->block = NULL;  
               }
            }

            free(set->lines);
            set->lines = NULL;  
         }
      }

      free(cache->sets);
      cache->sets = NULL;  
   }

   free(cache);
   cache = NULL;  // Set cache pointer to NULL after freeing
}

void create_cache(int cache_size, int block_size, int associativity, int rep_policy, int write_policy) {

   // Initialize the cache structure
   cache = (cache_struct*)malloc(sizeof(cache_struct));
   cache->cache_size = cache_size;
   cache->block_size = block_size;
   cache->associativity = associativity;
   cache->rep_policy = rep_policy;
   cache->write_policy = write_policy;

   // All stats are zero in the beginning
   cache->accesses = 0;
   cache->hits = 0;
   cache->misses = 0;

   // Calculate index, tag, and offset lengths
   cache->offset_length = (int)log2(block_size); // Number of bits for block offset
   cache->index_length = (int)log2(cache_size / (block_size * associativity)); // Index bits
   cache->tag_length = 20 - cache->index_length - cache->offset_length; // Remaining bits for the tag

   // Allocate memory for cache sets
   int num_sets = cache_size / (block_size * associativity);
   cache->sets = (cache_set*)malloc(num_sets * sizeof(cache_set));

   // Initialize each cache set
   for (int i = 0; i < num_sets; i++) {
      cache->sets[i].lines = (cache_line*)malloc(associativity * sizeof(cache_line));

      // Initialize each cache line
      for (int j = 0; j < associativity; j++) {
         cache->sets[i].lines[j].tag = 0;
         cache->sets[i].lines[j].block = (uint8_t*)malloc(block_size * sizeof(uint8_t));  // Allocate block memory
         cache->sets[i].lines[j].valid = 0;
         cache->sets[i].lines[j].dirty = 0;
         cache->sets[i].lines[j].last_use_time = 0;
         cache->sets[i].lines[j].arrival_time = 0;
      }
   }

   // printf("Cache Initialized:\n");
   // printf("Cache Size: %d bytes\n", cache_size);
   // printf("Block Size: %d bytes\n", block_size);
   // printf("Associativity: %d\n", associativity);
   // printf("Replacement Policy: %d\n", rep_policy);
   // printf("Write Policy: %d\n", write_policy);
   // printf("Index Length: %d bits\n", cache->index_length);
   // printf("Tag Length: %d bits\n", cache->tag_length);
   // printf("Offset Length: %d bits\n", cache->offset_length);
}