#pragma once

#include <Magnum/Resource.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/Color.h>

using namespace Magnum;

class PlasmaShader : public GL::AbstractShaderProgram
{
public:
	typedef GL::Attribute<0, Vector3> Position;
	typedef GL::Attribute<1, Vector2> TextureCoordinates;

	explicit PlasmaShader();

	PlasmaShader& setTransformationProjectionMatrix(const Matrix4 & matrix);
	PlasmaShader& setSize(const Vector2 & size);
	PlasmaShader& setTime(const Float time);

private:
	Int mTransformationProjectionMatrixUniform;
	Int mProjectionMatrixUniform;
	Int mSizeUniform, mTimeUniform;
};
