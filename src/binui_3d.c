#include <iron/full.h>
#include<microio.h>
#include "binui_types.h"
#include "binui.h"


binui_stack_register transform_3d_register ={.size =sizeof(mat4), .stack = {0}};

mat4 io_read_mat4(io_reader * rd){
  mat4 m;
  io_read(rd, &m, sizeof(m));
  return m;
}

void io_write_mat4(io_writer * wd, mat4 m){
  io_write(wd, &m, sizeof(m));
}


void transform_3d_enter(binui_context * ctx, io_reader * reader){
  mat4 current;
  if(!binui_stack_register_top(ctx, &transform_3d_register, &current)){
    current = mat4_identity();
  }
  mat4 new = io_read_mat4(reader);
  current = mat4_mul(current, new);
  binui_stack_register_push(ctx, &transform_3d_register, &current);
}

void transform_3d_exit(binui_context * ctx){
  binui_stack_register_top(ctx, &transform_3d_register, NULL);
}

mat4 transform_3d_current(binui_context * ctx){
  mat4 current;
  binui_stack_register_pop(ctx, &transform_3d_register, &current);
  return current;
}

binui_stack_register polygon_3d_register ={.size =sizeof(f32 *), .stack = {0}};

void polygon_3d_enter(binui_context * ctx, io_reader * reader){
  i32 vert_cnt = io_read_i32_leb(reader);
  i32 dim = io_read_i32_leb(reader);
  i32 count = dim * vert_cnt;
  
  f32 * pts = alloc0(count * sizeof(pts[0]));
  io_read(reader, pts, count * sizeof(pts[0]));
  binui_stack_register_push(ctx, &polygon_3d_register, &pts);  
}

void polygon_3d_exit(binui_context * ctx){
  f32 * pts;
  binui_stack_register_pop(ctx, &polygon_3d_register, &pts);
  dealloc(pts); 
}

binui_stack_register blit3d_entered ={.size =sizeof(bool), .stack = {0}};

void blit_3d_enter(binui_context * ctx, io_reader * reader){
  bool val = true;
  if(binui_stack_register_top(ctx, &blit3d_entered, NULL)){
    ERROR("Blit 3D already active");
  }
  binui_stack_register_push(ctx, &blit3d_entered, &val);
}

void blit_3d_exit(binui_context * ctx){

  binui_stack_register_pop(ctx, &blit3d_entered, NULL);
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

  
}
