#pragma once

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <Magnum/Magnum.h>

#include "../GameObject.h"
#include "OverlayGui.h"

using namespace Magnum;

class LevelSelector : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	LevelSelector(const Int parentIndex);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

private:
	Float mScroll;
	Int mClickIndex;
	std::shared_ptr<OverlayGui> mButtons[1];
	std::function<void()> mCallbacks[1];
};