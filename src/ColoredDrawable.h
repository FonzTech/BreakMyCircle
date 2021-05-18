#pragma once

#include <Magnum/Shaders/Phong.h>

#include "CommonTypes.h"
#include "IDrawCallback.h"

class ColoredDrawable : public Object3D, public SceneGraph::Drawable3D
{
public:
	explicit ColoredDrawable(SceneGraph::DrawableGroup3D& group, Shaders::Phong& shader, GL::Mesh& mesh, const Color4& color);

	Shaders::Phong mShader;
	GL::Mesh mMesh;
	Color4 mColor;

	void setDrawCallback(IDrawCallback* drawCallback);

private:
	IDrawCallback* mDrawCallback;

	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
};