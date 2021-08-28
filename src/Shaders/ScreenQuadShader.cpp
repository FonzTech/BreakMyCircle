#include "ScreenQuadShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Assert.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/Trade/AbstractImporter.h>

#include "../RoomManager.h"

using namespace Corrade;

ScreenQuadShader::ScreenQuadShader()
{
	setupShader();
	setupMesh();
}

void ScreenQuadShader::setupShader()
{
	MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL330);

	GL::Shader vert{ GL::Version::GL330, GL::Shader::Type::Vertex };
	GL::Shader frag{ GL::Version::GL330, GL::Shader::Type::Fragment };

	vert.addFile("shaders/screen_quad.vert");
	frag.addFile("shaders/screen_quad.frag");

	CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({ vert, frag }));

	attachShaders({ vert, frag });

	CORRADE_INTERNAL_ASSERT_OUTPUT(link());

	setUniform(uniformLocation("colorPerspFirst"), GOL_PERSP_FIRST);
	setUniform(uniformLocation("depthStencilPerspFirst"), GOL_PERSP_FIRST + TEXTURE_UNIT_DEPTHSTENCIL_OFFSET);
	setUniform(uniformLocation("colorPerspSecond"), GOL_PERSP_SECOND);
	setUniform(uniformLocation("colorOrthoFirst"), GOL_ORTHO_FIRST);
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