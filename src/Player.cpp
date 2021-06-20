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
#include "Bubble.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> Player::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Sint8 parent;
	params.at("parent").get_to(parent);

	// Instantiate player object
	std::shared_ptr<Player> p = std::make_shared<Player>(parent);
	return p;
}

Player::Player(const Sint8 parentIndex) : GameObject()
{
	// Assign parent index
	mParentIndex = parentIndex;

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

	// Set members
	mDiffuseColor = 0xffffff_rgbf;
	mProjColors[0] = getRandomEligibleColor();
	mProjColors[1] = getRandomEligibleColor();
	mShootAngle = Rad(0.0f);

	// Create game bubble
	mSphereManipulator = new Object3D{ &RoomManager::singleton->mScene };

	std::shared_ptr<ColoredDrawable<Shaders::Phong>> cd = CommonUtility::singleton->createGameSphere(mParentIndex, *mSphereManipulator, mProjColors[0].rgb(), this);
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
	const auto& bs = InputManager::singleton->mMouseStates[ImMouseButtons::Left];
	if (bs == IM_STATE_PRESSED)
	{
		mShootTimeline = 1.0f;
	}
	else if (bs == IM_STATE_RELEASED)
	{
		if (mProjectile.expired())
		{
			// Create projectile
			std::shared_ptr<Projectile> go = std::make_shared<Projectile>(mParentIndex, mProjColors[0].rgb());
			go->position = position;
			go->mVelocity = -Vector3(Math::cos(mShootAngle), Math::sin(mShootAngle), 0.0f);
			RoomManager::singleton->mGoLayers[mParentIndex].push_back(go);

			// Update color for next bubble
			mProjColors[0] = mProjColors[1];
			mProjColors[1] = getRandomEligibleColor();

			// Reset animation for new projectile
			mProjPath->mProgress = Float(mProjPath->getSize());

			// Prevent shooting by keeping a reference
			(*RoomManager::singleton->mGoLayers[mParentIndex].list).back();
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
			.setAmbientColor(mProjColors[0].rgb())
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

Color4 Player::getRandomEligibleColor()
{
	// Create array of bubbles
	std::vector<std::weak_ptr<GameObject>> bubbles;
	for (const auto& item : *RoomManager::singleton->mGoLayers[mParentIndex].list)
	{
		if (item->getType() == GOT_BUBBLE)
		{
			bubbles.push_back(item);
		}
	}

	// Check for array size
	if (!bubbles.size())
	{
		return 0x00000000_rgbaf;
	}

	// Get random color from one in-game bubble
	const Int index = std::rand() % bubbles.size();
	const auto& color = ((Bubble*) bubbles[index].lock().get())->mAmbientColor;
	return Color4(color.r(), color.g(), color.b());
}