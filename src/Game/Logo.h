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
#include "../Graphics/BaseDrawable.h"

typedef Animation::TrackView<Float, Vector3> AnimPosition;
typedef Animation::TrackView<Float, Deg> AnimRotation;

class Logo : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Logo(const Int parentIndex);

protected:
	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

private:
	// Keyframe structure
	struct Keyframe {
		Float time;
		Vector3 position;
		Deg rotation;
	};

	// Method
	void buildAnimations();
	void setCameraParameters();

	// Common members
	bool mLightDirection;
	Vector3 mLightPosition;

	// Logo manipulator
	Object3D* mLogoManipulator;
	Object3D* mLogoObjects[3];

	// Animation data structures
	Timeline mAnimTimeline;

	Keyframe mKeyframes[4][3];
	std::unique_ptr<AnimPosition> mTrackViewPositions[4];
	std::unique_ptr<AnimRotation> mTrackViewRotations[3];
	
	std::unique_ptr<Animation::Player<Float>> mAnimPlayer;

	// Falling Bubbles timer
	Float mBubbleTimer;
	Float mFinishTimer;
};