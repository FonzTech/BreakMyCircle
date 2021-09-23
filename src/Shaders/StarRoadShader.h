#pragma once

#include <Magnum/Resource.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/Color.h>

using namespace Magnum;

class StarRoadShader : public GL::AbstractShaderProgram
{
public:
	typedef GL::Attribute<0, Vector3> Position;
	typedef GL::Attribute<1, Vector2> TextureCoordinates;

	explicit StarRoadShader();

	StarRoadShader& setTransformationMatrix(const Matrix4& transformationMatrix);
	StarRoadShader& setProjectionMatrix(const Matrix4& projectionMatrix);

	StarRoadShader& bindDisplacementTexture(GL::Texture2D& texture);
	StarRoadShader& bindAlphaMapTexture(GL::Texture2D& texture);
	StarRoadShader& setIndex(const Float index);

private:
	enum : Int
	{
		DisplacementTextureUnit = 0,
		AlphaMapTextureUnit = 1
	};

	Int mTransformationMatrixUniform;
	Int mProjectionMatrixUniform;

	Int mIndexUniform;
};
#pragma once
