layout(location = 0) in vec3 position;
layout(location = 1) in vec2 textureCoordinates;

out vec2 interpolatedTextureCoordinates;

uniform mat4 transformationProjectionMatrix;
uniform mat4 projectionMatrix;

void main()
{
    interpolatedTextureCoordinates = textureCoordinates;

    gl_Position = transformationProjectionMatrix * vec4(position, 1.0);
}
