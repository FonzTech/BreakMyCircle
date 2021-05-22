#pragma once

#include <vector>
#include <memory>

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
protected:
	std::shared_ptr<Trade::MeshData> mMeshData;

public:
	GameObject();
	~GameObject();

	bool destroyMe;
	Float deltaTime;
	std::vector<std::shared_ptr<BaseDrawable>> drawables;

	Vector3 position;
	Range3D bbox;

	virtual void update() = 0;
	virtual void collidedWith(GameObject* gameObject) = 0;
};