#pragma once

#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>

#include "GameObject.h"
#include "TexturedDrawable.h"

class Bubble : public GameObject, public Object3D
{
public:
	Bubble(SceneGraph::DrawableGroup3D& group);

	Color4 mAmbientColor;

protected:
	void update() override;
	// void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera);

	// GL::Mesh mMesh;
	Shaders::Phong mShader;
	Color4 mDiffuseColor;
};