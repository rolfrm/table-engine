// This file is auto generated by icy-table.
#include "icydb.h"
typedef struct _u32_to_u32{
  char ** column_names;
  char ** column_types;
  size_t count;
  const bool is_multi_table;
  const int column_count;
  int (*cmp) (const u32 * k1, const u32 * k2);
  const size_t sizes[2];

  u32 * key;
  u32 * value;
  icy_mem * key_area;
  icy_mem * value_area;
}u32_to_u32;

u32_to_u32 * u32_to_u32_create(const char * optional_name);
void u32_to_u32_set(u32_to_u32 * table, u32 key, u32 value);
void u32_to_u32_insert(u32_to_u32 * table, u32 * key, u32 * value, size_t count);
void u32_to_u32_lookup(u32_to_u32 * table, u32 * keys, size_t * out_indexes, size_t count);
void u32_to_u32_remove(u32_to_u32 * table, u32 * keys, size_t key_count);
void u32_to_u32_clear(u32_to_u32 * table);
void u32_to_u32_unset(u32_to_u32 * table, u32 key);
bool u32_to_u32_try_get(u32_to_u32 * table, u32 * key, u32 * value);
void u32_to_u32_print(u32_to_u32 * table);
size_t u32_to_u32_iter(u32_to_u32 * table, u32 * keys, size_t keycnt, u32 * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator);
