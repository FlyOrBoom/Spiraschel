attribute vec4 vertex;
uniform float offset;
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
float spread(float x, float a){
   return exp(-x*x*a*a);
}
vec3 fade(vec3 c, float a){
   return mix(vec3(1), c, a);
}
void main() {

  float time = vertex.x;
  float index = vertex.y;
  float start = vertex.z;
  float end = vertex.w;

  float angle = time * 0.000872664625997;
  float elapsed = time - start;
  float total = end - start;
  float progress = elapsed/total;
  float proximity = offset - time;
  float edge = log(max(proximity, 0.00001));

  float slope = 0.000025;
  float thickness = mix(0.5, 1.0, spread(proximity,0.0005));
  thickness = 0.2 * mix(pow(thickness, 2.0), thickness, progress);
  float taper = 0.0015 / thickness;
  thickness *= left_cap(taper*elapsed, 1.2);
  thickness *= right_cap(taper*elapsed, taper*total - 1.2);
  float flipflop = mod(time, 2.0) - 0.5;
  
  float radius = 0.5 * exp( slope*(offset-time) + thickness*flipflop );
  gl_Position = vec4( radius*sin(angle), radius*cos(angle), 0, 1 );

  float early_index = floor(index * 0.5);
  vec3 color_start = cos( early_index + vec3(0,1,2) );
  vec3 color_end = cos( early_index + vec3(0.5,1.5,2.5) );
  color = mix(color_start, color_end, progress);
  color *= color;
  color = mix(fade(color, 0.3), color, mod(index, 2.0));
  color = mix(fade(color, 0.6), color, spread(edge*edge, 0.01));
  color = mix(fade(color, 0.0), color, spread(proximity, 0.00006));
}