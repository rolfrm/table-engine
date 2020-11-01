#include <iron/full.h>
#include <iron/gl.h>
#include <icydb.h>
#include "string_intern.h"
#include "graphics_module.h"
#include "u64_table.h"
#include "string_table.h"
#include "string_redirect_table.h"
u64_table * console_history_cnt;
u64_table * console_height;
u64_table * console_history;
u64_table * console_index;
string_table * console_strings;
string_redirect_table * string_redirect;
string_redirect_table * string_history_redirect;
u64 console_class;
//u64 console_add_history_methods;
u64 console_command_entered_method;

static void write_char(u64 console, char chr){
  u64 index = 0;
  u64_table_try_get(console_index, &console, &index);
  string_table_indexes string_index = {0};
  u64 length = 0;
  void * src = NULL;
  if(string_redirect_table_try_get(string_redirect, &console, &string_index)){
    src = console_strings->data + string_index.index;
    length = string_index.count;
  }
  
  string_table_indexes string_index2 = string_table_alloc_sequence(console_strings, length + 1);
  void * dest = console_strings->data + string_index2.index;
  memcpy(dest, src, length);
  if(length > 0)
    memmove(dest + index + 1, dest + index, length - index);
  memcpy(dest + index, &chr, sizeof(chr));
  string_table_remove_sequence(console_strings, &string_index);
  string_redirect_table_set(string_redirect, console, string_index2);
  u64_table_set(console_index, console, index + 1);  
}

void console_push_history(u64 console, const char * string){
  int len = strlen(string);
  string_table_indexes string_index2 = string_table_alloc_sequence(console_strings, len);
  char * outstr = console_strings->data + string_index2.index;
  memcpy(outstr, string, len);
  string_redirect_table_set(string_history_redirect, console, string_index2);
}

static void enter_command(u64 console){
  void (* m)(u64 console, char * string, u32 length) = class_get_method(console, console_command_entered_method);
  string_table_indexes string_index = {0};
  if(string_redirect_table_try_get(string_redirect, &console, &string_index)){
    string_redirect_table_unset(string_redirect, console);
    u64_table_unset(console_index, console);

    char * ptr = console_strings->data + string_index.index;
    if(m != NULL)
      m(console, ptr, string_index.count);
    string_table_remove_sequence(console_strings, &string_index);
  }else{
    if(m != NULL)
      m(console, NULL, 0);
  }
}

static void handle_backspace(u64 console){
  u64 index = 0;
  u64_table_try_get(console_index, &console, &index);
  if(index == 0) return;
  string_table_indexes string_index = {0};
  u64 length = 0;
  void * src = NULL;
  if(string_redirect_table_try_get(string_redirect, &console, &string_index)){
    src = console_strings->data + string_index.index;
    length = string_index.count;
  }
  
  string_table_indexes string_index2 = string_table_alloc_sequence(console_strings, length - 1);
  void * dest = console_strings->data + string_index2.index;
  memcpy(dest, src, index - 1);
  if(index < length)
    memcpy(dest + index - 1, src + index, length - index - 1);
  string_table_remove_sequence(console_strings, &string_index);
  string_redirect_table_set(string_redirect, console, string_index2);
  u64_table_set(console_index, console, index - 1);  

}

static void handle_key_event(u64 console, gl_window_event evt){
  if(evt.type == EVT_KEY_DOWN || evt.type == EVT_KEY_REPEAT){
    if(evt.key.ischar){
      write_char(console, evt.key.codept);
    }
    int indexmove = 0;
    if(evt.key.key == KEY_LEFT){
      indexmove -= 1;
    }
    if(evt.key.key == KEY_RIGHT){
      indexmove += 1;
    }
    if(indexmove != 0){
      u64 index = 0;
      u64_table_try_get(console_index, &console, &index);
      index += indexmove;
      string_table_indexes string_index = {0};
      if(string_redirect_table_try_get(string_redirect, &console, &string_index)){
	if(index <= string_index.count){
	  u64_table_set(console_index, console, index);
	}	
      }
    }
    if(evt.key.key == KEY_ENTER){
      enter_command(console);
    }
    else if(evt.key.key == KEY_BACKSPACE){
      handle_backspace(console);
    }
    else{
      //printf("Unhandled key: %i\n", evt.key.key);
    }
  }
  else if(evt.type == EVT_KEY_UP){

  }else{
    return;
  }
}

static u64 history_count(u64 console){
  size_t string_index[21];
  u64 cnt = 0;
  u64 tcnt = 0;
  u64 index = 0;
  while((cnt = string_redirect_table_iter(string_history_redirect, &console, 1, NULL, string_index, array_count(string_index),  &index))){
    tcnt += cnt;
  }
  return tcnt;
}

static void console_render(u64 console){
  
  string_table_indexes string_index = {0};
  u64 length = 0;
  char * src = NULL;
  u64 index = 0;
  u64 index_out = 0;
  u64 cnt = 0;
  u64 voffset = 0;
  vec2 offset = current_control_offset;
  u64 total_count = history_count(console);
  int skip_cnt = (int)total_count -10;
  while((cnt = string_redirect_table_iter(string_history_redirect, &console, 1, NULL, &index_out, 1,  &index))){
    string_index = string_history_redirect->value[index_out];
    src = console_strings->data + string_index.index;
    
    skip_cnt -= 1;
    if(skip_cnt > 0) continue;
    length = string_index.count;
    render_text(src, length, window_size, offset);
    offset.y += 16;
  }
  if(string_redirect_table_try_get(string_redirect, &console, &string_index)){
    src = console_strings->data + string_index.index;


    u64 index = 0;
    u64_table_try_get(console_index, &console, &index);
    length = string_index.count;
    vec2 s = measure_text(src, index);
    vec2 s2 = measure_text("|", 1);
    render_text(src, index, window_size, offset);
    offset.x += s.x;
    render_text("|", 1, window_size, offset);
    offset.x += s2.x;
    render_text(src + index, length - index, window_size, offset);
    
  }else{
    const char * l = "|";
    render_text(l, strlen(l), window_size, offset);
  }
}

void console_init(){
  console_strings = string_table_create("console/strings");
  console_class = intern_string("console class");
  console_index = u64_table_create("console index");
  string_redirect = string_redirect_table_create("string/redirect");
  string_history_redirect = string_redirect_table_create("string/redirect/history");
  console_command_entered_method = intern_string("console/command entered");
  ((bool *)&string_history_redirect->is_multi_table)[0] = true;
  class_set_method(console_class, render_control_method, (void *) console_render);
  class_set_method(console_class, key_event_method, (void *) handle_key_event);
}
