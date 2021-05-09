#include "GameObject.h"

using namespace Magnum::Math::Literals;

Vector2 GameObject::windowSize({ 1.0f, 1.0f });

GameObject::GameObject()
{
	// Create matrices
	mTransformation = Matrix4::rotationX(30.0_degf) * Matrix4::rotationY(40.0_degf);
	mProjection = Matrix4::perspectiveProjection(35.0_degf, windowSize.aspectRatio(), 0.01f, 100.0f) * Matrix4::translation(Vector3::zAxis(-10.0f));
}

GameObject::~GameObject()
{
	if (meshData != nullptr)
	{
		meshData = nullptr;
	}
}