
typedef struct {
  //binui_types * type_table;

  void * registers;
  u64 reg_ptr;
  u64 reg_cap;

  bool lisp;
  bool debug;

}binui_context;


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


 
