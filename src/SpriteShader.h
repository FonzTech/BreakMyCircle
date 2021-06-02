#pragma once

#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/Color.h>

using namespace Magnum;

class SpriteShader : public GL::AbstractShaderProgram
{
public:
	typedef GL::Attribute<0, Vector3> Position;
	typedef GL::Attribute<1, Vector2> TextureCoordinates;

	explicit SpriteShader(const Float & width, const Float & height, const Float & rows, const Float & columns, const Float & total, const Float & speed, const Color3 & color);

	SpriteShader& update(const Float & deltaTime);
	SpriteShader& setTransformationMatrix(const Matrix4& transformationMatrix);
	SpriteShader& setProjectionMatrix(const Matrix4& projectionMatrix);
	SpriteShader& bindTexture(GL::Texture2D& texture);

	Float mIndex;
	Float mTotal;

private:
	enum : Int
	{
		TextureUnit = 0
	};

	Float mSpeed;
	Color3 mColor;

	Int mColorUniform;
	Int mTransformationMatrixUniform;
	Int mProjectionMatrixUniform;
	Int mIndexUniform;
};
