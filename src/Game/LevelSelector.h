#pragma once

#define GO_LS_SCENERY_LENGTH 50.0f
#define GO_LS_SCENERY_LENGTH_DOUBLE GO_LS_SCENERY_LENGTH * 2.0f
#define GO_LS_MAX_SCROLL_THRESHOLD 0.01f
#define GO_LS_MAX_SCROLL_VELOCITY 0.025f
#define GO_LS_MAX_SCROLL_VELOCITY_MAX (GO_LS_MAX_SCROLL_VELOCITY * GO_LS_SCENERY_LENGTH)
#define GO_LS_RESET_MOUSE_VALUE -10000
#define GO_LS_CLICK_TAP_MAX_DELAY 0.3

#define GO_LS_MESH_PLATFORM "PlatformV"

#define GO_LS_TEXT_LEVEL 0U
#define GO_LS_TEXT_TIME 1U

#define GO_LS_GUI_LEVEL_PANEL 0U
#define GO_LS_GUI_SETTINGS 1U
#define GO_LS_GUI_PLAY 2U
#define GO_LS_GUI_REPLAY 3U
#define GO_LS_GUI_NEXT 4U
#define GO_LS_GUI_SHARE 5U
#define GO_LS_GUI_EXIT 6U
#define GO_LS_GUI_STAR 100U

#define GO_LS_LEVEL_INIT 0
#define GO_LS_LEVEL_STARTING 1
#define GO_LS_LEVEL_STARTED 2
#define GO_LS_LEVEL_FINISHED 3
#define GO_LS_LEVEL_RESTORING 4

#include <array>
#include <vector>
#include <memory>
#include <chrono>

#include <nlohmann/json.hpp>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Bezier.h>

#include "../GameObject.h"
#include "../Game/Callbacks/IShootCallback.h"
#include "../Graphics/BaseDrawable.h"
#include "OverlayGui.h"
#include "OverlayText.h"
#include "Scenery.h"

using namespace Magnum;

class LevelSelector : public GameObject, public IShootCallback, public std::enable_shared_from_this<LevelSelector>
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	LevelSelector(const Int parentIndex);
	~LevelSelector();

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	void shootCallback(const Int state) override;

private:
	static std::unordered_map<Int, std::array<Vector3, 6>> sLevelButtonPositions;

	struct LS_PickableObject
	{
		std::vector<std::weak_ptr<BaseDrawable>> drawables;
		Vector3 position;
		Float scale;
		bool selectable;
		UnsignedInt levelIndex;
		UnsignedInt objectId;
		Resource<GL::Texture2D> texture;
	};

	struct LS_ScenerySelector
	{
		std::shared_ptr<Scenery> scenery;
		std::vector<LS_PickableObject> buttons;
		Object3D* manipulator;
	};

	struct LS_ScreenButton
	{
		std::shared_ptr<OverlayGui> drawable;
		std::function<void()> callback;
		Float animation; // Factor from 0 to 1
	};

	struct LS_LevelInfo
	{
		UnsignedInt currentViewingLevelId;
		UnsignedInt repeatLevelId;
		UnsignedInt selectedLevelId;
		UnsignedInt maxLevelId;
		Int state;

		std::weak_ptr<GameObject> playerPointer;
		std::weak_ptr<GameObject> limitLinePointer;
	};

	constexpr void manageBackendAnimationVariable(Float & variable, const Float factor, const bool increment);
	void createSkyPlane();
	void handleScrollableCameraPosition(const Vector3 & delta);
	void handleScrollableScenery();
	void clickLevelButton(const UnsignedInt id);

	void windowForCommon();
	void windowForSettings();
	void windowForCurrentLevelView();

	void manageLevelState();
	void createLevelRoom();
	void finishCurrentLevel(const bool success);
	void prepareForReplay();
	void replayCurrentLevel();
	void checkForLevelEnd();

	std::shared_ptr<GameDrawable<Shaders::Flat3D>> mSkyPlane;
	Object3D* mSkyManipulator;

	Vector2i mPrevMousePos;
	Vector3 mScrollVelocity;
	Float mLevelButtonScaleAnim;

	Math::CubicBezier2D<Float> mCbEaseInOut;
	bool mSettingsOpened;
	Float mSettingsAnim;

	Float mLevelStartedAnim;
	bool mLevelEndingAnim;

	std::unordered_map<Int, LS_ScenerySelector> mSceneries;

	Int mClickIndex;
	std::unordered_map<Int, LS_ScreenButton> mScreenButtons;

	std::unordered_map<Int, std::shared_ptr<OverlayGui>> mLevelGuis;
	std::unordered_map<Int, std::shared_ptr<OverlayText>> mLevelTexts;
	Float mLevelAnim;

	std::chrono::system_clock::time_point mClickStartTime;
	std::unordered_map<UnsignedInt, Int> mPickableObjectRefs;

	LS_LevelInfo mLevelInfo;
	Float mLevelGuiAnim;
};