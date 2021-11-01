#include "SunShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

#include "../Common/CommonUtility.h"

TimedBubbleShader::TimedBubbleShader()
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
	frag.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/timed_bubble.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	mTransformationProjectionMatrixUniform = uniformLocation("transformationProjectionMatrix");
	mTimedRotationUniform = uniformLocation("timedRotation");

	setUniform(uniformLocation("colorData"), ColorTextureUnit);
	setUniform(uniformLocation("maskData"), MaskTextureUnit);
}

TimedBubbleShader& TimedBubbleShader::setTransformationProjectionMatrix(const Matrix4 & matrix)
{
	setUniform(mTransformationProjectionMatrixUniform, matrix);
	return *this;
}

TimedBubbleShader& TimedBubbleShader::setTimedRotation(const Float value)
{
	setUniform(mTimedRotationUniform, value);
	return *this;
}

TimedBubbleShader& TimedBubbleShader::bindColorTexture(GL::Texture2D& texture)
{
	texture.bind(ColorTextureUnit);
	return *this;
}

TimedBubbleShader& TimedBubbleShader::bindMaskTexture(GL::Texture2D& texture)
{
	texture.bind(MaskTextureUnit);
	return *this;
}