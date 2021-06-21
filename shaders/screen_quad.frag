uniform vec3 color = vec3(1.0, 1.0, 1.0);

uniform sampler2D textureMain;
uniform sampler2D textureLevel;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
	// Main
    fragmentColor.rgb = texture(textureMain, interpolatedTextureCoordinates).rgb;
    fragmentColor.a = 1.0;
	
	// Level
	vec4 level = texture(textureLevel, interpolatedTextureCoordinates);
    fragmentColor.rgb = mix(fragmentColor.rgb, level.rgb, level.a);
}