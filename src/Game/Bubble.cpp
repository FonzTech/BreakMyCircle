#include "Bubble.h"

#include <thread>
#include <future>
#include <queue>

#include <Magnum/Math/Math.h>
#include <Magnum/Math/Bezier.h>
#include <Magnum/Math/Constants.h>

#include "../Common/CommonUtility.h"
#include "../AssetManager.h"
#include "../RoomManager.h"
#include "FallingBubble.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> Bubble::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Parse color
	Color3 color;
	{
		const auto& values = params["color"];
		values.at("r").get_to(color[0]);
		values.at("g").get_to(color[1]);
		values.at("b").get_to(color[2]);
	}

	// Instantiate bubble
	std::shared_ptr<Bubble> p = std::make_shared<Bubble>(parent, color);
	return p;
}

Bubble::Bubble(const Int parentIndex, const Color3& ambientColor) : GameObject(parentIndex)
{
	// Assign members
	mParentIndex = parentIndex;
	mAmbientColor = ambientColor;
	mShakePos = { 0.0f };
	mShakeFact = 0.0f;

	// Create game sphere for this game object
	CommonUtility::singleton->createGameSphere(this, *mManipulator, mAmbientColor);

	// Load asset for "Coin" game object, if required
	if (mAmbientColor == BUBBLE_COIN)
	{
		mItemManipulator = new Object3D{ mManipulator.get() };

		AssetManager().loadAssets(*this, *mItemManipulator, RESOURCE_SCENE_COIN, this);
		mRotation = Float(Rad(Deg(std::rand() % 360)));
	}
	else
	{
		mRotation = 0.0f;
	}
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
	const bool& isCoin = mAmbientColor == BUBBLE_COIN;
	if (isCoin)
	{
		mRotation += mDeltaTime * Constants::pi();

		(*mItemManipulator)
			.resetTransformation()
			.scale(Vector3(0.75f))
			.rotateX(Rad(Deg(90.0f)))
			.rotateY(Rad(mRotation))
			.translate(Vector3(0.0f, 0.0f, -0.5f));
	}

	const Vector3 shakeVect = mShakeFact > 0.001f ? mShakePos * std::sin(mShakeFact * Constants::pi()) : Vector3(0.0f);

	(*mManipulator)
		.resetTransformation()
		.translate(mPosition + shakeVect);

	// Update bounding box
	updateBBox();
}

void Bubble::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	((Shaders::Phong&) baseDrawable->getShader())
		.setLightPosition(mPosition + Vector3(0.0f, 0.0f, 1.5f))
		.setLightColor(mAmbientColor == BUBBLE_COIN ? 0xd0d0d0_rgbf : 0x808080_rgbf)
		.setSpecularColor(0xffffff00_rgbaf)
		.setAmbientColor(0xc0c0c0_rgbf)
		.setDiffuseColor(0x808080_rgbf)
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
		.draw(*baseDrawable->mMesh);
}

void Bubble::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void Bubble::updateBBox()
{
	mBbox = Range3D{ mPosition - Vector3(0.8f), mPosition + Vector3(0.8f) };
}

void Bubble::applyRippleEffect(const Vector3& center)
{
	const Vector3 d = mPosition - center;
	const Vector3 p = d.normalized();
	const Math::Unit<Math::Rad, Float> unitRads(std::atan2(p.y(), p.x()));
	const Float rads(unitRads);

	const Float dist = getShakeSmooth(Math::clamp(0.0f, 1.0f, d.length() / 10.0f));
	mShakePos = Vector3(std::cos(rads), std::sin(rads), 0.0f) * dist;
	mShakeFact = 1.0f;
}

void Bubble::playStompSound()
{
	Resource<Audio::Buffer> buffer = CommonUtility::singleton->loadAudioData(RESOURCE_AUDIO_BUBBLE_STOMP);
	mPlayables[0] = std::make_shared<Audio::Playable3D>(*mManipulator.get(), &RoomManager::singleton->mAudioPlayables);
	mPlayables[0]->source()
		.setBuffer(buffer)
		.setLooping(false);

	playSfxAudio(0);
}

Int Bubble::destroyNearbyBubbles(const bool force, const Float offsetZ)
{
	// Data for later
	std::unique_ptr<std::queue<GraphNode>> fps = nullptr;

	// Check if destroy was forced
	if (force)
	{
		// Create graph
		fps = std::make_unique<std::queue<GraphNode>>();

		// Create graph node
		{
			GraphNode gn;
			gn.position = mPosition;
			gn.color = mAmbientColor;
			fps->push(gn);
		}

		// Destroy this bubble
		mDestroyMe = true;
	}
	// Make nearby bubbles with the same color explode
	else
	{
		auto future = std::async(std::launch::async, [&]() {
			// Check for nearby collisions
			BubbleCollisionGroup bg;
			bg.insert(this);
			destroyNearbyBubblesImpl(&bg);

			// Empty list for positions for future popping bubbles
			std::unique_ptr<std::queue<GraphNode>> fps = std::make_unique<std::queue<GraphNode>>();

			// Work on bubble collision group
#if NDEBUG or _DEBUG
			printf("Collided bubbles of color %d are %d\n", mAmbientColor.toSrgbInt(), bg.size());
#endif

			if (bg.size() >= MINIMUM_BUBBLE_TRAIL_SIZE)
			{
				for (auto& b : bg)
				{
					{
						GraphNode gn;
						gn.position = b->mPosition;
						gn.color = b->mAmbientColor;
						fps->push(gn);
					}

					b->mDestroyMe = true;
				}
			}

			// Return the list
			return fps;
		});

		fps = future.get();
	}

	Int shootAmount = Int(fps->size());

	Float posZ = offsetZ;
	if (!fps->empty())
	{
		bool playSound = !force;
		while (!fps->empty())
		{
			// Get a node from graph
			const auto& gn = fps->front();

			// Create sparkle
			const std::shared_ptr<FallingBubble> ib = std::make_shared<FallingBubble>(mParentIndex, gn.color, GO_FB_TYPE_SPARK);
			ib->mPosition = gn.position + Vector3(0.0f, 0.0f, posZ);
			RoomManager::singleton->mGoLayers[mParentIndex].push_back(ib);

			// Play sound only once
			if (playSound)
			{
				ib->buildSound();
				ib->playSfxAudio(0);
				playSound = false;
			}

			// Increment work variable
			posZ += 0.05f;

			// Remove from graph
			fps->pop();
		}
	}

	return shootAmount;
}

/*
	This algorithm is ugly, since it has an upper-bound of O(n^2).
	It can be reduced to O(nm) if adjacency matrix, accessible
	through some key (like an hash-map) is built during the room
	initialization. When a new bubble is added to the room, this
	adjacency matrix/list can be updated with a lookup by key,
	using bubble position (node coordinates).
*/
Int Bubble::destroyNearbyBubblesImpl(BubbleCollisionGroup* group)
{
	// Cycle through all collided game objects
	Range3D eb = { mPosition - Vector3(1.5f), mPosition + Vector3(1.5f) };
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
	return group->size();
}

Int Bubble::destroyDisjointBubbles()
{
	auto future = std::async(std::launch::async, [this]() {
		// Create list of bubbles
		std::unordered_set<Bubble*> group;
		for (const auto& go : *RoomManager::singleton->mGoLayers[mParentIndex].list)
		{
			if (go->getType() != GOT_BUBBLE || go->mDestroyMe)
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
			const auto& it = group.begin();
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
						gn.position = item->mPosition;
						gn.color = ((Bubble*)item)->mAmbientColor;
						fps->push(gn);
					}

					Bubble* ib = (Bubble*)item;
					ib->mDestroyMe = true;
				}
			}
		}
		
		// Return list of positions
		return fps;
	});
	auto fps = future.get();

	bool coinSound = true;
	Int shootAmount = Int(fps->size());
	while (!fps->empty())
	{
		// Get front node from graph
		auto& gn = fps->front();

		// Create "eye-candy" effect
		const auto& customType = getCustomTypeForFallingBubble(gn.color);
		std::shared_ptr<FallingBubble> ib = std::make_shared<FallingBubble>(mParentIndex, gn.color, customType);
		ib->mPosition = gn.position;

		// Special setup for picked-up coins
		if (customType == GO_FB_TYPE_COIN)
		{
			ib->mVelocity = -gn.position + Vector3(-5.0f, 0.0f, 0.0f);
			ib->mPosition += Vector3(0.0f, 0.0f, 0.5f);

			// Let "coin" sound play only once
			if (coinSound)
			{
				ib->buildSound();
				coinSound = false;
			}

			// Increment coin counter
			++RoomManager::singleton->mSaveData.coinCurrent;
		}
		else
		{
			ib->buildSound();
		}

		// Add to room
		RoomManager::singleton->mGoLayers[mParentIndex].push_back(ib);

		// Remove from graph
		fps->pop();
	}

	return shootAmount;
}

std::unique_ptr<Bubble::Graph> Bubble::destroyDisjointBubblesImpl(std::unordered_set<Bubble*> & group)
{
	// BBox for adjacent bubbles
	Range3D bbox(mPosition - Vector3(1.5f), mPosition + Vector3(1.5f));

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
				const bool attachedToCeiling = bi->mPosition.y() >= -0.1f;

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
	else if (mPosition.y() < -0.1f)
	{
		graph->set.insert(this);
	}

	return graph;
}

Float Bubble::getShakeSmooth(const Float xt)
{
	return CubicBezier2D(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.06f), Vector2(1.0f, 0.04f), Vector2(1.0f)).value(xt)[1];
}

Int Bubble::getCustomTypeForFallingBubble(const Color3 & color)
{
	if (color == BUBBLE_COIN)
	{
		return GO_FB_TYPE_COIN;
	}
	else
	{
		return GO_FB_TYPE_BUBBLE;
	}
}