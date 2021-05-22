#pragma once

#include <Magnum/Math/Color.h>

#include "GameObject.h"
#include "ColoredDrawable.h"

class Projectile : public GameObject
{
public:
	Projectile(Color3& ambientColor);

	Color3 mAmbientColor;
	Vector3 velocity;

protected:
	Color3 mDiffuseColor;
	std::shared_ptr<ColoredDrawable> cd;

	void update() override;
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
};
