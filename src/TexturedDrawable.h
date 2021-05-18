#pragma once

#include <Magnum/Shaders/Phong.h>

#include "CommonTypes.h"
#include "IDrawCallback.h"

class TexturedDrawable : public Object3D, public SceneGraph::Drawable3D
{
public:
	explicit TexturedDrawable(SceneGraph::DrawableGroup3D& group, Shaders::Phong& shader, GL::Mesh& mesh, GL::Texture2D& texture);

	void setDrawCallback(IDrawCallback* drawCallback);

private:
	IDrawCallback* mDrawCallback;

	Shaders::Phong mShader;
	GL::Mesh mMesh;
	GL::Texture2D mTexture;

	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
};