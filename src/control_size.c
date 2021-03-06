// This file is auto generated by icy-table.
#ifndef TABLE_COMPILER_INDEX
#define TABLE_COMPILER_INDEX
#define array_element_size(array) sizeof(array[0])
#define array_count(array) (sizeof(array)/array_element_size(array))
#include "icydb.h"
#include <stdlib.h>
#endif


control_size * control_size_create(const char * optional_name){
  static const char * const column_names[] = {(char *)"key", (char *)"width", (char *)"height"};
  static const char * const column_types[] = {"u64", "f64", "f64"};
  control_size * instance = calloc(sizeof(control_size), 1);
  instance->column_names = (char **)column_names;
  instance->column_types = (char **)column_types;
  
  icy_table_init((icy_table * )instance, optional_name, 3, (unsigned int[]){sizeof(u64), sizeof(f64), sizeof(f64)}, (char *[]){(char *)"key", (char *)"width", (char *)"height"});
  
  return instance;
}

void control_size_insert(control_size * table, u64 * key, f64 * width, f64 * height, size_t count){
  void * array[] = {(void* )key, (void* )width, (void* )height};
  icy_table_inserts((icy_table *) table, array, count);
}

void control_size_set(control_size * table, u64 key, f64 width, f64 height){
  void * array[] = {(void* )&key, (void* )&width, (void* )&height};
  icy_table_inserts((icy_table *) table, array, 1);
}

void control_size_lookup(control_size * table, u64 * keys, size_t * out_indexes, size_t count){
  icy_table_finds((icy_table *) table, keys, out_indexes, count);
}

void control_size_remove(control_size * table, u64 * keys, size_t key_count){
  size_t indexes[key_count];
  size_t index = 0;
  size_t cnt = 0;
  while(0 < (cnt = icy_table_iter((icy_table *) table, keys, key_count, NULL, indexes, array_count(indexes), &index))){
    icy_table_remove_indexes((icy_table *) table, indexes, cnt);
    index = 0;
  }
}

void control_size_clear(control_size * table){
  icy_table_clear((icy_table *) table);
}

void control_size_unset(control_size * table, u64 key){
  control_size_remove(table, &key, 1);
}

bool control_size_try_get(control_size * table, u64 * key, f64 * width, f64 * height){
  void * array[] = {(void* )key, (void* )width, (void* )height};
  void * column_pointers[] = {(void *)table->key, (void *)table->width, (void *)table->height};
  size_t __index = 0;
  icy_table_finds((icy_table *) table, array[0], &__index, 1);
  if(__index == 0) return false;
  unsigned int sizes[] = {sizeof(u64), sizeof(f64), sizeof(f64)};
  for(int i = 1; i < 3; i++){
    if(array[i] != NULL)
      memcpy(array[i], column_pointers[i] + __index * sizes[i], sizes[i]); 
  }
  return true;
}

void control_size_print(control_size * table){
  icy_table_print((icy_table *) table);
}

size_t control_size_iter(control_size * table, u64 * keys, size_t keycnt, u64 * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator){
  return icy_table_iter((icy_table *) table, keys, keycnt, optional_keys_out, indexes, cnt, iterator);

}
