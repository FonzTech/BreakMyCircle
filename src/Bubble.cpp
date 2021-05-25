#include "Bubble.h"

#include <thread>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/Math/Math.h>
#include <Magnum/Math/Bezier.h>

#include "ColoredDrawable.h"
#include "RoomManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

Bubble::Bubble(const Color3& ambientColor) : GameObject()
{
	// Assign members
	mAmbientColor = ambientColor;
	mShakePos = { 0.0f };
	mShakeFact = 0.0f;

	// Create test mesh
	Trade::MeshData meshData = Primitives::icosphereSolid(2U);

	GL::Buffer vertices;
	vertices.setData(MeshTools::interleave(meshData.positions3DAsArray(), meshData.normalsAsArray()));

	std::pair<Containers::Array<char>, MeshIndexType> compressed = MeshTools::compressIndices(meshData.indicesAsArray());
	GL::Buffer indices;
	indices.setData(compressed.first);

	GL::Mesh mesh;
	mesh
		.setPrimitive(meshData.primitive())
		.setCount(meshData.indexCount())
		.addVertexBuffer(std::move(vertices), 0, Shaders::Phong::Position{}, Shaders::Phong::Normal{})
		.setIndexBuffer(std::move(indices), 0, compressed.second);

	// Set diffuse color
	mDiffuseColor = 0xffffff_rgbf;

	// Create Phong shader
	Shaders::Phong shader;
	
	// Create colored drawable
	mColoredDrawable = std::make_shared<ColoredDrawable>(RoomManager::singleton->mDrawables, shader, mesh, mAmbientColor);
	mColoredDrawable->setParent(&RoomManager::singleton->mScene);
	mColoredDrawable->setDrawCallback(this);
	drawables.emplace_back(mColoredDrawable);
}

Int Bubble::getType()
{
	return GOT_BUBBLE;
}

void Bubble::update()
{
	// Update shake animation
	if (mShakeFact <= 0.0f)
	{
		mShakeFact = 0.0f;
	}
	else
	{
		mShakeFact -= deltaTime * 2.0f;
	}
}

void Bubble::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	const Vector3 shakeVect = mShakeFact > 0.001f ? mShakePos * std::sin(mShakeFact * Constants::pi()) : Vector3(0.0f);
	mColoredDrawable->mShader
		.setLightPositions({ position + Vector3({ 10.0f, 10.0f, 1.75f }) })
		.setDiffuseColor(mDiffuseColor)
		.setAmbientColor(mAmbientColor)
		.setTransformationMatrix(transformationMatrix * Matrix4::translation(position + shakeVect))
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.draw(mColoredDrawable->mMesh);
}

void Bubble::collidedWith(GameObject* gameObject)
{
}

void Bubble::updateBBox()
{
	bbox = Range3D{ position - Vector3(0.8f), position + Vector3(0.8f) };
}

void Bubble::applyRippleEffect(const Vector3& center)
{
	const Vector3 d = position - center;
	const Vector3 p = d.normalized();
	const Math::Unit<Math::Rad, Float> unitRads(std::atan2(p.y(), p.x()));
	const Float rads(unitRads);

	const Float dist = getShakeSmooth(Math::clamp(0.0f, 1.0f, d.length() / 10.0f));
	mShakePos = Vector3(std::cos(rads), std::sin(rads), 0.0f) * dist;
	mShakeFact = 1.0f;
}

void Bubble::destroyNearbyBubbles()
{
	std::thread tjob([this]() {
		// Check for nearby collisions
		BubbleCollisionGroup bg;
		bg.insert(this);
		Bubble::destroyNearbyBubblesImpl(&bg);

		// Work on bubble collision group
		#if DEBUG
		printf("Collided bubbles of color %d are %d\n", mAmbientColor.value(), bg.size());
		#endif

		if (bg.size() >= MINIMUM_BUBBLE_TRAIL_SIZE)
		{
			for (auto& b : bg)
			{
				b->destroyMe = true;
			}
		}
	});
	tjob.join();
}

/*
	This algorithm is ugly, since it has an upper-bound of O(n^2).
	It can be reduced to O(nm) if adjacency matrix, accessible
	through some key (like an hash-map) is built during the room
	initialization. When a new bubble is added to the room, this
	adjacency matrix/list can be updated with a lookup by key,
	using bubble position (node coordinates).
*/
void Bubble::destroyNearbyBubblesImpl(BubbleCollisionGroup* group)
{
	// Cycle through all bubbles in current room
	for (auto& go : RoomManager::singleton->mGameObjects)
	{
		// Check if game object is a bubble
		if (go.get() == this || go->getType() != GOT_BUBBLE || go->destroyMe)
		{
			continue;
		}

		// Check if the bubble has the same color of this
		Bubble* bubble = ((Bubble*)go.get());
		if (bubble->mAmbientColor != mAmbientColor || group->find(bubble) != group->end())
		{
			continue;
		}

		// Check if bubble collides nearby
		for (auto& item : *group)
		{
			Range3D eb{ item->position - Vector3(1.5f), item->position + Vector3(1.5f) };
			if (Math::intersects(eb, go->bbox))
			{
				group->insert(bubble);
				bubble->destroyNearbyBubblesImpl(group);
			}
		}
	}
}

Float Bubble::getShakeSmooth(const Float xt)
{
	return CubicBezier2D(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.06f), Vector2(1.0f, 0.04f), Vector2(1.0f)).value(xt)[1];
} 