#pragma once

#include <Magnum/Resource.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/Color.h>

using namespace Magnum;

class SpriteShader : public GL::AbstractShaderProgram
{
public:
	struct Parameters
	{
		Resource<GL::Texture2D> texture;
		Float index, total;
		Float texWidth, texHeight;
		Float rows, columns;
	};

	typedef GL::Attribute<0, Vector3> Position;
	typedef GL::Attribute<1, Vector2> TextureCoordinates;

	explicit SpriteShader();

	SpriteShader& setTransformationMatrix(const Matrix4& transformationMatrix);
	SpriteShader& setProjectionMatrix(const Matrix4& projectionMatrix);
	SpriteShader& bindTexture(GL::Texture2D& texture);

	SpriteShader& setColor(const Color4& color);
	SpriteShader& setIndex(const Float index);
	SpriteShader& setTextureWidth(const Float width);
	SpriteShader& setTextureHeight(const Float height);
	SpriteShader& setRows(const Float rows);
	SpriteShader& setColumns(const Float columns);

private:
	enum : Int
	{
		TextureUnit = 0
	};

	Int mTransformationMatrixUniform;
	Int mProjectionMatrixUniform;

	Int mColorUniform;
	Int mIndexUniform, mTotalUniform;
	Int mTexWidthUniform, mTexHeightUniform;
	Int mRowsUniform, mColumnsUniform;
};
