#pragma once

class IDrawCallback
{
public:
	virtual void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) = 0;
};