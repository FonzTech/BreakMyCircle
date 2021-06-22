#pragma once

#define MINIMUM_BUBBLE_TRAIL_SIZE 3

#define BUBBLE_COLOR_RED 0xff0000_rgbf
#define BUBBLE_COLOR_GREEN 0x00ff00_rgbf
#define BUBBLE_COLOR_BLUE 0x0000ff_rgbf
#define BUBBLE_COLOR_YELLOW 0xffff00_rgbf
#define BUBBLE_COLOR_PURPLE 0xff00ff_rgbf
#define BUBBLE_COLOR_ORANGE 0xffff_rgbf
#define BUBBLE_COLOR_CYAN 0x00ffff_rgbf

#include <unordered_set>
#include <nlohmann/json.hpp>
#include <Magnum/Magnum.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>

#include "GameObject.h"

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
			return b1->mAmbientColor == b2->mAmbientColor && b1->position == b2->position;
		}
	};

	// Hasher for sets
	struct HashByColorAndPos
	{
	public:
		std::size_t operator()(const Bubble* b) const
		{
			auto h = std::hash<Float>()(b->mAmbientColor.toSrgbInt());
			h ^= std::hash<Float>()(b->position[0]);
			h ^= std::hash<Float>()(b->position[1]);
			h ^= std::hash<Float>()(b->position[2]);
			return h;
		}
	};

	// Struct for aggregate data
	struct Graph
	{
		std::unordered_set<GameObject*> set;
		Sint8 attached;
	};

	// Typedef for alias
	typedef std::unordered_set<Bubble*, Bubble::HashByColorAndPos, Bubble::EqualByColorAndPos> BubbleCollisionGroup;

	// Class members
	Bubble(const Sint8 parentIndex, const Color3& ambientColor);

	void updateBBox();
	void applyRippleEffect(const Vector3& center);

	bool destroyNearbyBubbles();
	void destroyDisjointBubbles();

	void destroyNearbyBubblesImpl(BubbleCollisionGroup* group);
	std::unique_ptr<Graph> destroyDisjointBubblesImpl(std::unordered_set<Bubble*> & group);

	Color3 mAmbientColor;
	Color3 mDiffuseColor;

private:
	// Complex structures
	struct GraphNode
	{
		Vector3 position;
		Color3 color;
	};

	// Class members
	Vector3 mShakePos;
	Float mShakeFact;

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	Float getShakeSmooth(const Float xt);
};