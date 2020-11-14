#include <iron/full.h>
#include <iron/gl.h>
#include <icydb.h>
#include "string_intern.h"
#include "graphics_module.h"
#include "u64_table.h"
#include "test.h"

#include "f32_f32_vector.h"
#include "u64_f32_f32_vector_index.h"

extern f32_f32_vector * points;
extern u64_f32_f32_vector_index * polygon_table;
extern u64_table * canvas_polygons;

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

bool is_command(u64 cmd){
  return class_get_method(cmd, invoke_command_method) != NULL;
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
	    u64 next_id = intern_aggregate(id, id1);
	    if(is_command(next_id)){
	      id = next_id;
	    }else{
	      call_command(console, id, cmdp2);
	      return;
	    }
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
  printf("Priting table %s: %p\n", command, t);
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

int load_module(const char * name);
static void load_module_handler(char * module){
  load_module(module);
}

static void print_test(char * printcmd){
  printf("PRINT: |%s|\n", printcmd);
}

void iterate_tables(void (*f )(icy_table *table, void * userdata), void * userdata);

static void print_single_table(icy_table * table, void * userdata){
  icy_mem ** t = icy_table_get_memory_areas(table);
  
  console_log(t[0]->name);
}

static void print_tables(char * printcmd){
  iterate_tables(print_single_table, NULL);
}

bool should_exit = false;

void exit_command(char * args){
  UNUSED(args);
  should_exit = true;
  console_log("Application exited by user");
  printf("Application exited by user\n");
}


void continue_init_load();

void test_graphics(){

  { // window 1;
    u64 win = control_new_named("test_window");
    
    set_class(win, window_class);

    u64 decorator = control_new_named("test_window console decorator");
    set_class(decorator, control_class);
    u64 console = control_new_named("test_window console");
    printf("win/console: %i %i %s\n", win, console, control_get_name(console));
    if(get_class(console) == 0){
      control_add_sub(win, decorator);
      control_add_sub(decorator, console);
    }
    control_set_position(decorator, 10, 10);
    set_class(console, console_class);
    class_set_method(console, console_command_entered_method, handle_command_entered);
      
    control_set_focus(win, console);
    show_window(win);
  }
  

  { // window 2;
    u64 win2 = control_new_named("test_window2");
    u64 decorator = control_new_named("test window2 dec");
    set_class(decorator, control_class);
    u64 canvas = control_new_named("test_window2 canvas");
    
    set_class(canvas, canvas_class);
    set_class(win2, window_class);
    printf("Canvas: %i\n", canvas_class);
    control_add_sub(win2, decorator);
    control_add_sub(decorator, canvas);
    printf("Win2: %i\n", win2);
    control_set_position(decorator, 10, 10);
    control_set_size(canvas, 50, 50);
    u64 polygon1 = intern_string("polygon1");
    u64_table_set(canvas_polygons, canvas, polygon1);
    f32_f32_vector_indexes indx;
    if(!u64_f32_f32_vector_index_try_get(polygon_table, &polygon1, &indx)){
      indx = f32_f32_vector_alloc_sequence(points, 4);
      f32 * x = points->x + indx.index;
      f32 * y = points->y + indx.index;
      x[0] = 1; x[1] = 1; x[2] = 0; x[3] = -1;
      y[0] = 0; y[1] = 1; y[2] = 0; y[3] = 2;
      u64_f32_f32_vector_index_set(polygon_table, polygon1, indx);
      printf("Configure a new polygon");

    }
    show_window(win2);
  }

  // load the rest of the modules!
  continue_init_load();
  
  while(!should_exit && graphics_process_active_window_count()){
    iron_usleep(10000);
    graphics_process();
  }
  if(should_exit){

  }

  printf("all windows closed\n");
}

void print_named_control(u64 control, const char * name, void * userdata){
  console_log(name);
}

void print_named_controls(char * args){
  named_controls_iterate(print_named_control, NULL);
}


void init_module(){
  printf("running tests\n");
  test_string_intern();

  invoke_command_method = intern_string("console invoke command");
  u64 print_table_cmd = intern_aggregate(intern_string("print"), intern_string("table"));
  class_set_method(print_table_cmd, invoke_command_method, print_table);
  
  u64 load_module_cmd = intern_aggregate(intern_string("load"), intern_string("module"));
  class_set_method(load_module_cmd, invoke_command_method, load_module_handler);

  u64 print_test_cmd = intern_aggregate(intern_string("print"), intern_string("test"));
  class_set_method(print_test_cmd, invoke_command_method, print_test);

  u64 print_tables_cmd = intern_aggregate(intern_string("print"), intern_string("tables"));
  class_set_method(print_tables_cmd, invoke_command_method, print_tables);

  u64 print_controls_cmd = intern_aggregate(intern_string("print"), intern_string("named_controls"));
  class_set_method(print_controls_cmd, invoke_command_method, print_named_controls);
  
  u64 exit_cmd = intern_aggregate(intern_string("exit"), intern_string("now"));
  class_set_method(exit_cmd, invoke_command_method, exit_command);

  canvas_init();
  
  test_graphics();
}
