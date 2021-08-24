#include "AbstractCustomRenderer.h"

#include <Corrade/Corrade.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/TextureFormat.h>

#include "../RoomManager.h"

AbstractCustomRenderer::AbstractCustomRenderer(const Vector2i & size, const Color4 & clearColor) : mSize(size), mClearColor(clearColor), mFramebuffer(Range2Di({}, size))
{
	setup();
}

void AbstractCustomRenderer::renderTexture()
{
	// Bind custom framebuffer
	mFramebuffer
		.clearColor(0, mClearColor)
		.bind();

	// Custom draw
	renderInternal();

	// Restore original bound framebuffer (WITHOUT clear or anything like that)
	{
		const Int parentIndex = RoomManager::singleton->getCurrentBoundParentIndex();
		if (parentIndex == -1)
		{
			GL::defaultFramebuffer.bind();
		}
		else
		{
			RoomManager::singleton->mGoLayers[parentIndex].frameBuffer->bind();
		}
	}
}

GL::Texture2D &AbstractCustomRenderer::getRenderedTexture()
{
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