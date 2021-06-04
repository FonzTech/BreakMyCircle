#pragma once

#include <unordered_set>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>

#include "GameObject.h"
#include "TexturedDrawable.h"
#include "SpriteShader.h"

class FallingBubble : public GameObject
{
public:
	FallingBubble(const Color3& ambientColor, const bool spark);

	bool mSpark;
	Color3 mAmbientColor;
	Color3 mDiffuseColor;

private:
	Vector3 mVelocity;
	Float mDelay;

	Int getType() override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	std::shared_ptr<TexturedDrawable<SpriteShader>> createPlane(Object3D & parent, Resource<GL::Texture2D> & texture);
};