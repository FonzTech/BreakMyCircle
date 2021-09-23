#pragma once

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>

#include "../GameObject.h"

class Fireball : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Fireball(const Int parentIndex);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

protected:
	Float mFrame;
};