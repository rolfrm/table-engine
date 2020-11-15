#include <iron/full.h>
#include<microio.h>

#include "binui_types.h"
#include "binui_types.c"
#include "binui.h"
extern io_writer * binui_stdout;


io_writer * binui_stdout;

void binui_types_add(binui_types * types, u8 type, u8 opcode_size){

}

const u8 BINUI_OPCODE_VLEN = 255;

void binui_init(binui_context * ctx){
  ctx->type_table = binui_types_create(NULL);
  binui_types_add(ctx->type_table, 0, 0);
  binui_types_add(ctx->type_table, 0, BINUI_OPCODE_VLEN);
}

void binui_load(binui_context * ctx, io_reader * rd){

}

const int BINUI_OPCODE_NONE = 0;
const int BINUI_IMPORT_MODULE = 1;
const int BINUI_CANVAS = 2;
const int BINUI_RECTANGLE = 3;
const int BINUI_POSITION = 4;
const int BINUI_SIZE = 5;
const int BINUI_COLOR = 6;
const int BINUI_MAGIC = 0x5a;

static u32 current_color;
static u32 current_width, current_height;
static i32 current_x, current_y;

void binui_get_color(u32 * color){
  *color = current_color;
}

void binui_get_size(u32 * w, u32 * h){
  *w = current_width;
  *h = current_height;
}

void binui_get_position(i32 * x, i32 * y){
  *x = current_x;
  *y = current_y;
}

void (* rectangle_handle)(void * userdata);
const int BINUI_MAX_STACK_SIZE = 1024;

typedef struct{
  union{
    struct{
      u32 w, h;
    }size;
    struct{
      u32 x, y;
    }pos;


    void * user_data_ptr;
  };

}data_struct;

void binui_iterate_internal(binui_context * reg, io_reader * reader, void (* callback)(binui_context *registers, void * userdata), void * userdata){

  const bool debug = false;
  const bool lisp = false;
  
  void revert_color(void * userdata){
    if(debug)
      printf("revert color %i %i\n", current_color, userdata);
    current_color = (u32) (size_t)userdata;
  }

  void revert_size(void * userdata){
    
    data_struct ds;
    ds.user_data_ptr = userdata;
    if(debug)
      printf(":: SIZE: %i %i     %i %i\n", current_width, current_height, ds.size.w, ds.size.h);
    current_width = ds.size.w;
    current_height = ds.size.h;
  }

  void revert_position(void * userdata){
    data_struct ds;
    ds.user_data_ptr = userdata;
    if(debug)
      printf(":: POS: %i %i     %i %i\n", current_x, current_y, ds.pos.x, ds.pos.y);
    current_x = ds.pos.x;
    current_y = ds.pos.y;
    
  }
  
  int stack_level = 0;
  u32 childs[BINUI_MAX_STACK_SIZE];
  void * userdatas[BINUI_MAX_STACK_SIZE];
  void (* callbacks[BINUI_MAX_STACK_SIZE])(void * userdata);
  while(true){
    u64 opcode = io_read_u64_leb(reader);
    u32 children = 0;
    if(debug)
      printf("opcode: %i\n", opcode);
    if(opcode == BINUI_OPCODE_NONE)
      break;
    if(lisp){
    for(int i = 0; i < stack_level; i++)
      printf(" ");
    printf("(");
    }
    if(opcode == BINUI_IMPORT_MODULE){
      
      u32 len = 0;
      char * name = io_read_strn(reader, &len);
      if(lisp)
	printf("import '%s'", name);
      if(debug) printf("Load module '%s'\n", name);
    }
    userdatas[stack_level] = NULL;
    callbacks[stack_level] = NULL;
    if(opcode == BINUI_CANVAS){
      // this does nothing.
      // canvas has no special meaning
      if(lisp)
	printf("canvas");
      children = io_read_u64_leb(reader);
    }
    
    if(opcode == BINUI_COLOR){
      u32 color  = io_read_u32(reader);
      if(lisp)
	printf("color %p", color);
      userdatas[stack_level] = (void *) (size_t) current_color;
      current_color = color;
      children = io_read_u64_leb(reader);
      callbacks[stack_level] = revert_color;  
    }

    if(opcode == BINUI_POSITION){
      i32 x  = io_read_i32_leb(reader);
      i32 y  = io_read_i32_leb(reader);
      if(lisp)
	printf("translate %i %i", x, y);
      children = io_read_u64_leb(reader);
      data_struct ds;
      ds.pos.x = current_x;
      ds.pos.y = current_y;
      current_x += x;
      current_y += y;
      
      callbacks[stack_level] = revert_position;
      userdatas[stack_level] = ds.user_data_ptr;
    }

    if(opcode == BINUI_SIZE){
      u32 w  = io_read_i32_leb(reader);
      u32 h  = io_read_i32_leb(reader);
      data_struct ds;
      ds.size.w = current_width;
      ds.size.h = current_height;
      current_width = w;
      current_height = h;
      children = io_read_u64_leb(reader);

      callbacks[stack_level] = revert_size;
      userdatas[stack_level] = ds.user_data_ptr;
      if(lisp)
      printf("size %i %i ", w, h);
    }
    
    if(opcode == BINUI_RECTANGLE){
      if(lisp) printf("rect");
      if(rectangle_handle != NULL){
	rectangle_handle(userdata);
      }
      if(debug)
	printf("paint: Rectangle %p (%i %i) (%i %i)\n", current_color, current_width, current_height, current_x, current_y);   
    }

    opcode = io_read_u64_leb(reader);
    if(opcode != BINUI_MAGIC){
      printf("ERROR Expected magic!\n");
      break;
    }
    
    // if children >
    while(true){
      if(debug)
	printf("Children: %i stack level: %i\n", children, stack_level);
      if(children >= 1){
	childs[stack_level] = children;
	stack_level += 1;
	break;
      }else if(stack_level > 0){
	var pop = callbacks[stack_level];
	if(pop != NULL)
	  pop(userdatas[stack_level]);
	 
	stack_level -= 1;
	children = childs[stack_level] -= 1;
	if(lisp)
	  printf(")");
      }else{
	if(lisp)
	  printf(")");
	break;
      }
    }
    if(lisp)
      printf("\n");
  }
}

void handle_opcode(binui_context * registers, void * userdata){

}

void binui_iterate(binui_context * reg, io_reader * reader, void (* callback)(binui_context * registers, void * userdata), void * userdata){

  binui_iterate_internal(reg, reader, callback, userdata);
}

void binui_test_load(io_writer * wd){
 io_write_u32_leb(wd, BINUI_IMPORT_MODULE);

  const char * modname = "graphics";
  io_write_strn(wd, modname);
  io_write_u32_leb(wd, BINUI_MAGIC);

  io_write_u32_leb(wd, BINUI_CANVAS);
  io_write_u32_leb(wd, 1);
  io_write_u32_leb(wd, BINUI_MAGIC);
  io_write_u32_leb(wd, BINUI_COLOR);
  io_write_u32(wd, 0xFF0000FF);
  io_write_u32_leb(wd, 1);
  io_write_u32_leb(wd, BINUI_MAGIC);
  io_write_u32_leb(wd, BINUI_POSITION);
  io_write_i32_leb(wd, 200);
  io_write_i32_leb(wd, 200);
  io_write_u32_leb(wd, 1);
  io_write_u32_leb(wd, BINUI_MAGIC);

  io_write_u32_leb(wd, BINUI_SIZE);
  io_write_i32_leb(wd, 15);
  io_write_i32_leb(wd, 15);
  io_write_u32_leb(wd, 1);
  io_write_u32_leb(wd, BINUI_MAGIC);
  
  
  io_write_u32_leb(wd, BINUI_SIZE);
  io_write_i32_leb(wd, 10);
  io_write_i32_leb(wd, 10);
  io_write_u32_leb(wd, 2);
  io_write_u32_leb(wd, BINUI_MAGIC);

  io_write_u32_leb(wd, BINUI_POSITION);
  io_write_i32_leb(wd, 5);
  io_write_i32_leb(wd, 5);
  io_write_u32_leb(wd, 1);
  io_write_u32_leb(wd, BINUI_MAGIC);
  
  
  io_write_u32_leb(wd, BINUI_SIZE);
  io_write_i32_leb(wd, 50);
  io_write_i32_leb(wd, 50);
  io_write_u32_leb(wd, 1);
  io_write_u32_leb(wd, BINUI_MAGIC);
  
  io_write_u32_leb(wd, BINUI_RECTANGLE);
  io_write_u32_leb(wd, BINUI_MAGIC);
  
  io_write_u32_leb(wd, BINUI_COLOR);
  io_write_u32(wd, 0xFFFFFFFF);
  io_write_u32_leb(wd, 2);
  io_write_u32_leb(wd, BINUI_MAGIC);
  
  io_write_u32_leb(wd, BINUI_RECTANGLE);
  io_write_u32_leb(wd, BINUI_MAGIC);

  
  io_write_u32_leb(wd, BINUI_POSITION);
  io_write_i32_leb(wd, 15);
  io_write_i32_leb(wd, 15);
  io_write_u32_leb(wd, 2);
  
  io_write_u32_leb(wd, BINUI_MAGIC);
  
  io_write_u32_leb(wd, BINUI_RECTANGLE);
  io_write_u32_leb(wd, BINUI_MAGIC);

  io_write_u32_leb(wd, BINUI_POSITION);
  io_write_i32_leb(wd, 15);
  io_write_i32_leb(wd, 15);
  io_write_u32_leb(wd, 1);
  io_write_u32_leb(wd, BINUI_MAGIC);
  
  io_write_u32_leb(wd, BINUI_RECTANGLE);
  io_write_u32_leb(wd, BINUI_MAGIC);


  io_write_u32_leb(wd, BINUI_RECTANGLE);
  io_write_u32_leb(wd, BINUI_MAGIC);
  
  io_write_u32_leb(wd, BINUI_OPCODE_NONE);
  io_reset(wd);
 

}



void binui_test(){
  binui_context reg;
  binui_init(&reg);

  io_writer _wd = {0};
  io_writer * wd = &_wd;
  binui_test_load(wd);
    binui_iterate(&reg, wd, handle_opcode, NULL);
  /*io_write_u8(wd, BINUI_SIZE);
  // 500x500
  io_write_u16(wd, 500);
  io_write_u16(wd, 500);
  io_write_u8(wd, BINUI_COLOR);
  // rgba
  io_write_u8(wd, 255);
  io_write_u8(wd, 0);
  io_write_u8(wd, 0);
  io_write_u8(wd, 255);
  io_write_u8(wd, BINUI_ID);
  const char * grpname = "hello world";
  io_write_strn(wd, grpname);
  
  io_write_u8(wd, BINUI_RECT);

  io_write_u8(wd, BINUI_SIZE);
  // 500x500
  io_write_u16(wd, 500);
  io_write_u16(wd, 500);
  io_write_u8(wd, BINUI_COLOR);
  // rgba
  io_write_u8(wd, 255);
  io_write_u8(wd, 255);
  io_write_u8(wd, 0);
  io_write_u8(wd, 255);
  io_write_u8(wd, BINUI_ID);
  const char * grpname2 = "hello world2";
  io_write_strn(wd, grpname2);
  io_write_u8(wd, BINUI_CIRCLE);
  
  io_reset(wd);
  binui_describe(wd);
  io_reset(wd);
  binui_iterate(wd, test_iterate, NULL);*/
  

}
