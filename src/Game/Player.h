#pragma once

#define SHOOT_ANGLE_MIN_RAD -2.79253f
#define SHOOT_ANGLE_MAX_RAD -0.349066f

#include <vector>
#include <nlohmann/json.hpp>
#include <Magnum/Math/Color.h>
#include <Magnum/Timeline.h>

#include "../Common/CustomRenderers/PlasmaSquareRenderer.h"
#include "../GameObject.h"
#include "../Game/Callbacks/IShootCallback.h"
#include "../Game/ElectricBall.h"
#include "../Graphics/BaseDrawable.h"

class Player : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Player(const Int parentIndex);
	Player(const Int parentIndex, const std::shared_ptr<IShootCallback> & shootCallback);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

	void postConstruct();
	void setupProjectile(const Int index);
	void setPrimaryProjectile(const Color3 & color);

	// Class members
	bool mCanShoot;
	Float mCameraDist;

	// Optionals
	std::weak_ptr<IShootCallback> mShootCallback;

protected:
	std::unique_ptr<std::vector<Color3>> getRandomEligibleColor(const UnsignedInt times);
	Resource<GL::Texture2D> getTextureResourceForIndex(const UnsignedInt index);

	PlasmaSquareRenderer mPlasmaSquareRenderer;
	std::shared_ptr<ElectricBall> mElectricBall;

	// std::unique_ptr<LinePath> mProjPath;
	std::weak_ptr<GameObject> mProjectile;
	Float mShootTimeline;
	Rad mShootAngle;

	Color3 mProjColors[2];
	Object3D* mShooterManipulator;
	Object3D* mSphereManipulator[2];
	Object3D* mBombManipulator;
	Object3D* mSwapManipulator;
	Object3D* mShootPathManipulator[2];

	BaseDrawable* mSphereDrawables[2];
	BaseDrawable* mBombDrawables[3];
	std::unordered_set<BaseDrawable*> mFlatDrawables;
	std::unordered_set<BaseDrawable*> mShootPathDrawables;

	bool mIsSwapping;
	bool mSwapRequest;

	std::array<Float, 5> mAnimation;
	std::array<Float, 2> mAimLength;
	std::array<Rad, 2> mAimAngle;
	std::array<Float, 2> mAimCos;
	std::array<Float, 2> mAimSin;
	Vector3 mSecondPos;
	Float mAimTimer;

	// Methods 
	Range2Di getBubbleSwapArea();
	Float getFixedAtan(const Rad & angle);
};