#pragma once

#define GO_LS_FINISH_TIMER_STARTING_VALUE 6.0f
#define GO_LG_INTENT_SAFE_MINIGAME "game_safeminigame"

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <Magnum/Timeline.h>
#include <Magnum/Math/Bezier.h>
#include <Magnum/Animation/Animation.h>
#include <Magnum/Animation/Track.h>
#include <Magnum/Animation/Player.h>

#include "../GameObject.h"
#include "../Game/OverlayText.h"
#include "../Graphics/BaseDrawable.h"

typedef Animation::TrackView<Float, Vector3> AnimPosition;
typedef Animation::TrackView<Float, Deg> AnimRotation;

class Logo : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Logo(const Int parentIndex);
	~Logo();

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

private:
	// Keyframe structure
	struct Keyframe {
		Float time;
		Vector3 position;
		Deg rotation;
	};

	// Method
	void computeCanvasPadding();
	void buildAnimations();
	void setCameraParameters();
	void continueLogic();

	// Common members
	bool mSafeMinigame;
	Float mCanvasPadding;
	Vector3 mLightPosition;
	bool mLightDirection;

	// Logo manipulator
	Object3D* mPlaneManipulator;
	Object3D* mLogoManipulator;
	Object3D* mLogoObjects[3];

	// Animation data structures
	Float mAnimElapsed;
	Float mPlaneAlpha;

	Keyframe mKeyframes[4][3];
	std::unique_ptr<AnimPosition> mTrackViewPositions[4];
	std::unique_ptr<AnimRotation> mTrackViewRotations[3];
	
	std::unique_ptr<Animation::Player<Float>> mAnimPlayer;

	// Falling Bubbles timer
	Float mBubbleTimer;
	Float mFinishTimer;
	Float mLogoZoom;
	bool mIntroBubbles;

	// Texts
	std::shared_ptr<OverlayText> mTexts[3];
};