#pragma once

#include <nlohmann/json.hpp>
#include <Magnum/Magnum.h>

#include "../GameObject.h"
#include "../Common/SpriteShaderDataView.h"

class ElectricBall : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	// Class members
	ElectricBall(const Int parentIndex);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

protected:
	struct Pieces
	{
		Object3D* manipulator;
		Deg angleCurrent;
		Deg angleLimit;
	};

	SpriteShaderDataView mWrapper;

	Pieces mPieces[4];
	Object3D* mOrbManipulator;
};