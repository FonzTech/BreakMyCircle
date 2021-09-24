#pragma once

#include <Magnum/Resource.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/Color.h>

using namespace Magnum;

class SunShader : public GL::AbstractShaderProgram
{
public:
	typedef GL::Attribute<0, Vector3> Position;
	typedef GL::Attribute<1, Vector2> TextureCoordinates;

	explicit SunShader();

	SunShader& setTransformationProjectionMatrix(const Matrix4 & matrix);
	SunShader& bindDisplacementTexture(GL::Texture2D& texture);
	SunShader& bindColorTexture(GL::Texture2D& texture);
	SunShader& setIndex(const Float index);

private:
	enum : Int
	{
		DisplacementTextureUnit = 0,
		ColorTextureUnit = 1
	};

	Int mTransformationProjectionMatrixUniform;
	Int mProjectionMatrixUniform;

	Int mIndexUniform;
};
#pragma once
