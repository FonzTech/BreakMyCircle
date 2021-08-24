#pragma once

#include "../AbstractCustomRenderer.h"
#include "../../Game/OverlayGuiDetached.h"
#include "../../Game/OverlayText.h"

class PlasmaSquareRenderer : public AbstractCustomRenderer
{
public:
	PlasmaSquareRenderer(const Vector2i & size);

	void update(const Float deltaTime);

protected:
	void renderInternal() override;

	std::shared_ptr<OverlayGuiDetached> mOverlayGui;
	Float mTime;
};