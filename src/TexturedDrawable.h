#pragma once

#include <Magnum/Shaders/Phong.h>

#include "CommonTypes.h"
#include "BaseDrawable.h"

class TexturedDrawable : public BaseDrawable
{
public:
	explicit TexturedDrawable(SceneGraph::DrawableGroup3D& group, Shaders::Phong& shader, GL::Mesh& mesh, GL::Texture2D& texture);

protected:
	Shaders::Phong mShader;
	GL::Mesh mMesh;
	GL::Texture2D mTexture;

	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
};