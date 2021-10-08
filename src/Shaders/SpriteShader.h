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
		Float rows, columns;
	};

	typedef GL::Attribute<0, Vector3> Position;
	typedef GL::Attribute<1, Vector2> TextureCoordinates;

	explicit SpriteShader();

	SpriteShader& setTransformationProjectionMatrix(const Matrix4 & matrix);
	SpriteShader& bindTexture(GL::Texture2D& texture);
	SpriteShader& setColor(const Color4& color);
	SpriteShader& setIndex(const Float index);
	SpriteShader& setRows(const Float rows);
	SpriteShader& setColumns(const Float columns);

private:
	enum : Int
	{
		TextureUnit = 0
	};

	Int mTransformationProjectionMatrixUniform;
	Int mProjectionMatrixUniform;

	Float mColorUniform;
	Float mIndexUniform, mTotalUniform;
	Float mTexWidthUniform, mTexHeightUniform;
	Float mRowsUniform, mColumnsUniform;
};
