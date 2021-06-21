#include "ScreenQuadShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Assert.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/Trade/AbstractImporter.h>

#include "RoomManager.h"

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

	setUniform(uniformLocation("textureMain"), GOL_MAIN);
	setUniform(uniformLocation("textureLevel"), GOL_LEVEL);
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