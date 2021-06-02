#pragma once

#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/Color.h>

using namespace Magnum;

class SpriteShader : public GL::AbstractShaderProgram
{
public:
	typedef GL::Attribute<0, Vector2> Position;
	typedef GL::Attribute<1, Vector2> TextureCoordinates;

	explicit SpriteShader();

	SpriteShader& setColor(const Color3& color);
	SpriteShader& setTransformationMatrix(const Matrix4& transformationMatrix);
	SpriteShader& setProjectionMatrix(const Matrix4& projectionMatrix);
	SpriteShader& bindTexture(GL::Texture2D& texture);

private:
	enum : Int
	{
		TextureUnit = 0
	};

	Int mColorUniform;
	Int mTransformationMatrix;
	Int mProjectionMatrix;
};
