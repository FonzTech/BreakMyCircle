#pragma once

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <Magnum/Timeline.h>
#include <Magnum/Math/Bezier.h>
#include <Magnum/Animation/Animation.h>
#include <Magnum/Animation/Track.h>
#include <Magnum/Animation/Player.h>

#include "../GameObject.h"
#include "../Game/OverlayGui.h"
#include "../Game/OverlayText.h"

class Dialog : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Dialog(const Int parentIndex);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	void setText(const std::string & text);

protected:

	Float mOpacity;
	std::shared_ptr<OverlayGui> mBackground;
	std::shared_ptr<OverlayText> mText;
};