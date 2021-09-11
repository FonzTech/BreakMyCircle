#pragma once

#define GO_LS_MESH_PLATFORM "PlatformV"

#include <nlohmann/json.hpp>

#include "../GameObject.h"

class LevelSelectorSidecar : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	LevelSelectorSidecar(const Int parentIndex, const UnsignedInt levelIndex);
	~LevelSelectorSidecar();

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	void setScale(const Vector3 & scale);
	void setParameters(Resource<GL::Texture2D> & texture, UnsignedInt objectId);

protected:
	UnsignedInt mLevelIndex;
	Vector3 mScale;
};