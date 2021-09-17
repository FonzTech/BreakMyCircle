#pragma once

#include <nlohmann/json.hpp>

#include "../GameObject.h"

class SafeMinigame : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	SafeMinigame(const Int parentIndex);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

protected:
	Int mMode;
	Float mScale;
	Float mAnimY;
	Float mAngleHandle;
	std::weak_ptr<BaseDrawable> mDrawableHandle;
};