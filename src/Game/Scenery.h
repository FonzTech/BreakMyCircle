#pragma once

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <Magnum/Math/Bezier.h>

#include "../GameObject.h"
#include "../Graphics/TexturedDrawable.h"
#include "../Shaders/WaterShader.h"

class Scenery : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Scenery(const Int parentIndex);

protected:
	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	void createWaterDrawable();

	// Manipulator list
	std::vector<Object3D*> mManipulatorList;

	// Animation
	Float mFrame;
	std::unique_ptr<CubicBezier2D> mCubicBezier;

	// Drawables data
	std::shared_ptr<TexturedDrawable<WaterShader>> mWaterDrawable;
	WaterShader::Parameters mWaterParameters;
};