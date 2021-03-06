// This file is auto generated by icy-vector.
#ifndef TABLE_COMPILER_INDEX
#define TABLE_COMPILER_INDEX
#define array_element_size(array) sizeof(array[0])
#define array_count(array) (sizeof(array)/array_element_size(array))
#include "icydb.h"
#include <stdlib.h>
#endif

binui_types * binui_types_create(const char * optional_name){
  static const char * const column_names[] = {(char *)"type", (char *)"opcode_size"};
  static const char * const column_types[] = {"u8", "u8"};

  binui_types * instance = calloc(sizeof(binui_types), 1);
  unsigned int sizes[] = {sizeof(u8), sizeof(u8)};
  
  instance->column_names = (char **)column_names;
  instance->column_types = (char **)column_types;
  
  ((size_t *)&instance->column_count)[0] = 2;
  for(unsigned int i = 0; i < 2; i++)
    ((size_t *)instance->column_sizes)[i] = sizes[i];

  icy_vector_abs_init((icy_vector_abs * )instance, optional_name);

  return instance;
}


binui_types_index binui_types_alloc(binui_types * table){
  icy_index idx = icy_vector_abs_alloc((icy_vector_abs *) table);
  binui_types_index index;
  index.index = idx.index;
  return index;
}

binui_types_indexes binui_types_alloc_sequence(binui_types * table, size_t count){
  icy_indexes idx = icy_vector_abs_alloc_sequence((icy_vector_abs *) table, count);
  binui_types_indexes index;
  index.index = idx.index.index;
  index.count = count;
  return index;
}

void binui_types_remove(binui_types * table, binui_types_index index){
  icy_index idx = {.index = index.index};
  icy_vector_abs_remove((icy_vector_abs *) table, idx);
}

void binui_types_remove_sequence(binui_types * table, binui_types_indexes * indexes){
  icy_indexes idxs;
  idxs.index.index = (unsigned int)indexes->index;
  idxs.count = indexes->count;
  icy_vector_abs_remove_sequence((icy_vector_abs *) table, &idxs);
  memset(&indexes, 0, sizeof(indexes));
  
}
void binui_types_clear(binui_types * table){
  icy_vector_abs_clear((icy_vector_abs *) table);
}

void binui_types_optimize(binui_types * table){
  icy_vector_abs_optimize((icy_vector_abs *) table);
}
void binui_types_destroy(binui_types ** table){
  icy_vector_abs_destroy((icy_vector_abs **) table);
}
