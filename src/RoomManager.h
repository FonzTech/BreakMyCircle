#pragma once

#include <memory>
#include <vector>

#include "GameObject.h"
#include "Bubble.h"

class RoomManager
{
public:
	static std::shared_ptr<RoomManager> singleton;

	Vector3 cameraEye, cameraTarget;
	std::vector<std::shared_ptr<GameObject>> mGameObjects;
	Scene3D mScene;
	Object3D mCameraObject;
	std::shared_ptr<SceneGraph::Camera3D> mCamera;
	SceneGraph::DrawableGroup3D mDrawables;

	RoomManager();

	void clear();
	void setupRoom();
	void createTestRoom();
};