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

void transform_3d_enter(binui_context * ctx, io_reader * reader){
  mat4 current;
  if(!binui_stack_register_top(ctx, &transform_3d_register, &current)){
    current = mat4_identity();
  }
  mat4 new = io_read_mat4(reader);
  current = mat4_mul(current, new);
  binui_stack_register_push(ctx, &transform_3d_register, &current);
}

void transform_3d_exit(binui_context * ctx, io_reader * reader){
  binui_stack_register_pop(ctx, &transform_3d_register, NULL);
}
