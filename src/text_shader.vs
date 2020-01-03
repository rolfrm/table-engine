#version 100
precision mediump float;

uniform vec2 offset;
uniform vec2 size;
uniform vec2 window_size;
uniform vec2 uv_offset;
uniform vec2 uv_size;
attribute vec2 point;
varying vec2 uv;
void main(){
  
  uv = point * uv_size + uv_offset;
  vec2 vertpos = (offset + point * size) / window_size ;

  gl_Position = vec4((vertpos - vec2(0.5,0.5)) * vec2(2, -2), 0, 1);
}
