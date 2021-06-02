uniform vec3 color = vec3(1.0, 1.0, 1.0);
uniform sampler2D textureData;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
	vec4 t = texture(textureData, interpolatedTextureCoordinates);:
    fragmentColor.rgb = color * t.rgb;
    fragmentColor.a = t.a;
}
