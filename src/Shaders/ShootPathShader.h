#pragma once

#include <Magnum/Resource.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/Color.h>

using namespace Magnum;

class ShootPathShader : public GL::AbstractShaderProgram
{
public:
	typedef GL::Attribute<0, Vector3> Position;
	typedef GL::Attribute<1, Vector2> TextureCoordinates;

	explicit ShootPathShader();

	ShootPathShader& setTransformationProjectionMatrix(const Matrix4 & matrix);
	ShootPathShader& bindTexture(GL::Texture2D& texture);
	ShootPathShader& setIndex(const Float index);
	ShootPathShader& setSize(const Float size);

private:
	enum : Int
	{
		TextureUnit = 0
	};

	Int mTransformationProjectionMatrixUniform;
	Int mProjectionMatrixUniform;

	Float mIndexUniform;
	Float mSizeUniform;
};