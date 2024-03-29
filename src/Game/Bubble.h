#pragma once

#define MINIMUM_BUBBLE_TRAIL_SIZE 3

#define BUBBLE_COIN 0x000001_rgbf
#define BUBBLE_BOMB 0x000002_rgbf
#define BUBBLE_PLASMA 0x000003_rgbf
#define BUBBLE_ELECTRIC 0x000004_rgbf
#define BUBBLE_STONE 0x000005_rgbf
#define BUBBLE_BLACKHOLE 0x000006_rgbf
#define BUBBLE_TIMED 0x000007_rgbf
#define BUBBLE_COLOR_RED 0xff0000_rgbf
#define BUBBLE_COLOR_GREEN 0x00ff00_rgbf
#define BUBBLE_COLOR_BLUE 0x0000ff_rgbf
#define BUBBLE_COLOR_YELLOW 0xffff00_rgbf
#define BUBBLE_COLOR_PURPLE 0xff00ff_rgbf
#define BUBBLE_COLOR_ORANGE 0xff8000_rgbf
#define BUBBLE_COLOR_CYAN 0x00ffff_rgbf

#include <unordered_set>
#include <nlohmann/json.hpp>
#include <Magnum/Magnum.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/Math/Color.h>

#include "../GameObject.h"
#include "../Shaders/TimedBubbleShader.h"

class Bubble : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	// Equals for sets
	struct EqualByColorAndPos
	{
	public:
		bool operator()(const Bubble* b1, const Bubble* b2) const
		{
			return b1->mAmbientColor == b2->mAmbientColor && b1->mPosition == b2->mPosition;
		}
	};

	// Hasher for sets
	struct HashByColorAndPos
	{
	public:
		std::size_t operator()(const Bubble* b) const
		{
			auto h = std::hash<Float>()(Float(b->mAmbientColor.toSrgbInt()));
			h ^= std::hash<Float>()(b->mPosition[0]);
			h ^= std::hash<Float>()(b->mPosition[1]);
			h ^= std::hash<Float>()(b->mPosition[2]);
			return h;
		}
	};

	// Struct for aggregate data
	struct Graph
	{
		std::unordered_set<GameObject*> set;
		bool attached;
	};

	// Struct for explosion data
	struct Explosion
	{
		Vector3 position;
		Float radius;
	};

	// Typedef for alias
	typedef std::unordered_set<Bubble*, Bubble::HashByColorAndPos, Bubble::EqualByColorAndPos> BubbleCollisionGroup;

	// Class members
	Bubble(const Int parentIndex, const Color3& ambientColor, const Float timedDelay = 1.0f);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

	void updateBBox();
	void applyRippleEffect(const Vector3& center);
	void playStompSound();

	Int destroyNearbyBubbles(const bool force);
	Int destroyDisjointBubbles();

	Int destroyNearbyBubblesImpl(BubbleCollisionGroup* group);
	std::unique_ptr<Graph> destroyDisjointBubblesImpl(std::unordered_set<Bubble*> & group, const bool attached);

	Color3 mAmbientColor;

private:
	const Float getShakeSmooth(const Float xt);
	const Int getCustomTypeForFallingBubble(const Color3 & color);
	const Containers::Optional<Color3> getColorByIndex(const bool isRandom);

	// Complex structures
	struct GraphNode
	{
		Vector3 position;
		Color3 color;
	};

	struct Timed
	{
		bool enabled;
		float factor;
		UnsignedInt index;
		Resource<GL::AbstractShaderProgram, TimedBubbleShader> shader;
		Resource<GL::Texture2D> textureMask;
	};

	// Class members
	std::vector<Object3D*> mItemManipulator;
	std::vector<Float> mItemParams;
	Vector3 mShakePos;
	Float mShakeFact;
	Float mRotation;
	Float mBlackholeAnim;
	Timed mTimed;
	Resource<GL::AbstractShaderProgram, Shaders::Flat3D> mFlatShader;
};