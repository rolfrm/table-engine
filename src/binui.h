
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



typedef struct _binui_context binui_context;

typedef struct{
  void (* enter) (binui_context * ctx, io_reader * reader);
  void (* exit) (binui_context * ctx);
  bool has_children;
  
}binui_opcode_handler;

struct _binui_context {
  bool inited;
  void * registers;
  u64 reg_ptr;
  u64 reg_cap;

  binui_opcode_handler * opcode_handlers;
  
  int opcode_handler_count;

  bool lisp;
  bool debug;
  binui_opcode current_opcode;
};

typedef struct {
  u32 size;
  u32 id;
}binui_register;

typedef struct {
  u32 size;
  binui_register stack;
}binui_stack_register;

void binui_init(binui_context * ctx);
void binui_set_opcode_handler(int opcode, binui_context * ctx, binui_opcode_handler handler);
void binui_iterate_internal( binui_context * reg, io_reader * reader);

void binui_stack_register_push(binui_context * ctx, binui_stack_register * reg, void * value);
void binui_stack_register_pop(binui_context * ctx, binui_stack_register * reg, void * value);
bool binui_stack_register_top(binui_context * ctx, binui_stack_register * reg, void * value);

void binui_iterate_internal(binui_context * reg, io_reader * reader);

void binui_iterate(binui_context * reg, io_reader * reader);

void binui_get_position(binui_context * reg, i32 * x, i32 * y);
void binui_get_size(binui_context * reg, u32 * w, u32 * h);
void binui_get_color(binui_context * reg, u32 * color);

typedef struct{
  
  void (* after_enter)(void * userdata);
  void (* before_exit)(void * userdata);
  
  void * userdata;
}render_callback;

void render_callback_push(binui_context * ctx, render_callback callback);
render_callback render_callback_get(binui_context * ctx);
void render_callback_pop(binui_context * ctx);
void binui_3d_init(binui_context * ctx);

mat4 io_read_mat4(io_reader * rd);
void io_write_mat4(io_writer * wd, mat4 m);


typedef struct{
  u32 count;
  u32 dim;
  f32 * data;
  
}binui_polygon;

binui_polygon binui_polygon_get(binui_context * ctx);
