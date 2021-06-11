#pragma once

#include <vector>
#include <memory>
#include <unordered_set>

#include <Magnum/Trade/MeshData.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Math/Range.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>

#include "CommonTypes.h"
#include "BaseDrawable.h"

using namespace Magnum;

class GameObject : public IDrawCallback
{
public:
	GameObject();
	~GameObject();

	bool destroyMe;
	Float mDeltaTime;
	std::unique_ptr<Object3D> mManipulator;
	std::vector<std::shared_ptr<BaseDrawable>> mDrawables;

	Vector3 position;
	Range3D bbox;

	virtual const Int getType() const = 0;
	virtual void update() = 0;
	virtual void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) = 0;
};