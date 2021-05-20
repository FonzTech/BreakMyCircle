#pragma once

#include <Magnum/Shaders/Phong.h>

#include "CommonTypes.h"
#include "BaseDrawable.h"

class ColoredDrawable : public BaseDrawable
{
public:
	explicit ColoredDrawable(SceneGraph::DrawableGroup3D& group, Shaders::Phong& shader, GL::Mesh& mesh, const Color4& color);

	Shaders::Phong mShader;
	GL::Mesh mMesh;
	Color4 mColor;

protected:
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
};