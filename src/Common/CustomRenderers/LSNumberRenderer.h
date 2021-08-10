#pragma once

#include "../AbstractCustomRenderer.h"
#include "../../Game/OverlayGuiDetached.h"
#include "../../Game/OverlayText.h"

class LSNumberRenderer : public AbstractCustomRenderer
{
public:
	LSNumberRenderer(const Int parentIndex, const Vector2i & size, const std::string & text);

protected:
	void render() override;

	std::shared_ptr<OverlayGuiDetached> mOverlayGui;
	std::shared_ptr<OverlayText> mOverlayText;
};