#pragma once

#include <memory>
#include <Magnum/ResourceManager.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Shaders/Phong.h>

#include "../Common/CommonTypes.h"
#include "IDrawCallback.h"

class BaseDrawable : public Object3D, public SceneGraph::Drawable3D
{
public:
	explicit BaseDrawable(SceneGraph::DrawableGroup3D& group);

	~BaseDrawable()
	{
		mDrawCallback = nullptr;
		setParent(nullptr);
	}

	Resource<GL::Mesh> mMesh;
	Resource<GL::Texture2D> mTexture;
	Color4 mColor;

	void setDrawCallback(IDrawCallback* drawCallback);

	virtual GL::AbstractShaderProgram& getShader() = 0;

protected:
	IDrawCallback* mDrawCallback;

	virtual void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) = 0;
};