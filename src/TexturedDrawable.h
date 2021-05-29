#pragma once

#include "CommonTypes.h"
#include "BaseDrawable.h"

class TexturedDrawable : public BaseDrawable
{
public:
	explicit TexturedDrawable(SceneGraph::DrawableGroup3D& group, Shaders::Phong& shader, GL::Mesh& mesh, GL::Texture2D& texture);

protected:
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
};