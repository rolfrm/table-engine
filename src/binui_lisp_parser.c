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


typedef struct{
  io_reader * rd;
  size_t offset;
  int error;
}string_reader;

void fix_reader(string_reader rd){
  ASSERT(rd.error == 0);
  var rd2 = rd.rd;
  size_t loc = io_offset(rd2);
  if(loc > rd.offset){
    io_rewind(rd2, loc -rd.offset);
  }else if(loc < rd.offset){
    io_advance(rd2, rd.offset - loc);
  }
  ASSERT(rd.offset == io_offset(rd2));
}

string_reader read_until(string_reader rd, io_writer * writer, bool (* f)(char c)){
  var rd2 = rd.rd;
  fix_reader(rd);
  
  while(true){
    var ch = io_peek_u8(rd2);
    if(f(ch)) break;
    if(writer != NULL)
      io_write_u8(writer, ch);
    io_advance(rd.rd, 1);
    rd.offset += 1;
  }
  return rd;
}

static __thread bool (* current_f)(char c);
bool not_current_f(char c){
  return !current_f(c);
}

string_reader read_while(string_reader rd, io_writer * writer, bool (* f)(char c)){
  current_f = f;
  return read_until(rd, writer, not_current_f);
}

static __thread char selected_char;
bool check_selected_char(char c){
  return selected_char == c;
}

string_reader read_untilc(string_reader rd, io_writer * writer, char c){
  selected_char = c;
  return read_until(rd, writer, check_selected_char);
}

string_reader skip_until(string_reader rd, bool (*f)(char c)){
  return read_until(rd, NULL, f);
}

string_reader skip_untilc(string_reader rd, char c){
  return read_untilc(rd, NULL, c);
}

string_reader skip_while(string_reader rd, bool (*f)(char c)){
  return read_while(rd, NULL, f);
}

bool is_digit(char c){
  return c >= '0' && c <= '9';
}

bool is_alpha(char c){
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_alphanum(char c){
  return is_digit(c) || is_alpha(c);
}

bool is_whitespace(char c){
  return c == ' ' || c == '\t' || c == '\n';
}

bool is_hex(char c){
  return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool is_endexpr(char c){
  return c == ')' || c == '(' || is_whitespace(c) || c == 0 || c ==';';
}

u8 next_byte(string_reader rd){
  fix_reader(rd);
  
  var rd2 = rd.rd;
  return io_peek_u8(rd2);  
}

io_reader io_from_bytes(const void * bytes, size_t size){
  io_reader rd = {.data = (void *) bytes, .offset = 0, .size = size}; 
  return rd;
}

string_reader read_hex(string_reader rd, io_writer * buffer, u64 * out){
  u64 r = 0;
  io_reset(buffer);
  rd = read_until(rd, buffer, is_endexpr);
  char * str = buffer->data;
  for(size_t i = 0; i < buffer->offset; i++){
    var c = str[i];
    if(!is_hex(str[i])){
      rd.error = 1;
      break;
    }
    u64 v = 0;
    if(c >= '0' && c <= '9'){
      v = c - '0';
    }else if(c >= 'a' && c <= 'f'){
      v = c - 'a' + 10;
    }else if(c >= 'A' && c <= 'F'){
      v = c - 'A' + 10;    
    }else{
      ERROR("This is impossible!");
    }
    r = (r << 4) | v;
  }
  *out = r;
  return rd;
}


string_reader read_integer(string_reader rd, io_writer * buffer, i64 * out){
  i64 r = 0;
  bool negative = false;
  io_reset(buffer);
  rd = skip_while(rd, is_whitespace);
  rd = read_until(rd, buffer, is_endexpr);
  char * str = buffer->data;
  for(size_t i = 0; i < buffer->offset; i++){
    var c = str[i];
    if(c == '-'){
      negative = true;
      continue;
    }
    if(!is_digit(str[i])){
      rd.error = 1;
      break;
    }
    u64 v = 0;
    if(c >= '0' && c <= '9'){
      v = c - '0';
    }else{
      ERROR("This is impossible!");
    }
    r = (r * 10) + v;
  }
  *out = negative ? -r : r;
  return rd;
}

string_reader parse_sub(string_reader rd, io_writer * write){
  rd = skip_untilc(rd, '(');
  rd.offset += 1;
  rd = skip_while(rd, is_whitespace);
  
  io_writer name_buffer = {0};
  var rd4 = read_until(rd, &name_buffer, is_endexpr);
  ASSERT(!rd4.error);
  io_write_u8(&name_buffer, 0);
 
  binui_opcode opcode = binui_opcode_parse(name_buffer.data);
  io_write_u8(write, opcode);
  io_reset(&name_buffer);
  string_reader rd_after;
  rd4 = skip_while(rd4, is_whitespace);
  bool supports_child = true;
  if(opcode == BINUI_COLOR){
  
    char next = next_byte(rd4);
    
    ASSERT(next == '#');
    rd4.offset += 1;
    u64 hex_value = 0;
    var rd5 = read_hex(rd4, &name_buffer, &hex_value);
    
    io_write_u32(write, hex_value);
    rd5 = skip_while(rd5, is_whitespace);
    rd_after = rd5;
  }else if(opcode == BINUI_POSITION || opcode == BINUI_SIZE){
    rd4 = skip_while(rd4, is_whitespace);
    for(int i = 0; i < 2; i++){
      i64 x = 0;
      rd4 = read_integer(rd4, &name_buffer, &x);
      io_write_i64_leb(write, x);
    }
    rd_after = rd4;
  }else if(opcode == BINUI_CANVAS || opcode == BINUI_RECTANGLE){
    rd_after = rd4;
  }

  supports_child = opcode != BINUI_RECTANGLE;

  io_reset(&name_buffer);
  u32 child_count = 0;
  while(true){
    var rd2 = skip_while(rd_after, is_whitespace);
    
    char next = next_byte(rd2);
    if(next == '('){
      rd_after = parse_sub(rd2, &name_buffer);
      child_count += 1;
      continue;
    }
    else if(next == ')'){
      rd2.offset += 1;
      rd_after = rd2;
      break;
    }
    else{
      logd("Unexpected token '%c'", next);
      rd2.error = 1;
      rd_after = rd2;
      break;
    }
  }
  if(supports_child){
    
    io_write_u32_leb(write, child_count);
  }
  io_write_u32_leb(write, BINUI_MAGIC);
  
  io_write(write, name_buffer.data, name_buffer.offset);
  io_writer_clear(&name_buffer);
  return rd_after;
}

void binui_load_lisp(io_reader * rd, io_writer * write){
  string_reader r = {.rd = rd, .offset = io_offset(rd)};
  r = parse_sub(r, write);
  if(r.error){
    ERROR("ERROR!\n");
  }
  io_write_i8(write, BINUI_OPCODE_NONE);
}


void test_binui_string_reader(){
  logd("TEST Binui String Reader\n");
  const char * target = "   \n (color #112233fFff -123)";

  io_writer writer = {0};
  io_reader rd = io_from_bytes(target, strlen(target) + 1);
  string_reader rd2 = {.rd = &rd, .offset = 0};
  var rd3 = skip_untilc(rd2, '(');
  rd3.offset += 1;
  
  var next = next_byte(rd3);
  ASSERT(next == 'c');

  next = next_byte(rd2);
  ASSERT(next == ' ');

  next = next_byte(rd3);
  ASSERT(next == 'c');
  
  io_writer symbol_writer = {0};
  var rd4 = read_until(rd3, &symbol_writer, is_endexpr);
  ASSERT(symbol_writer.offset == strlen("color"));
  ASSERT(strncmp(symbol_writer.data, "color", symbol_writer.offset) == 0);
   
  var rd5 = skip_while(rd4, is_whitespace); 
  var next2 = next_byte(rd5);
  
  ASSERT(next2 == '#');
  rd5.offset += 1;
  io_reset(&symbol_writer);
  u64 hexv = 0;
  var rd6 = read_hex(rd5, &symbol_writer, &hexv);
  ASSERT(rd6.error == 0);
  ASSERT(hexv == 0x112233ffff);

  logd("Hex: %p\n", hexv);
  
  var next3 = next_byte(rd6);
  ASSERT(next3 == ' ');
  var rd7 = skip_while(rd6, is_whitespace);
  i64 i = 0;
  var rd8 = read_integer(rd7, &symbol_writer, &i);
  ASSERT(i == -123);
  rd8 = skip_while(rd8, is_whitespace);
  var next5 = next_byte(rd8);
  logd("next5: '%c'\n", next5);
  ASSERT(next5 == ')');
  var next4 = next_byte(rd3);
  ASSERT(next4 == 'c');

  io_writer_clear(&symbol_writer);
  logd("OK\n");
}
void test_write_lisp(void * buffer, size_t size);
void test_binui_lisp_loader(){
  logd("TEST Binui Lisp Loader\n");
  {
    const char * target = "   \n (color #112233fF)";
    io_writer writer = {0};
    io_reader rd = io_from_bytes(target, strlen(target) + 1);
    binui_load_lisp(&rd, &writer);
    char * buffer = writer.data;
    for(size_t i = 0; i < writer.offset; i++){
      logd("%x ", buffer[i]);
    }
    logd("\nDone loading lisp (%i bytes)\n", writer.offset);
  }
  {
    const char * target = "   \n (color #44332211 (color #55443322 (position 1 2 (size 10 10 (rectangle)) (size 20 20 (position 10 5 (rectangle))))))";
    io_writer writer = {0};
    io_reader rd = io_from_bytes(target, strlen(target) + 1);
    binui_load_lisp(&rd, &writer);
    char * buffer = writer.data;
    for(size_t i = 0; i < writer.offset; i++){
      logd("%i ", buffer[i]);
    }
    logd("\nDone loading lisp (%i bytes)\n", writer.offset);
    test_write_lisp(writer.data, writer.offset);
    logd("Rewriting lisp\n");
  }

}

void test_binui_load_lisp(){
  test_binui_string_reader();
  test_binui_lisp_loader();
}
