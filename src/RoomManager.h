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
	static std::unique_ptr<RoomManager> singleton;

	// Function creator mapper for room loader
	std::unordered_map<Uint8, std::function<std::shared_ptr<GameObject>(nlohmann::json params)>> gameObjectCreators;

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

	Vector2i windowSize;

	explicit RoomManager();

	void clear();
	void setup();
	void prepareRoom();
	void loadRoom(const std::string & name);
	void createTestRoom();
};