#include "AbstractCustomRenderer.h"

#include <Corrade/Corrade.h>
#include <Magnum/GL/TextureFormat.h>

#include "../RoomManager.h"

AbstractCustomRenderer::AbstractCustomRenderer(const Int parentIndex, const Vector2i & size) : mFramebuffer(Range2Di({}, size))
{
	mParentIndex = parentIndex;
	mSize = size;
}

GL::Texture2D &AbstractCustomRenderer::getRenderedTexture(const bool forceRender)
{
	if (forceRender)
	{
		// Bind custom framebuffer
		mFramebuffer
			.clear(GL::FramebufferClear::Color)
			.bind();

		// Custom draw
		render();

		// Restore original bound framebuffer (WITHOUT clear or anything like that)
		RoomManager::singleton->mGoLayers[mParentIndex].frameBuffer->bind();
	}
	return mTexture;
}

void AbstractCustomRenderer::setup()
{
	// Setup texture
	mTexture.setStorage(1, GL::TextureFormat::RGBA8, mSize);

	// Use texture as framebuffer's color attachment
	mFramebuffer.attachTexture(GL::Framebuffer::ColorAttachment{ 0 }, mTexture, 0);

	// Assert for framebuffer
	CORRADE_INTERNAL_ASSERT(mFramebuffer.checkStatus(GL::FramebufferTarget::Draw) == GL::Framebuffer::Status::Complete);
}