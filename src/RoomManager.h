#pragma once

#include <memory>
#include <vector>

#include "GameObject.h"
#include "CollisionManager.h"

class RoomManager
{
public:
	static std::unique_ptr<RoomManager> singleton;

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
	void setupRoom();
	void createTestRoom();
};