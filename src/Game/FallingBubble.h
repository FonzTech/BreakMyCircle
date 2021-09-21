#pragma once

#define GO_FB_TYPE_BUBBLE 1
#define GO_FB_TYPE_SPARK 2
#define GO_FB_TYPE_COIN 3
#define GO_FB_TYPE_BOMB 4
#define GO_FB_TYPE_STONE 5
#define GO_FB_TYPE_BLACKHOLE 6

#include <unordered_set>
#include <nlohmann/json.hpp>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>

#include "../GameObject.h"
#include "../Graphics/GameDrawable.h"
#include "../Shaders/SpriteShader.h"
#include "../Common/SpriteShaderDataView.h"

class FallingBubble : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	FallingBubble(const Int parentIndex, const Color3& ambientColor, const Int customType, const Float maxVerticalSpeed = -100.0f);

	const Int getType() const  override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

	std::shared_ptr<Audio::Playable3D>& buildSound();

	Int mCustomType;
	Vector3 mVelocity;

private:
	void checkForSpriteEnding();
	void playPrimarySound();

	Color3 mAmbientColor;
	Float mDelay;
	Float mMaxVerticalSpeed;
	SpriteShaderDataView mWrapper;
};