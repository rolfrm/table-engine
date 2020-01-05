#include <iron/full.h>
#include <icydb.h>
#include "string_intern.h"
#include "graphics_module.h"
#include "u64_table.h"
u64_table * console_history_cnt;
u64_table * console_height;
u64_table * console_history;
u64_table * console_index;

u64 console_class;
//u64 console_add_history_methods;
//u64 console_command_entered_method;

void console_render(u64 control){
  const char * l = "Console rendering";
  render_text(l, strlen(l), window_size, vec2_new(0, 0));
}

void console_init(){
  console_class = intern_string("console class");
  class_set_method(console_class, render_control_method, (void *) console_render);
}
