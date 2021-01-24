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
#include "binui.h"
void binui_test_load(binui_context * ctx, io_writer * wd);
f32_f32_vector * points;
u64_f32_f32_vector_index * polygon_table;
u64_table * canvas_polygons;
u64_table * loaded_polygons;

u64_table * canvas_contexts;
typedef struct {
  blit3d_context * blit3d;
  io_writer wd;
  binui_context * binui;
  char * resource_file;
  u64 timestamp;
}canvas_context;

u64 canvas_class;

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
      for(size_t i = 0; i < indx.count; i++){
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
  UNUSED(ctx, mod);
}

canvas_context * canvas_get_context(u64 canvas){
  u64 ptr;
  if(!u64_table_try_get(canvas_contexts, &canvas, &ptr)){
    
    canvas_context * ctx = alloc0(sizeof(canvas_context));
    ctx->binui = binui_new();
    
    ctx->blit3d  = blit3d_context_new();
 
    u64_table_set(canvas_contexts, canvas, (u64) ctx);
    return ctx;
  }
  return (canvas_context *) ptr;
}


static void blit_rectangle3(binui_stack_frame * frame, void * userdata){
  UNUSED(frame);
  var ctx = (canvas_context *) userdata;
  if(ctx == NULL) return;
  var opcode = binui_current_opcode(ctx->binui);

  binui_opcode BINUI_3D_POLYGON = binui_opcode_parse(ctx->binui, "polygon");
  if(opcode != BINUI_3D_POLYGON)
    return;
  u32 color;
  binui_get_color(ctx->binui, &color);
  f32 r,g,b,a;
  r = (color & 0xFF) * (1.0 / 255.0); 
  g = ((color >> 8) & 0xFF) * (1.0 / 255.0);
  b = ((color >> 16) & 0xFF) * (1.0 / 255.0);
  a = ((color >> 24) & 0xFF) * (1.0 / 255.0);
  
  
  f32_array array = blit3d_polygon_get(ctx->binui);
  if(array.count == 0) return;
  mat4 m = transform_3d_current(ctx->binui);
  mat4 c = camera_get(ctx->binui);
  // binui_polygon poly = binui_polygon_get(ctx->blit3d);
  blit3d_polygon * p = blit3d_polygon_new();
  blit3d_polygon_load_data(p, array.array, array.count * 3 * sizeof(f32));
  blit3d_polygon_configure(p, 3);
  blit3d_color(ctx->blit3d, vec4_new(r,g,b,a));
  blit3d_view(ctx->blit3d, mat4_mul(c, m));
  blit3d_polygon_blit(ctx->blit3d, p);
  
  blit3d_polygon_destroy(&p);
}

void render_canvas(u64 canvas){
  canvas_context * ctx = canvas_get_context(canvas);

  double w, h;
  if(!control_try_get_size(canvas, &w, &h)){
    w = window_size.x;
    h = window_size.y;
  }
  
  //vec2 pos = current_control_offset;
  //blit_rectangle(pos.x,pos.y,w,h,1,1,1,1);
  const char * path = "./scene1.lisp";
  if(file_exists(path) == false){
    logd("File %s does not exist\n", path);
    return;
  }
  let mod = file_modification_date(path);
  if(mod != ctx->timestamp){
    ctx->timestamp = mod;
    logd("Loading %s\n", path);
    //"(color 0xFF332244 (scale 1.0 1.0 1.0 (translate 0 0 0.5 (rotate 0 0 1 0.5 (polygon 0 0 0   0 1 0   1 0 0   1 1 0)))))";
    char * target = read_file_to_string(path);
    io_writer_clear(&ctx->wd);
    binui_load_lisp_string(ctx->binui, &ctx->wd, target);
    dealloc(target);
  }

  
  blit3d_context_load(ctx->blit3d);
  /*
  u64 index = 0, cnt = 0, idx;
  while((cnt = u64_table_iter(canvas_polygons, &canvas, 1, NULL, &idx, 1, &index))){
    model * mod = get_or_create_polygon(canvas_polygons->value[idx]);     
    canvas_render_polygon(ctx, mod);
    mat4_rotate_3d_transform(0,0,0,0,0,0);
  }
  */
  node_callback render = {
    .after_enter = blit_rectangle3,
    .before_exit = NULL,
    .userdata = ctx
  };

  node_callback_push(ctx->binui, render);
  
  io_reset(&ctx->wd);
  binui_iterate(ctx->binui, &ctx->wd);	
  node_callback_pop(ctx->binui);
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
