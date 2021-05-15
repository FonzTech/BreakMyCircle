#pragma once

#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>

#include "GameObject.h"

class Player : public GameObject
{
public:
	Player(SceneGraph::DrawableGroup3D& group);

	Color3 mAmbientColor;

protected:
	void update() override;
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

	GL::Mesh mMesh;
	Shaders::Phong mShader;
	Color3 mDiffuseColor;
};