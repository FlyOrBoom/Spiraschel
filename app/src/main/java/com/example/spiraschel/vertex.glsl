attribute vec4 vertex;
attribute float offset;
varying vec3 color;
float cap(float x, float a){
   return sqrt(1.0 - (a-x)*(a-x));
}
float left_cap(float x, float a){
   return cap(min(x,a), a);
}
float right_cap(float x, float a){
   return cap(max(x,a), a);
}
void main() {

  float time = vertex.x;
  float index = vertex.y;
  float start = vertex.z;
  float end = vertex.w;

  float angle = time * 0.0017453292519943;
  float elapsed = time - start;
  float total = end - start;
  float progress = elapsed/total;

  float slope = 0.00002;
  float taper = 0.05;
  float thickness = 0.06;
  thickness *= mix(0.5, 1.0, progress);
  thickness *= left_cap(elapsed*taper, 1.2);
  thickness *= right_cap(elapsed*taper, total*taper - 1.2);
  float flipflop = mod(time, 2.0) - 0.5;
  
  float radius = 0.8 * exp( slope*(offset-time) + thickness*flipflop );
  gl_Position = vec4( radius*sin(angle), radius*cos(angle), 0, 1 );

  float early_index = floor(index * 0.5);
  vec3 color_start = cos( early_index + vec3(0,1,2) );
  vec3 color_end = cos( early_index + vec3(0.5,1.5,2.5) );
  color = mix(color_start, color_end, progress);
  color *= color;
  color = mix(vec3(1), color, mod(index, 2.0));
}
