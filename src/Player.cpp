#include "Player.h"

#include <Magnum/Math/Angle.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "AssetManager.h"
#include "InputManager.h"
#include "RoomManager.h"
#include "Projectile.h"
#include "CommonUtility.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

Player::Player() : GameObject()
{
	// Load asset as first drawable
	AssetManager::singleton->loadAssets(*this, "scenes/cannon_1.glb", this);

	// Set diffuse color
	mDiffuseColor = 0xffffff_rgbf;

	// Set members
	mColors = {
		0x0000c0_rgbf,
		0x00c000_rgbf,
		0xc00000_rgbf,
		0x00c0c0_rgbf,
	};
	mAmbientColorIndex = std::rand() % mColors.size();
	mShootAngle = Rad(0.0f);

	// Create game bubble
	mSphereManipulator = new Object3D{ &RoomManager::singleton->mScene };

	std::shared_ptr<ColoredDrawable<Shaders::Phong>> cd = CommonUtility::singleton->createGameSphere(*mSphereManipulator, mColors[mAmbientColorIndex], this);
	drawables.emplace_back(cd);

	mSphereDrawables[0] = cd.get();
}

const Int Player::getType() const
{
	return GOT_PLAYER;
}

void Player::update()
{
	// Update shoot timeline for animation
	mShootTimeline -= mDeltaTime;

	// Get shooting angle by mouse
	{
		Vector2 p1 = Vector2(InputManager::singleton->mMousePosition);
		Vector2 p2 = Vector2({ RoomManager::singleton->windowSize.x() * 0.5f, Float(RoomManager::singleton->windowSize.y()) });
		Vector2 pdir = p2 - p1;

		Float value = std::atan2(-pdir.y(), pdir.x());

		Rad finalValue(Math::clamp(value, SHOOT_ANGLE_MIN_RAD, SHOOT_ANGLE_MAX_RAD));
		if (mShootTimeline >= 0.0f)
		{
			Rad fact = mShootAngle - finalValue;
			mShootAngle -= fact / 4.0f;
		}
		else
		{
			mShootAngle = finalValue;
		}
	}

	// Check for mouse input
	auto& bs = InputManager::singleton->mMouseStates[ImMouseButtons::Left];
	if (bs == IM_STATE_PRESSED)
	{
		mShootTimeline = 1.0f;
	}
	else if (bs == IM_STATE_RELEASED)
	{
		if (mProjectile.expired())
		{
			// Create projectile
			std::shared_ptr<Projectile> go = std::make_shared<Projectile>(mColors[mAmbientColorIndex]);
			go->position = position;
			go->mVelocity = -Vector3(Math::cos(mShootAngle), Math::sin(mShootAngle), 0.0f);
			RoomManager::singleton->mGameObjects.push_back(std::move(go));

			// Update color for next bubble
			mAmbientColorIndex = std::rand() % mColors.size();

			// Prevent shooting by keeping a reference
			mProjectile = RoomManager::singleton->mGameObjects.back();
		}
	}

	// Compute transformations
	Matrix4 translation = Matrix4::translation(position);
	Float finalAngle(Deg(mShootAngle) + Deg(90.0f));

	// Apply transformations to shooter
	const auto& m = translation * Matrix4::rotationZ(Deg(finalAngle)) * Matrix4::rotationX(Deg(90.0f));
	mManipulator->setTransformation(m);

	// Apply transformations to bubbles
	mSphereManipulator->setTransformation(translation);
}

void Player::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (baseDrawable == mSphereDrawables[0])
	{
		((Shaders::Phong&) baseDrawable->getShader())
			.setLightPositions({ position + Vector3({ 0.0f, 150.0f, 40.0f }) })
			.setDiffuseColor(mDiffuseColor)
			.setAmbientColor(mColors[mAmbientColorIndex])
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		((Shaders::Phong&) baseDrawable->getShader())
			.setLightPosition(camera.cameraMatrix().transformPoint(position + Vector3(0.0f, 0.0f, 20.0f)))
			.setLightColor(0xffffffff_rgbaf)
			.setSpecularColor(0xffffff00_rgbaf)
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
			.draw(*baseDrawable->mMesh);
	}
}

void Player::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}