#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef CACHE
#define CACHE

extern _Bool cache_enabled;

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

#endif
