#pragma once

class BaseDrawable;

class IDrawCallback
{
public:
	virtual void draw(BaseDrawable* drawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) = 0;
};