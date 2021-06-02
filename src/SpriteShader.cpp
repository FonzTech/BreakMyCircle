#include "SpriteShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

SpriteShader::SpriteShader()
{
	MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL330);

	GL::Shader vert{ GL::Version::GL330, GL::Shader::Type::Vertex };
	GL::Shader frag{ GL::Version::GL330, GL::Shader::Type::Fragment };

	vert.addFile("shaders/sprite_shader.vert");
	frag.addFile("shaders/sprite_shader.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	mColorUniform = uniformLocation("color");
	mTransformationMatrix = uniformLocation("transformationMatrix");
	mProjectionMatrix = uniformLocation("projectionMatrix");

	setUniform(uniformLocation("textureData"), TextureUnit);
}

SpriteShader& SpriteShader::setColor(const Color3& color)
{
	setUniform(mColorUniform, color);
	return *this;
}

SpriteShader& SpriteShader::setTransformationMatrix(const Matrix4& transformationMatrix)
{
	setUniform(mTransformationMatrix, transformationMatrix);
	return *this;
}

SpriteShader& SpriteShader::setProjectionMatrix(const Matrix4& projectionMatrix)
{
	setUniform(mProjectionMatrix, projectionMatrix);
	return *this;
}

SpriteShader& SpriteShader::bindTexture(GL::Texture2D& texture)
{
	texture.bind(TextureUnit);
	return *this;
}