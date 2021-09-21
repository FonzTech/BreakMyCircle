#pragma once

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>

#include "../GameObject.h"
#include "../Graphics/GameDrawable.h"
#include "../Shaders/WaterShader.h"

class Scenery : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Scenery(const Int parentIndex, const Int modelIndex);
	~Scenery();

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

	const Int getModelIndex() const;
	const void setLightPosition(const Vector3 & lightPosition);
	const void animateInGameCamera();

protected:

	struct ObjectAnimations
	{
		Float frame;
		Float rotateFactor;
		Float scaleFactor;
	};

	struct WaterDrawableHolder
	{
		std::shared_ptr<GameDrawable<WaterShader>> drawable;
		WaterShader::Parameters parameters;
	};

	void createWaterDrawable();
	void createWaterDrawable(const WaterDrawableHolder & fromWdh);

	// Manipulator list
	std::vector<Object3D*> mManipulatorList;

	// Object data
	Int mModelIndex;
	bool mAnimateInGameCamera;
	Vector3 mLightPosition;

	// Animation
	Float mAlphaCheckTimer;
	Float mFrame;
	ObjectAnimations mAnim;
	// std::unique_ptr<CubicBezier2D> mCubicBezier;

	// Water objects
	std::unordered_map<BaseDrawable*, WaterDrawableHolder> mWaterHolders;

	// Wind-animated objects
	std::vector<std::weak_ptr<BaseDrawable>> mWindRotateObjects;
	std::vector<std::weak_ptr<BaseDrawable>> mWindScaleObjects;
};