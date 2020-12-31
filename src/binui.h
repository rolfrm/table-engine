
/*typedef enum{
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
	     BINUI_TRANSLATE = 10,
	     BINUI_SCALE = 11,
	     BINUI_ROTATE = 12,
	     BINUI_MAGIC = 0x5a,
}binui_opcode;
*/

typedef u32 binui_opcode;

#define BINUI_OPCODE_NONE 0
#define BINUI_MAGIC 0x5a
typedef enum{
	     BINUI_INT8,
	     BINUI_INT16,
	     BINUI_INT32,
	     BINUI_INT64,
	     BINUI_INT64_LEB,
	     BINUI_UINT32,
	     BINUI_UINT64,
	     BINUI_F32,
	     BINUI_F64,
	     BINUI_STRING,
	     BINUI_VEC2,
	     BINUI_VEC3,
	     BINUI_VEC4,
	     BINUI_MAT3,
	     BINUI_MAT4,
	     BINUI_F32A,
	     BINUI_TYPE_NONE
	     
}binui_type_signature;

typedef struct _binui_stack_register binui_stack_register;


typedef struct{
  binui_stack_register * reg;
  binui_type_signature signature;
}binui_auto_type;

typedef struct _binui_context binui_context;

typedef struct _binui_stack_frame binui_stack_frame;


typedef struct{
  void (* enter) (binui_context * ctx);
  void (* exit) (binui_context * ctx);
  bool has_children;

  const binui_auto_type * typesig;
  u32 typesig_count;
  binui_opcode opcode;
  const char * opcode_name;
}binui_opcodedef;


typedef struct {
  u32 size;
  u32 id;
}binui_register;

struct _binui_stack_register{
  u32 size;
  binui_register stack;
};

const char * binui_opcode_name(binui_context * ctx, binui_opcode);
binui_opcode binui_opcode_parse(binui_context * ctx, const char * name);
binui_opcode binui_current_opcode(binui_context * ctx);
binui_context * binui_new();
void binui_load_opcode(binui_context * ctx, const char * name, const binui_auto_type * type, size_t type_count, void (* enter) (binui_context * ctx), void (* exit) (binui_context * ctx), bool children);

binui_opcodedef binui_get_opcodedef(binui_context * ctx, binui_opcode opcode);

void binui_iterate_internal( binui_context * reg, io_reader * reader);

void binui_stack_register_push(binui_context * ctx, binui_stack_register * reg, const void * value);
void binui_stack_register_pop(binui_context * ctx, binui_stack_register * reg, void * out_value);
bool binui_stack_register_top(binui_context * ctx, binui_stack_register * reg, void * out_value);

void binui_iterate_internal(binui_context * reg, io_reader * reader);

void binui_iterate(binui_context * reg, io_reader * reader);

void binui_get_position(binui_context * reg, vec2i *);
void binui_get_set_position(binui_context * reg, vec2i *);
void binui_get_size(binui_context * reg, vec2i *);
void binui_get_color(binui_context * reg, u32 * color);

struct _binui_stack_frame{
  u32 opcode;
  u32 child_count;
  u64 node_id;
};

typedef struct{
  
  void (* after_enter)(binui_stack_frame * frame, void * userdata);
  void (* before_exit)(binui_stack_frame * frame, void * userdata);
  
  void * userdata;
}node_callback;

void node_callback_push(binui_context * ctx, node_callback callback);
node_callback node_callback_get(binui_context * ctx);
void node_callback_pop(binui_context * ctx);
void binui_3d_init(binui_context * ctx);

mat4 io_read_mat4(io_reader * rd);
vec2 io_read_vec2(io_reader * rd);
vec3 io_read_vec3(io_reader * rd);
vec4 io_read_vec4(io_reader * rd);
void io_write_mat4(io_writer * wd, mat4 m);


typedef struct{
  u32 count;
  u32 dim;
  f32 * data;
  
}binui_polygon;

binui_polygon binui_polygon_get(binui_context * ctx);

vec4 rotate_3d_current(binui_context  * ctx);
void test_write_lisp(binui_context * reg, void * buffer, size_t size);
mat4 transform_3d_current(binui_context * ctx);

typedef struct{
  u32 count;
  f32 * array;

}f32_array;
