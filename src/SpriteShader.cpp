#include "SpriteShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

SpriteShader::SpriteShader(const Float & width, const Float & height, const Float & rows, const Float & columns, const Float & total, const Float & speed, const Color3 & color)
{
	// Assign members
	mTotal = total;
	mSpeed = speed;
	mColor = color;
	mIndex = 0;

	// Setup shader from file
	MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL330);

	GL::Shader vert{ GL::Version::GL330, GL::Shader::Type::Vertex };
	GL::Shader frag{ GL::Version::GL330, GL::Shader::Type::Fragment };

	vert.addFile("shaders/sprite_shader.vert");
	frag.addFile("shaders/sprite_shader.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	mTransformationMatrixUniform = uniformLocation("transformationMatrix");
	mProjectionMatrixUniform = uniformLocation("projectionMatrix");

	mIndexUniform = uniformLocation("index");

	setUniform(uniformLocation("textureData"), TextureUnit);

	setUniform(uniformLocation("color"), color);
	setUniform(uniformLocation("texWidth"), width);
	setUniform(uniformLocation("texHeight"), height);
	setUniform(uniformLocation("rows"), rows);
	setUniform(uniformLocation("columns"), columns);
}

SpriteShader& SpriteShader::update(const Float & deltaTime)
{
	// Advance animation
	mIndex += deltaTime * mSpeed;

	// Update uniforms
	setUniform(mIndexUniform, std::fmodf(mIndex, mTotal));

	return *this;
}

SpriteShader& SpriteShader::setTransformationMatrix(const Matrix4& transformationMatrix)
{
	setUniform(mTransformationMatrixUniform, transformationMatrix);
	return *this;
}

SpriteShader& SpriteShader::setProjectionMatrix(const Matrix4& projectionMatrix)
{
	setUniform(mProjectionMatrixUniform, projectionMatrix);
	return *this;
}

SpriteShader& SpriteShader::bindTexture(GL::Texture2D& texture)
{
	texture.bind(TextureUnit);
	return *this;
}