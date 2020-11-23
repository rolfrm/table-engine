
typedef struct {
  binui_types * type_table;

  void * registers;
  u64 reg_ptr;
  u64 reg_cap;

  bool lisp;
  bool debug;

}binui_context;


void binui_iterate_internal(binui_context * reg, io_reader * reader, void (* callback)(binui_context *registers, void * userdata), void * userdata);

void binui_iterate(binui_context * reg, io_reader * reader, void (* callback)(binui_context * registers, void * userdata), void * userdata);

void binui_get_position(binui_context * reg, i32 * x, i32 * y);
void binui_get_size(binui_context * reg, u32 * w, u32 * h);
void binui_get_color(binui_context * reg, u32 * color);


void (* rectangle_handle)(void * userdata);
