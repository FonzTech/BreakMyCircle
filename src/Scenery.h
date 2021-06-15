#pragma once

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <Magnum/Math/Bezier.h>

#include "GameObject.h"
#include "BaseDrawable.h"

class Scenery : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(nlohmann::json params);

	Scenery();

protected:
	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	std::vector<Object3D*> mManipulatorList;

	Float mFrame;
	std::unique_ptr<CubicBezier2D> mCubicBezier;
};