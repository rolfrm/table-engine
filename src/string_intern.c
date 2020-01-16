#include <iron/full.h>
//#include "main.h"
#include "string_intern.h"
#include "u64_table.h"
#include "u64_pair_to_u64.h"
 
u64_table * var_table_create(const char * optional_name, int count){
  static const char * const column_names[] = {(char *)"key", (char *)"value"};
  static const char * const column_types[] = {"u64", "u64"};
  u64_table * instance = calloc(sizeof(u64_table), 1);
  instance->column_names = (char **)column_names;
  instance->column_types = (char **)column_types;
  
  icy_table_init((icy_table * )instance, optional_name, 2, (unsigned int[]){sizeof(u64) * count, sizeof(u64)}, (char *[]){(char *)"key", (char *)"value"});
  
  return instance;
}

u64_table * string_table_get(u64 count){
  ASSERT(count < 64);
  static u64_table ** tables = NULL;
  if(tables == NULL){
    tables = alloc0(sizeof(u64_table) * 64);
  }
  
  if(tables[count] == NULL){
    char buf[100];
    sprintf(buf, "intern_u64x%i", count);
    tables[count] = var_table_create(buf, count);
  }
  return tables[count];

}

u64 id_new(){
  static u64 * mem = NULL;
  if(mem == NULL){
    var mem2 = icy_mem_create("ids");
    icy_mem_realloc(mem2, sizeof(u64));
    mem = mem2->ptr;
  }
  if(*mem == 0){
    *mem = 1;
  }
  u64 v = *mem;
  *mem += 1;
  return v;
}

u64 intern_string(const char * name){

  int len = strlen(name) / 8 + 1;
  if(len == -1)
    return 0;
  u64_table * table = string_table_get(len);
  char buffer[len * 8];
  memset(buffer, 0, len * 8);
  memmove(buffer, name, strlen(name));
  u64 id;
  if(u64_table_try_get(table, (u64 *) buffer, &id)){
    return id;
  }
 
  u64 newid = id_new();
  u64_table_insert(table, (u64 *) buffer, &newid, 1);
  return newid;
}

bool intern_string_get(const char * name, u64 * value){
  int len = strlen(name) / 8 + 1;
  if(len == -1)
    return false;
  u64_table * table = string_table_get(len);
  char buffer[len * 8];
  memset(buffer, 0, len * 8);
  memmove(buffer, name, strlen(name));
  return u64_table_try_get(table, (u64 *) buffer, value);
}

u64 intern_aggregate(u64 intern1, u64 intern2){
  static u64_pair_to_u64 * aggregate_table;
  if(aggregate_table == NULL){
    aggregate_table = u64_pair_to_u64_create("intern_aggregate");
  }
  union {
    u64 key[2];
    u64_pair key2;
  }x;
  x.key[0] = intern1;
  x.key[1] = intern2;
  u64 id;
  if(u64_pair_to_u64_try_get(aggregate_table, &x.key2, &id)){
    return id;
  }
  id = id_new();
  u64_pair_to_u64_set(aggregate_table, x.key2, id);
  return id;
}
