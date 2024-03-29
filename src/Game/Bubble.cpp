#include "Bubble.h"

#include <thread>
#include <future>
#include <queue>
#include <set>

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

	// Get timed delay
	Float timedDelay = 1.0f;
	{
		const auto& it = params.find("timedDelay");
		if (it != params.end())
		{
			it->get_to(timedDelay);
		}
	}

	// Instantiate bubble
	std::shared_ptr<Bubble> p = std::make_shared<Bubble>(parent, color, timedDelay);
	return p;
}

Bubble::Bubble(const Int parentIndex, const Color3& ambientColor, const Float timedDelay) : GameObject(parentIndex)
{
	// Assign members
	mParentIndex = parentIndex;
	mAmbientColor = ambientColor;
	mShakePos = Vector3(0.0f);
	mShakeFact = 0.0f;
	mBlackholeAnim = 0.0f;
	mTimed.enabled = false;

	// Load asset for "Coin" game object, if required
	if (mAmbientColor == BUBBLE_COIN)
	{
		mItemManipulator.push_back(std::move(new Object3D{ mManipulator }));

		AssetManager().loadAssets(*this, *mItemManipulator.at(0), RESOURCE_SCENE_COIN, this);
		mRotation = Float(Rad(Deg(std::rand() % 360)));

		CommonUtility::singleton->createGameSphere(this, *mManipulator, mAmbientColor);
	}
	else if (mAmbientColor == BUBBLE_STONE)
	{
		AssetManager().loadAssets(*this, *mManipulator, RESOURCE_SCENE_STONE, this);
		mRotation = 0.0f;
	}
	else if (mAmbientColor == BUBBLE_BLACKHOLE)
	{
		for (UnsignedInt i = 0; i < 5U; ++i)
		{
			// Variables
			const bool first = i == 0U;
			if (!first)
			{
				mItemManipulator.push_back(std::move(new Object3D{ mManipulator }));
				mItemParams.push_back(1.0f + 0.5f * Float(i));
			}

			// Load assets
			Resource<GL::Mesh> resMesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<Shaders::Flat3D::Position, Shaders::Flat3D::TextureCoordinates>(RESOURCE_MESH_PLANE_FLAT);
			Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(first ? RESOURCE_TEXTURE_BLACKHOLE_BG : RESOURCE_TEXTURE_BLACKHOLE_FG);
			Resource<GL::AbstractShaderProgram, Shaders::Flat3D> resShader = CommonUtility::singleton->getFlat3DShader();

			// Create drawable
			auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
			std::shared_ptr<GameDrawable<Shaders::Flat3D>> d = std::make_shared<GameDrawable<Shaders::Flat3D>>(*drawables, resShader, resMesh, resTexture);
			d->setParent(first ? mManipulator : mItemManipulator.at(i - 1U));
			d->setDrawCallback(this);
			d->setObjectId(i - 1);
			mDrawables.emplace_back(d);
		}

		// Init members
		mRotation = 0.0f;
	}
	else
	{
		if (mAmbientColor == BUBBLE_TIMED)
		{
			mTimed.enabled = true;
			mTimed.factor = timedDelay;
			mTimed.shader = CommonUtility::singleton->getTimedBubbleShader();
			mTimed.textureMask = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_BUBBLE_TIMED);

			const auto color = getColorByIndex(true);
			if (color != Containers::NullOpt)
			{
				mAmbientColor = *color;
			}
			else
			{
				const auto& it = std::next(RoomManager::singleton->sBubbleKeys.begin(), mTimed.index);
				mAmbientColor = Color3::fromSrgb(*it);
			}
		}

		CommonUtility::singleton->createGameSphere(this, *mManipulator, mAmbientColor);
		mRotation = 0.0f;

	}


	// Get flat shader separately
	if (!mTimed.enabled && mAmbientColor != BUBBLE_BLACKHOLE)
	{
		mFlatShader = CommonUtility::singleton->getFlat3DShader();
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
	if (mAmbientColor == BUBBLE_COIN)
	{
		mRotation += mDeltaTime;

		(*mItemManipulator.at(0))
			.resetTransformation()
			.scale(Vector3(0.75f))
			.rotateX(Rad(Deg(90.0f)))
			.rotateY(Deg(mRotation * 180.0f))
			.translate(Vector3(0.0f, 0.0f, -0.5f));

		(*mManipulator)
			.resetTransformation();
	}
	else if (mAmbientColor == BUBBLE_BLACKHOLE)
	{
		mRotation += mDeltaTime;

		mBlackholeAnim -= mDeltaTime * 0.25f;
		if (mBlackholeAnim < 0.0f)
		{
			mBlackholeAnim = 1.0f;
		}

		for (Int i = 0; i < 4; ++i)
		{
			mItemParams[i] = mItemParams[i] - mDeltaTime;
			if (mItemParams[i] < 0.0f)
			{
				mItemParams[i] = 1.0f;
			}

			(*mItemManipulator.at(i))
				.resetTransformation()
				.scale(Vector3(mItemParams[i] * 3.0f))
				.translate(Vector3(0.0f, 0.0f, 0.01f + 0.01f * Float(i)));
		}

		(*mManipulator)
			.resetTransformation()
			.rotateZ(Deg(mRotation * 90.0f))
			.scale(Vector3(2.0f));
	}
	else
	{
		(*mManipulator)
			.resetTransformation();
	}

	// Check for timed behaviour
	if (mTimed.enabled)
	{
		mTimed.factor -= mDeltaTime * 0.2f;

		if (mTimed.factor < 0.0f)
		{
			mTimed.factor = 1.0f;

			const auto color = getColorByIndex(false);
			if (color != Containers::NullOpt)
			{
				mAmbientColor = *color;
				mDrawables.back()->mTexture = CommonUtility::singleton->getTextureForBubble(mAmbientColor);
			}
		}
	}

	// Apply shake effect
	{
		const Vector3 shakeVect = mShakeFact > 0.001f ? mShakePos * std::sin(mShakeFact * Constants::pi()) : Vector3(0.0f);
		(*mManipulator)
			.translate(mPosition + shakeVect);
	}

	// Update bounding box
	updateBBox();
}

void Bubble::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (mTimed.enabled)
	{
		(*mTimed.shader)
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindColorTexture(*baseDrawable->mTexture)
			.bindMaskTexture(*mTimed.textureMask)
			.setTimedRotation(mTimed.factor)
			.draw(*baseDrawable->mMesh);
	}
	else if (mAmbientColor == BUBBLE_BLACKHOLE)
	{
		const Float alpha = baseDrawable == mDrawables[0].get() ? 1.0f : Math::sin(Deg(Math::clamp(mItemParams[baseDrawable->getObjectId()], 0.0f, 1.0f) * 180.0f));
		((Shaders::Flat3D&)baseDrawable->getShader())
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindTexture(*baseDrawable->mTexture)
			.setColor(Color4{ 1.0f, 1.0f, 1.0f, alpha })
			.setAlphaMask(0.001f)
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		(*mFlatShader)
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindTexture(*baseDrawable->mTexture)
			.setAlphaMask(0.001f)
			.setColor(Color4(1.0f))
			.draw(*baseDrawable->mMesh);
	}
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
	mPlayables[0] = std::make_shared<Audio::Playable3D>(*mManipulator, &RoomManager::singleton->mAudioPlayables);
	mPlayables[0]->source()
		.setBuffer(buffer)
		.setLooping(false);

	playSfxAudio(0);
}

Int Bubble::destroyNearbyBubbles(const bool force)
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
			Debug{} << "Collided bubbles of color" << mAmbientColor.toSrgbInt() << "are" << bg.size();

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

	if (!fps->empty())
	{
		bool playSound = !force;
		while (!fps->empty())
		{
			// Get a node from graph
			const auto& gn = fps->front();

			// Create sparkle
			const std::shared_ptr<FallingBubble> ib = std::make_shared<FallingBubble>(mParentIndex, gn.color, GO_FB_TYPE_SPARK);
			ib->mPosition = gn.position;
			RoomManager::singleton->mGoLayers[mParentIndex].push_back(ib);

			// Play sound only once
			if (playSound)
			{
				ib->buildSound();
				ib->playSfxAudio(0);
				playSound = false;
			}

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
	Range3D eb = { mPosition - Vector3(1.25f), mPosition + Vector3(1.25f) };
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
    return Int(group->size());
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

			// Check if bubble is about to be destroyed
			if (bubble->mDestroyMe)
			{
				continue;
			}

			// Perform DFS-like algorithm
			std::unique_ptr<Graph> graph = bubble->destroyDisjointBubblesImpl(group, false);
			// graph->set.insert(bubble);

			Debug{} << "Graph" << (graph->attached ? "attached" : "not attached") << "bubbles are" << graph->set.size();

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

std::unique_ptr<Bubble::Graph> Bubble::destroyDisjointBubblesImpl(std::unordered_set<Bubble*> & group, const bool attached)
{
	// BBox for adjacent bubbles
	Range3D bbox(mPosition - Vector3(1.25f), mPosition + Vector3(1.25f));

	// Check for collisions against other game objects (DFS-like graph)
	std::unique_ptr<std::unordered_set<GameObject*>> collided = RoomManager::singleton->mCollisionManager->checkCollision(bbox, this, { GOT_BUBBLE });
	std::unique_ptr<Graph> graph = std::make_unique<Graph>();
	graph->attached = attached;

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
				if (bi->mAmbientColor == mAmbientColor || !attachedToCeiling)
				{
					graph->set.insert(bi);
				}

				// Perform DFS-like algorithm
				std::unique_ptr<Graph> result = bi->destroyDisjointBubblesImpl(group, graph->attached || attachedToCeiling);

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
		if (mPosition.y() >= -0.1f)
		{
			graph->attached = true;
		}
		else
		{
			graph->set.insert(this);
		}
	}
	else if (mPosition.y() < -0.1f)
	{
		graph->set.insert(this);
	}
	else
	{
		graph->attached = true;
	}

	return graph;
}

const Float Bubble::getShakeSmooth(const Float xt)
{
	return CubicBezier2D(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.06f), Vector2(1.0f, 0.04f), Vector2(1.0f)).value(xt)[1];
}

const Int Bubble::getCustomTypeForFallingBubble(const Color3 & color)
{
	if (color == BUBBLE_COIN)
	{
		return GO_FB_TYPE_COIN;
	}
	else if (color == BUBBLE_STONE)
	{
		return GO_FB_TYPE_STONE;
	}
	else if (color == BUBBLE_BLACKHOLE)
	{
		return GO_FB_TYPE_BLACKHOLE;
	}
	else
	{
		return GO_FB_TYPE_BUBBLE;
	}
}

const Containers::Optional<Color3> Bubble::getColorByIndex(const bool isRandom)
{
	std::set<UnsignedInt> colors;
	for (const auto& item : *RoomManager::singleton->mGoLayers[mParentIndex].list)
	{
		if (item->getType() == GOT_BUBBLE)
		{
			const auto& p = (std::shared_ptr<Bubble>&)item;
			if (CommonUtility::singleton->isBubbleColorValid(p->mAmbientColor))
			{
				colors.insert(p->mAmbientColor.toSrgbInt());
			}
		}
	}

	if (colors.empty())
	{
		mTimed.index = 0;
		return Containers::NullOpt;
	}

	mTimed.index = UnsignedInt(isRandom ? std::rand() : ++mTimed.index) % UnsignedInt(colors.size());
	const auto& it = std::next(colors.begin(), mTimed.index);
	return Color3::fromSrgb(*it);
}
