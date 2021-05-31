#pragma once

#define SHOOT_ANGLE_MIN_RAD -2.79253f
#define SHOOT_ANGLE_MAX_RAD -0.349066f

#include <vector>
#include <Magnum/Math/Color.h>
#include <Magnum/Timeline.h>

#include "GameObject.h"
#include "BaseDrawable.h"

class Player : public GameObject
{
public:
	Player();

protected:
	Int getType() override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	Float mShootTimeline;
	Rad mShootAngle;

	Int mAmbientColorIndex;
	Color3 mDiffuseColor;
	std::vector<Color3> mColors;

	Object3D* mSphereManipulator;
	BaseDrawable* mSphereDrawables[1];
};