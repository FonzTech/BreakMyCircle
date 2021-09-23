#include "StarRoadShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

#include "../Common/CommonUtility.h"

StarRoadShader::StarRoadShader()
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

	vert.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/passthrough.vert");
	frag.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/starroad.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	mTransformationMatrixUniform = uniformLocation("transformationMatrix");
	mProjectionMatrixUniform = uniformLocation("projectionMatrix");
	mIndexUniform = uniformLocation("index");

	setUniform(uniformLocation("displacementData"), DisplacementTextureUnit);
	setUniform(uniformLocation("alphaMapData"), AlphaMapTextureUnit);
}

StarRoadShader& StarRoadShader::setTransformationMatrix(const Matrix4& transformationMatrix)
{
	setUniform(mTransformationMatrixUniform, transformationMatrix);
	return *this;
}

StarRoadShader& StarRoadShader::setProjectionMatrix(const Matrix4& projectionMatrix)
{
	setUniform(mProjectionMatrixUniform, projectionMatrix);
	return *this;
}

StarRoadShader& StarRoadShader::setIndex(const Float index)
{
	setUniform(mIndexUniform, index);
	return *this;
}

StarRoadShader& StarRoadShader::bindDisplacementTexture(GL::Texture2D& texture)
{
	texture.bind(DisplacementTextureUnit);
	return *this;
}

StarRoadShader& StarRoadShader::bindAlphaMapTexture(GL::Texture2D& texture)
{
	texture.bind(AlphaMapTextureUnit);
	return *this;
}