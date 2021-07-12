uniform vec3 color = vec3(1.0, 1.0, 1.0);

uniform sampler2D textureGolPrespFirst;
uniform sampler2D textureGolPrespSecond;
uniform sampler2D textureGolOrthoFirst;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
	// P First
    fragmentColor.rgb = texture(textureGolPrespFirst, interpolatedTextureCoordinates).rgb;
    fragmentColor.a = 1.0;
	
	// P Second
	{
		vec4 c = texture(textureGolPrespSecond, interpolatedTextureCoordinates);
		fragmentColor.rgb = mix(fragmentColor.rgb, c.rgb, c.a);
	}
	
	// O First
	{
		vec4 c = texture(textureGolOrthoFirst, interpolatedTextureCoordinates);
		fragmentColor.rgb = mix(fragmentColor.rgb, c.rgb, c.a);
	}
}