#include <iron/full.h>
#include <microio.h>
#include <iron/gl.h>
#include "graphics_module.h"

#include "string_intern.h"
#include "f32_f32_vector.h"
#include "f32_f32_vector.c"
#include "u64_f32_f32_vector_index.h"
#include "u64_f32_f32_vector_index.c"
#include "u64_table.h"
#include "model.h"
#include "binui_types.h"
#include "binui.h"
void binui_test_load(io_writer * wd);
f32_f32_vector * points;
u64_f32_f32_vector_index * polygon_table;
u64_table * canvas_polygons;
u64_table * loaded_polygons;

u64_table * canvas_contexts;
typedef struct {
  blit3d_context * blit3d;
  io_writer wd;
  binui_context binui;
}canvas_context;

u64 canvas_class;
static u32 camera_loc, color_loc, vertex_loc, depth_loc;

model * get_or_create_polygon(u64 polygon){
  u64 ptr;
  if(!u64_table_try_get(loaded_polygons, &polygon, &ptr)){
    model mod = {0};
    mod.scale = vec3_new(1,1,1);
    mod.color = vec4_new(1,1,1,1);
    mod.type = 0;
    mod.verts = blit3d_polygon_new();
    f32_f32_vector_indexes indx;
    if(u64_f32_f32_vector_index_try_get(polygon_table, &polygon, &indx)){
      f32 * x = points->x + indx.index;
      f32 * y = points->y + indx.index;
      float pts[2 * indx.count];
      for(int i = 0; i < indx.count; i++){
	pts[i * 2] = x[i];
	pts[i * 2 + 1] = y[i];
      }
      blit3d_polygon_load_data(mod.verts, pts, indx.count * 2 * sizeof(f32));
      blit3d_polygon_configure(mod.verts, 2);
    }
    
    model * _mod = iron_clone(&mod, sizeof(mod));
    u64_table_set(loaded_polygons, polygon, (u64)_mod);
    printf("Created polygon...\n");
    return _mod;
  }
  return (void *) ptr;
}

void canvas_render_polygon(canvas_context * ctx, model * mod){

}

void binui_test_load(io_writer * wd);
canvas_context * canvas_get_context(u64 canvas){
  u64 ptr;
  if(!u64_table_try_get(canvas_contexts, &canvas, &ptr)){
    canvas_context * ctx = alloc0(sizeof(canvas_context));
    binui_test_load(&ctx->wd);
    
    ctx->blit3d  = blit3d_context_new();
 
    u64_table_set(canvas_contexts, canvas, (u64) ctx);
    return ctx;
  }
  return (canvas_context *) ptr;
}

static void blit_rectangle3(void * userdata){
  var ctx = (canvas_context *) userdata;
  int x, y;
  u32 w, h;
  u32 color;
  binui_get_position(&x, &y);
  binui_get_size(&w, &h);
  binui_get_color(&color);
  f32 r,g,b,a;
  r = (color & 0xFF) * (1.0 / 255.0); 
  g = ((color >> 8) & 0xFF) * (1.0 / 255.0);
  b = ((color >> 16) & 0xFF) * (1.0 / 255.0);
  a = ((color >> 24) & 0xFF) * (1.0 / 255.0);
  blit_rectangle(x, y, w, h, r,g,b,a);
  
}

void render_canvas(u64 canvas){
  canvas_context * ctx = canvas_get_context(canvas);

  double w, h;
  if(!control_try_get_size(canvas, &w, &h)){
    w = window_size.x;
    h = window_size.y;
  }
  
  vec2 pos = current_control_offset;
  blit_rectangle(pos.x,pos.y,w,h,1,1,1,1);
  u64 index = 0, cnt = 0, idx;

  blit3d_context_load(ctx->blit3d);
  
  while((cnt = u64_table_iter(canvas_polygons, &canvas, 1, NULL, &idx, 1, &index))){
    model * mod = get_or_create_polygon(canvas_polygons->value[idx]);     
    canvas_render_polygon(ctx, mod);
    mat4_rotate_3d_transform(0,0,0,0,0,0);
  }
  rectangle_handle = blit_rectangle3;

  io_reset(&ctx->wd);
  blit_push();
  blit_begin(BLIT_MODE_PIXEL_SCREEN);
  binui_iterate(&ctx->binui, &ctx->wd, NULL, NULL);
  blit_pop();		
  rectangle_handle = NULL;
}

void canvas_init(){
  canvas_class = intern_string("canvas class");
  points = f32_f32_vector_create("canvas points");
  polygon_table = u64_f32_f32_vector_index_create("canvas polygons");
  canvas_polygons = u64_table_create("polygons");
  loaded_polygons = u64_table_create(NULL);
  canvas_contexts = u64_table_create(NULL);
  register_table(polygon_table);
  ((bool *)(&canvas_polygons->is_multi_table))[0] = true;
  class_set_method(canvas_class, render_control_method, render_canvas);
}
