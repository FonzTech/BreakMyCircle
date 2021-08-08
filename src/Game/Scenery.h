#pragma once

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>

#include "../GameObject.h"
#include "../Graphics/TexturedDrawable.h"
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
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	const Int getModelIndex() const;
	const void animateInGameCamera();

protected:

	struct WaterDrawableHolder
	{
		std::shared_ptr<TexturedDrawable<WaterShader>> drawable;
		WaterShader::Parameters parameters;
	};

	void createWaterDrawable();
	void createWaterDrawable(const WaterDrawableHolder & fromWdh);

	// Manipulator list
	std::vector<Object3D*> mManipulatorList;

	// Object data
	Int mModelIndex;
	bool mAnimateInGameCamera;

	// Animation
	Float mFrame;
	// std::unique_ptr<CubicBezier2D> mCubicBezier;

	// Water objects
	std::unordered_map<BaseDrawable*, WaterDrawableHolder> mWaterHolders;
};