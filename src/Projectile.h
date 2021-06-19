#pragma once

#define IMPOSSIBLE_PROJECTILE_XPOS -100.0f

#include <nlohmann/json.hpp>
#include <Magnum/Math/Color.h>

#include "GameObject.h"
#include "ColoredDrawable.h"

class Projectile : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Projectile(const Color3& ambientColor);

	void adjustPosition();

	Color3 mAmbientColor;
	Vector3 mVelocity;

protected:
	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	void snapToGrid();
	void updateBBox();

	Int getRowIndexByBubble();
	Float getSnappedYPos();

	Color3 mDiffuseColor;

	Float mLeftX, mRightX;
	Float mSpeed;
};
