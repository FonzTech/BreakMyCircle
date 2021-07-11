#pragma once

#define GOL_FIRST 0
#define GOL_SECOND 1

#include <memory>
#include <vector>
#include <unordered_map>
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

using namespace Magnum;

typedef std::vector<std::shared_ptr<GameObject>> GameObjectList;

class RoomManager
{
public:
	// Instantiator data holder
	struct Instantiator
	{
		UnsignedInt key;
		std::unique_ptr<nlohmann::json> params;
	};

	// Game Object layer data holder
	struct GameObjectsLayer
	{
		Int index;
		Vector3 mCameraEye, mCameraTarget;
		std::unique_ptr<GL::Framebuffer> frameBuffer;
		std::unique_ptr<GL::Texture2D> fbTexture;
		std::unique_ptr<GameObjectList> list;
		std::unique_ptr<SceneGraph::DrawableGroup3D> drawables;

		void push_back(const std::shared_ptr<GameObject> & go)
		{
			list->push_back(std::move(go));
			list->back()->mParentIndex = index;
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

	// Function creator mapper for room loader
	std::unordered_map<UnsignedInt, std::function<std::shared_ptr<GameObject>(const nlohmann::json & params)>> gameObjectCreators;

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
	std::uint32_t mSeed;
	std::unordered_map<UnsignedInt, BubbleData> mBubbleColors;

	// Window size
	Vector2i windowSize;

	// Class methods
	explicit RoomManager();
	~RoomManager();

	void clear();
	void setup();
	void prepareRoom(const bool stopBgMusic);
	void loadRoom(const std::string & name);
	void createLevelRoom();
	Instantiator getGameObjectFromNoiseValue(const double value);
};