#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Color.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Texture.h>

using namespace Magnum;

class AbstractCustomRenderer
{
public:
	AbstractCustomRenderer(const Int parentIndex, const Vector2i & size, const Color4 & clearColor);

	GL::Texture2D & getRenderedTexture(const bool forceRender);

protected:
	void setup();
	virtual void render() = 0;

	Int mParentIndex;
	Vector2i mSize;
	Color4 mClearColor;

	GL::Texture2D mTexture;
	GL::Framebuffer mFramebuffer;
};