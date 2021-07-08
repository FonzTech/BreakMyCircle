#pragma once

#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Math/Matrix4.h>

using namespace Magnum;

class CubeMapShader : public GL::AbstractShaderProgram
{
public:
	typedef GL::Attribute<0, Vector3> Position;

	explicit CubeMapShader();

	CubeMapShader& setTransformationProjectionMatrix(const Matrix4& matrix);
	CubeMapShader& setTexture(GL::CubeMapTexture& texture);

private:
	enum : Int
	{
		TextureUnit = 0
	};

	Int mTransformationProjectionMatrixUniform;
};