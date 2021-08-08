#pragma once

#define GO_LS_SCENERY_LENGTH 50.0f
#define GO_LS_SCENERY_LENGTH_DOUBLE GO_LS_SCENERY_LENGTH * 2.0f
#define GO_LS_MAX_SCROLL_THRESHOLD 0.01f
#define GO_LS_MAX_SCROLL_VELOCITY 0.025f
#define GO_LS_MAX_SCROLL_VELOCITY_MAX (GO_LS_MAX_SCROLL_VELOCITY * GO_LS_SCENERY_LENGTH)
#define GO_LS_RESET_MOUSE_VALUE -10000
#define GO_CLICK_TAP_MAX_DELAY 0.3

#include <array>
#include <vector>
#include <memory>
#include <chrono>

#include <nlohmann/json.hpp>
#include <Magnum/Magnum.h>

#include "../GameObject.h"
#include "../Graphics/BaseDrawable.h"
#include "OverlayGui.h"
#include "OverlayText.h"
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

	struct LS_PickableObject
	{
		std::vector<std::weak_ptr<BaseDrawable>> drawables;
		Vector3 position;
		UnsignedInt levelIndex;
		UnsignedInt objectId;
	};

	struct LS_ScenerySelector
	{
		std::shared_ptr<Scenery> scenery;
		std::vector<LS_PickableObject> buttons;
		Object3D* manipulator;
	};

	void createSkyPlane();
	void handleScrollableCameraPosition(const Vector3 & delta);
	void handleScrollableScenery();
	void clickLevelButton(const UnsignedInt id);
	void currentLevelView();

	std::shared_ptr<TexturedDrawable<Shaders::Flat3D>> mSkyPlane;
	Object3D* mSkyManipulator;

	Vector2i mPrevMousePos;
	Vector3 mScrollVelocity;
	Float mScreenButtonAnim[1];
	Float mLevelButtonScaleAnim;

	Int mClickIndex;
	std::unordered_map<Int, std::shared_ptr<OverlayGui>> mScreenButtons;
	std::function<void()> mCallbacks[1];
	std::unordered_map<Int, LS_ScenerySelector> mSceneries;

	std::unordered_map<Int, std::shared_ptr<OverlayGui>> mLevelDrawables;
	std::unordered_map<Int, std::shared_ptr<OverlayText>> mLevelTexts;
	Float mLevelAnim;

	std::chrono::system_clock::time_point mClickStartTime;
	std::unordered_map<UnsignedInt, Int> mPickableObjectRefs;

	UnsignedInt mCurrentViewingLevelId;
};