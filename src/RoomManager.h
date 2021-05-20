#pragma once

#include <memory>
#include <vector>

#include "GameObject.h"

class RoomManager
{
public:
	static std::shared_ptr<RoomManager> singleton;

	Vector3 mCameraEye, mCameraTarget;
	std::vector<std::shared_ptr<GameObject>> mGameObjects;
	Scene3D mScene;
	Object3D mCameraObject;
	std::shared_ptr<SceneGraph::Camera3D> mCamera;
	SceneGraph::DrawableGroup3D mDrawables;

	Vector2i windowSize;

	explicit RoomManager();

	void clear();
	void setupRoom();
	void createTestRoom();
};