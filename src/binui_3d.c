#include <iron/full.h>
#include<microio.h>
#include "binui_types.h"
#include "binui.h"

static binui_stack_register transform_3d_register ={.size =sizeof(mat4), .stack = {0}};
static binui_stack_register camera_register ={.size =sizeof(mat4), .stack = {0}};
static binui_stack_register perspective_register ={.size =sizeof(vec4), .stack = {0}};
static binui_stack_register rotate_register ={.size =sizeof(vec4), .stack = {0}};
static binui_stack_register scale_register ={.size =sizeof(vec3), .stack = {0}};
static binui_stack_register translate_register ={.size =sizeof(vec3), .stack = {0}};
static binui_stack_register polygon_register ={.size = sizeof(f32_array), .stack = {0}};
static binui_stack_register blit3d_entered ={.size =sizeof(bool), .stack = {0}};

f32_array blit3d_polygon_get(binui_context * ctx){
  f32_array array = {0};
  binui_stack_register_top(ctx, &polygon_register, &array);
  return array;
}

mat4 camera_get(binui_context * ctx){
  mat4 p;
  binui_stack_register_top(ctx, &camera_register, &p);
  return p;
}

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

vec2 io_read_vec2(io_reader * rd){
  vec2 m;
  io_read(rd, &m.x, sizeof(m.x));
  io_read(rd, &m.y, sizeof(m.y));
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
}

void transform_3d_pop(binui_context * ctx){
  binui_stack_register_pop(ctx, &transform_3d_register, NULL);
}

mat4 transform_3d_current(binui_context * ctx){
  mat4 current;
  binui_stack_register_top(ctx, &transform_3d_register, &current);
  return current;
}

vec3 translate_current(binui_context  * ctx){
  vec3 vec;
  if(binui_stack_register_top(ctx, &translate_register, &vec))
    return vec;
  return vec3_zero;
}

void translate_3d_enter(binui_context * ctx){
  let vec = translate_current(ctx);
  transform_3d_push(ctx, mat4_translate(vec.x, vec.y, vec.z));
}

void translate_3d_exit(binui_context * ctx){
  transform_3d_pop(ctx);
}

vec3 scale_current(binui_context  * ctx){
  vec3 vec;
  if(binui_stack_register_top(ctx, &scale_register, &vec))
     return vec;
  return vec3_zero;
}

void scale_3d_enter(binui_context * ctx){
  let vec = scale_current(ctx);
  transform_3d_push(ctx, mat4_scaled(vec.x, vec.y, vec.z));
}

void scale_3d_exit(binui_context * ctx){
  transform_3d_pop(ctx);
}

vec4 rotate_3d_current(binui_context  * ctx){
  vec4 vec;
  if(binui_stack_register_top(ctx, &rotate_register, &vec))
     return vec;
  return vec4_zero;
}

void rotate_3d_enter(binui_context * ctx){
  var m = mat4_identity();
  let vec = rotate_3d_current(ctx);
  m = mat4_rotate(m, vec.x, vec.y, vec.z, vec.w);
  transform_3d_push(ctx, m);
}

void rotate_3d_exit(binui_context * ctx){
  transform_3d_pop(ctx);
}

void blit_3d_enter(binui_context * ctx){
  bool val = true;
  if(binui_stack_register_top(ctx, &blit3d_entered, NULL)){
    ERROR("Blit 3D already active");
  }
  binui_stack_register_push(ctx, &blit3d_entered, &val);
}

void blit_3d_exit(binui_context * ctx){
  binui_stack_register_pop(ctx, &blit3d_entered, NULL);
}


void perspective_enter(binui_context * ctx){
  
  if(binui_stack_register_top(ctx, &camera_register, NULL)){
    ERROR("Blit 3D already active");
  }
  vec4 perspective;
  ASSERT(binui_stack_register_top(ctx, &perspective_register, &perspective));
  mat4 m = mat4_perspective(perspective.x, perspective.y, perspective.z, perspective.w);
  binui_stack_register_push(ctx, &camera_register, &m);
}

void perspective_exit(binui_context * ctx){
  binui_stack_register_pop(ctx, &camera_register, NULL);
}

void binui_3d_init(binui_context * ctx){

  binui_load_opcode(ctx, "3d", NULL, 0, blit_3d_enter, blit_3d_exit, true);
  
  {
    static binui_auto_type type;
    type.signature = BINUI_VEC3;
    type.reg = &translate_register;
    binui_load_opcode(ctx, "translate", &type, 1, translate_3d_enter, translate_3d_exit, true);
  }
  {
    static binui_auto_type type;
    type.signature = BINUI_VEC4;
    type.reg = &perspective_register;
    binui_load_opcode(ctx, "perspective", &type, 1, perspective_enter, perspective_exit, true);
  }
  
  {
    static binui_auto_type type;
    type.signature = BINUI_VEC3;
    type.reg = &scale_register;
    binui_load_opcode(ctx, "scale", &type, 1, scale_3d_enter, scale_3d_exit, true);
  }
  {
    static binui_auto_type type;
    type.signature = BINUI_VEC4;
    type.reg = &rotate_register;
    binui_load_opcode(ctx, "rotate", &type, 1, rotate_3d_enter, rotate_3d_exit, true); 
  }

  {
    static binui_auto_type type;
    type.signature = BINUI_F32A;
    type.reg = &polygon_register;
    binui_load_opcode(ctx, "polygon", &type, 1, NULL, NULL, true);
  }
}
