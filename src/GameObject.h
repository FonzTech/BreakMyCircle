#pragma once

#include <Magnum/Trade/MeshData.h>
#include <Magnum/Math/Matrix4.h>

using namespace Magnum;

class GameObject
{
protected:
	std::shared_ptr<Trade::MeshData> meshData;
	Matrix4 mTransformation, mProjection;

public:
	GameObject();
	~GameObject();

	static Vector2 windowSize;

	virtual void update() = 0;
	virtual void draw() = 0;

	void updateProjectionMatrix();
};