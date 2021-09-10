#include "PlasmaShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

#include "../Common/CommonUtility.h"

PlasmaShader::PlasmaShader()
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
	frag.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/plasma.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	mTransformationMatrixUniform = uniformLocation("transformationMatrix");
	mProjectionMatrixUniform = uniformLocation("projectionMatrix");
	mSizeUniform = uniformLocation("size");
	mTimeUniform = uniformLocation("time");
}

PlasmaShader& PlasmaShader::setTransformationMatrix(const Matrix4& transformationMatrix)
{
	setUniform(mTransformationMatrixUniform, transformationMatrix);
	return *this;
}

PlasmaShader& PlasmaShader::setProjectionMatrix(const Matrix4& projectionMatrix)
{
	setUniform(mProjectionMatrixUniform, projectionMatrix);
	return *this;
}

PlasmaShader& PlasmaShader::setSize(const Vector2 & size)
{
	setUniform(mSizeUniform, size);
	return *this;
}

PlasmaShader& PlasmaShader::setTime(const Float time)
{
	setUniform(mTimeUniform, time);
	return *this;
}