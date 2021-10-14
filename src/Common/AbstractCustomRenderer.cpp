#include "AbstractCustomRenderer.h"

#include <Corrade/Corrade.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/GL/Renderer.h>

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
	GL::Renderer::enable(GL::Renderer::Feature::Blending);
	GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha, GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::One);

	// Custom draw
	renderInternal();

	// Restore original bound framebuffer (WITHOUT clear or anything like that)
	{
		const Int parentIndex = RoomManager::singleton->getCurrentBoundParentIndex();
		if (parentIndex == -1)
		{
#if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
            RoomManager::singleton->mDefaultFramebufferPtr->bind();
#else
			GL::defaultFramebuffer.bind();
#endif
		}
		else
		{
			RoomManager::singleton->mGoLayers[parentIndex].frameBuffer->bind();
		}
	}
	GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha, GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::One);
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
