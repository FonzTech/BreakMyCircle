#pragma once

#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>

#include "GameObject.h"
#include "ColoredDrawable.h"

class Bubble : public GameObject
{
public:
	Bubble(const Color3& ambientColor);

	void destroyNearbyBubbles();

	Color3 mAmbientColor;
	Color3 mDiffuseColor;

private:
	std::shared_ptr<ColoredDrawable> mColoredDrawable;

	void update() override;
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(GameObject* gameObject) override;
};