#include "Projectile.h"

#include <thread>
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

Projectile::Projectile(const Int parentIndex, const Color3& ambientColor) : GameObject(parentIndex), mAnimation(0.0f)
{
	// Initialize members
	mAmbientColor = ambientColor;
	mVelocity = { 0.0f };
	mSpeed = 50.0f;

	updateBBox();

	// Set diffuse color
	mDiffuseColor = 0xffffff_rgbf;

	// Create game bubble
	if (mAmbientColor == BUBBLE_BOMB)
	{
		AssetManager().loadAssets(*this, *mManipulator, RESOURCE_SCENE_BOMB, this);
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
			.setMinGain(1.0f)
			.setMaxGain(1.0f)
			.setLooping(false)
			.play();
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

	// Check for collision against other bubbles
	const std::unique_ptr<std::unordered_set<GameObject*>> bubbles = RoomManager::singleton->mCollisionManager->checkCollision(mBbox, this, { GOT_BUBBLE });
	if (bubbles->size() > 0)
	{
		collidedWith(bubbles);
	}
	// Check if projectile reached the top ceiling
	else if (mPosition.y() > 0.0f)
	{
		snapToGrid();
	}

	// Advance animation
	mAnimation -= mVelocity[0];

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
		.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
		.draw(*baseDrawable->mMesh);
}

void Projectile::snapToGrid()
{
	// Stop this projectile
	mVelocity = Vector3(0.0f);

	// Get row index
	const Int thisRowIndex = getRowIndexByBubble();

	// Snap to grid
	Float offset = thisRowIndex % 2 ? 0.0f : 1.0f;
	mPosition = {
		round((mPosition.x() - offset) / 2.0f) * 2.0f + offset,
		getSnappedYPos(),
		0.0f
	};

	// Check if projectile is a bomb
	if (mAmbientColor == BUBBLE_BOMB)
	{
		// Create explosion sprite
		{
			const std::shared_ptr<FallingBubble> ib = std::make_shared<FallingBubble>(mParentIndex, 0xffffff_rgbf, GO_FB_TYPE_BOMB);
			ib->mPosition = mPosition + Vector3(0.0f, 0.0f, 0.5f);
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
			Float offsetZ = 0.0f;

			const Float r = getSquaredRadiusForExplosion();
			for (auto& item : bubbles)
			{
				const Float d = CommonUtility::singleton->getDistanceSquared<3>(item->mPosition, mPosition);
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
		if (b->destroyNearbyBubbles(false, 0.0f))
		{
			b->destroyDisjointBubbles();
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
		mShootCallback.lock()->shootCallback(ISC_STATE_SHOOT_FINISHED);
	}

	// Destroy me
	mDestroyMe = true;
}

void Projectile::updateBBox()
{
	// Update bounding box
	mBbox = Range3D{ mPosition - Vector3(0.8f), mPosition + Vector3(0.8f) };
}

void Projectile::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
	// Bubble* bubble = (Bubble*) *gameObjects->begin();
	snapToGrid();
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

const Float Projectile::getSnappedYPos() const
{
	return Math::round(mPosition.y() / 2.0f) * 2.0f;
}

const Float Projectile::getSquaredRadiusForExplosion() const
{
	return 32.0f;
}

const void Projectile::playStompSound()
{
	printf("qqqqq\n");
	mPlayables[0]->source()
		.setOffsetInBytes(0)
		.play();
}