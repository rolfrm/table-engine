typedef struct{
  u64 a;
  u64 b;
}u64_pair;

u64 intern_aggregate(u64 intern1, u64 intern2);
u64 intern_string(const char * name);
bool intern_string_get(const char * name, u64 * value);
u64 id_new();

