#include <iron/full.h>
#include <iron/gl.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>

#include "graphics_module.h"

#include "string_intern.h"
#include "f32_f32_vector.h"
#include "f32_f32_vector.c"
#include "u64_f32_f32_vector_index.h"
#include "u64_f32_f32_vector_index.c"
#include "u64_table.h"

f32_f32_vector * points;
u64_f32_f32_vector_index * polygon_table;
u64_table * canvas_polygons;
u64 canvas_class;
static u32 camera_loc, color_loc, vertex_loc, depth_loc;
void canvas_render_polygon(u64 polygon){
  
}

void render_canvas(u64 canvas){
  static int initialized = false;
  static int shader = -1;
  if(!initialized){
    initialized = true;
  }

  double w, h;
  if(!control_try_get_size(canvas, &w, &h)){
    w = window_size.x;
    h = window_size.y;
  }
  
  vec2 pos = current_control_offset;
  blit_rectangle(pos.x,pos.y,w,h,1,1,1,1);
  u64 index = 0, cnt = 0, idx;
  while((cnt = u64_table_iter(canvas_polygons, &canvas, 1, NULL, &idx, 1, &index))){
    
    canvas_render_polygon(canvas_polygons->value[idx]);
  }
}

void canvas_init(){
  canvas_class = intern_string("canvas class");
  points = f32_f32_vector_create("canvas points");
  polygon_table = u64_f32_f32_vector_index_create("canvas polygons");
  canvas_polygons = u64_table_create("polygons");
  register_table(polygon_table);
  ((bool *)(&canvas_polygons->is_multi_table))[0] = true;
  class_set_method(canvas_class, render_control_method, render_canvas);
}
