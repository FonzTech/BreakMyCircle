#include "Player.h"

#include <memory>

#include <Magnum/Math/Angle.h>
#include <Magnum/Math/Intersection.h>
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

std::shared_ptr<GameObject> Player::getInstance(nlohmann::json params)
{
	std::shared_ptr<Player> p = std::make_shared<Player>();
	return p;
}

Player::Player() : GameObject()
{
	// Load asset as first drawable
	{
		mShooterManipulator = new Object3D{ &RoomManager::singleton->mScene };

		AssetManager am;
		am.loadAssets(*this, *mShooterManipulator, "scenes/cannon_1.glb", this);
	}

	// Load path for new sphere animation
	{
		mProjPath = std::make_unique<LinePath>(RESOURCE_PATH_NEW_SPHERE);
		mProjPath->mProgress = Float(mProjPath->getSize());
	}

	// Set diffuse color
	mDiffuseColor = 0xffffff_rgbf;

	// Set members
	mColors = {
		0x0000c0_rgbf,
		0x00c000_rgbf,
		0xc00000_rgbf,
		0x00c0c0_rgbf,
	};
	mAmbientColorIndex[0] = std::rand() % mColors.size();
	mAmbientColorIndex[1] = std::rand() % mColors.size();
	mShootAngle = Rad(0.0f);

	// Create game bubble
	mSphereManipulator = new Object3D{ &RoomManager::singleton->mScene };

	std::shared_ptr<ColoredDrawable<Shaders::Phong>> cd = CommonUtility::singleton->createGameSphere(*mSphereManipulator, mColors[mAmbientColorIndex[0]], this);
	mDrawables.emplace_back(cd);

	mSphereDrawables[0] = cd.get();
}

const Int Player::getType() const
{
	return GOT_PLAYER;
}

void Player::update()
{
	// Update new projectile path
	mProjPath->update(mDeltaTime * Float(-mProjPath->getSize()));

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
			std::shared_ptr<Projectile> go = std::make_shared<Projectile>(mColors[mAmbientColorIndex[0]]);
			go->position = position;
			go->mVelocity = -Vector3(Math::cos(mShootAngle), Math::sin(mShootAngle), 0.0f);
			RoomManager::singleton->mGameObjects.push_back(std::move(go));

			// Update color for next bubble
			mAmbientColorIndex[0] = mAmbientColorIndex[1];
			mAmbientColorIndex[1] = std::rand() % mColors.size();

			// Reset animation for new projectile
			mProjPath->mProgress = Float(mProjPath->getSize());

			// Prevent shooting by keeping a reference
			mProjectile = RoomManager::singleton->mGameObjects.back();
		}
	}

	// Shooter manipulation
	{
		// Compute transformations
		Matrix4 translation = Matrix4::translation(position);
		Float finalAngle(Deg(mShootAngle) + Deg(90.0f));

		// Apply transformations to shooter
		mShooterManipulator->setTransformation(Matrix4());
		mShooterManipulator->transform(Matrix4::rotationX(Deg(90.0f)));
		mShooterManipulator->transform(Matrix4::rotationZ(Deg(finalAngle)));
		mShooterManipulator->transform(translation);
	}

	// Sphere manipulation
	{
		// Apply transformations to bubbles
		mSphereManipulator->setTransformation(Matrix4());
		mSphereManipulator->transform(Matrix4::translation(mProjPath->getCurrentPosition()));
		mSphereManipulator->transform(Matrix4::rotationX(Deg(-90.0f)));
		mSphereManipulator->transform(Matrix4::translation(position + Vector3(0.0f, 0.0f, -1.20f)));
	}
}

void Player::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (baseDrawable == mSphereDrawables[0])
	{
		((Shaders::Phong&) baseDrawable->getShader())
			.setLightPositions({ position + Vector3({ 0.0f, 150.0f, 40.0f }) })
			.setDiffuseColor(mDiffuseColor)
			.setAmbientColor(mColors[mAmbientColorIndex[0]])
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