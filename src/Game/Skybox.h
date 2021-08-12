#pragma once

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <Magnum/Math/Bezier.h>

#include "../GameObject.h"
#include "../Graphics/GameDrawable.h"
#include "../Shaders/WaterShader.h"

class Skybox : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Skybox(const Int parentIndex, const std::string & name, const Vector3 & position);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

protected:

	void createDrawable(const std::string & name);

	Resource<GL::CubeMapTexture> resTexture;
};