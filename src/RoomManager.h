#pragma once

#define GOL_PERSP_FIRST 0
#define GOL_PERSP_SECOND 1
#define GOL_ORTHO_FIRST 100

#include <memory>
#include <vector>
#include <nlohmann/json.hpp>

#include <Magnum/Magnum.h>
#include <Magnum/Audio/AbstractImporter.h>
#include <Magnum/Audio/Buffer.h>
#include <Magnum/Audio/Context.h>
#include <Magnum/Audio/Listener.h>
#include <Magnum/Audio/Playable.h>
#include <Magnum/Audio/PlayableGroup.h>
#include <Magnum/Audio/Source.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Texture.h>

#include "GameObject.h"
#include "CollisionManager.h"
#include "Audio/StreamedAudioPlayable.h"
#include "Game/Callbacks/IShootCallback.h"

using namespace Magnum;

typedef std::vector<std::shared_ptr<GameObject>> GameObjectList;

class RoomManager
{
public:
	// Game Object layer data holder
	struct GameObjectsLayer
	{
		Int index;
		bool depthTestEnabled;
		bool orderingByZ;
		Matrix4 projectionMatrix;
		Vector3 cameraEye, cameraTarget;
		std::unique_ptr<GL::Framebuffer> frameBuffer;
		std::unique_ptr<GL::Texture2D> colorTexture;
		std::unique_ptr<GL::Texture2D> depthTexture;
		std::unique_ptr<GameObjectList> list;
		std::unique_ptr<SceneGraph::DrawableGroup3D> drawables;

		void push_back(const std::shared_ptr<GameObject> & go)
		{
			list->push_back(std::move(go));
			list->back()->mParentIndex = index;
		}

		std::shared_ptr<GameObject> & push_back(const std::shared_ptr<GameObject> & go, const bool _dummy)
		{
			push_back(go);
			return list->back();
		}
	};

	// Bubble data holder
	struct BubbleData
	{
		Color3 color;
		std::string textureKey;
	};

	// Global game save data
	struct SaveData
	{
		UnsignedInt maxLevelId;
		Int coinTotal;
		Int coinCurrent;
		std::unordered_map<UnsignedInt, Int> powerupAmounts;
	};

	// Singleton
	static std::unique_ptr<RoomManager> singleton;

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

	// Members for randomizer
	std::unordered_map<UnsignedInt, BubbleData> mBubbleColors;

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
	const void setBgMusicGain(const Float level);

	const Float getSfxGain() const;
	const void setSfxGain(const Float level);

	void clear();
	void setup();
	void prepareRoom(const bool stopBgMusic);
	void loadRoom(const std::string & name);
	void createLevelRoom(const std::shared_ptr<IShootCallback> & shootCallback, const Int xlen, const Int ylen, const std::uint32_t seed, const std::int32_t octaves, const double frequency);

protected:
	// Instantiator data holder
	struct Instantiator
	{
		Int key;
		std::unique_ptr<nlohmann::json> params;
	};

	// Methods
	std::unique_ptr<RoomManager::Instantiator> getGameObjectFromNoiseValue(const double value);

	// Window parameters
	Int mCurrentBoundParentIndex;
	Vector2 mWindowSize;
	Float mAspectRatio;

	// Audio parameters
	Float mSfxLevel;
};