#pragma once

#include <memory>
#include <nlohmann/json.hpp>

#include <Magnum/Shaders/Flat.h>
#include "../GameObject.h"

using namespace Magnum;

class OverlayGui : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	// Class members
	OverlayGui(const Int parentIndex);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	void setPosition(const Vector2 & position);
	void setSize(const Vector2 & size);
	void setAnchor(const Vector2 & anchor);
	void setOpacity(const Float opacity);
	Range3D getBoundingBox(const Vector2 & windowSize);

protected:
	void updateAspectRatioFactors();
	void updateTransformations();

	Float mArs[2];
	Float mOpacity;
	Vector2 mSize;
	Vector2 mAnchor;
};