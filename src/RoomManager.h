#pragma once

#define GOL_PERSP_FIRST 0
#define GOL_PERSP_SECOND 1
#define GOL_ORTHO_FIRST 2

#define GO_RM_SD_FLAG_FIRST_SAFE 1U << 0
#define GO_RM_SD_FLAG_ONBOARDING_A 1U << 1
#define GO_RM_SD_FLAG_ONBOARDING_B 1U << 2
#define GO_RM_SD_FLAG_ONBOARDING_C 1U << 3
#define GO_RM_SD_ONBOARDING_INIT_MAX 2

#include <memory>
#include <unordered_set>
#include <nlohmann/json.hpp>

#include <Magnum/Magnum.h>
#include <Magnum/Image.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/Audio/AbstractImporter.h>
#include <Magnum/Audio/Buffer.h>
#include <Magnum/Audio/Context.h>
#include <Magnum/Audio/Listener.h>
#include <Magnum/Audio/Playable.h>
#include <Magnum/Audio/PlayableGroup.h>
#include <Magnum/Audio/Source.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/TextureFormat.h>

#include "GameObject.h"
#include "CollisionManager.h"
#include "Audio/StreamedAudioPlayable.h"
#include "Game/Callbacks/IShootCallback.h"

using namespace Magnum;

typedef std::unordered_set<std::shared_ptr<GameObject>> GameObjectList;

class RoomManager
{
public:
	// Save data structure
	class SaveData
	{
	public:
		SaveData();
		~SaveData();

		bool load();
		bool save();

		UnsignedInt flags;
		UnsignedInt maxLevelId;
		Int coinTotal;
		Int coinCurrent;
		std::unordered_map<UnsignedInt, Int> powerupAmounts;
		std::unordered_map<UnsignedInt, Int> levelScores;

		Int onboardIndex;
		bool musicEnabled;
		bool sfxEnabled;
	};

	// Game Object layer data holder
	struct GameObjectsLayer
	{
		Int index;
		bool depthTestEnabled;
		bool orderingByZ;
		bool updateEnabled;
		bool drawEnabled; // If draw is disabled, framebuffer is NOT cleared
		Matrix4 projectionMatrix;
		Vector3 cameraEye, cameraTarget;
		std::unique_ptr<GL::Framebuffer> frameBuffer;
		std::unique_ptr<GL::Texture2D> colorTexture;
		std::unique_ptr<GL::Texture2D> depthTexture;
		std::unique_ptr<GL::Renderbuffer> objectIdBuffer;
		std::unique_ptr<GameObjectList> list;
		std::unique_ptr<SceneGraph::DrawableGroup3D> drawables;

		void push_back(const std::shared_ptr<GameObject> & go)
		{
			go->mParentIndex = index;
			list->insert(go);
		}
	};

	// Bubble data holder
	struct BubbleData
	{
		Color3 color;
		std::string textureKey;
	};

	// Singleton
	static std::unique_ptr<RoomManager> singleton;

	// Static members
	static std::unordered_map<UnsignedInt, BubbleData> sBubbleColors;
	static std::array<UnsignedInt, 7U> sBubbleKeys;

	// Function creator mapper for room loader
	std::unordered_map<Int, std::function<std::shared_ptr<GameObject>(const nlohmann::json & params)>> gameObjectCreators;

	// Scene
	Scene3D mScene;

	// Camera
	Object3D mCameraObject;
	std::shared_ptr<SceneGraph::Camera3D> mCamera;

	// Game Objects and Drawables
	std::unordered_map<Int, GameObjectsLayer> mGoLayers;

	// Sound manager
	std::unique_ptr<Audio::Context> mAudioContext;
	std::unique_ptr<Audio::Listener3D> mAudioListener;
	Audio::PlayableGroup3D mAudioPlayables;

	// Background music
	std::unique_ptr<StreamedAudioPlayable> mBgMusic;

	// Collision Manager
	std::unique_ptr<CollisionManager> mCollisionManager;
    
#if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
    GL::Framebuffer* mDefaultFramebufferPtr; // This pointer is completely unmanaged, be careful
#endif

	// Game save data
	SaveData mSaveData;

	// Class methods
	explicit RoomManager();
	~RoomManager();

	const Float getWindowAspectRatio() const;

	const Vector2 getWindowSize();
	const void setWindowSize(const Vector2 & size);

	const Int getCurrentBoundParentIndex() const;
	const void setCurrentBoundParentIndex(const Int parentIndex);

	const Float getBgMusicGain() const;
	const bool setBgMusicGain(const Float level);

	const Float getSfxGain() const;
	const void setSfxGain(const Float level);

	void clear();
	void setup();
	void prepareRoom(const bool stopBgMusic);
	void loadRoom(const std::string & name);
	void createLevelRoom(const std::shared_ptr<IShootCallback> & shootCallback, const Int xlen, const Int ylen, const std::uint32_t seed, const std::int32_t octaves, const double frequency);
	void fixLevelTransparency();

protected:
	// Instantiator data holder
	struct Instantiator
	{
		Int key;
		std::unique_ptr<nlohmann::json> params;
	};

	// Methods
	std::unique_ptr<RoomManager::Instantiator> getGameObjectFromNoiseValue(const std::uint32_t seed, const double value);

	// Window parameters
	Int mCurrentBoundParentIndex;
	Vector2 mWindowSize;
	Float mAspectRatio;

	// Audio parameters
	Float mSfxLevel;

	// Methods
	Float isInRange(const double source, const double dest, const double range = 0.01);
};
