#include "LSNumberRenderer.h"
#include "../../RoomManager.h"

LSNumberRenderer::LSNumberRenderer(const Vector2i & size, const std::string & text) : AbstractCustomRenderer(size, Color4(1.0f, 1.0f, 1.0f, 1.0f))
{
	// Create overlay gui
	mOverlayGui = std::make_shared<OverlayGuiDetached>(-1, RESOURCE_TEXTURE_WHITE);
	mOverlayGui->setPosition({ -0.0f, 0.0f });
	mOverlayGui->setSize({ 1.0f, 1.0f });
	mOverlayGui->setAnchor({ 0.0f, 0.0f });

	// Create overlay text
	mOverlayText = std::make_shared<OverlayText>(-1, Text::Alignment::MiddleCenter, UnsignedInt(text.length()));
	mOverlayText->mPosition = Vector3(0.0f);
	mOverlayText->setScale(Vector2(0.055f - 0.01f * Float(text.length())));
	mOverlayText->mColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
	mOverlayText->mOutlineRange = Vector2(0.6f, 1.0f);
	mOverlayText->setText(text);
}

void LSNumberRenderer::renderInternal()
{
	mOverlayGui->drawDetached();
	mOverlayText->drawDetached();
}