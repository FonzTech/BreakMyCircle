#include "WaterShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

#include "../Common/CommonUtility.h"

WaterShader::WaterShader()
{
	// Setup shader from file
#ifdef TARGET_MOBILE
	MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GLES300);

	GL::Shader vert{ GL::Version::GLES300, GL::Shader::Type::Vertex };
	GL::Shader frag{ GL::Version::GLES300, GL::Shader::Type::Fragment };
#else
	MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL330);

	GL::Shader vert{ GL::Version::GL330, GL::Shader::Type::Vertex };
	GL::Shader frag{ GL::Version::GL330, GL::Shader::Type::Fragment };
#endif

	vert.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/passthrough.vert");
	frag.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/water.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	mTransformationProjectionMatrixUniform = uniformLocation("transformationProjectionMatrix");
	mFrameUniform = uniformLocation("frame");
	mSpeedUniform = uniformLocation("speed");
	mSizeUniform = uniformLocation("size");
	mHorizonColorUniform = uniformLocation("horizonColor");

	setUniform(uniformLocation("displacementData"), DisplacementTextureUnit);
	setUniform(uniformLocation("textureData"), WaterTextureUnit);
	setUniform(uniformLocation("effectsData"), EffectsTextureUnit);
}

WaterShader& WaterShader::setTransformationProjectionMatrix(const Matrix4 & matrix)
{
	setUniform(mTransformationProjectionMatrixUniform, matrix);
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
