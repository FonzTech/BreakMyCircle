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

Player::Player(const Int parentIndex) : GameObject(), mPlasmaSquareRenderer(Vector2i(32)), mCanShoot(true), mElectricBall(nullptr)
{
	// Assign parent index
	mParentIndex = parentIndex;

	// Initialize members
	mIsSwapping = false;
	mSwapRequest = false;
	mAnimation = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
	mAimLength = { 0.0f, -1.0f };
	mAimAngle = { Rad(0.0f), Rad(0.0f) };
	mAimCos = { 0.0f, 0.0f };
	mAimSin = { 0.0f, 0.0f };
	mAimTimer = 1.0f;

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

	// Create lines
	for (UnsignedInt i = 0; i < 3; ++i)
	{
		// Create manipulator
		Object3D* m;

		const bool isShoot = i < 2;
		if (isShoot)
		{
			mShootPathManipulator[i] = new Object3D{ mManipulator.get() };
			m = mShootPathManipulator[i];
		}
		else
		{
			mSwapManipulator = new Object3D{ mManipulator.get() };
			m = mSwapManipulator;
		}

		// Create drawable
		auto& drawables = RoomManager::singleton->mGoLayers[parentIndex].drawables;

		std::shared_ptr<BaseDrawable> td;
		if (isShoot)
		{
			Resource<GL::Mesh> mesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<ShootPathShader::Position, ShootPathShader::TextureCoordinates>(RESOURCE_MESH_PLANE_FLAT);
			Resource<GL::AbstractShaderProgram, ShootPathShader> shader = CommonUtility::singleton->getShootPathShader();
			Resource<GL::Texture2D> texture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_SHOOT_PATH);

			td = std::static_pointer_cast<GameDrawable<ShootPathShader>>(std::make_shared<GameDrawable<ShootPathShader>>(*drawables, shader, mesh, texture));
		}
		else
		{
			Resource<GL::Mesh> mesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<Shaders::Flat3D::Position, Shaders::Flat3D::TextureCoordinates>(RESOURCE_MESH_PLANE_FLAT);
			Resource<GL::AbstractShaderProgram, Shaders::Flat3D> shader = CommonUtility::singleton->getFlat3DShader();
			Resource<GL::Texture2D> texture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_SWAP);

			td = std::static_pointer_cast<GameDrawable<Shaders::Flat3D>>(std::make_shared<GameDrawable<Shaders::Flat3D>>(*drawables, shader, mesh, texture));
		}

		td->setParent(m);
		td->setDrawCallback(this);
		td->setObjectId(100U + i);
		mDrawables.emplace_back(td);

		(isShoot ? mShootPathDrawables : mFlatDrawables).insert(td.get());
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

		/*
		mProjTextures[0] = getTextureResourceForIndex(0);
		mProjTextures[1] = getTextureResourceForIndex(1);
		*/
	}

	// Create game bubbles
	for (UnsignedInt i = 0; i < 2; ++i)
	{
		mSphereManipulator[i] = new Object3D{ mManipulator.get() };
		CommonUtility::singleton->createGameSphere(this, *mSphereManipulator[i], mProjColors[i]);

		mSphereDrawables[i] = mDrawables.back().get();
		setupProjectile(i);
		// mSphereDrawables[i]->mTexture = mProjTextures[i];
	}

	// Load sound effects
	for (UnsignedInt i = 0; i < 4; ++i)
	{
		Resource<Audio::Buffer> buffer = CommonUtility::singleton->loadAudioData(i < 3 ? RESOURCE_AUDIO_SHOT_PREFIX + std::to_string(i + 1) : RESOURCE_AUDIO_SWAP);
		mPlayables[i] = std::make_shared<Audio::Playable3D>(*mManipulator.get(), &RoomManager::singleton->mAudioPlayables);
		mPlayables[i]->source()
			.setBuffer(buffer)
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

	// Advance animations
	mPlasmaSquareRenderer.update(mDeltaTime);
	mAnimation[0] += mDeltaTime;

	// Update shoot timeline for animation
	mShootTimeline -= mDeltaTime;

	// Get shooting angle by mouse
	{
		const auto& w = RoomManager::singleton->getWindowSize();

		Vector2 p1 = Vector2(InputManager::singleton->mMousePosition);
		Vector2 p2 = Vector2( w.x() * 0.5f, w.y() );
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

	// Check for bubble swap
	const bool isInSwap = getBubbleSwapArea().contains(InputManager::singleton->mMousePosition);
	if (mIsSwapping)
	{
		// Check for end
		mAnimation[2] -= mDeltaTime;
		if (mAnimation[2] <= 0.0f)
		{
			mAnimation[2] = 0.0f;
			mIsSwapping = false;
		}
	}
	// Check for mouse input
	else if (mCanShoot)
	{
		const auto& bs = InputManager::singleton->mMouseStates[PRIMARY_BUTTON];
		if (bs == IM_STATE_PRESSED)
		{
			// Check if bubble swap was requested
			if (getBubbleSwapArea().contains(InputManager::singleton->mMousePosition))
			{
				mSwapRequest = true;
			}
			else
			{
				mShootTimeline = 1.0f;
			}
		}
		else if (bs == IM_STATE_RELEASED)
		{
			if (mProjectile.expired())
			{
				// Check if bubble swap was requested
				if (mSwapRequest)
				{
					if (isInSwap && CommonUtility::singleton->isBubbleColorValid(mProjColors[0]))
					{
						// Start animation
						mAnimation[2] = 1.0f;
						mIsSwapping = true;

						// Swap colors
						const auto x = mProjColors[1];
						mProjColors[1] = mProjColors[0];
						mProjColors[0] = x;

						setupProjectile(0);
						setupProjectile(1);

						playSfxAudio(3);

						// Swap animation
						if (mAnimation[3] >= 1.0f)
						{
							mAnimation[3] = 0.0f;
						}
					}
				}
				else if (!isInSwap)
				{
					// Create projectile
					std::shared_ptr<Projectile> go = std::make_shared<Projectile>(mParentIndex, mProjColors[0]);
					go->mPosition = mPosition;
					go->mVelocity = -Vector3(Math::cos(mShootAngle), Math::sin(mShootAngle), 0.0f);

					if (mProjColors[0] == BUBBLE_PLASMA)
					{
						go->setCustomTexture(mPlasmaSquareRenderer.getRenderedTexture());
					}

					// Assign shoot callback
					if (!mShootCallback.expired())
					{
						go->mShootCallback = mShootCallback;
					}

					// Prevent shooting by keeping a reference (also, add the projectile to game object list)
					mProjectile = go;
					RoomManager::singleton->mGoLayers[mParentIndex].push_back(go);

					// Update color for next bubble
					mProjColors[0] = mProjColors[1];
					mProjColors[1] = getRandomEligibleColor(1)->at(0);

					setupProjectile(0);
					setupProjectile(1);

					/*
					mProjTextures[0] = mProjTextures[1];
					mProjTextures[1] = getTextureResourceForIndex(1);

					mSphereDrawables[0]->mTexture = mProjTextures[0];
					*/

					/*
					// Reset animation for new projectile
					mProjPath->mProgress = Float(mProjPath->getSize());
					*/

					// Play random sound
					{
						const UnsignedInt index = std::rand() % 3;
						playSfxAudio(index);
					}

					// Launch callback
					if (!mShootCallback.expired())
					{
						mShootCallback.lock()->shootCallback(ISC_STATE_SHOOT_STARTED, go->mAmbientColor, go->mAmbientColor, 0);
					}

					// Reset animation factor
					mAnimation[1] = 1.0f;
				}

				// Reset swap state
				mSwapRequest = false;
			}
		}
	}

	// Main manipulation
	(*mManipulator)
		.resetTransformation()
		.translate(mPosition);

	// Swap manipulation
	{
		mAnimation[3] += mDeltaTime * 0.5f;
		if (mAnimation[3] > 6.0f)
		{
			mAnimation[3] = 0.0f;
		}
	}

	(*mSwapManipulator)
		.resetTransformation()
		.rotateZ(Deg(mAnimation[3] * 210.0f))
		.scale(Vector3(4.0f))
		.translate(Vector3(0.0f, 0.0f, -0.5f));

	// Shooter manipulation
	(*mShooterManipulator)
		.resetTransformation()
		.rotateX(Deg(90.0f))
		.rotateZ(Deg(mShootAngle) + Deg(90.0f));

	// Projectiles animation
	{
		mAnimation[4] += mDeltaTime;
		while (mAnimation[4] > 30.0f)
		{
			mAnimation[4] -= 30.0f;
		}

		const Rad angle = Rad(Deg((1.0f - mAnimation[2]) * 180.0f));
		const Float ac = Math::cos(angle);
		const Float as = Math::sin(angle);

		// Primary projectile
		{
			// Apply transformations to bubbles
			// const Vector3 pos = position + mProjPath->getCurrentPosition();

			// Main - Plasma bubble
			if (mProjColors[0] == BUBBLE_BOMB)
			{
				// Make bubble invisible
				(*mSphereManipulator[0])
					.resetTransformation()
					.scale(Vector3(0.0f))
					.translate(Vector3(10000.0f));

				// Make bomb visible
				(*mBombManipulator)
					.resetTransformation()
					.translate(Vector3(0.0f, 0.0f, 0.05f));

				// Make spark rotate
				(*mBombDrawables[0])
					.resetTransformation()
					.rotateZ(Deg(Math::floor(mAnimation[0] * 1440.0f / 90.0f) * 90.0f));

				// Make electric bubble invisible
				mElectricBall->mPosition = Vector3(10000.0f);
				mElectricBall->mPlayables[0]->setGain(0.0f);
			}
			// Main - Electric bubble
			else if (mProjColors[0] == BUBBLE_ELECTRIC)
			{
				// Make bubble invisible
				(*mSphereManipulator[0])
					.resetTransformation()
					.scale(Vector3(0.0f))
					.translate(Vector3(10000.0f));

				// Make bomb invisible
				(*mBombManipulator)
					.resetTransformation()
					.scale(Vector3(0.0f))
					.translate(Vector3(10000.0f));

				// Make spark rotate
				(*mBombDrawables[0])
					.resetTransformation()
					.rotateZ(Deg(Math::floor(mAnimation[0] * 1440.0f / 90.0f) * 90.0f));

				// Make electric bubble visible
				mElectricBall->mPosition = mPosition;
				mElectricBall->mPlayables[0]->setGain(RoomManager::singleton->getSfxGain());
			}
			// Otherwise, it's an ordinary bubble / plasma bubble
			else
			{
				const Float s1 = 1.0f + mAnimation[1] * 0.5f;
				const Float s2 = mAnimation[2] * 0.5f;

				const Float x1 = 3.0f + ac * 3.0f;
				const Float x2 = 6.0f * mAnimation[1];
				const Float y1 = as * 3.0f;

				// Make bubbles visible
				(*mSphereManipulator[0])
					.resetTransformation()
					.scale(Vector3(s1 + s2))
					.translate(Vector3(x1 + x2, y1, 0.0f));

				// Make bomb invisible
				(*mBombManipulator)
					.resetTransformation()
					.scale(Vector3(0.0f))
					.translate(Vector3(10000.0f));

				// Make electric bubble invisible
				mElectricBall->mPosition = Vector3(10000.0f);
				mElectricBall->mPlayables[0]->setGain(0.0f);
			}
		}

		// Secondary projectile
		{
			mAnimation[1] -= mDeltaTime * 2.0f;
			if (mAnimation[1] < 0.0f)
			{
				mAnimation[1] = 0.0f;
			}

			{
				const Float s1 = 1.5f - mAnimation[1] * 1.5f;
				const Float s2 = mAnimation[2] * -0.5f;

				const Float x1 = 3.0f - ac * 3.0f;
				const Float y1 = as * -3.0f;

				(*mSphereManipulator[1])
					.resetTransformation()
					.scale(Vector3(s1 + s2))
					.translate(Vector3(x1, y1, 0.0f));
			}
		}
	}

	// Compute aiming
	if (mSwapRequest || isInSwap || InputManager::singleton->mMouseStates[PRIMARY_BUTTON] <= IM_STATE_NOT_PRESSED)
	{
		(*mShootPathManipulator[0])
			.resetTransformation()
			.scale(Vector3(0.0f));


		(*mShootPathManipulator[1])
			.resetTransformation()
			.scale(Vector3(0.0f));
	}
	else
	{
		mAimTimer -= mDeltaTime;
		if (mAimTimer < 0.0f)
		{
			// Reset timer
#ifdef TARGET_MOBILE
			mAimTimer = 0.035f;
#else
			mAimTimer = 0.02f;
#endif

			// Compute two-path aim
			Vector3 fp = mPosition;
			for (UnsignedInt i = 0; i < 2; ++i)
			{
				// Compute work variables
				if (i == 0)
				{
					mAimAngle[i] = mShootAngle + Rad(Deg(180.0f));
					mAimLength[1] = -1.0f;
				}
				else
				{
					if (mAimLength[i] < -1.5f || mAimAngle[0] == Rad(Deg(90.0f)))
					{
						break;
					}
					mAimAngle[i] = Rad(Deg(180.0f)) - mAimAngle[0];

					mSecondPos = fp;
					fp += mPosition;
				}

				mAimCos[i] = Math::cos(mAimAngle[i]);
				mAimSin[i] = Math::sin(mAimAngle[i]);

				mAimLength[i] = 0.5f;

				while (true)
				{
					const Vector3 p = fp + Vector3(mAimCos[i] * mAimLength[i], mAimSin[i] * mAimLength[i], 0.0f);
					const Range3D bbox = {
						p - Vector3(0.5f),
						p + Vector3(0.5f)
					};

					const std::unique_ptr<std::unordered_set<GameObject*>> collided = RoomManager::singleton->mCollisionManager->checkCollision(bbox, this, { GOT_BUBBLE });
					if (collided != nullptr && !collided->empty())
					{
						const auto& b = collided->cbegin();

						Float xx = (*b)->mPosition.x() - fp.x();
						Float yy = ((*b)->mPosition.y() - 1.0f) - fp.y();

						mAimLength[i] = Math::sqrt(xx * xx + yy * yy) * 0.5f;
						fp = Vector3(xx, yy, mPosition.z());

						if (i == 0)
						{
							mAimLength[1] = -2.0f;
						}
						break;
					}

					if (p.y() >= 1.0f)
					{
						const Float yy = fp.y() - 1.0f;
						const Float xx = getFixedAtan(Rad(Deg(90.0f)) - mAimAngle[i]) * yy;

						mAimLength[i] = Math::sqrt(xx * xx + yy * yy) * 0.5f;
						fp = Vector3(xx, yy, mPosition.z());

						if (i == 0)
						{
							mAimLength[1] = -2.0f;
						}
						break;
					}
					else
					{
						const bool c1 = p.x() <= Projectile::LEFT_X;
						const bool c2 = p.x() >= Projectile::RIGHT_X;
						if (c1 || c2)
						{
							const Float xx = (c1 ? Projectile::LEFT_X : Projectile::RIGHT_X) - fp.x();
							const Float yy = getFixedAtan(mAimAngle[i]) * xx + 0.5f;

							mAimLength[i] = Math::sqrt(xx * xx + yy * yy) * 0.5f + 0.5f;
							fp = Vector3(xx, yy, mPosition.z());

							break;
						}
						else
						{
							mAimLength[i] += 1.0f;
						}
					}
				}
			}
		}

		{
			const Float lsc1 = mAimLength[0] - 4.0f;

			if (lsc1 >= 0.25f)
			{
				const Float len1 = mAimLength[0] + 3.0f;

				(*mShootPathManipulator[0])
					.resetTransformation()
					.scale(Vector3(lsc1, 0.5f, 1.0f))
					.rotateZ(mAimAngle[0])
					.translate(Vector3(mAimCos[0] * len1, mAimSin[0] * len1, -0.02f));

				const Float lsc2 = Math::max(0.0f, mAimLength[1] - 0.5f);

				(*mShootPathManipulator[1])
					.resetTransformation()
					.scale(Vector3(lsc2, 0.5f, 1.0f))
					.rotateZ(mAimAngle[1])
					.translate(mSecondPos + Vector3(mAimCos[1] * lsc2, mAimSin[1] * lsc2, -0.02f));
			}
			else
			{
				(*mShootPathManipulator[0])
					.resetTransformation()
					.scale(Vector3(0.0f));

				(*mShootPathManipulator[1])
					.resetTransformation()
					.scale(Vector3(0.0f));
			}

		}
	}
}

void Player::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	const auto& it = mShootPathDrawables.find(baseDrawable);
	if (it != mShootPathDrawables.end())
	{
		const UnsignedInt id = (*it)->getObjectId() - 100U;
		((ShootPathShader&)baseDrawable->getShader())
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindTexture(*baseDrawable->mTexture)
			.setIndex(mAnimation[4])
			.setSize(mAimLength[id] - (id ? 0.5f : 4.0f))
			.draw(*baseDrawable->mMesh);
	}
	else if (mFlatDrawables.find(baseDrawable) != mFlatDrawables.end())
	{
		((Shaders::Flat3D&)baseDrawable->getShader())
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindTexture(*baseDrawable->mTexture)
			.setColor(Color4{ 1.0f, 1.0f, 1.0f, Math::sin(Deg(Math::min(180.0f, mAnimation[3] * 180.0f))) })
			.setAlphaMask(0.001f)
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		// Setup light shading
		const auto& isBubble = baseDrawable == mSphereDrawables[0] || baseDrawable == mSphereDrawables[1];
		const auto& isBomb = baseDrawable == mBombDrawables[0] || baseDrawable == mBombDrawables[1];

		auto& shader = (Shaders::Phong&) mSphereDrawables[0]->getShader();
		if (isBubble || isBomb)
		{
			shader
				.setLightPosition(mPosition + Vector3(-1.0f, 10.0f * RoomManager::singleton->getWindowAspectRatio(), 10.0f))
				.setLightColor(0x202020_rgbf)
				.setSpecularColor(0xffffff00_rgbaf)
				.setAmbientColor(0x909090_rgbf)
				.setDiffuseColor(0x909090_rgbf);
		}
		else
		{
			shader
				.setLightPosition(mPosition + Vector3(-4.0f, 30.0f * RoomManager::singleton->getWindowAspectRatio(), 20.0f))
				.setLightColor(0xffffff60_rgbaf)
				.setSpecularColor(0xffffff00_rgbaf)
				.setAmbientColor(0xa0a0a0_rgbf)
				.setDiffuseColor(0xffffff_rgbf);
		}

		// Setup matrixes
		shader
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix());

		// Bind textures
		if (baseDrawable == mSphereDrawables[0] && mProjColors[0] == BUBBLE_PLASMA)
		{
			mPlasmaSquareRenderer.renderTexture();
			GL::Texture2D &texture = mPlasmaSquareRenderer.getRenderedTexture();
			shader.bindTextures(&texture, &texture, nullptr, nullptr);
		}
		else
		{
			shader.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr);
		}

		// Draw mesh using setup shader
		shader.draw(*baseDrawable->mMesh);
	}
}

void Player::postConstruct()
{
	const std::shared_ptr<ElectricBall> go = std::make_shared<ElectricBall>(mParentIndex);
	mElectricBall = go;
	mElectricBall->mPosition = Vector3(10000.0f);
	mElectricBall->mPlayables[0]->setGain(0.0f);
	mElectricBall->playSfxAudio(0);
	RoomManager::singleton->mGoLayers[mParentIndex].push_back(go);
}

void Player::setupProjectile(const Int index)
{
	mSphereDrawables[index]->mTexture = getTextureResourceForIndex(index);
}

void Player::setPrimaryProjectile(const Color3 & color)
{
	mProjColors[0] = color;
	setupProjectile(0);

	if (color == BUBBLE_ELECTRIC)
	{
		mElectricBall->pushToFront();
	}
}

std::unique_ptr<std::vector<Color3>> Player::getRandomEligibleColor(const UnsignedInt times)
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
	std::unique_ptr<std::vector<Color3>> list = std::make_unique<std::vector<Color3>>();

	// Check for array size
	if (bubbles.size())
	{
		// Get random color from one in-game bubble
		for (UnsignedInt i = 0; i < times;)
		{
			const Int index = std::rand() % bubbles.size();
			const auto& color = ((Bubble*)bubbles[index].lock().get())->mAmbientColor;

			if (!CommonUtility::singleton->isBubbleColorValid(color))
			{
				continue;
			}

			list->emplace_back(color);
			++i;
		}
	}
	else
	{
		list->emplace_back(0x000000_rgbf);
	}

	// Return list
	return list;
}

Resource<GL::Texture2D> Player::getTextureResourceForIndex(const UnsignedInt index)
{
	// ALERT: No validity checks are performed!!
	const auto& color = RoomManager::singleton->sBubbleColors[mProjColors[index].toSrgbInt()];
	const auto& rk = color.textureKey;

	Debug{} << "Loading texture" << rk << "for player projectile with index" << index;

	return CommonUtility::singleton->loadTexture(rk);
}

Range2Di Player::getBubbleSwapArea()
{
	const Vector2 ws = RoomManager::singleton->getWindowSize();
	return {
		Vector2i(Int(ws.x() * 0.35f), Int(ws.y() * 0.75f)),
		Vector2i(Int(ws.x() * 0.65f), Int(ws.y()))
	};
}

Float Player::getFixedAtan(const Rad & angle)
{
	return Math::abs(Float(angle) - Constants::piHalf()) < 0.001f ? 0.0f : Math::tan(angle);
}
