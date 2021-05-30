#include "GameObject.h"
#include "RoomManager.h"

GameObject::GameObject() : IDrawCallback()
{
	destroyMe = false;
	mManipulator = std::make_unique<Object3D>(&RoomManager::singleton->mScene);
}

GameObject::~GameObject()
{
	drawables.clear();
}