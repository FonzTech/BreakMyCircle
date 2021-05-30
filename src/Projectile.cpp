#include "Projectile.h"

#include <thread>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "ColoredDrawable.h"
#include "RoomManager.h"
#include "Bubble.h"
#include "CommonUtility.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

Projectile::Projectile(const Color3& ambientColor) : GameObject()
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
	std::shared_ptr<ColoredDrawable> cd = CommonUtility::createGameSphere(*mManipulator, mAmbientColor, this);
	drawables.emplace_back(cd);
}

Int Projectile::getType()
{
	return GOT_PROJECTILE;
}

void Projectile::update()
{
	// Affect position by velocity
	position += mVelocity * deltaTime * mSpeed;

	// Bounce against side walls
	if (position.x() <= mLeftX && mVelocity.x() < 0.0f)
	{
		position[0] = mLeftX + 0.01f;
		mVelocity[0] *= -1.0f;
	}
	else if (position.x() > mRightX && mVelocity.x() > 0.0f)
	{
		position[0] = mRightX - 0.01f;
		mVelocity[0] *= -1.0f;
	}

	// Update bounding box
	updateBBox();

	// Check for collision against other bubbles
	std::shared_ptr<GameObject> bubble = RoomManager::singleton->mCollisionManager->checkCollision(this);
	if (bubble != nullptr)
	{
		collidedWith(bubble.get());
	}
}

void Projectile::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	(*baseDrawable->mShader)
		.setLightPositions({ position + Vector3({ 10.0f, 10.0f, 1.75f }) })
		.setDiffuseColor(mDiffuseColor)
		.setAmbientColor(mAmbientColor)
		.setTransformationMatrix(transformationMatrix * Matrix4::translation(position))
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.draw(*baseDrawable->mMesh);
}

void Projectile::updateBBox()
{
	// Update bounding box
	bbox = Range3D{ position - Vector3(0.8f), position + Vector3(0.8f) };
}

void Projectile::collidedWith(GameObject* gameObject)
{
	Bubble* bubble = (Bubble*)gameObject;

	// Stop this projectile
	mVelocity = Vector3(0.0f);

	// Get row index
	const Int thisRowIndex = getRowIndexByBubble();

	// Snap to grid
	Float offset = thisRowIndex % 2 ? 0.0f : 1.0f;
	position = {
		round((position.x() - offset) / 2.0f) * 2.0f + offset,
		getSnappedYPos(),
		0.0f
	};

	// Correct position to protect against overlaps
	std::thread tjob(&Projectile::adjustPosition, this);
	tjob.join();
	
	// Create new bubbles with the same color
	std::shared_ptr<Bubble> b = std::make_shared<Bubble>(mAmbientColor);
	b->position = position;
	b->updateBBox();
	RoomManager::singleton->mGameObjects.push_back(b);

	// Apply ripple effect
	for (auto& go : RoomManager::singleton->mGameObjects)
	{
		if (go != b && go->getType() == GOT_BUBBLE)
		{
			((Bubble*)go.get())->applyRippleEffect(position);
		}
	}

	// Destroy nearby bubbles
	b->destroyNearbyBubbles();

	// Destroy me
	destroyMe = true;
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

	for (auto& go : RoomManager::singleton->mGameObjects)
	{
		if (go.get() == this || go->position.y() != position.y())
		{
			continue;
		}
		else if (go->position.x() == position.x())
		{
			overlaps = true;
		}
		else if (go->position.x() == position.x() - 2.0f)
		{
			toLeftX = go->position.x();
		}
		else if (go->position.x() == position.x() + 2.0f)
		{
			toRightX = go->position.x();
		}
	}

	if (!overlaps)
	{
		return;
	}

	if (position.x() >= mLeftX + (mRightX - mLeftX) * 0.5f)
	{
		position[0] += toLeftX <= IMPOSSIBLE_PROJECTILE_XPOS ? -2.0f : 2.0f;
	}
	else
	{
		position[0] += toRightX <= IMPOSSIBLE_PROJECTILE_XPOS ? 2.0f : -2.0f;
	}
}

Float Projectile::getSnappedYPos()
{
	return round(position.y() / 2.0f) * 2.0f;
}