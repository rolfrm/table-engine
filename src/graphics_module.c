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

u64_table * windows;

void show_window(u64 windowid){
  u64_table_set(windows, windowid, 0);
}

void unshow_window(u64 windowid){
  u64_table_unset(windows, windowid);
}

u64_table * window_pointers;

void init_module(){
  printf("Initialized graphics\n");
  control_size_table = control_size_create("control size");
  class = u64_table_create("control class");
  control_window = intern_string("control window");
  windows = u64_table_create("control_window");
  window_pointers = u64_table_create(NULL);
  //initialized_fonts();
}

vec2 window_size;

void render_control(u64 control){
  UNUSED(control);
  render_text("Hello world!", 12, window_size, vec2_new(0, 0));
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
      //gl_window_destroy(&win2);
    }else if(type == EVT_WINDOW_RESIZE){
      int w = evt.window_size_change.width;
      int h = evt.window_size_change.height;
      control_set_size(key, w, h);
    }
  }

}

void render_window(u64 winid){
  gl_window * win;
  if(!u64_table_try_get(window_pointers, &winid, (u64 *) &win)){
    win = gl_window_open(512, 512);
    process_events();
    u64_table_set(window_pointers, winid, (u64) win);   
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
  
  glClear(GL_COLOR_BUFFER_BIT);
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

