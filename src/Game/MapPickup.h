#pragma once

#define GO_MP_TYPE_COIN 1

#include <nlohmann/json.hpp>

#include "../GameObject.h"

class MapPickup : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	MapPickup(const Int parentIndex, const Int customType);
	~MapPickup();

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	void setObjectId(const UnsignedInt id);
	bool isPickupable();
	void setDestroyState(const bool state);

	Color3 mAmbientColor;

private:
	Float mAnimation;
	Float mPickupDestroy;
	Int mCustomType;
};