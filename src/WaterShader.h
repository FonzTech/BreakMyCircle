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
		Float size;
		Color3 horizonColor;
	};

	typedef GL::Attribute<0, Vector3> Position;
	typedef GL::Attribute<1, Vector2> TextureCoordinates;

	explicit WaterShader();

	WaterShader& setTransformationMatrix(const Matrix4& transformationMatrix);
	WaterShader& setProjectionMatrix(const Matrix4& projectionMatrix);
	WaterShader& setFrame(const Float frame);
	WaterShader& setSpeed(const Float speed);
	WaterShader& setSize(const Float size);
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

	Int mTransformationMatrixUniform;
	Int mProjectionMatrixUniform;
	Int mFrameUniform, mSpeedUniform, mSizeUniform;
	Int mHorizonColorUniform;
};
