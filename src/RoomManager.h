#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>

#include "GameObject.h"
#include "CollisionManager.h"

class RoomManager
{
public:
	// Instantiator data holder
	struct InstantiatorDataHolder
	{
		Uint8 key;
		std::unique_ptr<nlohmann::json> params;
	};

	// Singleton
	static std::unique_ptr<RoomManager> singleton;

	// Function creator mapper for room loader
	std::unordered_map<Uint8, std::function<std::shared_ptr<GameObject>(const nlohmann::json & params)>> gameObjectCreators;

	// Scene
	Scene3D mScene;

	// Camera
	Vector3 mCameraEye, mCameraTarget;
	Object3D mCameraObject;
	std::shared_ptr<SceneGraph::Camera3D> mCamera;

	// Game Objects and Drawables
	std::vector<std::shared_ptr<GameObject>> mGameObjects;
	SceneGraph::DrawableGroup3D mDrawables;

	// Collision Manager
	std::unique_ptr<CollisionManager> mCollisionManager;

	// Members for randomizer
	std::uint32_t mSeed;
	std::vector<Color3> mBubbleColors;

	// Window size
	Vector2i windowSize;

	// Class methods
	explicit RoomManager();

	void clear();
	void setup();
	void prepareRoom();
	void loadRoom(const std::string & name);
	void createRoom();
	InstantiatorDataHolder getGameObjectFromNoiseValue(const double value);
};