#include "AbstractGuiElement.h"
#include "../RoomManager.h"

std::shared_ptr<GameObject> AbstractGuiElement::getInstance(const nlohmann::json & params)
{
	// No default constructor exists for this class!!
	return nullptr;
}

AbstractGuiElement::AbstractGuiElement(const Int parentIndex) : GameObject(parentIndex), mColor(1.0f, 1.0f, 1.0f, 1.0f), mSize{ 1.0f }, mCustomCanvasSize{ 0.0f }
{
}

void AbstractGuiElement::setPosition(const Vector2 & position)
{
	mPosition = Vector3(position.x(), position.y(), 0.0f);
}

void AbstractGuiElement::setSize(const Vector2 & size)
{
	mSize = size;
}

void AbstractGuiElement::setAnchor(const Vector2 & anchor)
{
	mAnchor = anchor;
}

void AbstractGuiElement::updateAspectRatioFactors()
{
	if (mCustomCanvasSize.x() <= -1.0f)
	{
		mAspectRatio = mCustomCanvasSize.normalized();
	}
	else
	{
		const auto& w = RoomManager::singleton->getWindowSize();
		if (w.x() < w.y())
		{
			mAspectRatio = Vector2(1.0f, Float(w.x()) / Float(w.y()));
		}
		else
		{
			mAspectRatio = Vector2(Float(w.x()) / Float(w.y()), 1.0f);
		}
	}
}

void AbstractGuiElement::resetCustomCanvasSize()
{
	mCustomCanvasSize = Vector2(0.0f);
}

void AbstractGuiElement::setCustomCanvasSize(const Vector2 & size)
{
	mCustomCanvasSize = size;
}

void AbstractGuiElement::setIdentityCanvasSize()
{
	mCustomCanvasSize = Vector2(-1.0f);
}