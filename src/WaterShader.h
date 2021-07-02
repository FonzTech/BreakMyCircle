#pragma once

#include <Magnum/Resource.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/Color.h>

using namespace Magnum;

class WaterShader : public GL::AbstractShaderProgram
{
public:
	struct Parameters
	{
		Float speed;
	};

	typedef GL::Attribute<0, Vector3> Position;
	typedef GL::Attribute<1, Vector2> TextureCoordinates;

	explicit WaterShader();

	WaterShader& setTransformationMatrix(const Matrix4& transformationMatrix);
	WaterShader& setProjectionMatrix(const Matrix4& projectionMatrix);
	WaterShader& bindTexture(GL::Texture2D& texture);

private:
	enum : Int
	{
		TextureUnit = 0
	};

	Int mTransformationMatrixUniform;
	Int mProjectionMatrixUniform;
};
