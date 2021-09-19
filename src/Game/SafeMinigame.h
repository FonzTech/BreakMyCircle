#pragma once

#include <nlohmann/json.hpp>

#include "OverlayText.h"

class SafeMinigame : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	SafeMinigame(const Int parentIndex, const Float startingScale = 0.0f);
	~SafeMinigame();

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	void setupCamera();

protected:
	void animateTextAlpha(const UnsignedInt index, const bool increment);
	void obtainPowerup();

	Object3D* mGlowManipulator;
	Object3D* mSafeManipulator;
	Object3D* mPowerupManipulator;

	UnsignedInt mPowerupIndex;
	Int mMode;
	Float mGlowAnim;
	Float mScale;
	Float mAnimY;
	Float mAngleHandle;
	Float mAngleDoor;
	Float mPuRotation;
	Float mPuAnimation;

	// Drawables
	std::weak_ptr<BaseDrawable> mDrawableGlow;
	std::weak_ptr<BaseDrawable> mDrawableKnob;
	std::weak_ptr<BaseDrawable> mDrawableHandle;
	std::weak_ptr<BaseDrawable> mDrawableDoor;

	// Texts
	std::shared_ptr<OverlayText> mTexts[1];
};