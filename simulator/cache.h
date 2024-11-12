#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "assembler.h"
#include "simulator.h"
#include "utils.h"


#ifndef CACHE
#define CACHE

extern _Bool cache_enabled;
extern FILE* cache_output_file;

typedef struct cache_line {
   uint32_t tag;
   uint8_t *block;      // max block size is 64 bytes

   _Bool valid;
   _Bool dirty;
   int last_use_time;
   int arrival_time;
}  cache_line;

typedef struct cache_set {
   cache_line* lines;
} cache_set;

typedef struct cache_struct {
   int cache_size;
   int block_size;
   int associativity;   // the ABC of cache

   int accesses;
   int hits;
   int misses;    // cache stats

   int rep_policy;      // 0 is LRU, 1 is FIFO, 2 is RANDOM
   int write_policy;    // 0 is write back, 1 is write through

   int index_length;
   int tag_length;
   int offset_length;

   cache_set* sets;
} cache_struct;

extern cache_struct* cache;


void enable_cache(char* config_file_name);
void disable_cache();
void free_cache();
void create_cache(int cache_size, int block_size, int associativity, int rep_policy, int write_policy);
void output_cache_status();
void clear_cache();
void open_cache_output_file(char* file_name);

uint8_t *read_cache(uint32_t address);

void write_cache(uint32_t address, uint64_t data, int size);

void cache_invalidate();

int64_t get_data_for_register(uint32_t address, uint8_t funct3);








#endif
