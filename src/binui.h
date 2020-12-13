
typedef struct {
  //binui_types * type_table;

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


void binui_iterate_internal(binui_context * reg, io_reader * reader);

void binui_stack_register_push(binui_context * ctx, binui_stack_register * reg, void * value);
void binui_stack_register_pop(binui_context * ctx, binui_stack_register * reg, void * value);
bool binui_stack_register_top(binui_context * ctx, binui_stack_register * reg, void * value);

void binui_iterate_internal(binui_context * reg, io_reader * reader);

void binui_iterate(binui_context * reg, io_reader * reader);

void binui_get_position(binui_context * reg, i32 * x, i32 * y);
void binui_get_size(binui_context * reg, u32 * w, u32 * h);
void binui_get_color(binui_context * reg, u32 * color);

typedef struct{
  
  void (* callback)(void * userdata);
  void * userdata;
}render_callback;

void render_callback_push(binui_context * ctx, render_callback callback);
render_callback render_callback_get(binui_context * ctx);
void render_callback_pop(binui_context * ctx);


typedef enum{
	     BINUI_OPCODE_NONE = 0,
	     BINUI_IMPORT_MODULE = 1,
	     BINUI_CANVAS = 2,
	     BINUI_RECTANGLE = 3,
	     BINUI_POSITION = 4,
	     BINUI_SIZE = 5,
	     BINUI_COLOR = 6,
	     BINUI_3D = 7,
	     BINUI_3D_TRANSFORM = 8,
	     BINUI_3D_POLYGON = 9,
	     BINUI_MAGIC = 0x5a,
}binui_opcode;
