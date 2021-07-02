uniform sampler2D textureData;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
	fragmentColor = texture(textureData, interpolatedTextureCoordinates);
	fragmentColor.rgb = vec3(1.0, 0.0, 0.0);
	fragmentColor.a = 1.0;
}
