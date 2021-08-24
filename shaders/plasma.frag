#define PI 3.1415926538

uniform vec2 size;
uniform float time;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
  // Get UV coordinates repeated NxM times
  vec2 r = interpolatedTextureCoordinates * size;

  // Horizontal lines
  float v1 = sin(r.x + time);
  
  // Vertical lines
  float v2 = sin(r.y + time);
  
  // Diagonal lines
  float v3 = sin(r.x + r.y + time);
  
  // Centered circles for N times
  float v4 = sin(sqrt(r.x * r.x + r.y * r.y) + 5.0 * time);
  
  // Mix all above factors
  float v = v1 + v2 + v3 + v4;

  // Mix them together to obtain a "repeated coloured" fragment
  vec3 ret = vec3(sin(v), sin(v + (0.5 + time) * PI), sin(v + 1.0 * PI) - 0.5);
  
  // Shift to the top half to avoid dark colours
  ret = 0.5 + 0.5 * ret;
  
  // Assign fragment color
	fragmentColor = vec4(ret, 1.0);
}
