#pragma once

#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>

#include "GameObject.h"

class Bubble : public GameObject
{
public:
	Bubble();

protected:
	void update() override;
	void draw() override;

	GL::Mesh mMesh;
	Shaders::Phong mShader;
	Color3 mColor;
};