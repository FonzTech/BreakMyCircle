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
	explicit BaseDrawable(const BaseDrawable & d);

	~BaseDrawable()
	{
		mDrawCallback = nullptr;
		setParent(nullptr);
		mObjectId = 0U;
	}

	Resource<GL::Mesh> mMesh;
	Resource<GL::Texture2D> mTexture;
	Color4 mColor;

	void setDrawCallback(IDrawCallback* drawCallback);
	UnsignedInt getObjectId() const;
	const void setObjectId(const UnsignedInt objectId);

	virtual GL::AbstractShaderProgram& getShader() = 0;
	virtual void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) = 0;

	const void pushToFront();

protected:
	IDrawCallback* mDrawCallback;
	UnsignedInt mObjectId;
};