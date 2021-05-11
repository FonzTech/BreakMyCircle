#pragma once

#include <Magnum/Trade/MeshData.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>

#include "CommonTypes.h"

using namespace Magnum;

class GameObject : public Object3D, SceneGraph::Drawable3D
{
protected:
	std::shared_ptr<Trade::MeshData> meshData;

public:
	GameObject(SceneGraph::DrawableGroup3D& group);
	~GameObject();

	virtual void update() = 0;
	virtual void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) = 0;

	Float deltaTime;
	Vector3 position;
};