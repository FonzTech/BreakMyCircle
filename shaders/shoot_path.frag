#ifdef GL_ES
  precision mediump float;
#endif

uniform sampler2D textureData;
uniform float index;
uniform float size;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
  float x = abs(interpolatedTextureCoordinates.x - index * (1.0 / size)) * size * 2.0;
  vec2 dc;
  dc.x = fract(x);
  dc.y = interpolatedTextureCoordinates.y;
  fragmentColor = texture(textureData, dc);
}
