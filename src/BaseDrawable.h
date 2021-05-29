#pragma once

#include <Magnum/Shaders/Phong.h>

#include "CommonTypes.h"
#include "IDrawCallback.h"

class BaseDrawable: public Object3D, public SceneGraph::Drawable3D
{
public:
	explicit BaseDrawable(SceneGraph::DrawableGroup3D& group);

	GL::Mesh mMesh;
	GL::Texture2D mTexture;
	Shaders::Phong mShader;
	Color4 mColor;

	void setDrawCallback(IDrawCallback* drawCallback);

protected:
	IDrawCallback* mDrawCallback;

	virtual void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) = 0;
};