#pragma once

#include <memory>
#include <nlohmann/json.hpp>

#include "../Game/AbstractGuiElement.h"

using namespace Magnum;

class OverlayGui : public AbstractGuiElement
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	// Class members
	OverlayGui(const Int parentIndex);
	OverlayGui(const Int parentIndex, const std::string & textureName);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	Range3D getBoundingBox(const Vector2 & windowSize) override;

	void setRotationInDegrees(const Float rotation);
	void setColor(const Color4 & color);

	const Resource<GL::Texture2D> & getTextureResource() const;
	const void setTexture(const std::string & textureName);
	Float* color();
	const Vector2 getSize() const;

protected:
	void updateTransformations() override;

	Rad mRotation;
};