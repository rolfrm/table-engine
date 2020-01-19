#include <iron/full.h>

#include "string_intern.h"

u64 canvas_class;

void canvas_init(){
  canvas_class = intern_string("canvas class");
}
