#include "SpriteShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

#include "../Common/CommonUtility.h"

SpriteShader::SpriteShader()
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
	frag.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/sprite.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	mTransformationProjectionMatrixUniform = uniformLocation("transformationProjectionMatrix");
	mColorUniform = uniformLocation("color");
	mIndexUniform = uniformLocation("index");
	mRowsUniform = uniformLocation("rows");
	mColumnsUniform = uniformLocation("columns");

	setUniform(uniformLocation("textureData"), TextureUnit);
}

SpriteShader& SpriteShader::setTransformationProjectionMatrix(const Matrix4 & matrix)
{
	setUniform(mTransformationProjectionMatrixUniform, matrix);
	return *this;
}

SpriteShader& SpriteShader::bindTexture(GL::Texture2D& texture)
{
	texture.bind(TextureUnit);
	return *this;
}

SpriteShader& SpriteShader::setColor(const Color4& color)
{
	setUniform(mColorUniform, color);
	return *this;
}

SpriteShader& SpriteShader::setIndex(const Float index)
{
	setUniform(mIndexUniform, index);
	return *this;
}

SpriteShader& SpriteShader::setRows(const Float rows)
{
	setUniform(mRowsUniform, rows);
	return *this;
}

SpriteShader& SpriteShader::setColumns(const Float columns)
{
	setUniform(mColumnsUniform, columns);
	return *this;
}
