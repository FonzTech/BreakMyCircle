#pragma once

#define GO_LS_MAX_SCROLL_THRESHOLD 0.01f
#define GO_LS_MAX_SCROLL_VELOCITY 0.025f
#define GO_LS_MAX_SCROLL_VELOCITY_MAX (GO_LS_MAX_SCROLL_VELOCITY * 50.0f)
#define GO_LS_RESET_MOUSE_VALUE -10000

#include <array>
#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <Magnum/Magnum.h>

#include "../GameObject.h"
#include "../Graphics/BaseDrawable.h"
#include "OverlayGui.h"
#include "Scenery.h"

using namespace Magnum;

class LevelSelector : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	LevelSelector(const Int parentIndex);
	~LevelSelector();

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

private:
	static std::unordered_map<Int, std::array<Vector3, 6>> sLevelButtonPositions;

	struct LS_ButtonSelector
	{
		std::shared_ptr<TexturedDrawable<Shaders::Phong>> drawables;
		Vector3 position;
		Int index;
	};

	struct LS_ScenerySelector
	{
		std::shared_ptr<Scenery> scenery;
		std::vector<LS_ButtonSelector> buttons;
	};

	void handleScrollableCameraPosition(const Vector3 & delta);
	void handleScrollableScenery();

	Vector2i mPrevMousePos;
	Vector3 mScrollVelocity;
	Float mButtonAnim[1];

	Int mClickIndex;
	std::shared_ptr<OverlayGui> mScreenButtons[1];
	std::unordered_set<BaseDrawable*> mButtonDrawables;
	std::function<void()> mCallbacks[1];
	std::unordered_map<Int, LS_ScenerySelector> mSceneries;
};