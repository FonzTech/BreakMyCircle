#define PI 3.1415926538
#define PI2 (PI * 2.0)

#ifdef GL_ES
  precision mediump float;
#endif

uniform vec2 size;
uniform float time;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
  // Get UV coordinates repeated NxM times
  vec2 r = interpolatedTextureCoordinates * size;

  // Horizontal lines
  float pt = time * PI2;
  float v1 = sin(r.x + pt);
  
  // Vertical lines
  float v2 = sin(r.y + pt);
  
  // Diagonal lines
  float v3 = sin(r.x + r.y + pt);
  
  // Centered circles for N times
  float v4 = sin(sqrt(r.x * r.x + r.y * r.y) + pt);
  
  // Mix all above factors
  float v = v1 + v2 + v3 + v4;

  // Mix them together to obtain a "repeated coloured" fragment
  vec3 ret = vec3(sin(v), cos(v), sin(v + PI2) - 0.5);
  
  // Shift to the top half to avoid dark colours
  ret = 0.35 + 0.65 * ret;
  
  // Assign fragment color
	fragmentColor = vec4(ret, 1.0);
}
