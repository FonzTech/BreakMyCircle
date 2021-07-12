#pragma once

#define SHOOT_ANGLE_MIN_RAD -2.79253f
#define SHOOT_ANGLE_MAX_RAD -0.349066f

#include <vector>
#include <nlohmann/json.hpp>
#include <Magnum/Math/Color.h>
#include <Magnum/Timeline.h>

#include "../GameObject.h"
#include "../Graphics/BaseDrawable.h"
#include "../Common/LinePath.h"

class Player : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Player(const Int parentIndex);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

protected:

	std::unique_ptr<std::vector<Color4>> getRandomEligibleColor(const UnsignedInt times);
	Resource<GL::Texture2D> getTextureResourceForIndex(const UnsignedInt index);

	std::weak_ptr<GameObject> mProjectile;
	std::unique_ptr<LinePath> mProjPath;
	Float mShootTimeline;
	Rad mShootAngle;

	Color4 mProjColors[2];
	Resource<GL::Texture2D> mProjTextures[2];

	Object3D* mShooterManipulator;
	Object3D* mSphereManipulator;

	BaseDrawable* mSphereDrawables[1];
};