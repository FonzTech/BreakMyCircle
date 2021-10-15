#pragma once

#define GO_LS_TEXT_LEVEL 0U
#define GO_LS_TEXT_TIME 1U
#define GO_LS_TEXT_COIN 2U
#define GO_LS_TEXT_HELP 10U
#define GO_LS_TEXT_POWERUP_TITLE 3U
#define GO_LS_TEXT_POWERUP_ICON 1000U
#define GO_LS_TEXT_POWERUP_PRICE 1500U
#define GO_LS_TEXT_VOTE_ME 200U
#define GO_LS_TEXT_OTHER_APPS 201U

#define GO_LS_GUI_LEVEL_PANEL 0U
#define GO_LS_GUI_SETTINGS 1U
#define GO_LS_GUI_PLAY 2U
#define GO_LS_GUI_REPLAY 3U
#define GO_LS_GUI_NEXT 4U
#define GO_LS_GUI_SHARE 5U
#define GO_LS_GUI_EXIT 6U
#define GO_LS_GUI_COIN 7U
#define GO_LS_GUI_TIME 8U
#define GO_LS_GUI_BGMUSIC 9U
#define GO_LS_GUI_SFX 10U
#define GO_LS_GUI_WHITEGLOW 11U
#define GO_LS_GUI_SCROLL_BACK 12U
#define GO_LS_GUI_EMOJI 13U
#define GO_LS_GUI_HELP 14U
#define GO_LS_GUI_STAR 100U
#define GO_LS_GUI_POWERUP 1000U

#define GO_LS_LEVEL_INIT 0
#define GO_LS_LEVEL_STARTING 1
#define GO_LS_LEVEL_STARTED 2
#define GO_LS_LEVEL_FINISHED 3
#define GO_LS_LEVEL_RESTORING 4
#define GO_LS_LEVEL_SAFE_MINIGAME 5

#define GO_LS_AUDIO_WIN 1
#define GO_LS_AUDIO_LOSE 2
#define GO_LS_AUDIO_POWERUP 3
#define GO_LS_AUDIO_PAUSE_IN 4
#define GO_LS_AUDIO_PAUSE_OUT 5
#define GO_LS_AUDIO_EXPLOSION 6
#define GO_LS_AUDIO_COIN 7
#define GO_LS_AUDIO_WRONG 8
#define GO_LS_AUDIO_WRONG 8
#define GO_LS_AUDIO_TIME 9
#define GO_LS_AUDIO_STAR 1000

#define GO_LS_MAX_POWERUP_COUNT 4

#define GO_LS_INTENT_GP_EXPIRE "game_powerup_expire"
#define GO_LS_INTENT_GP_AMOUNT "game_powerup_amount"
#define GO_LS_INTENT_PLAY_AD_THRESHOLD "play_ad_threshold"

#define GO_LS_METHOD_CLEAR_POWERUP_DATA "clearPowerupData"
#define GO_LS_METHOD_WATCH_AD_POWERUP "watchAdForPowerup"
#define GO_LS_METHOD_SHOW_INTERSTITIAL "showInterstitial"
#define GO_LS_METHOD_GAME_VOTE_ME "gameVoteMe"
#define GO_LS_METHOD_GAME_OTHER_APPS "gameOtherApps"

#include <array>
#include <vector>
#include <memory>
#include <chrono>

#include <nlohmann/json.hpp>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Bezier.h>

#include "../GameObject.h"
#include "Dialog.h"
#include "Onboarding.h"
#include "MapPickup.h"
#include "LevelSelectorSidecar.h"
#include "Callbacks/IShootCallback.h"
#include "../Graphics/BaseDrawable.h"
#include "OverlayGui.h"
#include "OverlayText.h"
#include "Scenery.h"

#define GO_LS_SCENERY_LENGTH 50.0f
#define GO_LS_CLICK_TAP_MAX_DELAY 0.3

#if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
#define GO_LS_MAX_SCROLL_VELOCITY 2.0f
#else
#define GO_LS_MAX_SCROLL_VELOCITY 0.5f
#endif

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

	void shootCallback(const Int state, const Color3 & preColor, const Color3 & postColor, const Int amount) override;

private:
	static std::unordered_map<Int, std::array<Vector3, 6>> sLevelButtonPositions;

	class LS_PickableObject
	{
	public:
		explicit LS_PickableObject();
		~LS_PickableObject();
		
		std::weak_ptr<LevelSelectorSidecar> sidecar;
		Vector3 position;
		Float scale;
		UnsignedInt levelIndex;
		UnsignedInt objectId;
		Resource<GL::Texture2D> texture;
	};
	
	class LS_ScenerySelector
	{
	public:
		~LS_ScenerySelector();

		std::weak_ptr<Scenery> scenery;
		std::vector<std::shared_ptr<LS_PickableObject>> buttons;
	};

	class LS_ScreenButton
	{
	public:
		explicit LS_ScreenButton();
		~LS_ScreenButton();

		std::shared_ptr<AbstractGuiElement> drawable;
		std::function<bool(UnsignedInt)> callback;
	};

	struct LS_PickableObjectRef
	{
		Int sceneryIndex;
	};

	struct LS_LevelInfo
	{
		UnsignedInt currentViewingLevelId;
		UnsignedInt repeatLevelId;
		UnsignedInt selectedLevelId;
		Int state;
		Int numberOfRetries;
		Int numberOfPlays;
		Int score;
		Int playedScore;
		Float difficulty;
		Float startingTime;
		Vector3 lastLevelPos;
		bool isSafeMinigameDone;

		std::weak_ptr<GameObject> playerPointer;
		std::weak_ptr<GameObject> limitLinePointer;

		bool delayedChecks;
		Vector3 currentLevelPos, nextLevelPos;
		bool success;
	};

	struct LS_PickupHandler
	{
		Float timer;
		std::unordered_map<UnsignedInt, std::weak_ptr<MapPickup>> pickups;
	};

	template <typename S, typename T>
	struct LS_CachedVariable
	{
		S value;
		T cached;
	};

	struct LS_PowerupView
	{
		Containers::Optional<Int> startX;
		Float scrollX;
		std::unordered_map<UnsignedInt, LS_CachedVariable<Int, Int>> counts;
		std::unordered_map<UnsignedInt, Int> prices;
	};

	struct LS_Scroll
	{
		Containers::Optional<Vector2i> prevMousePos;
		Vector3 release;
		Vector3 velocity;
		Float factor;
        bool disableObjectPicking;

#ifdef TARGET_MOBILE
		Float touchTimer;
		Containers::Optional<Vector3> touchVelocity;
#endif
	};

	constexpr void manageBackendAnimationVariable(Float & variable, const Float factor, const bool increment);
	void createSkyPlane();
	void handleScrollableCameraPosition(const Vector3 & delta);
	void handleScrollableScenery();
	void clickLevelButton(const LS_ScenerySelector * sc, const LS_PickableObject * po);
	const std::string getHelpTipText(const Int index) const;

	void windowForCommon();
	void windowForSettings();
	void windowForCurrentLevelView();

	void manageLevelState();
	void createLevelRoom();
	void finishCurrentLevel(const bool success);
	void prepareForReplay();
	void replayCurrentLevel();
	void checkForLevelEnd();
	void startLevel(const UnsignedInt levelId);
	Int computeScore();
	Int getModelIndex(const Int yp);
	Vector3 getLastLevelPos();
	bool isTimeExpiring(const Float time);

	void manageGuiLevelAnim(const UnsignedInt index, const bool increment, const Float factor = 1.0f);
	void updateTimeCounter(const Int value);
	void closeDialog();

	void createPowerupView();
	void usePowerup(const UnsignedInt index);
	void watchAdForPowerup(const UnsignedInt index);
	void showInterstitial();

	void createGuis();
	void createTexts();

	void callNativeMethod(const std::string & methodName);

	Vector2 getSquareOffset(const Float size);

	Float getScaledVerticalPadding();
	Float getWidthReferenceFactor();

	std::weak_ptr<Dialog> mDialog;
	std::weak_ptr<Onboarding> mOnboarding;

#ifdef GO_LS_SKY_PLANE_ENABLED
	std::shared_ptr<GameDrawable<Shaders::Flat3D>> mSkyPlane;
	Object3D* mSkyManipulator;
#endif

	LS_Scroll mScrolling;
	Float mLevelButtonScaleAnim;

	Math::CubicBezier2D<Float> mCbEaseInOut;
	bool mSettingsOpened;
	Float mSettingsAnim;

	Float mLevelStartedAnim;
	bool mLevelEndingAnim;
	Float mHelpTipsTimer;
	bool mDisplayMiniOnboarding;

	std::unordered_map<Int, LS_ScenerySelector> mSceneries;

	Int mClickIndex;
	Float mClickTimer; // Timer for touch screens, because often fingers make small mouse moves when tapping
	std::unordered_map<UnsignedInt, std::unique_ptr<LS_ScreenButton>> mScreenButtons;

	std::unordered_map<UnsignedInt, std::shared_ptr<OverlayGui>> mLevelGuis;
	std::unordered_map<UnsignedInt, std::shared_ptr<OverlayText>> mLevelTexts;
	Float mLevelAnim;

	std::chrono::system_clock::time_point mClickStartTime;
	std::unordered_map<UnsignedInt, Int> mPickableObjectRefs;

	LS_LevelInfo mLevelInfo;
	Float mLevelGuiAnim[6];

	LS_CachedVariable<Float, Int> mTimer;
	LS_CachedVariable<Float, Int> mCoins;

	LS_PowerupView mPuView;
	LS_PickupHandler mPickupHandler;

	UnsignedInt mWatchForPowerup;
};
