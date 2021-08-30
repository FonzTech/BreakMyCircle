#pragma once

#include <memory>
#include <nlohmann/json.hpp>

#include "../GameObject.h"

using namespace Magnum;

class AbstractGuiElement : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	// Class members
	AbstractGuiElement(const Int parentIndex);

	virtual Range3D getBoundingBox(const Vector2 & windowSize) = 0;

	void setPosition(const Vector2 & position);
	void setSize(const Vector2 & size);
	void setAnchor(const Vector2 & anchor);

	void resetCustomCanvasSize();
	void setCustomCanvasSize(const Vector2 & size);
	void setIdentityCanvasSize();

	Color4 mColor;
	Vector2 mSize;

protected:
	virtual void updateTransformations() = 0;

	void updateAspectRatioFactors();

	Vector2 mAnchor;
	Vector2 mAspectRatio;
	Vector2 mCustomCanvasSize;
};