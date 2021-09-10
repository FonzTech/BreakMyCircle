#include "PlasmaSquareRenderer.h"
#include "../../RoomManager.h"

PlasmaSquareRenderer::PlasmaSquareRenderer(const Vector2i & size) : AbstractCustomRenderer(size, Color4(1.0f, 1.0f, 1.0f, 1.0f))
{
	// Create overlay gui
	mOverlayGui = std::make_shared<OverlayGuiDetached>(-1, RESOURCE_TEXTURE_WHITE, GO_OGD_PLASMA);
	mOverlayGui->setPosition({ -0.0f, 0.0f });
	mOverlayGui->setSize({ 1.0f, 1.0f });
	mOverlayGui->setAnchor({ 0.0f, 0.0f });

	(*(PlasmaShader*)mOverlayGui->getShader())
		.setSize(Vector2(4.0f));

	mTime = 0.0f;
}

void PlasmaSquareRenderer::renderInternal()
{
	(*(PlasmaShader*)mOverlayGui->getShader())
		.setTime(mTime);

	mOverlayGui->drawDetached();
}

void PlasmaSquareRenderer::update(const Float deltaTime)
{
	mTime += deltaTime;
	while (mTime > 1.0f)
	{
		mTime -= 1.0f;
	}
}