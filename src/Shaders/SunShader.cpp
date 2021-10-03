#include "SunShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

#include "../Common/CommonUtility.h"

SunShader::SunShader()
{
	// Setup shader from file
#if defined(CORRADE_TARGET_ANDROID) or defined(CORRADE_TARGET_IOS) or defined(CORRADE_TARGET_IOS_SIMULATOR)
	MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GLES300);

	GL::Shader vert{ GL::Version::GLES300, GL::Shader::Type::Vertex };
	GL::Shader frag{ GL::Version::GLES300, GL::Shader::Type::Fragment };
#else
	MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL300);

	GL::Shader vert{ GL::Version::GL330, GL::Shader::Type::Vertex };
	GL::Shader frag{ GL::Version::GL330, GL::Shader::Type::Fragment };
#endif

	vert.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/passthrough.vert");
	frag.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/sun.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	mTransformationProjectionMatrixUniform = uniformLocation("transformationProjectionMatrix");
	mIndexUniform = uniformLocation("index");

	setUniform(uniformLocation("displacementData"), DisplacementTextureUnit);
	setUniform(uniformLocation("colorData"), ColorTextureUnit);
}

SunShader& SunShader::setTransformationProjectionMatrix(const Matrix4 & matrix)
{
	setUniform(mTransformationProjectionMatrixUniform, matrix);
	return *this;
}

SunShader& SunShader::setIndex(const Float index)
{
	setUniform(mIndexUniform, index);
	return *this;
}

SunShader& SunShader::bindDisplacementTexture(GL::Texture2D& texture)
{
	texture.bind(DisplacementTextureUnit);
	return *this;
}

SunShader& SunShader::bindColorTexture(GL::Texture2D& texture)
{
	texture.bind(ColorTextureUnit);
	return *this;
}
