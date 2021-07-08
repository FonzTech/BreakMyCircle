#pragma once

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <Magnum/Math/Bezier.h>

#include "GameObject.h"
#include "TexturedDrawable.h"
#include "WaterShader.h"

class Skybox : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Skybox(const Sint8 parentIndex, const std::string & name, const Vector3 & position);

protected:
	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	void createDrawable(const std::string & name);

	Resource<GL::CubeMapTexture> resTexture;
};