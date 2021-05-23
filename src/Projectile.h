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
	void update() override;
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(GameObject* gameObject) override;

	void updateBBox();
	Int getRowIndexByBubble();
	Float getSnappedYPos();

	Color3 mDiffuseColor;
	std::shared_ptr<ColoredDrawable> mColoredDrawable;

	Float mLeftX, mRightX;
	Float mSpeed;
};
