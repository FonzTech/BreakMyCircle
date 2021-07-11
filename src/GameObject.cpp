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
	mDrawables.clear();
	mPlayables.clear();
	mManipulator = nullptr;
}