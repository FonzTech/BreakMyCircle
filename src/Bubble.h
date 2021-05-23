#pragma once

#define MINIMUM_BUBBLE_TRAIL_SIZE 3

#include <unordered_set>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>

#include "GameObject.h"
#include "ColoredDrawable.h"

class Bubble : public GameObject
{
public:
	// Equals for sets
	static struct EqualByColorAndPos
	{
	public:
		bool operator()(const Bubble* b1, const Bubble* b2) const
		{
			return b1->mAmbientColor == b2->mAmbientColor && b1->position == b2->position;
		}
	};

	// Hasher for sets
	static struct HashByColorAndPos
	{
	public:
		std::size_t operator()(const Bubble* b) const
		{
			auto h = std::hash<Float>()(b->mAmbientColor.value());
			h ^= std::hash<Float>()(b->position[0]);
			h ^= std::hash<Float>()(b->position[1]);
			h ^= std::hash<Float>()(b->position[2]);
			return h;
		}
	};

	// Typedef for alias
	typedef std::unordered_set<Bubble*, Bubble::HashByColorAndPos, Bubble::EqualByColorAndPos> BubbleCollisionGroup;

	// Class members
	Bubble(const Color3& ambientColor);

	void destroyNearbyBubbles();
	void destroyNearbyBubblesImpl(BubbleCollisionGroup* group);
	void updateBBox();
	void applyRippleEffect(const Vector3& center);

	Color3 mAmbientColor;
	Color3 mDiffuseColor;

private:
	std::shared_ptr<ColoredDrawable> mColoredDrawable;

	Int getType() override;
	void update() override;
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(GameObject* gameObject) override;
};