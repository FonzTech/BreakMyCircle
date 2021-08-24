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

	PlasmaShader& setTransformationMatrix(const Matrix4& transformationMatrix);
	PlasmaShader& setProjectionMatrix(const Matrix4& projectionMatrix);
	PlasmaShader& setSize(const Vector2 & size);
	PlasmaShader& setTime(const Float time);

private:
	Int mTransformationMatrixUniform;
	Int mProjectionMatrixUniform;
	Int mSizeUniform, mTimeUniform;
};
