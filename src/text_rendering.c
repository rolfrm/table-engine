
//#include <GL/glew.h>
#include <iron/full.h>
#include <iron/gl.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "text.shader.c"
#include "stb_truetype.h"
#include "utf8.h"
#include "graphics_module.h"
static u32 ftex;

static unsigned char temp_bitmap[1024*1024 * 2 * 2];
#define CHAR_DATA_SIZE 300
static stbtt_bakedchar cdata[CHAR_DATA_SIZE];

void render_text(const char * text, size_t len, vec2 window_size, vec2 shared_offset){
  static int initialized = false;
  static int shader = -1;
  static int quadbuffer = -1;
  if(!initialized){
    shader = gl_shader_compile2(src_text_shader_vs, src_text_shader_vs_len, src_text_shader_fs, src_text_shader_fs_len);
    initialized = true;

    float quad[] = {0,0, 1,0, 0,1, 1,1};
    glCreateBuffers(1, &quadbuffer); 
    glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    initialized_fonts();
  }
  glUseProgram(shader);

  glBindTexture(GL_TEXTURE_2D, ftex);
  int color_loc = glGetUniformLocation(shader, "color");
  int offset_loc = glGetUniformLocation(shader, "offset");
  int size_loc = glGetUniformLocation(shader, "size");
  int window_size_loc = glGetUniformLocation(shader, "window_size");
  int uv_offset_loc = glGetUniformLocation(shader, "uv_offset");
  int uv_size_loc = glGetUniformLocation(shader, "uv_size");
  
  glUniform4f(color_loc, 1, 1, 1, 1.0);

  glUniform2f(window_size_loc, window_size.x, window_size.y);
  float x = 0;
  float y = 0;
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  
  for(u64 i = 0; i < len;){
    if(text[i] == 0) break;
    size_t l = 0;
    int codepoint = utf8_to_codepoint(text + i, &l);
    if(codepoint >= 32 && codepoint < (32 + CHAR_DATA_SIZE)){
      
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(cdata, 1024 * 2, 1024 * 2, codepoint-32, &x,&y,&q,1);
    
      vec2 size = vec2_new(q.x1 - q.x0, q.y1 - q.y0);
      vec2 offset = vec2_new(q.x0 + shared_offset.x, shared_offset.y + q.y0 + 15);
      glUniform2f(offset_loc, offset.x, offset.y);
      glUniform2f(size_loc, size.x, size.y);
      glUniform2f(uv_offset_loc, q.s0, q.t0);
      glUniform2f(uv_size_loc, q.s1 - q.s0, q.t1 - q.t0);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    i += MAX((u32)l, (u32)1);
  }
  glDisable(GL_BLEND);
}


vec2 measure_text(const char * text, size_t len){
  float x = 0;
  float y = 0;
  for(u64 i = 0; i < len; i++){
    if(text[i] == 0) break;
    size_t l = 0;
    int codepoint = utf8_to_codepoint(text + i, &l);
    if(codepoint >= 32 && codepoint < (32 + CHAR_DATA_SIZE)){
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(cdata, 1024 * 2, 1024 * 2, codepoint-32, &x,&y,&q,1);
    }
  }
  return vec2_new(x, 15);
}

void initialized_fonts(){
  const char * fontfile = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
  u64 buffersize;
  void * buffer = read_file_to_buffer(fontfile, &buffersize);

  stbtt_BakeFontBitmap(buffer,0, 17.0, temp_bitmap, 2048,2048, 32,CHAR_DATA_SIZE, cdata);
  glGenTextures(1, &ftex);
  glBindTexture(GL_TEXTURE_2D, ftex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024 * 2,1024*2, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
  // can free temp_bitmap at this point
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}
