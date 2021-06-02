uniform sampler2D textureData;
uniform vec3 color;
uniform float texWidth;
uniform float texHeight;
uniform float rows;
uniform float columns;
uniform float index;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
	float xp = texWidth / columns;
	float yp = texHeight / rows;

	float xs = xp / texWidth;
	float ys = yp / texHeight;

	vec2 tc;	
	tc.x = xs * (mod(floor(index), columns) + interpolatedTextureCoordinates.x);
	tc.y = ys * (floor(index / rows) + interpolatedTextureCoordinates.y);
	
	fragmentColor = texture(textureData, tc);
	fragmentColor.rgb *= color;
}
