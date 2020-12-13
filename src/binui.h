
typedef struct {
  binui_types * type_table;

  void * registers;
  u64 reg_ptr;
  u64 reg_cap;

  bool lisp;
  bool debug;

}binui_context;

typedef struct {
  u32 size;
  u32 id;
}binui_register;

typedef struct {
  u32 size;
  binui_register stack;
}binui_stack_register;

void binui_stack_register_push(binui_context * ctx, binui_stack_register * reg, void * value);
void binui_stack_register_pop(binui_context * ctx, binui_stack_register * reg, void * value);
bool binui_stack_register_top(binui_context * ctx, binui_stack_register * reg, void * value);

void binui_iterate_internal(binui_context * reg, io_reader * reader, void (* callback)(binui_context *registers, void * userdata), void * userdata);

void binui_iterate(binui_context * reg, io_reader * reader, void (* callback)(binui_context * registers, void * userdata), void * userdata);

void binui_get_position(binui_context * reg, i32 * x, i32 * y);
void binui_get_size(binui_context * reg, u32 * w, u32 * h);
void binui_get_color(binui_context * reg, u32 * color);


void (* rectangle_handle)(void * userdata);


