#include "AbstractCustomRenderer.h"

#include <Corrade/Corrade.h>
#include <Magnum/GL/TextureFormat.h>

#include "../RoomManager.h"

AbstractCustomRenderer::AbstractCustomRenderer(const Int parentIndex, const Vector2i & size, const Color4 & clearColor) : mParentIndex(parentIndex), mSize(size), mClearColor(clearColor), mFramebuffer(Range2Di({}, size))
{
	setup();
}

GL::Texture2D &AbstractCustomRenderer::getRenderedTexture(const bool forceRender)
{
	if (forceRender)
	{
		// Bind custom framebuffer
		mFramebuffer
			.clearColor(0, mClearColor)
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