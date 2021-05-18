#pragma once

#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>

#include "GameObject.h"
#include "ColoredDrawable.h"

class Bubble : public GameObject
{
public:
	Bubble(Color3& ambientColor);

	Color3 mAmbientColor;
	Color3 mDiffuseColor;

	std::shared_ptr<ColoredDrawable> cd;

private:
	void update() override;
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
};