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
		Resource<GL::Texture2D> displacementTexture;
		Resource<GL::Texture2D> waterTexture;
		Resource<GL::Texture2D> effectsTexture;
		Float frame;
		Float speed;
		Vector2 size;
		Color3 horizonColor;
	};

	typedef GL::Attribute<0, Vector3> Position;
	typedef GL::Attribute<1, Vector2> TextureCoordinates;

	explicit WaterShader();

	WaterShader& setTransformationProjectionMatrix(const Matrix4 & matrix);
	WaterShader& setFrame(const Float frame);
	WaterShader& setSpeed(const Float speed);
	WaterShader& setSize(const Vector2 & size);
	WaterShader& setHorizonColorUniform(const Color3& color);
	WaterShader& bindDisplacementTexture(GL::Texture2D& texture);
	WaterShader& bindWaterTexture(GL::Texture2D& texture);
	WaterShader& bindEffectsTexture(GL::Texture2D& texture);

private:
	enum : Int
	{
		DisplacementTextureUnit = 0,
		WaterTextureUnit = 1,
		EffectsTextureUnit = 2
	};

	Int mTransformationProjectionMatrixUniform;
	Int mFrameUniform, mSpeedUniform, mSizeUniform;
	Int mHorizonColorUniform;
};
