#include "ShootPathShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

#include "../Common/CommonUtility.h"

ShootPathShader::ShootPathShader()
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
	frag.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/shoot_path.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	mTransformationProjectionMatrixUniform = uniformLocation("transformationProjectionMatrix");
	mIndexUniform = uniformLocation("index");
	mSizeUniform = uniformLocation("size");

	setUniform(uniformLocation("textureData"), TextureUnit);
}

ShootPathShader& ShootPathShader::setTransformationProjectionMatrix(const Matrix4 & matrix)
{
	setUniform(mTransformationProjectionMatrixUniform, matrix);
	return *this;
}

ShootPathShader& ShootPathShader::setIndex(const Float index)
{
	setUniform(mIndexUniform, index);
	return *this;
}

ShootPathShader& ShootPathShader::setSize(const Float size)
{
	setUniform(mSizeUniform, size);
	return *this;
}

ShootPathShader& ShootPathShader::bindTexture(GL::Texture2D& texture)
{
	texture.bind(TextureUnit);
	return *this;
}
