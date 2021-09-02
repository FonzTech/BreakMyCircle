#pragma once

#include <unordered_set>
#include <nlohmann/json.hpp>
#include <Magnum/Magnum.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>

#include "../GameObject.h"
#include "../Game/OverlayGui.h"
#include "../Game/OverlayText.h"

class Congrats : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	// Class members
	Congrats(const Int parentIndex, const Int customType);
	~Congrats();

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

private:
	Float mAnimation;
	std::shared_ptr<OverlayGui> mOverlayGui;
	std::shared_ptr<OverlayText> mOverlayText;
};