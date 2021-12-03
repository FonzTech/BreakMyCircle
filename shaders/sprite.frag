#ifdef GL_ES
  precision mediump float;
#endif

uniform sampler2D textureData;
uniform vec4 color;
uniform float texWidth;
uniform float texHeight;
uniform float rows;
uniform float columns;
uniform float rowspan;
uniform float columnspan;
uniform float index;
uniform float alphaMask;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
	float xs = 1.0 / columns;
	float ys = 1.0 / rows;

	vec2 tc;
	tc.x = fract(xs * mod(floor(index), columns) + xs * interpolatedTextureCoordinates.x);
	tc.y = fract(ys * floor(index / rows) + ys * interpolatedTextureCoordinates.y);
	
	fragmentColor = texture(textureData, tc);
	fragmentColor.rgb *= color.rgb;
  
  if (fragmentColor.a < alphaMask)
  {
    discard;
  }
}