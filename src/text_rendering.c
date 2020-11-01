#include <iron/full.h>
#include <iron/gl.h>
#include "stb_truetype.h"
#include "graphics_module.h"

void render_text(const char * text, size_t len, vec2 window_size, vec2 shared_offset){
  initialized_fonts();
  blit_push();
  blit_translate(shared_offset.x, shared_offset.y);

  char substr[len + 1];
  memcpy(substr, text, len);
  substr[len] = 0;
  blit_text(substr);
  blit_pop();
}

vec2 measure_text(const char * text, size_t len){
  
  initialized_fonts();
  char substr[len + 1];
  memcpy(substr, text, len);
  substr[len] = 0;
  return blit_measure_text(substr);
}

void initialized_fonts(){
  static bool font_initialized = false;
  if(font_initialized == false){
    font_initialized = true;
    const char * fontfile = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    font * fnt = blit_load_font_file(fontfile, 17.0);
    blit_set_current_font(fnt);
  }
}
