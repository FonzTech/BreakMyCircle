#include "GameObject.h"

GameObject::GameObject()
{
}

GameObject::~GameObject()
{
	/*
	while (!drawables.empty())
	{
		std::shared_ptr<SceneGraph::Drawable3D> back = drawables.back();
		drawables.pop_back();

		RoomManager::singleton->mDrawables.remove(*back);
	}
	*/
}