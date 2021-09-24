#define PI 3.1415926538

precision mediump float;

uniform sampler2D displacementData;
uniform sampler2D colorData;
uniform float index;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
  vec2 dc = interpolatedTextureCoordinates;
 
  float xd = texture(displacementData, interpolatedTextureCoordinates).r; 
  dc.x += xd + index * 0.23;
	dc.y += xd + index * 0.12;

	fragmentColor = texture(colorData, dc);
}
