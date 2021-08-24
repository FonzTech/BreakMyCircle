#pragma once

#include <Magnum/Magnum.h>
#include <Magnum/Math/Color.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Texture.h>

using namespace Magnum;

class AbstractCustomRenderer
{
public:
	AbstractCustomRenderer(const Vector2i & size, const Color4 & clearColor);

	void renderTexture();
	GL::Texture2D & getRenderedTexture();

protected:
	void setup();
	virtual void renderInternal() = 0;
	
	Vector2i mSize;
	Color4 mClearColor;

	GL::Texture2D mTexture;
	GL::Framebuffer mFramebuffer;
};