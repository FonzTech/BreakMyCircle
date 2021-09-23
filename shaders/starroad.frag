#define PI 3.1415926538

precision mediump float;

uniform sampler2D displacementData;
uniform sampler2D alphaMapData;
uniform float index;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
  vec2 coords = interpolatedTextureCoordinates;
  coords.y += index;
  
  vec4 colorDisp = texture(displacementData, coords);
  vec4 colorAlpha = texture(alphaMapData, interpolatedTextureCoordinates);
  
  float alpha;
  alpha = mix(colorDisp.r, 1.0, colorAlpha.r);
  alpha = mix(alpha, 0.0, colorAlpha.g);
  
	fragmentColor = vec4(vec3(1.0), alpha);
}
