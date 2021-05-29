#pragma once

#define IMPOSSIBLE_PROJECTILE_XPOS -100.0f

#include <Magnum/Math/Color.h>

#include "GameObject.h"
#include "ColoredDrawable.h"

class Projectile : public GameObject
{
public:
	Projectile(const Color3& ambientColor);

	void adjustPosition();

	Color3 mAmbientColor;
	Vector3 mVelocity;

protected:
	Int getType() override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(GameObject* gameObject) override;

	void updateBBox();
	Int getRowIndexByBubble();
	Float getSnappedYPos();

	Color3 mDiffuseColor;

	Float mLeftX, mRightX;
	Float mSpeed;
};
