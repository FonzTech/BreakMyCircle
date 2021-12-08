#include "GameObject.h"
#include "RoomManager.h"

GameObject::GameObject() : IDrawCallback()
{
	// Init members
	mDestroyMe = false;
	mDeltaTime = 0.0f;

	// Create manipulator
	mManipulator = new Object3D{ &RoomManager::singleton->mScene };
}

GameObject::GameObject(const Int parentIndex) : GameObject()
{
	mParentIndex = parentIndex;
}

GameObject::~GameObject()
{
	// Remove all drawables from its layer
	for (const auto& d : mDrawables)
	{
		auto* p = d->group();
		if (p != nullptr)
		{
			p->remove(*d);
		}
	}

	// Clear references
	mDrawables.clear();
	mPlayables.clear();

	// De-reference manipulator
	mManipulator->parent()->erase(mManipulator);
	mManipulator = nullptr;
}

const void GameObject::playSfxAudio(const Int index, const Float offset)
{
	const Float level = RoomManager::singleton->getSfxGain();
	(*mPlayables[index])
		.setGain(level)
		.source()
			.setOffsetInSeconds(offset)
			.play();
}

const void GameObject::pushToFront()
{
	for (auto i = 0; i < mDrawables.size(); ++i)
	{
		mDrawables[i]->pushToFront();
	}
}