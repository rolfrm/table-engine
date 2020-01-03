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
  return false;
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

void render_control(u64 control){
  render_text("Hello world!", 12, vec2_new(512,512), vec2_new(0, 0));
}


void render_window(u64 winid){
  gl_window * win;
  if(!u64_table_try_get(window_pointers, &winid, (u64 *) &win)){
    win = gl_window_open(512, 512);
    u64_table_set(window_pointers, winid, (u64) win);   
  }
  //printf("Window: %i\n", winid);
  gl_window_make_current(win);
  glClear(GL_COLOR_BUFFER_BIT);
  render_control(win);
  gl_window_swap(win);
}

int graphics_process_active_window_count(){
  return windows->count;
}

void graphics_process(){
  for(u32 i = 0; i <windows->count; i++){
    render_window(windows->key[i + 1]);
  }
  
  gl_window_event evt[10];
  size_t evt_count = gl_get_events(evt, array_count(evt));
  for(size_t i = 0; i < evt_count; i++){
    if(evt[i].type == EVT_WINDOW_CLOSE){
      printf("Window closed!\n");
      for(u32 i = 0; i < window_pointers->count; i++){
	gl_window * win2 = (gl_window *) window_pointers->value[i + 1];
	if(win2 == evt[i].win){
	  u64 key = window_pointers->key[i + 1];
	  
	  u64_table_unset(window_pointers, key);
	  unshow_window(key);
	  gl_window_destroy(&win2);
	  break;
	}
      }
      
    }
  }
  
  /*
  for(u32 i = 0; i < window_pointers; i++){
    if(!u64_table_try_get(windows, window_pointers->key[i], NULL)){
      
    }
    }*/
  
  gl_window_poll_events();
}

