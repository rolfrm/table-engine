#version 100
precision mediump float;
uniform vec4 color;
varying vec2 uv;
uniform sampler2D tex;
void main(){
     vec4 col = color;
     col.a *= texture2D(tex, uv).r;
     gl_FragColor = col;// + vec4(0.2,0.2,0.2,0.2);
}