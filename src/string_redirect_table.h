// This file is auto generated by icy-table.
#include "icydb.h"
typedef struct _string_redirect_table{
  char ** column_names;
  char ** column_types;
  size_t count;
  const bool is_multi_table;
  const int column_count;
  int (*cmp) (const u64 * k1, const u64 * k2);
  const size_t sizes[2];

  u64 * key;
  string_table_indexes * value;
  icy_mem * key_area;
  icy_mem * value_area;
}string_redirect_table;

string_redirect_table * string_redirect_table_create(const char * optional_name);
void string_redirect_table_set(string_redirect_table * table, u64 key, string_table_indexes value);
void string_redirect_table_insert(string_redirect_table * table, u64 * key, string_table_indexes * value, size_t count);
void string_redirect_table_lookup(string_redirect_table * table, u64 * keys, size_t * out_indexes, size_t count);
void string_redirect_table_remove(string_redirect_table * table, u64 * keys, size_t key_count);
void string_redirect_table_clear(string_redirect_table * table);
void string_redirect_table_unset(string_redirect_table * table, u64 key);
bool string_redirect_table_try_get(string_redirect_table * table, u64 * key, string_table_indexes * value);
void string_redirect_table_print(string_redirect_table * table);
size_t string_redirect_table_iter(string_redirect_table * table, u64 * keys, size_t keycnt, u64 * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator);