layout(location = 0) in vec4 position;
layout(location = 1) in vec2 textureCoordinates;

out vec2 interpolatedTextureCoordinates;

uniform mat4 transformationMatrix;
uniform mat4 projectionMatrix;

void main()
{
    interpolatedTextureCoordinates = textureCoordinates;

    gl_Position = projectionMatrix * transformationMatrix * position;
}
