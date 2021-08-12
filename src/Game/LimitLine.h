#pragma once

#include <nlohmann/json.hpp>

#include "../GameObject.h"
#include "../Graphics/ColoredDrawable.h"

class LimitLine : public GameObject
{
public:
	static Color4 RED_COLOR;

	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	LimitLine(const Int parentIndex);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;
};
