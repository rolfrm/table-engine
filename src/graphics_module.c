#include <stdio.h>
#include <iron/full.h>
#include <iron/gl.h>
#include <GL/gl.h>
#include "string_intern.h"
#include "control_size.h"
#include "control_size.c"
#include "u64_table.h"
#include "graphics_module.h"

u64 control_new(){
  return id_new();
}

u64 control_new_named(const char * name){
  return intern_string(name);
}

control_size * control_size_table;

void control_set_size(u64 control, double width, double height){
  control_size_set(control_size_table, control, width, height);
}

bool control_try_get_size(u64 control, double * width, double * height){
  if(control == 0) return false;
  if(false == control_size_try_get(control_size_table, &control, width, height))
    return control_try_get_size(get_class(control), width, height);
  return true;
}

control_size * control_position_table;

void control_set_position(u64 control, f64 x, f64 y){
  control_size_set(control_position_table, control, x, y);
}

bool control_get_position(u64 control, f64 *x, f64 * y){
  return control_size_try_get(control_position_table, &control, x, y);
}

static control_size * window_position_table;

void window_set_position(u64 control, f64 x, f64 y){
  control_size_set(window_position_table, control, x, y);
}

bool window_get_position(u64 control, f64 *x, f64 * y){
  return control_size_try_get(window_position_table, &control, x, y);
}

u64_table * class;

void set_class(u64 object, u64 base){
  u64_table_set(class, object, base);
}

u64 get_class(u64 object){
  u64 base;
  if(u64_table_try_get(class, &object, &base))
    return base;
  return 0;
}

u64_table * class_methods;


void class_set_method(u64 class, u64 method, void * methodf){
  u64 agg = intern_aggregate(class, method);
  if(methodf == NULL)
    u64_table_unset(class_methods, agg);
  else
    u64_table_set(class_methods, agg, (u64) methodf);
}

void * class_get_method(u64 object, u64 method){
  while(object != 0)
    {
      u64 agg = intern_aggregate(object, method);
      u64 methodf;
      if(u64_table_try_get(class_methods, &agg, &methodf))
	return (void *)methodf;
      object = get_class(object);
    }
  return NULL;
}

u64_table * sub_controls;

void control_add_sub(u64 object, u64 subobject){
  u64_table_set(sub_controls, object, subobject);
}

u64 control_get_subs(u64 object, u64 * array, u64 count, u64 * index){
  u64 indexes[10];
  u64 cnt = 0;
  u64 tcnt = 0;
  while(0 < (cnt = u64_table_iter(sub_controls, &object, 1, NULL, indexes, MIN(count, 10), index))){
    for(u64 i = 0; i < cnt; i++)
      array[i] = sub_controls->value[indexes[i]];
  
    array += cnt;
    count -= cnt;
    tcnt += cnt;
  }
  return tcnt;
}



u64_table * windows;

void show_window(u64 windowid){
  u64_table_set(windows, windowid, 0);
}

void unshow_window(u64 windowid){
  u64_table_unset(windows, windowid);
}




u64_table * window_pointers;
u64 render_control_method;
u64 key_event_method;
u64 control_class;
u64 window_class;
vec2 current_control_offset;
static void control_render(u64 control){
  u64 idx = 0;
  u64 subs[10];
  u64 cnt = 0;
  f64 x, y;
  bool pos = control_get_position(control, &x, &y);
  vec2 ppos;
  if(pos){
    ppos = current_control_offset;
    current_control_offset.x += x;
    current_control_offset.y += y;
  }
  while((cnt = control_get_subs(control, subs, array_count(subs), &idx))){
    for(u64 i = 0; i < cnt; i++)
      render_control(subs[i]);
  }
  if(pos){
    current_control_offset = ppos;
  }
}

u64_table * focused_controls;

void control_set_focus(u64 window, u64 control){
  u64_table_set(focused_controls, window, control);
}

void init_module(){
  printf("Initialized graphics\n");
  control_size_table = control_size_create("control size");
  control_position_table = control_size_create("control position");
  window_position_table = control_size_create("window position");
  class = u64_table_create("control class");
  sub_controls = u64_table_create("control subs");
  
  ((bool *)&sub_controls->is_multi_table)[0] = true;
  
  windows = u64_table_create("control_window");
  window_pointers = u64_table_create(NULL);
  class_methods = u64_table_create(NULL);

  focused_controls = u64_table_create("focus");

  render_control_method = intern_string("render control method");
  key_event_method = intern_string("key event method");  
  control_class = intern_string("control class");
  window_class = intern_string("window class");
  
  set_class(window_class, control_class);
  class_set_method(control_class, render_control_method, control_render);

  console_init();
  textbox_init();
}

vec2 window_size;

void render_control(u64 control){
  void ( * render)(u64 control) = class_get_method(control, render_control_method);
  if(render != NULL)
    render(control);
}


void process_events(){
  gl_window_event evts[10];
  size_t evt_count = gl_get_events(evts, array_count(evts));
  for(size_t i = 0; i < evt_count; i++){
    var evt = evts[i];
    var type = evt.type;
    u64 key = 0;
    gl_window * win2 = NULL;
    
    for(u32 j = 0; j < window_pointers->count; j++){
      win2 = (gl_window *) window_pointers->value[j + 1];	  
      if(win2 == evt.win){
	key = window_pointers->key[j + 1];
	break;
      }
    }
    if(key == 0) continue;
    if(type == EVT_WINDOW_CLOSE){
      printf("Window closed!\n");
      u64_table_unset(window_pointers, key);
      unshow_window(key);
      gl_window_destroy(&win2);
    }else if(type == EVT_WINDOW_RESIZE){
      int w = evt.window_size_change.width;
      int h = evt.window_size_change.height;
      control_set_size(key, w, h);
    }
    else if(type == EVT_WINDOW_MOVE){
      int x = evt.window_position_change.x;
      int y = evt.window_position_change.y;
      window_set_position(key, (f64) x, (f64) y);
    }else{
      u64 out_ctrl = 0;
      if(u64_table_try_get(focused_controls, &key, &out_ctrl)){
	void (* f)(u64 control, gl_window_event evt) = class_get_method(out_ctrl, key_event_method);
	if(f != NULL){
	  f(out_ctrl, evt);
	}
      }	 
    }
  }
}

void render_window(u64 winid){
  gl_window * win;
  if(!u64_table_try_get(window_pointers, &winid, (u64 *) &win)){
    win = gl_window_open(512, 512);
    process_events();
    u64_table_set(window_pointers, winid, (u64) win);
    f64 x, y;
    if(window_get_position(winid, &x, &y)){
      gl_window_set_position(win, (int)x, (int)y);
    }
  }
  int w,h;
  gl_window_get_size(win, &w, &h);
  gl_window_make_current(win);
  double w2 = 0,h2 = 0;
  if(control_try_get_size(winid, &w2, &h2)){
    if(w2 <= 0 || h2 <= 0){
      w2 = 512;
      h2 = 512;
      control_set_size(winid, w2, h2);
    }
    int w2i = (int)w2;
    int h2i = (int)h2;
    if(w2i != w || h != h2i){
      gl_window_set_size(win, w2i, h2i);
    }
    w = w2i;
    h = h2i;
    glViewport(0,0,w,h);
  }

  window_size = vec2_new(w, h);
  current_control_offset = vec2_zero;
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_BLEND);
  render_control(winid);
  gl_window_swap(win);
}

int graphics_process_active_window_count(){
  return windows->count;
}

void graphics_process(){
  for(u32 i = 0; i <windows->count; i++){
    render_window(windows->key[i + 1]);
  }
  process_events();
    
  gl_window_poll_events();
}

