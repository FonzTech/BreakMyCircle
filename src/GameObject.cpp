#include "GameObject.h"

GameObject::GameObject() : IDrawCallback()
{
}

GameObject::~GameObject()
{
	drawables.clear();
}