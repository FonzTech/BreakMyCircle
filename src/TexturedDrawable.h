#pragma once

#include "CommonTypes.h"
#include "BaseDrawable.h"

class TexturedDrawable : public BaseDrawable
{
public:
	explicit TexturedDrawable(SceneGraph::DrawableGroup3D& group, const std::shared_ptr<GL::AbstractShaderProgram>& shader, const std::shared_ptr<GL::Mesh>& mesh, const std::shared_ptr<GL::Texture2D>& texture);

protected:
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
};