#pragma once

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <Magnum/Timeline.h>
#include <Magnum/Math/Bezier.h>
#include <Magnum/Animation/Animation.h>
#include <Magnum/Animation/Track.h>
#include <Magnum/Animation/Player.h>

#include "GameObject.h"
#include "BaseDrawable.h"

typedef Animation::TrackView<Float, Vector3> AnimPosition;
typedef Animation::TrackView<Float, Deg> AnimRotation;

class Logo : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Logo(const Sint8 parentIndex);

protected:
	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

private:
	struct Keyframe {
		Float time;
		Vector3 position;
		Deg rotation;
	};

	void buildAnimations();

	Keyframe mKeyframes[2];
	std::unique_ptr<AnimPosition> mTrackViewPositions;
	std::unique_ptr<AnimRotation> mTrackViewRotations;
	
	std::unique_ptr<Animation::Player<Float>> mAnimPlayer;
	Timeline mAnimTimeline;
};