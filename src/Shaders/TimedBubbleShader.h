#pragma once

#include <Magnum/Resource.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/Color.h>

using namespace Magnum;

class TimedBubbleShader : public GL::AbstractShaderProgram
{
public:
	typedef GL::Attribute<0, Vector3> Position;
	typedef GL::Attribute<1, Vector2> TextureCoordinates;

	explicit TimedBubbleShader();

	TimedBubbleShader& setTransformationProjectionMatrix(const Matrix4 & matrix);
	TimedBubbleShader& setTimedRotation(const Float value);
	TimedBubbleShader& bindColorTexture(GL::Texture2D& texture);
	TimedBubbleShader& bindMaskTexture(GL::Texture2D& texture);

private:
	enum : Int
	{
		ColorTextureUnit = 0,
		MaskTextureUnit = 1
	};

	Int mTransformationProjectionMatrixUniform;
	Int mProjectionMatrixUniform;
	Int mTimedRotationUniform;
};