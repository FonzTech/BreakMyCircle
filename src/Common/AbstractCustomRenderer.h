#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Texture.h>

using namespace Magnum;

class AbstractCustomRenderer
{
public:
	AbstractCustomRenderer(const Int parentIndex, const Vector2i & size);

	GL::Texture2D & getRenderedTexture(const bool forceRender);

protected:
	void setup();
	virtual void render() = 0;

	Int mParentIndex;
	Vector2i mSize;

	GL::Texture2D mTexture;
	GL::Framebuffer mFramebuffer;
};