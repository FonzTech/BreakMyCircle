#include "CubeMapShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Assert.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/CubeMapTexture.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

CubeMapShader::CubeMapShader()
{
	MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL330);

	GL::Shader vert{ GL::Version::GL330, GL::Shader::Type::Vertex };
	GL::Shader frag{ GL::Version::GL330, GL::Shader::Type::Fragment };

	vert.addFile("shaders/cubemap.vert");
	frag.addFile("shaders/cubemap.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	_transformationProjectionMatrixUniform = uniformLocation("transformationProjectionMatrix");

	setUniform(uniformLocation("textureData"), TextureUnit);
}

CubeMapShader& CubeMapShader::setTransformationProjectionMatrix(const Matrix4& matrix)
{
	setUniform(_transformationProjectionMatrixUniform, matrix);
	return *this;
}

CubeMapShader& CubeMapShader::setTexture(GL::CubeMapTexture& texture)
{
	texture.bind(TextureUnit);
	return *this;
}