#include "Projectile.h"

#include <thread>
#include <functional>
#include <Magnum/GL/DefaultFramebuffer.h>

#include "../AssetManager.h"
#include "../RoomManager.h"
#include "../Graphics/GameDrawable.h"
#include "../Common/CommonUtility.h"
#include "Bubble.h"
#include "FallingBubble.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

Float Projectile::LEFT_X = 0.0f;
Float Projectile::RIGHT_X = 0.0f;
Float Projectile::MID_X = 0.0f;

void Projectile::setGlobalParameters(const Float leftX, const Float rightX)
{
	LEFT_X = leftX;
	RIGHT_X = rightX;
	MID_X = LEFT_X + (RIGHT_X - LEFT_X) * 0.5f;
}

std::shared_ptr<GameObject> Projectile::getInstance(const nlohmann::json & params)
{
	// No default constructor exists for this class!!
	return nullptr;
}

Projectile::Projectile(const Int parentIndex, const Color3& ambientColor) : GameObject(parentIndex), mAnimation(0.0f), mCustomTexture(nullptr)
{
	// Initialize members
	mParentIndex = parentIndex;
	mAmbientColor = ambientColor;
	mVelocity = Vector3(0.0f);
	mSpeed = 50.0f;

	updateBBox();

	// Set diffuse color
	mDiffuseColor = 0xffffff_rgbf;

	// Create game bubble
	if (mAmbientColor == BUBBLE_BOMB)
	{
		AssetManager().loadAssets(*this, *mManipulator, RESOURCE_SCENE_BOMB, this);
	}
	else if (mAmbientColor == BUBBLE_ELECTRIC)
	{
		const std::shared_ptr<ElectricBall> go = std::make_shared<ElectricBall>(mParentIndex);
		mElectricBall = (std::shared_ptr<ElectricBall>&) RoomManager::singleton->mGoLayers[mParentIndex].push_back(go, true);
		mElectricBall->playSfxAudio(0);
	}
	else
	{
		CommonUtility::singleton->createGameSphere(this, *mManipulator, mAmbientColor);
	}

	// Load stomp sound
	{
		Resource<Audio::Buffer> buffer = CommonUtility::singleton->loadAudioData(RESOURCE_AUDIO_BUBBLE_STOMP);
		mPlayables[0] = std::make_shared<Audio::Playable3D>(*mManipulator.get(), &RoomManager::singleton->mAudioPlayables);
		mPlayables[0]->source()
			.setBuffer(buffer)
			.setLooping(false);
		// playSfxAudio(0);
	}
}

Projectile::~Projectile()
{
	if (mElectricBall != nullptr)
	{
		mElectricBall->mDestroyMe = true;
		mElectricBall = nullptr;
	}
}

const Int Projectile::getType() const
{
	return GOT_PROJECTILE;
}

void Projectile::update()
{
	// Affect position by velocity
	mPosition += mVelocity * mDeltaTime * mSpeed;

	// Setup electric ball, if required
	if (mElectricBall != nullptr)
	{
		mElectricBall->mPosition = mPosition;
	}

	// Bounce against side walls
	if (mPosition.x() <= LEFT_X && mVelocity.x() < 0.0f)
	{
		mPosition[0] = LEFT_X + 0.01f;
		mVelocity[0] *= -1.0f;

		playStompSound();
	}
	else if (mPosition.x() > RIGHT_X && mVelocity.x() > 0.0f)
	{
		mPosition[0] = RIGHT_X - 0.01f;
		mVelocity[0] *= -1.0f;

		playStompSound();
	}

	// Update bounding box
	updateBBox();

	// Check for collision against blackhole
	{
		const std::function<bool(GameObject* go)> verifier = [](GameObject* go) {
			const Bubble* b = static_cast<Bubble*>(go);
			return b->mAmbientColor == BUBBLE_BLACKHOLE;
		};

		const auto bbox = Range3D{ mBbox.min() - Vector3(6.0f), mBbox.max() + Vector3(6.0f) };
		const std::unique_ptr<std::unordered_set<GameObject*>> bubbles = RoomManager::singleton->mCollisionManager->checkCollision(bbox, this, { GOT_BUBBLE }, verifier);
		{
			for (const auto& b : *bubbles)
			{
				if (b->mPosition.x() < mPosition.x())
				{
					mVelocity.data()[0] -= mDeltaTime * 4.0f;
				}
				else
				{
					mVelocity.data()[0] += mDeltaTime * 4.0f;
				}
			}
		}
	}

	// Check for collision against other bubbles
	{
		const std::unique_ptr<std::unordered_set<GameObject*>> bubbles = RoomManager::singleton->mCollisionManager->checkCollision(mBbox, this, { GOT_BUBBLE });
		{
			if (!bubbles->empty())
			{
				collidedWith(bubbles);
			}
		}
	}

	// Check if projectile reached the top ceiling
	if (mPosition.y() > 0.0f)
	{
		snapToGrid(nullptr);
	}

	// Rotate animation
	if (mAmbientColor == BUBBLE_BOMB)
	{
		// Advance animation
		mAnimation -= mVelocity[0];
	}

	// Upgrade transformations
	(*mManipulator)
		.resetTransformation()
		.rotateZ(Deg(mAnimation * 10.0f))
		.translate(mPosition);
}

void Projectile::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	((Shaders::Phong&) baseDrawable->getShader())
		.setLightPosition(mPosition + Vector3(0.0f, 0.0f, 1.0f))
		.setLightColor(0xffffff60_rgbaf)
		.setSpecularColor(0xffffff00_rgbaf)
		.setAmbientColor(0x808080_rgbf)
		.setDiffuseColor(0x808080_rgbf)
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.bindTextures(mCustomTexture != nullptr ? mCustomTexture : baseDrawable->mTexture, mCustomTexture != nullptr ? mCustomTexture : baseDrawable->mTexture, nullptr, nullptr)
		.draw(*baseDrawable->mMesh);
}

void Projectile::snapToGrid(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
	// Stop this projectile
	mVelocity = Vector3(0.0f);

	// Get row index
	const Color3 preColor = mAmbientColor;
	const Int thisRowIndex = getRowIndexByBubble();

	// Get nearest bubble
	Containers::Optional<Color3> collidedColor = Containers::NullOpt;
	{
		Bubble* nearestBubble = nullptr;
		Bubble* leftBubble = nullptr;
		Bubble* rightBubble = nullptr;
		Float nearestDistance = 0.0f;

		if (gameObjects != nullptr && !gameObjects->empty())
		{
			const Float sy = getSnappedYPos();
			for (const auto& item : *gameObjects)
			{
				if (item->getType() == GOT_BUBBLE)
				{
					const Float distance = (mPosition - item->mPosition).length();
					if (nearestBubble == nullptr || distance < nearestDistance)
					{
						Bubble* b = (Bubble*)item;

						nearestDistance = (mPosition - item->mPosition).length();
						nearestBubble = b;

						if (distance <= 0.6f)
						{
							(item->mPosition.x() < mPosition.x() ? leftBubble : rightBubble)= b;
						}

						if (CommonUtility::singleton->isBubbleColorValid(b->mAmbientColor))
						{
							collidedColor = b->mAmbientColor;
						}
					}
				}
			}
		}

		if (nearestBubble != nullptr)
		{
			if (leftBubble != nullptr && rightBubble == nullptr)
			{
				mPosition = leftBubble->mPosition + Vector3(1.0f, 0.0f, 0.0f);
			}
			else if (leftBubble == nullptr && rightBubble != nullptr)
			{
				mPosition = rightBubble->mPosition + Vector3(-1.0f, 0.0f, 0.0f);
			}
			else if (leftBubble != nullptr && rightBubble != nullptr)
			{
				mPosition = nearestBubble->mPosition + Vector3(0.0f, -1.0f, 0.0f);
			}
		}
	}

	// Snap to grid
	Float offset = thisRowIndex % 2 ? 0.0f : 1.0f;
	mPosition = {
		Math::round((mPosition.x() - offset) / 2.0f) * 2.0f + offset,
		getSnappedYPos(),
		0.0f
	};

	// Mutate the color, if this projectile is a plasma or electric bubble
	if (mAmbientColor == BUBBLE_PLASMA || mAmbientColor == BUBBLE_ELECTRIC)
	{
		if (collidedColor != Containers::NullOpt)
		{
			mAmbientColor = *collidedColor;
		}
		else
		{
			while (true)
			{
				const auto& it = std::next(std::begin(RoomManager::singleton->sBubbleColors), std::rand() % RoomManager::singleton->sBubbleColors.size());
				if (CommonUtility::singleton->isBubbleColorValid(it->second.color))
				{
					mAmbientColor = it->second.color;
					break;
				}
			}
		}
	}
	Debug{} << "mAmbientColormAmbientColor" << mAmbientColor;

	// Check if projectile is a bomb
	Int shootAmount = 0;
	if (preColor == BUBBLE_BOMB)
	{
		// Create explosion sprite
		{
			const std::shared_ptr<FallingBubble> ib = std::make_shared<FallingBubble>(mParentIndex, 0xffffff_rgbf, GO_FB_TYPE_BOMB);
			ib->mPosition = mPosition + Vector3(0.0f, 0.0f, 1.0f);
			ib->buildSound();
			RoomManager::singleton->mGoLayers[mParentIndex].push_back(ib);
		}

		// Force destroy of nearby bubbles
		{
			std::vector<std::shared_ptr<Bubble>> bubbles;
			for (auto& item : *RoomManager::singleton->mGoLayers[mParentIndex].list)
			{
				if (item->getType() == GOT_BUBBLE)
				{
					bubbles.push_back((std::shared_ptr<Bubble>&)item);
				}
			}

			std::shared_ptr<Bubble> destroyLater = nullptr;
			Float offsetZ = 0.3f;

			const Float r = getRadiusForExplosion();
			for (auto& item : bubbles)
			{
				const Float d = Math::abs((item->mPosition - mPosition).length());
				if (d <= r && item->mAmbientColor != BUBBLE_COIN && item->destroyNearbyBubbles(true, offsetZ))
				{
					destroyLater = item;
					offsetZ += 0.05f;
				}
			}

			if (destroyLater != nullptr)
			{
				destroyLater->destroyDisjointBubbles();
			}
		}
	}
	// Check if projectile is an electric bubble
	else if (preColor == BUBBLE_ELECTRIC)
	{
		std::vector<std::shared_ptr<Bubble>> bubbles;
		const auto& list = RoomManager::singleton->mGoLayers[mParentIndex].list;

		for (Int i = 0, j = std::rand(); i != list->size(); ++i, j = std::rand())
		{
			const auto& item = list->at((i + j) % list->size());
			if (item->getType() == GOT_BUBBLE)
			{
				const auto& b = (std::shared_ptr<Bubble>&)item;
				if (b->mAmbientColor == mAmbientColor)
				{
					bubbles.push_back(b);
					if (bubbles.size() >= 3)
					{
						break;
					}
				}
			}
		}

		// Destroy nearby bubbles and disjoint bubble groups
		for (auto& b : bubbles)
		{
			if (shootAmount = b->destroyNearbyBubbles(false, 0.6f))
			{
				shootAmount += b->destroyDisjointBubbles();
			}
		}
	}
	else
	{
		// Correct position to protect against overlaps
		std::thread tjob(&Projectile::adjustPosition, this);
		tjob.join();

		// Create new bubbles with the same color
		std::shared_ptr<Bubble> b = std::make_shared<Bubble>(mParentIndex, mAmbientColor);
		b->mPosition = mPosition;
		b->updateBBox();

		// Apply ripple effect
		for (auto& go : *RoomManager::singleton->mGoLayers[mParentIndex].list)
		{
			if (go != b && go->getType() == GOT_BUBBLE)
			{
				((Bubble*)go.get())->applyRippleEffect(mPosition);
			}
		}

		// Destroy nearby bubbles and disjoint bubble groups
		if (shootAmount = b->destroyNearbyBubbles(false, 0.6f))
		{
			shootAmount += b->destroyDisjointBubbles();
		}
		else
		{
			b->playStompSound();
		}

		// Add to room
		RoomManager::singleton->mGoLayers[mParentIndex].push_back(b);
	}

	// Launch callback
	if (!mShootCallback.expired())
	{
		mShootCallback.lock()->shootCallback(ISC_STATE_SHOOT_FINISHED, preColor, mAmbientColor, shootAmount);
	}

	// Move sparkles on top
	for (auto& go : *RoomManager::singleton->mGoLayers[mParentIndex].list)
	{
		if (go->getType() == GOT_FALLING_BUBBLE)
		{
			const auto& sp = (std::shared_ptr<FallingBubble>&)go;
			if (sp->mCustomType == GO_FB_TYPE_SPARK)
			{
				sp->pushToFront();
			}
		}
	}

	// Destroy me
	mDestroyMe = true;
}

void Projectile::updateBBox()
{
	// Update bounding box
	mBbox = Range3D{ mPosition - Vector3(0.9f), mPosition + Vector3(0.9f, 1.0f + mDeltaTime, 0.0f) };
}

void Projectile::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
	// Bubble* bubble = (Bubble*) *gameObjects->begin();
	snapToGrid(gameObjects);
}

const Int Projectile::getRowIndexByBubble() const
{
	return std::abs(Int(getSnappedYPos() * 0.5f));
}

void Projectile::adjustPosition()
{
	bool overlaps = false;
	Float toLeftX = IMPOSSIBLE_PROJECTILE_XPOS;
	Float toRightX = IMPOSSIBLE_PROJECTILE_XPOS;

	for (const auto& go : *RoomManager::singleton->mGoLayers[mParentIndex].list)
	{
		if (go.get() == this || go->mPosition.y() != mPosition.y())
		{
			continue;
		}
		else if (go->mPosition.x() == mPosition.x())
		{
			overlaps = true;
		}
		else if (go->mPosition.x() == mPosition.x() - 2.0f)
		{
			toLeftX = go->mPosition.x();
		}
		else if (go->mPosition.x() == mPosition.x() + 2.0f)
		{
			toRightX = go->mPosition.x();
		}
	}

	if (!overlaps)
	{
		return;
	}

	if (mPosition.x() >= MID_X)
	{
		mPosition[0] += toLeftX <= IMPOSSIBLE_PROJECTILE_XPOS ? -2.0f : 2.0f;
	}
	else
	{
		mPosition[0] += toRightX <= IMPOSSIBLE_PROJECTILE_XPOS ? 2.0f : -2.0f;
	}
}

void Projectile::setCustomTexture(GL::Texture2D & texture)
{
	mCustomTexture = &texture;
}

const Float Projectile::getSnappedYPos() const
{
	return Math::round(mPosition.y() / 2.0f) * 2.0f;
}

const Float Projectile::getRadiusForExplosion() const
{
	return 6.0f;
}

const void Projectile::playStompSound()
{
	playSfxAudio(0);
}