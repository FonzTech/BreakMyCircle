#pragma once

#include <Magnum/Math/Color.h>

#include "GameObject.h"

class Player : public GameObject
{
public:
	Player();

	Color3 mAmbientColor;

protected:
	Int getType() override;
	void update() override;
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(GameObject* gameObject) override;

	Color3 mDiffuseColor;
};