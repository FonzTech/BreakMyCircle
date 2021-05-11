#include "GameObject.h"

using namespace Magnum::Math::Literals;

GameObject::GameObject(SceneGraph::DrawableGroup3D& group) : SceneGraph::Drawable3D{ *this, &group }
{
}

GameObject::~GameObject()
{
	if (meshData != nullptr)
	{
		meshData = nullptr;
	}
}