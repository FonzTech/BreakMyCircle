#pragma once

#define IMPOSSIBLE_PROJECTILE_XPOS -100.0f

#include <nlohmann/json.hpp>
#include <Magnum/Math/Color.h>

#include "../GameObject.h"
#include "../Game/Callbacks/IShootCallback.h"
#include "../Game/ElectricBall.h"
#include "../Graphics/GameDrawable.h"

class Projectile : public GameObject
{
public:
	static void setGlobalParameters(const Float leftX, const Float rightX);

	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Projectile(const Int parentIndex, const Color3& ambientColor);
	~Projectile();

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects);
	void adjustPosition();
	void setCustomTexture(GL::Texture2D & texture);

	// Class members
	Color3 mAmbientColor;
	Vector3 mVelocity;

	// Optionals
	std::weak_ptr<IShootCallback> mShootCallback;

protected:
	static Float LEFT_X, RIGHT_X, MID_X;

	void snapToGrid(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects);
	void updateBBox();

	const Int getRowIndexByBubble() const;
	const Float getSnappedYPos() const;
	const Float getRadiusForExplosion() const;
	const void playStompSound();

	std::shared_ptr<ElectricBall> mElectricBall;

	Color3 mDiffuseColor;
	GL::Texture2D* mCustomTexture;

	Float mSpeed;
	Float mAnimation;
};
