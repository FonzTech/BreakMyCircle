#include "ScreenQuadShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Assert.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/Trade/AbstractImporter.h>

#include "../RoomManager.h"
#include "../Common/CommonUtility.h"

using namespace Corrade;

ScreenQuadShader::ScreenQuadShader()
{
}

void ScreenQuadShader::setup()
{
	setupShader();
	setupMesh();
}

void ScreenQuadShader::setupShader()
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

	vert.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/screen_quad.vert");
	frag.addFile(CommonUtility::singleton->mConfig.assetDir + "shaders/screen_quad.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	setUniform(uniformLocation("colorPerspFirst"), GOL_PERSP_FIRST);
	setUniform(uniformLocation("colorPerspSecond"), GOL_PERSP_SECOND);
	setUniform(uniformLocation("colorOrthoFirst"), GOL_ORTHO_FIRST);
	// setUniform(uniformLocation("depthStencilPerspFirst"), GOL_PERSP_FIRST + TEXTURE_UNIT_DEPTHSTENCIL_OFFSET);
}

void ScreenQuadShader::setupMesh()
{
	struct TriangleVertex
	{
		Vector2 position;
		Vector2 textureCoordinates;
	};

	const TriangleVertex data[]{
		{{ -1.0f, -1.0f }, { 0.0f, 0.0f }},
		{{ 1.0f, -1.0f }, { 1.0f, 0.0f }},
		{{ 1.0f, 1.0f }, { 1.0f, 1.0f }},
		{{ 1.0f, 1.0f }, { 1.0f, 1.0f }},
		{{ -1.0f, 1.0f }, { 0.0f, 1.0f }},
		{{ -1.0f, -1.0f }, { 0.0f, 0.0f }},
	};

	GL::Buffer buffer;
	buffer.setData(data);
	mMesh.setCount(6)
		.addVertexBuffer(std::move(buffer), 0,
			ScreenQuadShader::Position{},
			ScreenQuadShader::TextureCoordinates{});
}

ScreenQuadShader& ScreenQuadShader::bindColorTexture(const Int layer, GL::Texture2D & texture)
{
	texture.bind(layer);
	return *this;
}

ScreenQuadShader& ScreenQuadShader::bindDepthStencilTexture(const Int layer, GL::Texture2D & texture)
{
	texture.bind(layer + TEXTURE_UNIT_DEPTHSTENCIL_OFFSET);
	return *this;
}