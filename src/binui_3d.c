#include <iron/full.h>
#include<microio.h>
#include "binui_types.h"
#include "binui.h"


static binui_stack_register transform_3d_register ={.size =sizeof(mat4), .stack = {0}};
static binui_stack_register set_transform_3d_register ={.size =sizeof(mat4), .stack = {0}};
static binui_stack_register rotate_register ={.size =sizeof(vec4), .stack = {0}};

mat4 io_read_mat4(io_reader * rd){
  mat4 m;
  io_read(rd, &m, sizeof(m));
  return m;
}

vec3 io_read_vec3(io_reader * rd){
  vec3 m;
  io_read(rd, &m.x, sizeof(m.x));
  io_read(rd, &m.y, sizeof(m.y));
  io_read(rd, &m.z, sizeof(m.z));
  return m;
}

vec4 io_read_vec4(io_reader * rd){
  vec4 m;
  io_read(rd, &m, sizeof(m));
  ASSERT(sizeof(m) == sizeof(f32) * 4);
  return m;
}
void io_write_mat4(io_writer * wd, mat4 m){
  io_write(wd, &m, sizeof(m));
}

void transform_3d_push(binui_context * ctx, mat4 m){
  mat4 current;
  if(!binui_stack_register_top(ctx, &transform_3d_register, &current)){
    current = mat4_identity();
  }
  current = mat4_mul(current, m);
  binui_stack_register_push(ctx, &transform_3d_register, &current);
  binui_stack_register_push(ctx, &set_transform_3d_register, &m);

}

void transform_3d_pop(binui_context * ctx){
  binui_stack_register_pop(ctx, &transform_3d_register, NULL);
  binui_stack_register_pop(ctx, &set_transform_3d_register, NULL);

}

mat4 transform_3d_current(binui_context * ctx){
  mat4 current;
  binui_stack_register_top(ctx, &transform_3d_register, &current);
  return current;
}


mat4 transform_3d_current_set(binui_context * ctx){
  mat4 current;
  binui_stack_register_top(ctx, &set_transform_3d_register, &current);
  return current;
}


void transform_3d_enter(binui_context * ctx, io_reader * reader){
  mat4 new = io_read_mat4(reader);
  transform_3d_push(ctx, new);
}

void transform_3d_exit(binui_context * ctx){
  transform_3d_pop(ctx);
}

void translate_3d_enter(binui_context * ctx, io_reader * reader){
  let vec = io_read_vec3(reader);
  transform_3d_push(ctx, mat4_translate(vec.x, vec.y, vec.z));
}

void translate_3d_exit(binui_context * ctx){
  transform_3d_pop(ctx);
}

void scale_3d_enter(binui_context * ctx, io_reader * reader){
  let vec = io_read_vec3(reader);
  transform_3d_push(ctx, mat4_scaled(vec.x, vec.y, vec.z));
}

void scale_3d_exit(binui_context * ctx){
  transform_3d_pop(ctx);
}

void rotate_3d_enter(binui_context * ctx, io_reader * reader){
  let vec = io_read_vec4(reader);
  var m = mat4_identity();
  m = mat4_rotate(m, vec.x, vec.y, vec.z, vec.w);
  transform_3d_push(ctx, m);
  binui_stack_register_push(ctx, &rotate_register, &vec);
}

vec4 rotate_3d_current(binui_context  * ctx){
  vec4 vec;
  if(binui_stack_register_top(ctx, &rotate_register, &vec))
     return vec;
  return vec4_zero;
}

void rotate_3d_exit(binui_context * ctx){
  transform_3d_pop(ctx);
  binui_stack_register_pop(ctx, &rotate_register, NULL);
  
}

binui_stack_register polygon_3d_register ={.size =sizeof(binui_polygon), .stack = {0}};

void polygon_3d_enter(binui_context * ctx, io_reader * reader){
  i32 vert_cnt = io_read_i32_leb(reader);
  i32 dim = io_read_i32_leb(reader);
  i32 count = dim * vert_cnt;
  
  f32 * pts = alloc0(count * sizeof(pts[0]));

  io_read(reader, pts, count * sizeof(pts[0]));

  binui_polygon d = {
		     .data = pts,
		     .dim = dim,
		     .count = vert_cnt
  };
  //logd("Loading a polygon of size %ix%i\n", vert_cnt, dim);
  binui_stack_register_push(ctx, &polygon_3d_register, &d);  

}

void polygon_3d_exit(binui_context * ctx){
  binui_polygon pts;
  binui_stack_register_pop(ctx, &polygon_3d_register, &pts);
  dealloc(pts.data); 
}

/*
binui_stack_register blit3d_context_reg = {.size = sizeof(blit3d_context *), .stack = {0}};



blit3d_context * blit3d_getctx(binui_context * ctx){
  blit3d_context * ptr = NULL;
  if(binui_stack_register_top(ctx, &blit3d_context_reg, &ptr)){
    return ptr;
  }
  ptr = blit3d_context_new();
  / binui_stack_register_push(ctx, &blit3d_context_reg, &ptr);
  return ptr;
}
*/

binui_polygon binui_polygon_get(binui_context * ctx){
  binui_polygon pts = {0};
  binui_stack_register_top(ctx, &polygon_3d_register, &pts);
  return pts;
  }

binui_stack_register blit3d_entered ={.size =sizeof(bool), .stack = {0}};

void blit_3d_enter(binui_context * ctx, io_reader * reader){
  bool val = true;
  if(binui_stack_register_top(ctx, &blit3d_entered, NULL)){
    ERROR("Blit 3D already active");
  }
  binui_stack_register_push(ctx, &blit3d_entered, &val);

  //blit3d_context * b = blit3d_getctx(ctx);
  //blit3d_context_load(b);
}

void blit_3d_exit(binui_context * ctx){

  binui_stack_register_pop(ctx, &blit3d_entered, NULL);
  //blit_begin(BLIT_MODE_PIXEL);
}



void binui_3d_init(binui_context * ctx){
  binui_opcode_handler h;
  h.enter = transform_3d_enter;
  h.exit = transform_3d_exit;
  h.has_children = true;
  binui_set_opcode_handler(BINUI_3D_TRANSFORM, ctx, h);

  h.enter = polygon_3d_enter;
  h.exit = polygon_3d_exit;
  h.has_children = false;
  binui_set_opcode_handler(BINUI_3D_POLYGON, ctx, h);

  h.enter = blit_3d_enter;
  h.exit = blit_3d_exit;
  h.has_children = true;
  binui_set_opcode_handler(BINUI_3D, ctx, h);

  h.enter = translate_3d_enter;
  h.exit = translate_3d_exit;
  h.has_children = true;
  binui_set_opcode_handler(BINUI_TRANSLATE, ctx, h);

  h.enter = scale_3d_enter;
  h.exit = scale_3d_exit;
  h.has_children = true;
  binui_set_opcode_handler(BINUI_SCALE, ctx, h);


  h.enter = rotate_3d_enter;
  h.exit = rotate_3d_exit;
  h.has_children = true;
  binui_set_opcode_handler(BINUI_ROTATE, ctx, h);
}
