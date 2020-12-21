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
}string_reader;

void fix_reader(string_reader rd){
  var rd2 = rd.rd;
  size_t loc = io_getloc(rd2);
  if(loc > rd.offset){
    io_rewind(rd2, loc -rd.offset);
  }else if(loc < rd.offset){
    io_advance(rd2, rd.offset - loc);
  }
  ASSERT(rd.offset == io_getloc(rd2));
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

bool is_endexpr(char c){
  return c == ')' || c == '(' || is_whitespace(c) || c == 0 || c ==';';
}

u8 next_byte(string_reader rd){
  fix_reader(rd);
  
  var rd2 = rd.rd;
  return io_peek_u8(rd2);  
}

/*void binui_load_lisp(io_reader * rd, io_writer * write){
  string_reader r = {.rd = rd, .offset = io_getloc(rd)};
  r = string_reader_readuntil(r, "(");
  }*/

io_reader io_from_bytes(const void * bytes, size_t size){
  io_reader rd = {.data = (void *) bytes, .offset = 0, .size = size}; 
  return rd;
}

void test_binui_load_lisp(){
  logd("TEST BINUI LOAD LISP\n");
  const char * target = "   \n (color #0x11223344)";

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
  logd("OK\n");
}
