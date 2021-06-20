#include "Bubble.h"

#include <thread>
#include <future>
#include <queue>

#include <Magnum/Math/Math.h>
#include <Magnum/Math/Bezier.h>

#include "CommonUtility.h"
#include "ColoredDrawable.h"
#include "RoomManager.h"
#include "FallingBubble.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> Bubble::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Sint8 parent;
	params.at("parent").get_to(parent);

	// Parse color
	Color3 color;
	{
		auto& values = params["color"];
		values.at("r").get_to(color[0]);
		values.at("g").get_to(color[1]);
		values.at("b").get_to(color[2]);
	}

	// Instantiate bubble
	std::shared_ptr<Bubble> p = std::make_shared<Bubble>(parent, color);
	return p;
}

Bubble::Bubble(const Sint8 parentIndex, const Color3& ambientColor) : GameObject(parentIndex)
{
	// Assign members
	mAmbientColor = ambientColor;
	mShakePos = { 0.0f };
	mShakeFact = 0.0f;

	// Set diffuse color
	mDiffuseColor = 0xffffff_rgbf;

	// Create game bubble
	std::shared_ptr<ColoredDrawable<Shaders::Phong>> cd = CommonUtility::singleton->createGameSphere(mParentIndex, *mManipulator, mAmbientColor, this);
	mDrawables.emplace_back(cd);
}

const Int Bubble::getType() const
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
		mShakeFact -= mDeltaTime * 3.0f;
	}

	// Update transformations
	const Vector3 shakeVect = mShakeFact > 0.001f ? mShakePos * std::sin(mShakeFact * Constants::pi()) : Vector3(0.0f);
	for (auto& d : mDrawables)
	{
		d->setTransformation(Matrix4::translation(position + shakeVect));
	}

	// Update bounding box
	updateBBox();
}

void Bubble::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	((Shaders::Phong&) baseDrawable->getShader())
		.setLightPositions({ position + Vector3({ 0.0f, 40.0f, 5.0f }) })
		.setDiffuseColor(mDiffuseColor)
		.setAmbientColor(mAmbientColor)
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.draw(*baseDrawable->mMesh);
}

void Bubble::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
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

bool Bubble::destroyNearbyBubbles()
{
	auto future = std::async(std::launch::async, [this]() {
		// Check for nearby collisions
		BubbleCollisionGroup bg;
		bg.insert(this);
		Bubble::destroyNearbyBubblesImpl(&bg);

		// Empty list for positions for future popping bubbles
		std::unique_ptr<std::queue<GraphNode>> fps = std::make_unique<std::queue<GraphNode>>();

		// Work on bubble collision group
		#if NDEBUG or _DEBUG
		printf("Collided bubbles of color %d are %d\n", mAmbientColor.value(), bg.size());
		#endif

		if (bg.size() >= MINIMUM_BUBBLE_TRAIL_SIZE)
		{
			for (auto& b : bg)
			{
				{
					GraphNode gn;
					gn.position = b->position;
					gn.color = b->mAmbientColor;
					fps->push(gn);
				}

				b->destroyMe = true;
			}
		}

		// Return the list
		return fps;
	});
	auto fps = future.get();
	bool nonZero = fps->size() > 0;

	Float posZ = 1.1f;
	while (!fps->empty())
	{
		auto& gn = fps->front();

		std::shared_ptr<FallingBubble> ib = std::make_shared<FallingBubble>(mParentIndex, gn.color, true);
		ib->position = gn.position + Vector3(0.0f, 0.0f, posZ);
		RoomManager::singleton->mGoLayers[mParentIndex].push_back(ib);

		fps->pop();

		posZ += 0.1f;
	}

	return nonZero;
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
	// Cycle through all collided game objects
	Range3D eb = { position - Vector3(1.5f), position + Vector3(1.5f) };
	std::unique_ptr<std::unordered_set<GameObject*>> collided = RoomManager::singleton->mCollisionManager->checkCollision(eb, this, { GOT_BUBBLE });
	for (const auto& item : *collided)
	{
		// Check if the bubble has the same color of this
		Bubble* bubble = ((Bubble*)item);
		if (bubble->mAmbientColor != mAmbientColor || group->find(bubble) != group->end())
		{
			continue;
		}

		// Insert into collision group and iterate recursively
		group->insert(bubble);
		bubble->destroyNearbyBubblesImpl(group);
	}
}

void Bubble::destroyDisjointBubbles()
{
	auto future = std::async(std::launch::async, [this]() {
		// Create list of bubbles
		std::unordered_set<Bubble*> group;
		for (auto& go : *RoomManager::singleton->mGoLayers[mParentIndex].list)
		{
			if (go->getType() != GOT_BUBBLE || go->destroyMe)
			{
				continue;
			}
			group.insert((Bubble*)go.get());
		}

		// Empty list for positions for future falling bubbles
		std::unique_ptr<std::queue<GraphNode>> fps = std::make_unique<std::queue<GraphNode>>();

		// DFS-like algorithm on all remaining nodes
		while (!group.empty())
		{
			// Pick the first node in list
			auto it = group.begin();
			Bubble* bubble = *it;
			group.erase(it);

			// Perform DFS-like algorithm
			std::unique_ptr<Graph> graph = bubble->destroyDisjointBubblesImpl(group);
			// graph->set.insert(bubble);

			// Debug print
			#if NDEBUG or _DEBUG
			printf("Graph %s bubbles are %d\n", graph->attached ? "attached" : "not attached", graph->set.size());
			#endif

			// Destroy all bubbles obtained from the previous function
			if (!graph->attached)
			{
				for (auto& item : graph->set)
				{
					{
						GraphNode gn;
						gn.position = item->position;
						gn.color = ((Bubble*)item)->mAmbientColor;
						fps->push(gn);
					}

					Bubble* ib = (Bubble*)item;
					ib->destroyMe = true;
				}
			}
		}
		
		// Return list of positions
		return fps;
	});
	auto fps = future.get();

	while (!fps->empty())
	{
		auto& gn = fps->front();

		std::shared_ptr<FallingBubble> ib = std::make_shared<FallingBubble>(mParentIndex, gn.color, false);
		ib->position = gn.position;
		RoomManager::singleton->mGoLayers[mParentIndex].push_back(ib);

		fps->pop();
	}
}

std::unique_ptr<Bubble::Graph> Bubble::destroyDisjointBubblesImpl(std::unordered_set<Bubble*> & group)
{
	// BBox for adjacent bubbles
	Range3D bbox(position - Vector3(1.5f), position + Vector3(1.5f));

	// Check for collisions against other game objects (DFS-like graph)
	std::unique_ptr<std::unordered_set<GameObject*>> collided = RoomManager::singleton->mCollisionManager->checkCollision(bbox, this, { GOT_BUBBLE });
	std::unique_ptr<Graph> graph = std::make_unique<Graph>();
	graph->attached = 0;

	// Cycle through all collided game objects
	if (collided->size() > 0)
	{
		for (const auto& item : *collided)
		{
			// Check if node of graph (bubble) is marked as "explored" or not
			Bubble* bi = (Bubble*)item;
			if (group.find(bi) == group.end())
			{
				continue;
			}

			// Mark it as "explored"
			group.erase(bi);

			// Check if node is present in this connected graph
			if (graph->set.find(bi) == graph->set.end())
			{
				// Check if bubble is attached to the ceiling
				const bool attachedToCeiling = bi->position.y() >= -0.1f;

				// Insert it, if color is the same or it's not attached to the ceiling
				if (!attachedToCeiling || bi->mAmbientColor == mAmbientColor)
				{
					graph->set.insert(bi);
				}

				// Perform DFS-like algorithm
				std::unique_ptr<Graph> result = bi->destroyDisjointBubblesImpl(group);

				/*
					Check if graph is eligible or not for deletion.
					For example, if it has at least one node, attached
					to the ceiling, then it's NOT eligible for deletion.
				*/
				if (attachedToCeiling || result->attached)
				{
					graph->attached = 1;
				}

				// Merge the two lists without repetition
				graph->set.insert(result->set.begin(), result->set.end());
			}
		}

		// Add self-bubble, if necessary
		if (!graph->attached)
		{
			graph->set.insert(this);
		}
	}
	else if (position.y() < -0.1f)
	{
		graph->set.insert(this);
	}

	return graph;
}

Float Bubble::getShakeSmooth(const Float xt)
{
	return CubicBezier2D(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.06f), Vector2(1.0f, 0.04f), Vector2(1.0f)).value(xt)[1];
} 