#pragma once

#include "CommonTypes.h"
#include "BaseDrawable.h"

class ColoredDrawable : public BaseDrawable
{
public:
	explicit ColoredDrawable(SceneGraph::DrawableGroup3D& group, Shaders::Phong& shader, GL::Mesh& mesh, const Color4& color);

protected:
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
};