uniform sampler2D textureData;
uniform vec4 color;
uniform float texWidth;
uniform float texHeight;
uniform float rows;
uniform float columns;
uniform float index;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
	float xs = 1.0 / columns;
	float ys = 1.0 / rows;

	vec2 tc;	
	tc.x = xs * (mod(floor(index), columns) + interpolatedTextureCoordinates.x);
	tc.y = ys * (floor(index / rows) + interpolatedTextureCoordinates.y);
	
	fragmentColor = texture(textureData, tc);
	fragmentColor.rgb *= color.rgb;
}
