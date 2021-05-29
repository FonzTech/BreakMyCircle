#pragma once

#define SHOOT_ANGLE_MIN_RAD -2.79253f
#define SHOOT_ANGLE_MAX_RAD -0.0349066f

#include <vector>
#include <Magnum/Math/Color.h>

#include "GameObject.h"

class Player : public GameObject
{
public:
	Player();

protected:
	Int getType() override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(GameObject* gameObject) override;

	Rad mShootAngle;

	Int mAmbientColorIndex;
	Color3 mDiffuseColor;
	std::vector<Color3> mColors;
};