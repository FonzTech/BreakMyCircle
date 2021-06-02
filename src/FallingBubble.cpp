#include "FallingBubble.h"

#include <thread>

#include <Magnum/Shaders/Flat.h>

#include "CommonUtility.h"
#include "ColoredDrawable.h"
#include "RoomManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

FallingBubble::FallingBubble(const Color3& ambientColor, const bool spark) : GameObject()
{
	// Assign members
	mAmbientColor = ambientColor;
	mSpark = spark;

	mDelay = Float(std::rand() % 250) * 0.001f;

	mDiffuseColor = 0xffffff_rgbf;
	mVelocity = { 0.0f };

	// Create game bubble
	{
		std::shared_ptr<ColoredDrawable> cd = CommonUtility::createGameSphere(*mManipulator, mAmbientColor, this);
		drawables.emplace_back(cd);
	}

	// Create sparkle plane
	if (mSpark)
	{
		std::shared_ptr<ColoredDrawable> cd = CommonUtility::createPlane(*mManipulator, this);
		drawables.emplace_back(cd);
	}
}

Int FallingBubble::getType()
{
	return GOT_FALLING_BUBBLE;
}

void FallingBubble::update()
{
	// Update motion
	if (mSpark)
	{
	}
	else
	{
		// Advance falling animation
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
	}

	// Update transformations
	for (auto& d : drawables)
	{
		d->setTransformation(Matrix4::translation(position));
	}
}

void FallingBubble::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (baseDrawable == drawables.at(0).get())
	{
		(*(Shaders::Phong*) baseDrawable->mShader.get())
			.setLightPositions({ position + Vector3({ 0.0f, 40.0f, 5.0f }) })
			.setDiffuseColor(mDiffuseColor)
			.setAmbientColor(mAmbientColor)
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		(*(Shaders::Flat3D*) baseDrawable->mShader.get())
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.draw(*baseDrawable->mMesh);
	}
}

void FallingBubble::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}