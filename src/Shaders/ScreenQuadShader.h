#pragma once

#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/AbstractShaderProgram.h>

using namespace Magnum;

class ScreenQuadShader : public GL::AbstractShaderProgram
{
public:
	GL::Mesh mMesh;

	typedef GL::Attribute<0, Vector2> Position;
	typedef GL::Attribute<1, Vector2> TextureCoordinates;

	explicit ScreenQuadShader();

	ScreenQuadShader& bindTexture(const Int layer, GL::Texture2D & texture)
	{
		texture.bind(layer);
		return *this;
	}

private:

	void setupShader();
	void setupMesh();
};