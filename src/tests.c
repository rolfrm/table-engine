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

void test_graphics(){

  u64 win = control_new_named("test_window");
  f64 w, h;
  if(control_try_get_size(win, &w, &h)){
    printf("%i %f %f\n", win, w, h);
  }
  control_set_size(win, 512, 512);
  show_window(win);
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
