#include <iron/full.h>
#include "graphics_module.h"
#include "string_table.h"
#include "string_intern.h"
#include "string_redirect_table.h"
#include "string_redirect_table.c"

u64 textbox_class;

string_table * textbox_strings;
string_redirect_table * textbox_redirect;

static void textbox_render(u64 id){
  
}

void textbox_set_text(u64 object, const char * text){

}

void textbox_init(){

  textbox_class = intern_string("textbox class");
  class_set_method(textbox_class, render_control_method, textbox_render);
  textbox_strings = string_table_create("textbox strings");
  textbox_redirect = string_redirect_table_create("textbox redirect");
  
}
