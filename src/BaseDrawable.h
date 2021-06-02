#pragma once

#include <memory>
#include <Magnum/GL/AbstractShaderProgram.h>

#include "CommonTypes.h"
#include "IDrawCallback.h"

class BaseDrawable: public Object3D, public SceneGraph::Drawable3D
{
public:
	explicit BaseDrawable(SceneGraph::DrawableGroup3D& group);

	std::shared_ptr<GL::Mesh> mMesh;
	std::shared_ptr<GL::Texture2D> mTexture;
	std::shared_ptr<GL::AbstractShaderProgram> mShader;
	Color4 mColor;

	void setDrawCallback(IDrawCallback* drawCallback);

protected:
	IDrawCallback* mDrawCallback;

	virtual void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) = 0;
};