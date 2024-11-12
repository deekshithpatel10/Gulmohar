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

// Function to clear cache lines when new file is loaded(set valid bit to 0)
void clear_cache() {
   if (cache == NULL) {
      return; // If cache is not initialized, do nothing
   }

   int no_of_sets = cache->cache_size / (cache->block_size * cache->associativity);
   // Iterate over all cache sets
   for (int i = 0; i < no_of_sets; i++) {
      // Iterate over each cache line in the set
      for (int j = 0; j < cache->associativity; j++) {
         // Set the valid bit to 0 (invalidate the cache line)
         cache->sets[i].lines[j].valid = 0;
         cache->sets[i].lines[j].arrival_time = 0;
         cache->sets[i].lines[j].last_use_time = 0;
         cache->sets[i].lines[j].dirty = 0;
      }
   }

   cache->accesses = 0;
   cache->hits = 0;
   cache->misses = 0;

}

void open_cache_output_file(char* file_name) {
   char input_filename[256];  // don't mutate the file_name
   strcpy(input_filename, file_name);

   
   char output_filename[256];  

   char *dot_pos = strrchr(input_filename, '.');
   if (dot_pos != NULL) {
      // Terminate the string at the dot to remove the extension and append ".output"
      *dot_pos = '\0';
   }

   // Append ".output" to the modified filename
   strcpy(output_filename, input_filename);
   strcat(output_filename, ".output");

   if(cache_output_file) {
      // close the previous file if not closed. 
      fclose(cache_output_file);
      cache_output_file = NULL;
   }

   cache_output_file = fopen(output_filename, "w");

   if (cache_output_file == NULL) {
      red("Error opening cache output file\n");
      return;
   }

}

void cache_invalidate() {
   // first write back dirty blocks, and use clear_cache();
}

int64_t get_data_for_register(uint32_t address, uint8_t funct3) {
   int byte_offset = address & ((1 << cache->offset_length) - 1);
   uint8_t *data = read_cache(address);

   switch (funct3) {
      case 0x0: {  // load byte
         int8_t value = 0;
         for (int i = 0; i < 1; i++)
            value |= (data[byte_offset + i] << (i * 8));
         return (int64_t)value;
      }

      case 0x1: {  // load half
         int16_t value = 0;
         for (int i = 0; i < 2; i++)
            value |= (data[byte_offset + i] << (i * 8));
         return (int64_t)value;
      }

      case 0x2: {  // load word
         int32_t value = 0;
         for (int i = 0; i < 4; i++) 
            value |= (data[byte_offset + i] << (i * 8));
         return (int64_t)value;
      }

      case 0x3: {  // load dword
         int64_t value = 0;
         for (int i = 0; i < 8; i++) 
            value |= (data[byte_offset + i] << (i * 8));
         return (int64_t)value;
      }

      case 0x4: {  // load byte (unsigned)
         uint8_t value = 0;
         for (int i = 0; i < 1; i++)
            value |= (data[byte_offset + i] << (i * 8));
         return (uint64_t)value;
      }

      case 0x5: {  // load half (unsigned)
         uint16_t value = 0;
         for (int i = 0; i < 2; i++)
            value |= (data[byte_offset + i] << (i * 8));
         return (uint64_t)value;
      }

      case 0x6: {  // load word (unsigned)
         uint32_t value = 0;
         for (int i = 0; i < 4; i++)
            value |= (data[byte_offset + i] << (i * 8));
         return (uint64_t)value;
      }

      default:
         return 0;
   }

}

uint8_t *read_cache(uint32_t address) {
   // check for hit
   cache->accesses++;
   int index = (address >> cache->offset_length) & ((1 << cache->index_length) - 1);
   uint32_t tag = (address >> (cache->offset_length + cache->index_length)) & ((1 << cache->tag_length) - 1);

   for (int i = 0; i < cache->associativity; i++) {
      if (cache->sets[index].lines[i].valid && cache->sets[index].lines[i].tag == tag) {
         cache->hits++;
         cache->sets[index].lines[i].last_use_time = cache->accesses;

         if (cache->sets[index].lines[i].dirty == 0) 
            fprintf(cache_output_file, "R: Address: 0x%X, Set: 0x%X, Hit, Tag: 0x%X, Clean\n", address, index, tag);
         else 
            fprintf(cache_output_file, "R: Address: 0x%X, Set: 0x%X, Hit, Tag: 0x%X, Dirty\n", address, index, tag);

         fflush(cache_output_file);
         return cache->sets[index].lines[i].block;
      }
   }


   // cache read miss

   cache->misses++;

   // rep_policy --> 0 is LRU, 1 is FIFO, 2 is RANDOM
   // write_policy --> 0 is write back, 1 is write through 

   int replacement_line = 0;
   int min_of_line;
   

   if (cache->rep_policy == 0){
      min_of_line = cache->sets[index].lines[0].last_use_time;
      for (int i = 0; i < cache->associativity; i++) {
         if (cache->sets[index].lines[i].last_use_time < min_of_line) {
            min_of_line = cache->sets[index].lines[i].last_use_time;
            replacement_line = i;
         }
      }
   }
   else if (cache->rep_policy == 1) {
      min_of_line = cache->sets[index].lines[0].arrival_time;
      for (int i = 0; i < cache->associativity; i++) {
         if (cache->sets[index].lines[i].arrival_time < min_of_line) {
            min_of_line = cache->sets[index].lines[i].arrival_time;
            replacement_line = i;
         }
      }
   }
   else if (cache->rep_policy == 2) {
      replacement_line = rand() % cache->associativity;
   }

   if (cache->write_policy == 0 && cache->sets[index].lines[replacement_line].dirty && cache->sets[index].lines[replacement_line].valid) {
      uint32_t address = (cache->sets[index].lines[replacement_line].tag << (cache->offset_length + cache->index_length) | (index << cache->offset_length));
      for (int i = 0; i < cache->block_size; i++) 
         write_memory_byte(address + i, cache->sets[index].lines[replacement_line].block[i]);
   }

   cache->sets[index].lines[replacement_line].arrival_time = cache->accesses;
   cache->sets[index].lines[replacement_line].last_use_time = cache->accesses;
   cache->sets[index].lines[replacement_line].tag = tag;
   cache->sets[index].lines[replacement_line].valid = 1;
   cache->sets[index].lines[replacement_line].dirty = 0;

   uint32_t block_addr = (cache->sets[index].lines[replacement_line].tag << (cache->offset_length + cache->index_length) | (index << cache->offset_length));

   for (int i = 0; i < cache->block_size; i++) 
         cache->sets[index].lines[replacement_line].block[i] = read_memory_byte(block_addr + i);


   fprintf(cache_output_file, "R: Address: 0x%X, Set: 0x%X, Miss, Tag: 0x%X, Clean\n", address, index, tag);
   fflush(cache_output_file);
   return cache->sets[index].lines[replacement_line].block;

   // 
}

void write_cache(uint32_t address, uint64_t data, int size) {
   //check for hit
   int offset = address & ((1 << cache->offset_length) - 1);
   int index = (address >> cache->offset_length) & ((1 << cache->index_length) - 1);
   uint32_t tag = (address >> (cache->offset_length + cache->index_length)) & ((1 << cache->tag_length) - 1);


   // rep_policy --> 0 is LRU, 1 is FIFO, 2 is RANDOM
   // write_policy --> 0 is write back, 1 is write through 
   cache->accesses++;
   for (int i = 0; i < cache->associativity; i++) {
      if (cache->sets[index].lines[i].valid && cache->sets[index].lines[i].tag == tag) {
         cache->hits++;
         cache->sets[index].lines[i].last_use_time = cache->accesses;

         if (cache->write_policy == 0) {
            cache->sets[index].lines[i].dirty = 1;
            for (int j = 0; j < size; j++)
               cache->sets[index].lines[i].block[offset + j] = (data >> (j * 8)) & 0xFF;
         }
         else if (cache->write_policy == 1) {
            for (int j = 0; j < size; j++)
               cache->sets[index].lines[i].block[offset + j] = (data >> (j * 8)) & 0xFF;

            for (int j = 0; j < size; j++)
               write_memory_byte(address + j, (data >> (j * 8)) & 0xFF);
         }

         if (cache->sets[index].lines[i].dirty == 0) 
            fprintf(cache_output_file, "W: Address: 0x%X, Set: 0x%X, Hit, Tag: 0x%X, Clean\n", address, index, tag);
         else 
            fprintf(cache_output_file, "W: Address: 0x%X, Set: 0x%X, Hit, Tag: 0x%X, Dirty\n", address, index, tag);

         fflush(cache_output_file);
         return;
      }
   }

   cache->misses++;
   int replacement_line = 0;
   int min_of_line;

   if (cache->rep_policy == 0){
      min_of_line = cache->sets[index].lines[0].last_use_time;
      for (int i = 0; i < cache->associativity; i++) {
         if (cache->sets[index].lines[i].last_use_time < min_of_line) {
            replacement_line = i;
            min_of_line = cache->sets[index].lines[i].last_use_time;
         }
      }
   }
   else if (cache->rep_policy == 1) {
      min_of_line = cache->sets[index].lines[0].arrival_time;
      for (int i = 0; i < cache->associativity; i++) {
         if (cache->sets[index].lines[i].arrival_time < min_of_line) {
            replacement_line = i;
            min_of_line = cache->sets[index].lines[i].arrival_time;
         }
      }
   }
   else if (cache->rep_policy == 2) {
      replacement_line = rand() % cache->associativity;
   }

   if (cache->write_policy == 0 && cache->sets[index].lines[replacement_line].dirty && cache->sets[index].lines[replacement_line].valid) {
      uint32_t address = ((cache->sets[index].lines[replacement_line].tag << (cache->offset_length + cache->index_length)) | (index << cache->offset_length));
      for (int i = 0; i < cache->block_size; i++) 
         write_memory_byte(address + i, cache->sets[index].lines[replacement_line].block[i]);
   }

   // for write back, also do allocate when miss
   if (cache->write_policy == 0) {
      cache->sets[index].lines[replacement_line].arrival_time = cache->accesses;
      cache->sets[index].lines[replacement_line].dirty = 1;
      cache->sets[index].lines[replacement_line].last_use_time = cache->accesses;
      cache->sets[index].lines[replacement_line].tag = tag;
      cache->sets[index].lines[replacement_line].valid = 1;

      uint32_t block_addr = ((cache->sets[index].lines[replacement_line].tag << (cache->offset_length + cache->index_length)) | (index << cache->offset_length));
      for (int i = 0; i < cache->block_size; i++) 
            cache->sets[index].lines[replacement_line].block[i] = read_memory_byte(block_addr + i);

      for (int i = 0; i < size; i++)
         cache->sets[index].lines[replacement_line].block[offset + i] = ((data >> (i * 8)) & 0xFF);

   }
   else if (cache->write_policy == 1) {
      // very few lines since no allocate
      for (int i = 0; i < size; i++)
         write_memory_byte(address + i, (data >> (i * 8)) & 0xFF);
   }

   if (cache->sets[index].lines[replacement_line].dirty == 0) 
      fprintf(cache_output_file, "W: Address: 0x%X, Set: 0x%X, Miss, Tag: 0x%X, Clean\n", address, index, tag);
   else 
      fprintf(cache_output_file, "W: Address: 0x%X, Set: 0x%X, Miss, Tag: 0x%X, Dirty\n", address, index, tag);

   fflush(cache_output_file);

   return;
}

void output_cache_stats() {
   if(cache_enabled) {
      printf("D-cache statistics: Accesses=%d, Hit=%d, Miss=%d, ", cache->accesses, cache->hits, cache->misses);
      float hit_rate = 0;

      if(cache->accesses != 0) hit_rate = (float)cache->hits/cache->accesses;
      
      printf("Hit Rate=%.2f\n", hit_rate);
   } else {
      printf("Cache disabled.\n");
   }
}





















































