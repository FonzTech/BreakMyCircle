#pragma once

#include <unordered_set>
#include <nlohmann/json.hpp>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>

#include "GameObject.h"
#include "TexturedDrawable.h"
#include "SpriteShader.h"
#include "SpriteShaderDataView.h"

class FallingBubble : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	FallingBubble(const Sint8 parentIndex, const Color3& ambientColor, const bool spark, const Float maxVerticalSpeed = 100.0f);

	bool mSpark;
	Color3 mAmbientColor;

private:
	Vector3 mVelocity;
	Float mDelay;
	Float mMaxVerticalSpeed;
	SpriteShaderDataView mWrapper;

	const Int getType() const  override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	std::shared_ptr<TexturedDrawable<SpriteShader>> createPlane(Object3D & parent, Resource<GL::Texture2D> & texture);
};