#pragma once

#include <vector>
#include <memory>

#include <Magnum/Trade/MeshData.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>

#include "CommonTypes.h"
#include "IDrawCallback.h"

using namespace Magnum;

class GameObject : public IDrawCallback
{
protected:
	std::shared_ptr<Trade::MeshData> mMeshData;

public:
	GameObject();
	~GameObject();

	virtual void update() = 0;

	std::vector<std::shared_ptr<SceneGraph::Drawable3D>> drawables;

	Float deltaTime;
	Vector3 position;
};