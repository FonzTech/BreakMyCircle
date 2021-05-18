#pragma once

#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>

#include "GameObject.h"

class Player : public GameObject
{
public:
	Player();

	Color3 mAmbientColor;

protected:
	void update() override;
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

	Color3 mDiffuseColor;
};