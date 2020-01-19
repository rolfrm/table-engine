#include <iron/full.h>
#include <iron/gl.h>
#include <icydb.h>
#include "string_intern.h"
#include "graphics_module.h"
#include "u64_table.h"

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
  blit_begin(BLIT_MODE_UNIT);
  blit_rectangle(-1,-1,2,2,1,0,0,0.1);
  //blit_end();
  const char * l = "Hello world from win 2";
  render_text(l, strlen(l), window_size, vec2_new(0, 0));
}
u64 invoke_command_method;

u64 call_command_console;
void console_log(const char * line){
  if(call_command_console == 0)
    ERROR("console_log");
  console_push_history(call_command_console, line);
}

void call_command(u64 console, u64 method, const char * cmd){
  call_command_console = console;
  void (* f)(const char * cmd) = class_get_method(method, invoke_command_method);
  if(f == NULL){
    console_push_history(console, "unknown command");
  }else{
    f(cmd);
  }
  call_command_console = 0;
}

void parse_command(u64 console, const char * cmd){
  const char * cmdp = cmd;
  const char * cmdp2 = cmd;
  u64 id = 0;
  while(*cmdp != 0){
    if(cmdp[0] == ' ' || cmdp[1] == 0){
      
      int l = cmdp - cmdp2;
      if(cmdp[1] == 0)
	l += 1;
      if(l > 1){
	char buf[l + 1];
	memcpy(buf, cmdp2, l);
	buf[l] = 0;
	u64 id1;
	if(intern_string_get(buf, &id1)){
	  if(id == 0){
	    id = id1;
	  }else{
	    id = intern_aggregate(id, id1);
	  }
	}else{
	  call_command(console, id, cmdp2);
	  return;
	}
      }
      cmdp2 = cmdp + 1;
    }

    cmdp += 1;
  }
  if(id != 0){
    call_command(console, id, "");    
  }
}

void handle_command_entered(u64 console, char * command, u32 length){
  u8 cmd[length + 1];
  memcpy(cmd, command, length);
  cmd[length] = 0;
  var cmd2 = fmtstr("Executed> %s", cmd);
  console_push_history(console, cmd2);
  parse_command(console, cmd);
  dealloc(cmd2);
}

static void print_table(char * command){
  u64 p;
  if(intern_string_get(command, &p)){
    printf("Key exists! %i\n", p);
  }
  icy_table * t = table_get_named(command);
  //char * s = fmtstr("print table '%s' %p", command, t);
  //console_log(s);
  //dealloc(s);
  
  if(t != NULL){
    char buf[200];
    int offset = 0;
    for(int i = 0; i < t->column_count; i++){
      offset += sprintf(buf + offset, "%s ", t->column_names[i]);
    }
    console_log(buf);

    icy_mem ** mems = icy_table_get_memory_areas(t);
    
    for(int j = 1; j < t->count + 1; j++){
      char buf[200] = {0};
      int offset = 0;
      for(int i = 0; i < t->column_count; i++){
	void * ptr = mems[i]->ptr;
	char * type = t->column_types[i];
	if(strcmp(type, "u64") == 0){
	  offset += sprintf(buf + offset, "%i ", ((u64 *) ptr)[j]);
	}else if(strcmp(type, "f64") == 0){
	  offset += sprintf(buf + offset, "%f ", ((f64 *) ptr)[j]);
	}else if(strcmp(type, "f32") == 0){
	  offset += sprintf(buf + offset, "%f ", ((f32 *) ptr)[j]);
	}
      }
      console_log(buf);
    }
  }
  
}

void test_graphics(){
  
  u64 win = control_new_named("test_window");
  u64 win2 = control_new_named("test_window2");
  set_class(win, window_class);

  u64 decorator = control_new_named("test_window console decorator");
  set_class(decorator, control_class);
  u64 console = control_new_named("test_window console");
  printf("win/console: %i %i\n", win, console);
  if(get_class(console) == 0){
    control_add_sub(win, decorator);
    control_add_sub(decorator, console);
  }
  control_set_position(decorator, 10, 100);
  set_class(console, console_class);
  class_set_method(win, render_control_method, NULL);
  class_set_method(win2, render_control_method, render_test_window2);
  class_set_method(console, console_command_entered_method, handle_command_entered);

  control_set_focus(win, console);
  invoke_command_method = intern_string("console invoke command");
  u64 print_table_cmd = intern_aggregate(intern_string("print"), intern_string("table"));
  class_set_method(print_table_cmd, invoke_command_method, print_table);


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
