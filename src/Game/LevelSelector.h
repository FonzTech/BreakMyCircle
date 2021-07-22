#pragma once

#define GO_LS_MAX_SCROLL_THRESHOLD 0.01f
#define GO_LS_MAX_SCROLL_VELOCITY 0.025f
#define GO_LS_MAX_SCROLL_VELOCITY_MAX (GO_LS_MAX_SCROLL_VELOCITY * 50.0f)

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <Magnum/Magnum.h>

#include "../GameObject.h"
#include "OverlayGui.h"
#include "Scenery.h"

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
	void handleScrollableCameraPosition(const Vector3 & delta);
	void handleScrollableScenery();

	Vector2i mPrevMousePos;
	Vector3 mScrollVelocity;

	Int mClickIndex;
	std::shared_ptr<OverlayGui> mButtons[1];
	std::function<void()> mCallbacks[1];
	std::unordered_map<Int, std::shared_ptr<Scenery>> mSceneries;
};