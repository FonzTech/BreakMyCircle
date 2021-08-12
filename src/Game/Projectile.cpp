#include "Projectile.h"

#include <thread>
#include <Magnum/GL/DefaultFramebuffer.h>

#include "../Graphics/GameDrawable.h"
#include "../RoomManager.h"
#include "../Common/CommonUtility.h"
#include "Bubble.h"
#include "FallingBubble.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> Projectile::getInstance(const nlohmann::json & params)
{
	// No default constructor exists for this class!!
	return nullptr;
}

Projectile::Projectile(const Int parentIndex, const Color3& ambientColor) : GameObject(parentIndex)
{
	// Initialize members
	mAmbientColor = ambientColor;
	mVelocity = { 0.0f };
	mLeftX = 1.0f;
	mRightX = 15.0f;
	mSpeed = 50.0f;

	updateBBox();

	// Set diffuse color
	mDiffuseColor = 0xffffff_rgbf;

	// Create game bubble
	CommonUtility::singleton->createGameSphere(this, *mManipulator, mAmbientColor);
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
	if (mPosition.x() <= mLeftX && mVelocity.x() < 0.0f)
	{
		mPosition[0] = mLeftX + 0.01f;
		mVelocity[0] *= -1.0f;
	}
	else if (mPosition.x() > mRightX && mVelocity.x() > 0.0f)
	{
		mPosition[0] = mRightX - 0.01f;
		mVelocity[0] *= -1.0f;
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

	// Upgrade transformations
	mDrawables.at(0)->setTransformation(Matrix4::translation(mPosition));
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
	if (b->destroyNearbyBubbles())
	{
		b->destroyDisjointBubbles();
	}
	else
	{
		b->playStompSound();
	}

	// Add to room
	RoomManager::singleton->mGoLayers[mParentIndex].push_back(b);

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

Int Projectile::getRowIndexByBubble()
{
	return std::abs(Int(getSnappedYPos() * 0.5f));
}

void Projectile::adjustPosition()
{
	bool overlaps = false;
	Float toLeftX = IMPOSSIBLE_PROJECTILE_XPOS;
	Float toRightX = IMPOSSIBLE_PROJECTILE_XPOS;

	for (auto& go : *RoomManager::singleton->mGoLayers[mParentIndex].list)
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

	if (mPosition.x() >= mLeftX + (mRightX - mLeftX) * 0.5f)
	{
		mPosition[0] += toLeftX <= IMPOSSIBLE_PROJECTILE_XPOS ? -2.0f : 2.0f;
	}
	else
	{
		mPosition[0] += toRightX <= IMPOSSIBLE_PROJECTILE_XPOS ? 2.0f : -2.0f;
	}
}

Float Projectile::getSnappedYPos()
{
	return round(mPosition.y() / 2.0f) * 2.0f;
}