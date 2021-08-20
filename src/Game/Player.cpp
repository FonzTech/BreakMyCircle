#include "Player.h"

#include <memory>

#include <Magnum/Math/Angle.h>
#include <Magnum/Math/Intersection.h>
#include <Magnum/GL/DefaultFramebuffer.h>

#include "../AssetManager.h"
#include "../InputManager.h"
#include "../RoomManager.h"
#include "../Common/CommonUtility.h"
#include "Projectile.h"
#include "Bubble.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> Player::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate player object
	std::shared_ptr<Player> p = std::make_shared<Player>(parent);
	return p;
}

Player::Player(const Int parentIndex) : GameObject(), mCanShoot(true), mAnimation(0.0f)
{
	// Assign parent index
	mParentIndex = parentIndex;

	// Load asset as first drawable
	{
		mShooterManipulator = new Object3D{ mManipulator.get() };
		AssetManager().loadAssets(*this, *mShooterManipulator, RESOURCE_SCENE_CANNON_1, this);
	}

	// Load hidden drawables (for later use)
	{
		// Load asset
		mBombManipulator = new Object3D{ mManipulator.get() };
		AssetManager().loadAssets(*this, *mBombManipulator, RESOURCE_SCENE_BOMB, this);

		// Keep references
		for (UnsignedInt i = 0; i < 3; ++i)
		{
			mBombDrawables[i] = mDrawables[mDrawables.size() - i - 1].get();
		}

		// Put "Spark" mesh always at the beginning
		for (UnsignedInt i = 0; i < 3; ++i)
		{
			if (mBombDrawables[i]->mMesh->label() == "SparkV")
			{
				if (i != 0)
				{
					const auto& p = mBombDrawables[0];
					mBombDrawables[0] = mBombDrawables[i];
					mBombDrawables[i] = p;
				}
				break;
			}
		}
	}

	/*
	// Load path for new sphere animation
	{
		mProjPath = std::make_unique<LinePath>(RESOURCE_PATH_NEW_SPHERE);
		mProjPath->mProgress = Float(mProjPath->getSize());
	}
	*/

	// Set members
	mShootAngle = Rad(0.0f);

	{
		const auto& list = getRandomEligibleColor(2);
		mProjColors[0] = list->at(0);
		mProjColors[1] = list->at(1);

		mProjTextures[0] = getTextureResourceForIndex(0);
		mProjTextures[1] = getTextureResourceForIndex(1);
	}

	// Create game bubble
	mSphereManipulator[0] = new Object3D{ mManipulator.get() };
	CommonUtility::singleton->createGameSphere(this, *mSphereManipulator[0], mProjColors[0].rgb());

	mSphereDrawables[0] = mDrawables.back().get();
	mSphereDrawables[0]->mTexture = mProjTextures[0];

	// Load sound effects
	for (UnsignedInt i = 0; i < 3; ++i)
	{
		Resource<Audio::Buffer> buffer = CommonUtility::singleton->loadAudioData(RESOURCE_AUDIO_SHOT_PREFIX + std::to_string(i + 1));
		mPlayables[i] = std::make_shared<Audio::Playable3D>(*mManipulator.get(), &RoomManager::singleton->mAudioPlayables);
		mPlayables[i]->source()
			.setBuffer(buffer)
			.setMinGain(1.0f)
			.setMaxGain(1.0f)
			.setLooping(false);
	}
}

Player::Player(const Int parentIndex, const std::shared_ptr<IShootCallback> & shootCallback) : Player(parentIndex)
{
	mShootCallback = shootCallback;
}

const Int Player::getType() const
{
	return GOT_PLAYER;
}

void Player::update()
{
	/*
	// Update new projectile path
	mProjPath->update(mDeltaTime * Float(-mProjPath->getSize()));
	*/

	// Advance animation
	mAnimation += mDeltaTime;

	// Update shoot timeline for animation
	mShootTimeline -= mDeltaTime;

	// Get shooting angle by mouse
	{
		const auto& w = RoomManager::singleton->getWindowSize();

		Vector2 p1 = Vector2(InputManager::singleton->mMousePosition);
		Vector2 p2 = Vector2({ w.x() * 0.5f, w.y() });
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
	if (mCanShoot)
	{
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
				go->mPosition = mPosition;
				go->mVelocity = -Vector3(Math::cos(mShootAngle), Math::sin(mShootAngle), 0.0f);

				// Assign shoot callback
				if (!mShootCallback.expired())
				{
					go->mShootCallback = mShootCallback;
				}

				// Prevent shooting by keeping a reference (also, add the projectile to game object list)
				mProjectile = RoomManager::singleton->mGoLayers[mParentIndex].push_back(go, true);

				// Update color for next bubble
				mProjColors[0] = mProjColors[1];
				mProjColors[1] = getRandomEligibleColor(1)->at(0);

				mProjTextures[0] = mProjTextures[1];
				mProjTextures[1] = getTextureResourceForIndex(1);

				mSphereDrawables[0]->mTexture = mProjTextures[0];

				/*
				// Reset animation for new projectile
				mProjPath->mProgress = Float(mProjPath->getSize());
				*/

				// Play random sound
				{
					const UnsignedInt index = std::rand() % 3;
					mPlayables[index]->source()
						.setOffsetInSamples(0)
						.play();
				}

				// Launch callback
				if (!mShootCallback.expired())
				{
					mShootCallback.lock()->shootCallback(ISC_STATE_SHOOT_STARTED);
				}
			}
		}
	}

	// Shooter manipulation
	{
		// Compute transformations
		Matrix4 translation = Matrix4::translation(mPosition);
		Float finalAngle(Deg(mShootAngle) + Deg(90.0f));

		// Apply transformations to shooter
		mShooterManipulator->setTransformation(Matrix4());
		mShooterManipulator->transform(Matrix4::rotationX(Deg(90.0f)));
		mShooterManipulator->transform(Matrix4::rotationZ(Deg(finalAngle)));
		mShooterManipulator->transform(translation);
	}

	// Projectiles manipulation
	{
		// Apply transformations to bubbles
		// const Vector3 pos = position + mProjPath->getCurrentPosition();

		// Check if main projectile is a bomb
		if (mProjColors[0].rgb() == BUBBLE_BOMB)
		{
			// Make bubble invisible
			(*mSphereManipulator[0])
				.resetTransformation()
				.scale(Vector3(0.0f))
				.translate(Vector3(10000.0f));

			// Make bomb visible
			(*mBombManipulator)
				.resetTransformation()
				.translate(mPosition + Vector3(0.0f, 0.0f, 0.05f));

			// Make spark rotate
			(*mBombDrawables[0])
				.resetTransformation()
				.rotateZ(Deg(Math::floor(mAnimation * 1440.0f / 90.0f) * 90.0f));
		}
		// Otherwise, it's an ordinary bubble
		else
		{
			// Make bubble visible
			(*mSphereManipulator[0])
				.resetTransformation()
				.translate(mPosition);

			// Make bomb invisible
			(*mBombManipulator)
				.resetTransformation()
				.scale(Vector3(0.0f))
				.translate(Vector3(10000.0f));
		}
	}
}

void Player::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	const auto& isBubble = baseDrawable == mSphereDrawables[0];
	const auto& isBomb = baseDrawable == mBombDrawables[0] || baseDrawable == mBombDrawables[1];

	auto& shader = (Shaders::Phong&) baseDrawable->getShader();
	if (isBubble || isBomb)
	{
		shader
			.setLightPosition(mPosition + Vector3(0.0f, 0.0f, 1.0f))
			.setLightColor(0x202020_rgbf)
			.setSpecularColor(0xffffff00_rgbaf)
			.setAmbientColor(0xc0c0c0_rgbf)
			.setDiffuseColor(0x202020_rgbf);
	}
	else
	{
		shader
			.setLightPosition(mPosition + Vector3(-4.0f, 14.0f, 20.0f))
			.setLightColor(0xffffff60_rgbaf)
			.setSpecularColor(0xffffff00_rgbaf)
			.setAmbientColor(0xa0a0a0_rgbf)
			.setDiffuseColor(0xffffff_rgbf);
	}

	shader
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
		.draw(*baseDrawable->mMesh);
}

void Player::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void Player::setPrimaryProjectile(const Color3 & color)
{
	mProjColors[0] = color;
}

std::unique_ptr<std::vector<Color4>> Player::getRandomEligibleColor(const UnsignedInt times)
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

	// Create list
	std::unique_ptr<std::vector<Color4>> list = std::make_unique<std::vector<Color4>>();

	// Check for array size
	if (bubbles.size())
	{
		// Get random color from one in-game bubble
		for (UnsignedInt i = 0; i < times;)
		{
			const Int index = std::rand() % bubbles.size();
			const auto& color = ((Bubble*)bubbles[index].lock().get())->mAmbientColor;

			if (color == BUBBLE_COIN || color == BUBBLE_BOMB)
			{
				continue;
			}

			list->emplace_back(color.r(), color.g(), color.b());
			++i;
		}
	}
	else
	{
		list->emplace_back(0x00000000_rgbaf);
	}

	// Return list
	return list;
}

Resource<GL::Texture2D> Player::getTextureResourceForIndex(const UnsignedInt index)
{
	// ALERT: No validity checks are performed!!
	return CommonUtility::singleton->loadTexture(RoomManager::singleton->mBubbleColors[mProjColors[index].rgb().toSrgbInt()].textureKey);
}