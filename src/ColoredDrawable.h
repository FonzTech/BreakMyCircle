#pragma once

#include <memory>
#include <Magnum/Shaders/Phong.h>
#include "CommonTypes.h"

class ColoredDrawable : public Object3D, SceneGraph::Drawable3D
{
public:
	explicit ColoredDrawable(SceneGraph::DrawableGroup3D& group, Shaders::Phong& shader, GL::Mesh& mesh, const Color4& color);

private:
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

	Shaders::Phong& mShader;
	GL::Mesh& mMesh;
	Color4 mColor;
};