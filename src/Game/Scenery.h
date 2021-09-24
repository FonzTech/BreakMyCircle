#pragma once

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>

#include "../GameObject.h"
#include "../Graphics/GameDrawable.h"
#include "../Shaders/WaterShader.h"
#include "../Shaders/SunShader.h"
#include "../Shaders/StarRoadShader.h"

class Scenery : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Scenery(const Int parentIndex, const Int modelIndex, const Int subType = 0);
	~Scenery();

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

	Int getModelIndex() const;
	void setLightPosition(const Vector3 & lightPosition);
	void animateInGameCamera();
	void createFireball();

protected:

	struct ObjectAnimations
	{
		Float inc;
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
	Int mSubType;
	bool mAnimateInGameCamera;
	Vector3 mLightPosition;

	// Animation
	Float mAlphaCheckTimer;
	Float mFrame;
	ObjectAnimations mAnim;

	// Star road
	Resource<GL::AbstractShaderProgram, SunShader> mSunShader;
	Resource<GL::AbstractShaderProgram, StarRoadShader> mStarRoadShader;
	Resource<GL::Texture2D> mSunAlphaMap;
	Resource<GL::Texture2D> mStarRoadAlphaMap;
	std::weak_ptr<BaseDrawable> mStarRoad;
	std::weak_ptr<BaseDrawable> mSun;

	// Water objects
	std::unordered_map<BaseDrawable*, WaterDrawableHolder> mWaterHolders;

	// Wind-animated objects
	std::vector<std::weak_ptr<BaseDrawable>> mWindRotateObjects;
	std::vector<std::weak_ptr<BaseDrawable>> mWindScaleObjects;
};