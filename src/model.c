#include <iron/full.h>
#include <iron/gl.h>
#include "model.h"
void render_model(model_context * ctx, model * object, mat4 transform, u32 id, vec4 color){

  var blit3d = ctx->blit3d;
  
  var o = object->offset.data;
  var r = object->rotation.data;
  var s = object->scale;
  var O = rotate_offset_3d_transform(o[0], o[1], o[2], r[0], r[1], r[2]);
  if(vec3_compare(v3_one, s, 0.0001) == false){
    O = mat4_mul(O, mat4_scaled(s.x, s.y, s.z));

  }
  color = vec4_mul(object->color, color);
  if(color.w <= 0.0)
    return;
  if(color.w < 1.0 && alpha_pass == false) return;
  
  mat4 C = mat4_invert(ctx->camera_matrix);
  
  O = mat4_mul(transform, O);
  var view = ctx->views + object->view_id;
  if(object->view_id != 0){
    if(view->loaded == false && view->model != 0){
      var dim = view->dim_px;
      if(dim.x <= 0.5 || dim.y <= 0.5)
	dim = view->dim_px = vec2_new(512, 512);
      blit_framebuffer buf = {.width = (int)dim.x, .height = (int)dim.y,
			      .channels = view->alpha ? 4 : 3};
      
      blit_begin(BLIT_MODE_PIXEL_SCREEN);
      blit_create_framebuffer(&buf);
      blit_use_framebuffer(&buf);


      view->loaded = true;
      
      glViewport(0, 0, dim.x, dim.y);
      glClearColor(0.0, 0.0, 0, 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable(GL_BLEND);
      blit3d_context_load(blit3d);
      bool pass = alpha_pass;
      var view_matrix = ctx->view_matrix;
      var camera_matrix = ctx->camera_matrix;
      ctx->view_matrix = view->view_matrix;
      ctx->camera_matrix = view->camera_matrix;
      glDisable(GL_BLEND);      
      glEnable(GL_DEPTH_TEST);
      alpha_pass = false;
      render_model(ctx, mat4_identity(), view->model, vec4_new(1,1,1,1));
      alpha_pass = true;
      glEnable(GL_BLEND);

      render_model(ctx, mat4_identity(), view->model, vec4_new(1,1,1,1));
      blit_unuse_framebuffer(&buf);
      blit3d_context_load(blit3d);

      alpha_pass = pass;
      ctx->view_matrix = view_matrix;
      ctx->camera_matrix = camera_matrix;
      
      view->tex = blit_framebuffer_as_texture(&buf);
      
    }
  }
  if(object->view_id == 0 || view->loaded == false)
    view = NULL;
  
  if(object->cache.loaded == false){
    if(object->cache.path != NULL){
      image img = image_from_file(object->cache.path);
      if(img.width != 0){
	object->cache.tex = texture_from_image(&img);
	printf("loaded image: %i\n", img.channels);
	if(img.channels == 4)
	  object->cache.alpha = true;
	image_delete(&img);
	object->cache.loaded = true;
	
      }else{
	printf("Failed to load texture from \"%s\"\n", object->cache.path);
      }
    }
  if(object->type == MODEL_TYPE_TEXT && object->text != NULL && alpha_pass){
    printf("Loading texture cache\n");
    vec2 dim = blit_measure_text(object->text);
    vec2_print(dim);logd("\n");
    object->cache.dim_px = dim;
    object->cache.loaded = true;
    object->cache.alpha = true;
    
    blit_framebuffer buf = {.width = (int)dim.x, .height = (int)dim.y,
			    .channels = 4};
    
    blit_begin(BLIT_MODE_PIXEL_SCREEN);
    blit_create_framebuffer(&buf);
    blit_use_framebuffer(&buf);
    
    glViewport(0, 0, dim.x, dim.y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    blit_begin(BLIT_MODE_PIXEL_SCREEN);
    blit_color(1, 1, 1, 1);
    blit_text(object->text);
    blit_unuse_framebuffer(&buf);
    
    blit3d_context_load(blit3d);
    object->verts = blit3d_polygon_new();
    float aspect = dim.x / dim.y;
    
    float xy[] = {-1 * aspect,-1,0, 1 * aspect,-1,0, -1 * aspect,1,0, 1 * aspect,1,0};
    
    blit3d_polygon_load_data(object->verts, xy, 12 * sizeof(f32));
    blit3d_polygon_configure(object->verts, 3);
    glEnable(GL_BLEND);
    object->uvs = blit3d_polygon_new();
    float uv[] = {0,0, 1,0,0,1,1,1};
    blit3d_polygon_load_data(object->uvs, uv, 8 * sizeof(f32));
    blit3d_polygon_configure(object->uvs, 2);
    object->cache.tex = blit_framebuffer_as_texture(&buf);
  }
  }
  
  if((object->type == MODEL_TYPE_POLYGONAL || object->type == MODEL_TYPE_TEXT) && object->verts != NULL){
    bool isalpha = color.w < 1.0 || (object->cache.loaded && object->cache.alpha) || (view != NULL && view->alpha);

    
    if((!isalpha && alpha_pass == false) || (isalpha && alpha_pass)){
      mat4 V = ctx->view_matrix;
      // World coord: Object * Vertex
      // Camera coord: InvCamera * World
      // View coord: View * Camera(ve
      // Vertx * Object * InvCamera * View
      vertex_buffer * buffers[2] = {object->verts, object->uvs};
      int bufcnt = 1;
      if(object->uvs != NULL)
	bufcnt = 2;
      if(object->cache.loaded)
	blit3d_bind_texture(blit3d, &object->cache.tex);
      else if(view != NULL){
	blit3d_bind_texture(blit3d, &view->tex);
      }
      blit3d_view(blit3d, mat4_mul(V, mat4_mul(C, O)));
      blit3d_color(blit3d, color);
      blit3d_polygon_blit2(blit3d, buffers, bufcnt);
      if(object->cache.loaded)
	blit3d_bind_texture(blit3d, NULL);
      else if(view != NULL){
	blit3d_bind_texture(blit3d, NULL);
      }
    }
  }

  int err = glGetError();
  if(err != 0)
    printf("ERR? %i\n", err);
  
  size_t indexes[10];
  size_t cnt;
  size_t index = 0;
  while((cnt = u32_to_u32_table_iter(ctx->model_to_sub_model, &id, 1, NULL, indexes, array_count(indexes), &index))){
    for(size_t i = 0 ; i < cnt; i++){
      var sub = ctx->model_to_sub_model->value[indexes[i]];
      //printf("render sub\n");

      if(sub == id){
	print_object_sub_models();
	ERROR("LOOP\n");
      }
      render_model(ctx, O, sub,  color);
    }
  }

}
