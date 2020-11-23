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
  memset(ctx, 0, sizeof(ctx[0]));
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

// registers

void (* rectangle_handle)(void * userdata);
const int BINUI_MAX_STACK_SIZE = 1024;


typedef struct{
  void * elements;
  size_t capacity;
  size_t count;
}stack;

void stack_push(stack * stk, void * data, size_t count){
  if(stk->count + count >= stk->capacity){
    size_t newcap = stk->count * 2 + count;
    stk->capacity = newcap;
    stk->elements= realloc(stk->elements, newcap);
  }
  memcpy(stk->elements + stk->count, data, count);
  stk->count += count;
}

void stack_pop(stack * stk, void * data, size_t count){
  ASSERT(stk->count >= count);
  stk->count -= count;
  if(data != NULL)
    memcpy(data, stk->elements + stk->count, count);
}

void stack_top(stack * stk, void * data, size_t count){
  memcpy(data, stk->elements + stk->count - count, count);
}


typedef struct {
  u32 size;
  u32 id;
}binui_register;

void * binui_get_register(binui_context * ctx, binui_register * registerID){
  static u32 register_counter = 1;
  if(registerID->id == 0){
    registerID->id = register_counter;
    register_counter += registerID->size;
  }
  
  while(registerID->id + registerID->size >= ctx->reg_cap){
    u32 new_size = (registerID->id + registerID->size) * 2;
    ctx->registers = realloc(ctx->registers, new_size);
    // set all new registers to 0.
    memset(ctx->registers + ctx->reg_cap, 0, new_size - ctx->reg_cap);
    ctx->reg_cap = new_size;
  }

  // this assert is for catching possibily errors early. New registers should not be created very often.
  // in the future remove this.
  ASSERT(ctx->reg_cap < 10000); 
  return ctx->registers + registerID->id;
}

typedef struct {
  u32 size;
  binui_register stack;
}binui_stack_register;


void binui_stack_register_push(binui_context * ctx, binui_stack_register * reg, void * value){
  reg->stack.size = sizeof(stack);
  stack * stk = binui_get_register(ctx, &reg->stack);
  stack_push(stk, value, reg->size);
}

void binui_stack_register_pop(binui_context * ctx, binui_stack_register * reg, void * value){
  reg->stack.size = sizeof(stack);
  stack * stk = binui_get_register(ctx, &reg->stack);
  stack_pop(stk, value, reg->size);
}

bool binui_stack_register_top(binui_context * ctx, binui_stack_register * reg, void * value){
  reg->stack.size = sizeof(stack);
  stack * stk = binui_get_register(ctx, &reg->stack);
  if(stk->count == 0) return false;
  stack_top(stk, value, reg->size);
  return true;
}

binui_stack_register color_register ={.size =sizeof(u32), .stack = {0}};
void push_color(binui_context * ctx, u32 color){
  binui_stack_register_push(ctx, &color_register, &color);
}

u32 pop_color(binui_context * ctx){
  u32 color;
  binui_stack_register_push(ctx, &color_register, &color);
  return color;
}

u32 get_color(binui_context * ctx){
  u32 color;
  binui_stack_register_top(ctx, &color_register, &color);
  return color;
}

void binui_get_color(binui_context * ctx, u32 *color){
  *color = get_color(ctx);
}

void color_enter(binui_context * ctx, io_reader * reader){
  u32 color = io_read_u32(reader);
  push_color(ctx, color);
  if(ctx->lisp)
    printf("color %p", color);  
}

void color_exit(binui_context * ctx){
  pop_color(ctx);
}


typedef struct{
  i32 x, y;
}posi;

binui_stack_register position_reg = {.size = sizeof(posi), .stack = {0}};

void position_get(binui_context * ctx, i32 * x, i32 * y){
  posi pos = {0};
  
  if(binui_stack_register_top(ctx, &position_reg, &pos)){
    *x = pos.x;
    *y = pos.y;
  }else{
    *x = 0;
    *y = 0;
  }
     
}

void binui_get_position(binui_context * ctx, i32 * x, i32 * y){
  position_get(ctx, x, y);
}

void position_push(binui_context * ctx, int x, int y){
  posi pos = {0};
  binui_stack_register_top(ctx, &position_reg, &pos);
  pos.x += x;
  pos.y += y;
  binui_stack_register_push(ctx, &position_reg, &pos);
}

void position_pop(binui_context * ctx){
  binui_stack_register_pop(ctx, &position_reg, NULL);
}

void position_enter(binui_context * ctx, io_reader * reader){
  i32 x  = io_read_i32_leb(reader);
  i32 y  = io_read_i32_leb(reader);
  if(ctx->lisp)
    printf("translate %i %i", x, y);
  position_push(ctx, x, y);
}

void position_exit(binui_context * ctx){
  position_pop(ctx);
}

binui_stack_register size_reg = {.size = sizeof(posi), .stack = {0}};

void size_get(binui_context * ctx, int * x, int * y){
  posi size = {0};
  binui_stack_register_top(ctx, &size_reg, &size);
  *x = size.x;
  *y = size.y;
}

void size_push(binui_context * ctx, int x, int y){
  posi size = {.x = x, .y = y};
  binui_stack_register_push(ctx, &size_reg, &size);
}

void size_pop(binui_context * ctx){
  binui_stack_register_pop(ctx, &size_reg, NULL);
}

void size_enter(binui_context * ctx, io_reader * reader){
  i32 x  = io_read_i32_leb(reader);
  i32 y  = io_read_i32_leb(reader);
  if(ctx->lisp)
    printf("size %i %i\n", x, y);
  size_push(ctx, x, y);
}

void size_exit(binui_context * ctx){
  size_pop(ctx);
}

void binui_get_size(binui_context * ctx, u32 * w, u32 * h){
  int _w = 0, _h = 0;
  size_get(ctx, &_w, &_h);
  *w = _w;
  *h = _h;
}

void module_enter(binui_context * ctx, io_reader *reader){
  u32 len = 0;
  char * name = io_read_strn(reader, &len);
  if(ctx->lisp)
    printf("import '%s'", name);
  if(ctx->debug) printf("Load module '%s'\n", name);
}

void rectangle_enter(binui_context * ctx, io_reader * reader){
  if(ctx->lisp) printf("rect");
}

void canvas_enter(binui_context * ctx, io_reader * reader){
  if(ctx->lisp)
    printf("canvas");
}


void binui_iterate_internal(binui_context * reg, io_reader * reader, void (* callback)(binui_context *registers, void * userdata), void * userdata){

  bool debug = reg->debug;
  bool lisp = reg->lisp;
  
  int stack_level = 0;
  u32 childs[BINUI_MAX_STACK_SIZE];
  void * userdatas[BINUI_MAX_STACK_SIZE];
  void (* callbacks[BINUI_MAX_STACK_SIZE])(void * userdata);
  
  while(true){
    u64 opcode = io_read_u64_leb(reader);
    u32 children = 0;
    if(debug)
      printf("opcode: %i\n", opcode);
    userdatas[stack_level] = NULL;
    callbacks[stack_level] = NULL;
    
    if(opcode == BINUI_OPCODE_NONE)
      break;
    if(lisp){
    for(int i = 0; i < stack_level; i++)
      printf(" ");
    printf("(");
    }
    if(opcode == BINUI_IMPORT_MODULE){
      module_enter(reg, reader);
    }

    if(opcode == BINUI_CANVAS){
      // this does nothing.
      // canvas has no special meaning
      canvas_enter(reg, reader);

      children = io_read_u64_leb(reader);
    }
    
    if(opcode == BINUI_COLOR){
      color_enter(reg, reader);
      children = io_read_u64_leb(reader);
      userdatas[stack_level] = reg;
      callbacks[stack_level] = color_exit;  
    }

    if(opcode == BINUI_POSITION){
      position_enter(reg, reader);
      children = io_read_u64_leb(reader);
      userdatas[stack_level] = reg;      
      callbacks[stack_level] = position_exit;
    }

    if(opcode == BINUI_SIZE){
      size_enter(reg, reader);
      children = io_read_u64_leb(reader);
      userdatas[stack_level] = reg;
      callbacks[stack_level] = size_exit;
    }
    
    if(opcode == BINUI_RECTANGLE){
      rectangle_enter(reg, reader);

      if(rectangle_handle != NULL){
	rectangle_handle(userdata);
      }
      if(debug){
	i32 x, y;
	position_get(reg, &x, &y);
	int w, h;
	size_get(reg, &w, &h);
	printf("paint: Rectangle %p (%i %i) (%i %i)\n", get_color(reg), w, h, x, y);
      }
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
  //reg.debug = true;
  push_color(&reg, 1);
  push_color(&reg, 2);
  push_color(&reg, 3);
  printf("Color: %u\n", get_color(&reg));
  printf("Color: %u\n", pop_color(&reg));
  printf("Color: %u\n", pop_color(&reg));
  printf("Color: %u\n", pop_color(&reg));
  int x, y;
  position_push(&reg, 1, 1);
  position_get(&reg, &x, &y);
  printf(" %i %i\n", x, y);
  position_push(&reg, 2, 2);
  position_push(&reg, 3, 3);

  position_get(&reg, &x, &y);
  printf(" %i %i\n", x, y);
  ASSERT(x == 6 && y == 6);
  position_pop(&reg);
  position_get(&reg, &x, &y);
  ASSERT(x == 3 && y == 3);
  position_pop(&reg);
  position_pop(&reg);

  size_push(&reg, 1, 1);
  size_push(&reg, 2, 2);
  size_get(&reg, &x, &y);
  ASSERT(x == 2 && y == 2);
  size_pop(&reg);
  size_get(&reg, &x, &y);
  ASSERT(x == 1 && y == 1);
  
  
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
