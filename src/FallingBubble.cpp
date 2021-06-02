#include "FallingBubble.h"

#include <thread>

#include "CommonUtility.h"
#include "ColoredDrawable.h"
#include "RoomManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

FallingBubble::FallingBubble(const Color3& ambientColor) : GameObject()
{
	// Assign members
	mAmbientColor = ambientColor;
	mDelay = Float(std::rand() % 250) * 0.001f;

	mDiffuseColor = 0xffffff_rgbf;
	mVelocity = { 0.0f };

	// Create game bubble
	std::shared_ptr<ColoredDrawable> cd = CommonUtility::createGameSphere(*mManipulator, mAmbientColor, this);
	drawables.emplace_back(cd);
}

Int FallingBubble::getType()
{
	return GOT_FALLING_BUBBLE;
}

void FallingBubble::update()
{
	// Update motion
	if (mDelay > 0.0f)
	{
		mDelay -= mDeltaTime;
	}
	else
	{
		mVelocity += { 0.0f, -2.0f, 0.0f };
		position += mVelocity * mDeltaTime;
	}

	// Check for off-screen position
	if (position.y() < -300.0f)
	{
		destroyMe = true;
		return;
	}

	// Update transformations
	for (auto& d : drawables)
	{
		d->setTransformation(Matrix4::translation(position));
	}
}

void FallingBubble::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	(*baseDrawable->mShader.get())
		.setLightPositions({ position + Vector3({ 10.0f, 10.0f, 1.75f }) })
		.setDiffuseColor(mDiffuseColor)
		.setAmbientColor(mAmbientColor)
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.draw(*baseDrawable->mMesh);
}

void FallingBubble::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}