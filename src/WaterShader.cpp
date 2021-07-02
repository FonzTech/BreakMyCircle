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
	MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL330);

	GL::Shader vert{ GL::Version::GL330, GL::Shader::Type::Vertex };
	GL::Shader frag{ GL::Version::GL330, GL::Shader::Type::Fragment };

	vert.addFile("shaders/passthrough.vert");
	frag.addFile("shaders/water.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	mTransformationMatrixUniform = uniformLocation("transformationMatrix");
	mProjectionMatrixUniform = uniformLocation("projectionMatrix");

	setUniform(uniformLocation("textureData"), TextureUnit);
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

WaterShader& WaterShader::bindTexture(GL::Texture2D& texture)
{
	texture.bind(TextureUnit);
	return *this;
}