#include "GameObject.h"
#include "RoomManager.h"

GameObject::GameObject() : IDrawCallback()
{
	mDestroyMe = false;
	mDeltaTime = 0.0f;
	mManipulator = std::make_unique<Object3D>(&RoomManager::singleton->mScene);
}

GameObject::GameObject(const Int parentIndex) : GameObject()
{
	mParentIndex = parentIndex;
}

GameObject::~GameObject()
{
	/*
	// Remove all drawables from its layer
	for (const auto& d : mDrawables)
	{
		RoomManager::singleton->mGoLayers[mParentIndex].drawables->remove(*d);
	}
	*/

	// Clear references
	mDrawables.clear();
	mPlayables.clear();

	// De-reference manipulator
	mManipulator = nullptr;
}