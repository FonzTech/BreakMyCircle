#include "CubeMapShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Assert.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/CubeMapTexture.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

#include "../Common/CommonUtility.h"

CubeMapShader::CubeMapShader()
{
	// Setup shader from file
#ifdef CORRADE_TARGET_ANDROID
	MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GLES300);

	GL::Shader vert{ GL::Version::GLES300, GL::Shader::Type::Vertex };
	GL::Shader frag{ GL::Version::GLES300, GL::Shader::Type::Fragment };
#else
	MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL300);

	GL::Shader vert{ GL::Version::GL330, GL::Shader::Type::Vertex };
	GL::Shader frag{ GL::Version::GL330, GL::Shader::Type::Fragment };
#endif

	vert.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/cubemap.vert");
	frag.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/cubemap.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	mTransformationProjectionMatrixUniform = uniformLocation("transformationProjectionMatrix");

	setUniform(uniformLocation("textureData"), TextureUnit);
}

CubeMapShader& CubeMapShader::setTransformationProjectionMatrix(const Matrix4& matrix)
{
	setUniform(mTransformationProjectionMatrixUniform, matrix);
	return *this;
}

CubeMapShader& CubeMapShader::setTexture(GL::CubeMapTexture& texture)
{
	texture.bind(TextureUnit);
	return *this;
}