#pragma once

#define SHOOT_ANGLE_MIN_RAD -2.79253f
#define SHOOT_ANGLE_MAX_RAD -0.349066f

#include <vector>
#include <nlohmann/json.hpp>
#include <Magnum/Math/Color.h>
#include <Magnum/Timeline.h>

#include "../GameObject.h"
#include "../Game/Callbacks/IShootCallback.h"
#include "../Graphics/BaseDrawable.h"
#include "../Common/LinePath.h"

class Player : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Player(const Int parentIndex);
	Player(const Int parentIndex, const std::shared_ptr<IShootCallback> & shootCallback);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	void setPrimaryProjectile(const Color3 & color);

	// Class members
	bool mCanShoot;
	Float mCameraDist;

	// Optionals
	std::weak_ptr<IShootCallback> mShootCallback;

protected:

	std::unique_ptr<std::vector<Color4>> getRandomEligibleColor(const UnsignedInt times);
	Resource<GL::Texture2D> getTextureResourceForIndex(const UnsignedInt index);

	// std::unique_ptr<LinePath> mProjPath;
	std::weak_ptr<GameObject> mProjectile;
	Float mShootTimeline;
	Rad mShootAngle;

	Color4 mProjColors[2];
	Resource<GL::Texture2D> mProjTextures[2];

	Object3D* mShooterManipulator;
	Object3D* mSphereManipulator[2];
	Object3D* mBombManipulator;

	BaseDrawable* mSphereDrawables[2];
	BaseDrawable* mBombDrawables[3];

	Float mAnimation[2];
};