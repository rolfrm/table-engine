//#include <iron/full.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <iron/types.h>
#include <iron/utils.h>
#include <iron/log.h>
#include <iron/mem.h>
#include <iron/linmath.h>
#include <microio.h>

#include "binui.h"
extern io_writer * binui_stdout;

#ifdef NO_STDLIB
void  memcpy(void *, const void *, unsigned long);
void * realloc(void *, unsigned long);
void memset(void *, int chr, unsigned long );
#endif

io_writer * binui_stdout;

const u8 BINUI_OPCODE_VLEN = 255;


void binui_load(binui_context * ctx, io_reader * rd){

}


// registers
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
  if(value != NULL)
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
    logd("color %p", color);  
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
    logd("translate %i %i", x, y);
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
    logd("size %i %i\n", x, y);
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
    logd("import '%s'", name);
  if(ctx->debug) logd("Load module '%s'\n", name);
}

void rectangle_enter(binui_context * ctx, io_reader * reader){
  var cb = render_callback_get(ctx);
  
  if(cb.callback != NULL){
    cb.callback(cb.userdata);
  }
  if(ctx->debug){
    i32 x, y;
    position_get(ctx, &x, &y);
    int w, h;
    size_get(ctx, &w, &h);
    logd("paint: Rectangle %p (%i %i) (%i %i)\n", get_color(ctx), w, h, x, y);
  }
  
  UNUSED(reader);
  if(ctx->lisp) logd("rect");
}

void canvas_enter(binui_context * ctx, io_reader * reader){
  UNUSED(reader);
  if(ctx->lisp)
    logd("canvas");
}

void binui_set_opcode_handler(int opcode, binui_context * ctx, binui_opcode_handler handler){
  if(ctx->opcode_handler_count <= opcode){
    ctx->opcode_handlers = realloc(ctx->opcode_handlers, sizeof(handler) * (opcode + 1));
    
    for(int i = ctx->opcode_handler_count; i <= opcode; i++){
      ctx->opcode_handlers[i] = (binui_opcode_handler){0};
    }
    ctx->opcode_handler_count = opcode + 1;
  }
  ctx->opcode_handlers[opcode] = handler;
}

binui_stack_register render_callback_reg = {.size = sizeof(render_callback), .stack = {0}};

void render_callback_push(binui_context * ctx, render_callback callback){
  binui_stack_register_push(ctx, &render_callback_reg, &callback);
}

void render_callback_pop(binui_context * ctx){
  binui_stack_register_pop(ctx, &render_callback_reg, NULL);
}

render_callback render_callback_get(binui_context * ctx){
  render_callback callback = {0};
  binui_stack_register_top(ctx, &render_callback_reg, &callback);
  return callback;
}

void binui_iterate_internal(binui_context * reg, io_reader * reader){
  ASSERT(reg->inited == true);
  bool debug = reg->debug;
  bool lisp = reg->lisp;
  
  int stack_level = 0;
  u32 childs[BINUI_MAX_STACK_SIZE];
  void (* callbacks[BINUI_MAX_STACK_SIZE])(binui_context * userdata);
  
  while(true){
    u64 opcode = io_read_u64_leb(reader);
    reg->current_opcode = opcode;
    u32 children = 0;
    if(debug)
      logd("opcode: %i\n", opcode);
    callbacks[stack_level] = NULL;
    
    if(opcode == BINUI_OPCODE_NONE)
      break;
    if(lisp){
    for(int i = 0; i < stack_level; i++)
      logd(" ");
    logd("(");
    }
    if(opcode == BINUI_IMPORT_MODULE){
      module_enter(reg, reader);
    }

    if(reg->opcode_handler_count > opcode){
      var handler = reg->opcode_handlers[opcode];
      if(handler.enter != NULL){
	handler.enter(reg, reader);
	if(handler.has_children)
	  children = io_read_u64_leb(reader);
	callbacks[stack_level] = handler.exit;
      }
    }

    //if(children == 0){
      var cb = render_callback_get(reg);
      if(cb.callback != NULL){
	cb.callback(cb.userdata);
      }
      //}
    
    opcode = io_read_u64_leb(reader);
    if(opcode != BINUI_MAGIC){
      logd("ERROR Expected magic! %i\n", opcode);
      break;
    }
    
    // if children >
    while(true){
      if(debug)
	logd("Children: %i stack level: %i\n", children, stack_level);
      if(children >= 1){
	childs[stack_level] = children;
	stack_level += 1;
	break;
      }else if(stack_level > 0){
	var pop = callbacks[stack_level];
	if(pop != NULL)
	  pop(reg);
	 
	stack_level -= 1;
	children = childs[stack_level] -= 1;
	if(lisp)
	  logd(")");
      }else{
	if(lisp)
	  logd(")");
	break;
      }
    }
    if(lisp)
      logd("\n");
  }
}

void binui_3d_init(binui_context * ctx);
void binui_init(binui_context * ctx){
  memset(ctx, 0, sizeof(ctx[0]));
  ctx->inited = true;

  binui_opcode_handler h;
  h.enter = color_enter;
  h.exit = color_exit;
  h.has_children = true;
  binui_set_opcode_handler(BINUI_COLOR, ctx, h);

  h.enter = size_enter;
  h.exit = size_exit;
  binui_set_opcode_handler(BINUI_SIZE, ctx, h);

  h.enter = position_enter;
  h.exit = position_exit;
  binui_set_opcode_handler(BINUI_POSITION, ctx, h);

  h.enter = canvas_enter;
  h.exit = NULL;
  binui_set_opcode_handler(BINUI_CANVAS, ctx, h);

  h.enter = rectangle_enter;
  h.has_children = false;
  binui_set_opcode_handler(BINUI_RECTANGLE, ctx, h);
  binui_3d_init(ctx);  
  
}

void handle_opcode(binui_context * registers, void * userdata){
  UNUSED(registers);
  UNUSED(userdata);
}

void binui_iterate(binui_context * reg, io_reader * reader){
  binui_iterate_internal(reg, reader);
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
  io_write_u32_leb(wd, 2);
  io_write_u32_leb(wd, BINUI_MAGIC);
  
  io_write_u32_leb(wd, BINUI_RECTANGLE);
  io_write_u32_leb(wd, BINUI_MAGIC);

  io_write_u32_leb(wd, BINUI_3D);
  
  io_write_u32_leb(wd, 1);
  io_write_u32_leb(wd, BINUI_MAGIC);

  io_write_u32_leb(wd, BINUI_3D_TRANSFORM);

  mat4 t = mat4_translate(1,1,1);
  io_write_mat4(wd, t);
  io_write_u32_leb(wd, 1);
  io_write_u32_leb(wd, BINUI_MAGIC);
  
  io_write_u32_leb(wd, BINUI_3D_POLYGON);
  io_write_u32_leb(wd, 4); // vertexes
  io_write_u32_leb(wd, 2); // vertex dimensions
  float pts[8] = {0,0,1,0,1,1,0,1};
  io_write(wd, pts, 8 * sizeof(pts[0]));
  
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
  logd("Color: %u\n", get_color(&reg));
  logd("Color: %u\n", pop_color(&reg));
  logd("Color: %u\n", pop_color(&reg));
  logd("Color: %u\n", pop_color(&reg));
  int x, y;
  position_push(&reg, 1, 1);
  position_get(&reg, &x, &y);
  logd(" %i %i\n", x, y);
  position_push(&reg, 2, 2);
  position_push(&reg, 3, 3);

  position_get(&reg, &x, &y);
  logd(" %i %i\n", x, y);
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
  binui_iterate(&reg, wd);
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
