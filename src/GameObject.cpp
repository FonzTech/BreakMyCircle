#include "GameObject.h"

GameObject::GameObject() : IDrawCallback()
{
	destroyMe = false;
}

GameObject::~GameObject()
{
	drawables.clear();
}