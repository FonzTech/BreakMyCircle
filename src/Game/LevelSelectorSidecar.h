#pragma once

#define GO_LS_MESH_PLATFORM "PlatformV"

#include <nlohmann/json.hpp>
#include <Magnum/Shaders/Flat.h>

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
	void setGlow(const bool enabled);

protected:
	UnsignedInt mLevelIndex;
	Vector3 mScale;
	
	Object3D* mGlowManipulator;
	std::weak_ptr<BaseDrawable> mGlowDrawable;
	Resource<GL::AbstractShaderProgram, Shaders::Flat3D> mFlat3DShader;
};