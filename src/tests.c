#include <iron/full.h>
#include <icydb.h>
#include "string_intern.h"
#include "graphics_module.h"

void test_string_intern(){
 u64 a = intern_string("Hello world");
  u64 b = intern_string("HHHHHHHHHHHEEEEEEEEEEELLLLLLLLLLLLLLOOOOOOOO");
  u64 c = intern_string("Hello world");
  u64 d = intern_string("Hello world!");
  u64 e = intern_aggregate(a,b);
  u64 f = intern_aggregate(b,c);
  u64 g = intern_aggregate(a, b);
  u64 h = intern_string("");
  u64 j = intern_string("");
  u64 i = intern_string("Hello world!!");
  u64 k = intern_string("Hello world");
  ASSERT(a == c);
  ASSERT(a != b);
  printf("%i %i %i %i %i %i %i %i %i %i %i\n", a,b,c,d, e, f, g, h, j, i, k);
 
}

void render_test_window(u64 id){
  const char * l = "Hello world from win 1";
  render_text(l, strlen(l), window_size, vec2_new(0, 0));
  
  render_text(l, strlen(l), window_size, vec2_new(0, 20));
  
  render_text(l, strlen(l), window_size, vec2_new(0, 40));
  
  render_text(l, strlen(l), window_size, vec2_new(0, 60));
}

void render_test_window2(u64 id){
  const char * l = "Hello world from win 2";
  render_text(l, strlen(l), window_size, vec2_new(0, 0));
}



void test_graphics(){

  u64 win = control_new_named("test_window");
  u64 win2 = control_new_named("test_window2");
  set_class(win, window_class);
  u64 console = control_new_named("test_window console");
  control_add_sub(win, console);
  set_class(console, console_class);
  class_set_method(win, render_control_method, NULL);
  class_set_method(win2, render_control_method, render_test_window2);
  
  show_window(win);
  show_window(win2);
  
  while(graphics_process_active_window_count()){
    iron_usleep(10000);
    graphics_process();
  }

  printf("all windows closed\n");
}

void init_module(){
  printf("running tests\n");
  test_string_intern();
  test_graphics();
}
