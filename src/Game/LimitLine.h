#pragma once

#define GO_LL_TYPE_BLACK 1
#define GO_LL_TYPE_RED 2

#include <nlohmann/json.hpp>

#include "../GameObject.h"
#include "../Graphics/GameDrawable.h"

class LimitLine : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	LimitLine(const Int parentIndex, const Color4 & color, const Int customType);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

	const Int getCustomType() const;
	const void setScale(const Vector3 & scale);

protected:
	Color4 mColor;
	Int mCustomType;
	Vector3 mScale;
};
