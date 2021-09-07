#include "WaterShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

WaterShader::WaterShader()
{
	// Setup shader from file
#ifdef CORRADE_TARGET_ANDROID
	MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GLES300);

	GL::Shader vert{ GL::Version::GLES300, GL::Shader::Type::Vertex };
	GL::Shader frag{ GL::Version::GLES300, GL::Shader::Type::Fragment };
#else
	MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL330);

	GL::Shader vert{ GL::Version::GL330, GL::Shader::Type::Vertex };
	GL::Shader frag{ GL::Version::GL330, GL::Shader::Type::Fragment };
#endif

	vert.addFile("shaders/passthrough.vert");
	frag.addFile("shaders/water.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	mTransformationMatrixUniform = uniformLocation("transformationMatrix");
	mProjectionMatrixUniform = uniformLocation("projectionMatrix");
	mFrameUniform = uniformLocation("frame");
	mSpeedUniform = uniformLocation("speed");
	mSizeUniform = uniformLocation("size");
	mHorizonColorUniform = uniformLocation("horizonColor");

	setUniform(uniformLocation("displacementData"), DisplacementTextureUnit);
	setUniform(uniformLocation("textureData"), WaterTextureUnit);
	setUniform(uniformLocation("effectsData"), EffectsTextureUnit);
}

WaterShader& WaterShader::setTransformationMatrix(const Matrix4& transformationMatrix)
{
	setUniform(mTransformationMatrixUniform, transformationMatrix);
	return *this;
}

WaterShader& WaterShader::setProjectionMatrix(const Matrix4& projectionMatrix)
{
	setUniform(mProjectionMatrixUniform, projectionMatrix);
	return *this;
}

WaterShader& WaterShader::setFrame(const Float frame)
{
	setUniform(mFrameUniform, frame);
	return *this;
}

WaterShader& WaterShader::setSpeed(const Float speed)
{
	setUniform(mSpeedUniform, speed);
	return *this;
}

WaterShader& WaterShader::setSize(const Vector2 & size)
{
	setUniform(mSizeUniform, size);
	return *this;
}

WaterShader& WaterShader::setHorizonColorUniform(const Color3& color)
{
	setUniform(mHorizonColorUniform, color);
	return *this;
}

WaterShader& WaterShader::bindDisplacementTexture(GL::Texture2D& texture)
{
	texture.bind(DisplacementTextureUnit);
	return *this;
}

WaterShader& WaterShader::bindWaterTexture(GL::Texture2D& texture)
{
	texture.bind(WaterTextureUnit);
	return *this;
}

WaterShader& WaterShader::bindEffectsTexture(GL::Texture2D& texture)
{
	texture.bind(EffectsTextureUnit);
	return *this;
}