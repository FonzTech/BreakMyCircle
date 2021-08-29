#include "LSNumberRenderer.h"
#include "../../RoomManager.h"

LSNumberRenderer::LSNumberRenderer(const Vector2i & size, const std::string & text) : AbstractCustomRenderer(size, Color4(1.0f, 1.0f, 1.0f, 1.0f))
{
	// Create overlay gui
	mOverlayGui = std::make_shared<OverlayGuiDetached>(-1, RESOURCE_TEXTURE_WHITE, GO_OGD_FLAT);
	mOverlayGui->setPosition({ 0.0f, 0.0f });
	mOverlayGui->setSize(Vector2(3.0f));
	mOverlayGui->setAnchor({ 0.0f, 0.0f });
	mOverlayGui->setIdentityCanvasSize();

	// Create overlay text
	mOverlayText = std::make_shared<OverlayText>(-1, Text::Alignment::MiddleCenter, UnsignedInt(text.length()));
	mOverlayText->mPosition = Vector3(0.0f);
	mOverlayText->setSize(Vector2(0.75f - 0.035f * Float(text.length())));
	mOverlayText->mColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
	mOverlayText->mOutlineColor = Color4(0.0f);
	mOverlayText->mOutlineRange = Vector2(0.5f, 1.0f);
	mOverlayText->setText(text);
	mOverlayText->setCustomCanvasSize(Vector2(size));
}

void LSNumberRenderer::renderInternal()
{
	mOverlayGui->update();
	mOverlayText->update();

	mOverlayGui->drawDetached();
	mOverlayText->drawDetached();
}