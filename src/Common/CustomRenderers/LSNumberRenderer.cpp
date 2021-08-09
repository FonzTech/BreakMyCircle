#include "LSNumberRenderer.h"
#include "../../RoomManager.h"

LSNumberRenderer::LSNumberRenderer(const Int parentIndex, const Vector2i & size, const std::string & text) : AbstractCustomRenderer(parentIndex, size, Color4(1.0f, 1.0f, 1.0f, 1.0f))
{
	mOverlayText = std::make_shared<OverlayText>(-1);
	mOverlayText->mPosition = Vector3(0.0f, 0.0f, 0.0f);
	mOverlayText->setScale(Vector2(0.055f - 0.015f * Float(text.length())));
	mOverlayText->mColor = Color4(1.0f, 1.0f, 1.0f, 1.0f);
	mOverlayText->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
	mOverlayText->mOutlineRange = Vector2(0.5f, 0.3f);
	mOverlayText->setText(text);
}

void LSNumberRenderer::render()
{
	mOverlayText->draw(nullptr, Matrix4(Math::ZeroInit), *RoomManager::singleton->mCamera);
}